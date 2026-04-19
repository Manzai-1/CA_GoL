#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "config.h"

typedef struct {
    char alive;  // 0 dead && 1 alive
    CellState state;
} cell;

typedef struct {
    int zoom;
    int grid_pos_y;
    int grid_pos_x;
} ViewState;

typedef struct {
    bool mouse_held;
    int mouse_pos_y;
    int mouse_pos_x;
} InputState;

typedef struct {
    int refresh_rate_ms;
    int generations;
    bool running;
} SimState;

static const uint32_t CELL_COLORS[] = {[CELL_STATE_BIRTH] = COLOR_BIRTH,
                                       [CELL_STATE_STASIS] = COLOR_STASIS,
                                       [CELL_STATE_DEATH] = COLOR_DEATH,
                                       [CELL_STATE_NOTHING] = COLOR_NOTHING};

cell calculate_cell(int sum, cell c);
void pan_view(int event_y, int event_x, ViewState *view, InputState *input);
bool init_sdl(SDL_Window **win, SDL_Renderer **rend, SDL_Texture **texture);
bool init_grid(cell **grid, cell **temp_grid);
void process_input(SDL_Event *ev, ViewState *view, SimState *sim,
                   InputState *input);
void update_grid(cell *grid, cell *temp_grid);
void render_grid(cell *grid, SDL_Renderer *rend, SDL_Texture *texture,
                 ViewState *view);
void cleanup(cell *grid, cell *temp_grid, SDL_Window *win, SDL_Renderer *rend,
             SDL_Texture *texture);

#define CELL(grid, row, col) (grid)[(row) * GRID_WIDTH + (col)]

int main() {
    SDL_Window *win = NULL;
    SDL_Renderer *rend = NULL;
    SDL_Texture *texture = NULL;
    cell *grid = NULL;
    cell *temp_grid = NULL;

    if (!init_sdl(&win, &rend, &texture)) return 1;
    if (!init_grid(&grid, &temp_grid)) return 1;

    ViewState view = {1, 0, 0};
    InputState input = {0, 0, 0};
    SimState sim = {INITIAL_REFRESH_RATE_MS, 0, true};
    SDL_Event ev;

    while (sim.running) {
        process_input(&ev, &view, &sim, &input);
        update_grid(grid, temp_grid);
        render_grid(grid, rend, texture, &view);

        SDL_Delay(sim.refresh_rate_ms);
        sim.generations++;
    }

    printf("Generations: %i\n", sim.generations);
    cleanup(grid, temp_grid, win, rend, texture);
    return 0;
}

bool init_sdl(SDL_Window **win, SDL_Renderer **rend, SDL_Texture **texture) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "SDL failed to initialise: %s\n", SDL_GetError());
        return false;
    }

    *win = SDL_CreateWindow("CA-SDL GoL", SDL_WINDOWPOS_CENTERED,
                            SDL_WINDOWPOS_CENTERED, GRID_WIDTH, GRID_HEIGHT,
                            SDL_WINDOW_SHOWN);

    if (*win == NULL) {
        fprintf(stderr, "Unable to create window.\n");
        return false;
    }

    *rend = SDL_CreateRenderer(
        *win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    if (*rend == NULL) {
        fprintf(stderr, "Unable to create renderer.\n");
        return false;
    }

    *texture =
        SDL_CreateTexture(*rend, SDL_PIXELFORMAT_ARGB8888,
                          SDL_TEXTUREACCESS_STREAMING, GRID_WIDTH, GRID_HEIGHT);

    if (*texture == NULL) {
        fprintf(stderr, "Unable to create texture.\n");
        return false;
    }

    return true;
}

bool init_grid(cell **grid, cell **temp_grid) {
    *grid = malloc(GRID_HEIGHT * GRID_WIDTH * sizeof(cell));
    if (*grid == NULL) {
        fprintf(stderr, "Failed to allocate grid.\n");
        return false;
    }

    *temp_grid = malloc(GRID_HEIGHT * GRID_WIDTH * sizeof(cell));
    if (*temp_grid == NULL) {
        fprintf(stderr, "Failed to allocate temp_grid.\n");
        free(*grid);
        return false;
    }

    srand(time(0));
    for (int i = 0; i < GRID_HEIGHT; i++) {
        for (int j = 0; j < GRID_WIDTH; j++) {
            int value = rand() % INITIAL_ALIVE_DENOMINATOR;
            CELL(*grid, i, j).alive = (value == 0);
            CELL(*grid, i, j).state = (CELL(*grid, i, j).alive)
                                          ? CELL_STATE_BIRTH
                                          : CELL_STATE_NOTHING;
        }
    }

    return true;
}

void process_input(SDL_Event *ev, ViewState *view, SimState *sim,
                   InputState *input) {
    while (SDL_PollEvent(ev) != 0) {
        if (ev->type == SDL_QUIT) {
            sim->running = false;
        } else if (ev->type == SDL_MOUSEBUTTONDOWN) {
            // activate mouse_control
            input->mouse_held = true;

            // store x / y position
            input->mouse_pos_x = ev->button.x;
            input->mouse_pos_y = ev->button.y;
        } else if (ev->type == SDL_MOUSEBUTTONUP) {
            // de-activate mouse_control
            input->mouse_held = false;
        } else if (ev->type == SDL_MOUSEMOTION && input->mouse_held) {
            pan_view(ev->motion.y, ev->motion.x, view, input);
        } else if (ev->type == SDL_MOUSEWHEEL) {
            // get direction and update zoom accordingly
            view->zoom += ev->wheel.y;
            if (view->zoom < MIN_ZOOM) view->zoom = MIN_ZOOM;
            if (view->zoom > GRID_HEIGHT / MAX_ZOOM_DIVISOR) {
                view->zoom = GRID_HEIGHT / MAX_ZOOM_DIVISOR;
            }

            // position render area around scroll position
            pan_view(ev->wheel.y, ev->wheel.x, view, input);
        } else if (ev->type == SDL_KEYUP) {
            // if key = arrow up / arrow down, adjust delay value
            if (ev->key.keysym.scancode == SDL_SCANCODE_UP) {
                sim->refresh_rate_ms += REFRESH_RATE_STEP_MS;
                if (sim->refresh_rate_ms > MAX_REFRESH_RATE_MS)
                    sim->refresh_rate_ms = MAX_REFRESH_RATE_MS;
            } else if (ev->key.keysym.scancode == SDL_SCANCODE_DOWN) {
                if (sim->refresh_rate_ms > MIN_REFRESH_RATE_MS) {
                    sim->refresh_rate_ms -= REFRESH_RATE_STEP_MS;
                }
            }
        }
    }
}

void update_grid(cell *grid, cell *temp_grid) {
    for (int i = 0; i < GRID_HEIGHT; i++) {
        for (int j = 0; j < GRID_WIDTH; j++) {
            // will contain neighboring values
            int neighborhood_sum = 0;

            // gather neighborhood values
            for (int k = i - 1; k <= i + 1; k++) {
                for (int z = j - 1; z <= j + 1; z++) {
                    if (k >= 0 && k < GRID_HEIGHT && z >= 0 && z < GRID_WIDTH) {
                        neighborhood_sum += CELL(grid, k, z).alive;
                    }
                }
            }

            // calculate cell value based on rules
            neighborhood_sum -= CELL(grid, i, j).alive;
            CELL(temp_grid, i, j) =
                calculate_cell(neighborhood_sum, CELL(grid, i, j));
        }
    }

    memcpy(grid, temp_grid, GRID_HEIGHT * GRID_WIDTH * sizeof(cell));
}

void render_grid(cell *grid, SDL_Renderer *rend, SDL_Texture *texture,
                 ViewState *view) {
    uint32_t *pixels;
    int pitch = GRID_WIDTH * BYTES_PER_PIXEL;

    // stores initial and previous row/col/color to avoid redundant lookups when
    // zoomed in
    int last_row = 0;
    int last_col = 0;
    uint32_t color =
        CELL_COLORS[CELL(grid, view->grid_pos_y, view->grid_pos_x).state];

    SDL_LockTexture(texture, NULL, (void **)&pixels, &pitch);

    for (int i = 0; i < GRID_HEIGHT * GRID_WIDTH; i++) {
        int row_index = i / GRID_WIDTH;
        int col_index = i % GRID_WIDTH;

        int row = view->grid_pos_y + (row_index / view->zoom);
        int col = view->grid_pos_x + (col_index / view->zoom);

        // only calculate argb if its a new grid index, otherwise use previous
        // values
        if (row != last_row || col != last_col) {
            color = CELL_COLORS[CELL(grid, row, col).state];
            last_row = row;
            last_col = col;
        }

        *(pixels + i) = color;
    }

    SDL_UnlockTexture(texture);

    SDL_RenderClear(rend);
    SDL_RenderCopy(rend, texture, NULL, NULL);
    SDL_RenderPresent(rend);
}

cell calculate_cell(int sum, cell c) {
    if (sum >= 2 && sum <= 3 && c.alive == 1) {
        // stasis
        c.state = CELL_STATE_STASIS;
        return c;
    } else if (sum == 3 && c.alive == 0) {
        // birth
        c.alive = 1;
        c.state = CELL_STATE_BIRTH;
        return c;
    } else if (c.alive == 1) {
        // death
        c.alive = 0;
        c.state = CELL_STATE_DEATH;
        return c;
    } else {
        // nothingness
        c.alive = 0;
        c.state = CELL_STATE_NOTHING;
        return c;
    }
}

void pan_view(int event_y, int event_x, ViewState *view, InputState *input) {
    int new_y = view->grid_pos_y;
    int new_x = view->grid_pos_x;

    new_y += (event_y - input->mouse_pos_y);
    new_x += (event_x - input->mouse_pos_x);

    if (new_y >= 0 && (new_y + (GRID_HEIGHT / view->zoom)) <=
                          GRID_HEIGHT)  // CAP rendering area at GRID_HEIGHT
    {
        view->grid_pos_y = new_y;
        input->mouse_pos_y = event_y;
    }
    if (new_x >= 0 && (new_x + (GRID_WIDTH / view->zoom)) <=
                          GRID_WIDTH - 1)  // CAP rendering area at GRID_WIDTH
    {
        view->grid_pos_x = new_x;
        input->mouse_pos_x = event_x;
    }
}

void cleanup(cell *grid, cell *temp_grid, SDL_Window *win, SDL_Renderer *rend,
             SDL_Texture *texture) {
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(rend);
    SDL_DestroyWindow(win);
    SDL_Quit();
    free(grid);
    free(temp_grid);
}
