#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>

#define HEIGHT 1024
#define WIDTH 2048

typedef struct {
    char alive; // 0 dead && 1 alive
    char state; // 0 born && 1 stasis && 2 death
} cell;

typedef struct {
    uint8_t alpha;
    uint8_t red;
    uint8_t green;
    uint8_t blue;
} argbt;

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

cell calculate_cell(int sum, cell c);
argbt calculate_argb(cell c);
void pan_view(int event_y, int event_x, ViewState *view, InputState *input);
bool init_sdl(SDL_Window **win, SDL_Renderer **rend, SDL_Texture **texture);
bool init_grid(cell **grid, cell **temp_grid);
void process_input(SDL_Event *ev, ViewState *view, SimState *sim, InputState *input);
void update_grid(cell *grid, cell *temp_grid);
void render_grid(cell *grid, SDL_Renderer *rend, SDL_Texture *texture, ViewState *view);
void cleanup(cell *grid, cell *temp_grid, SDL_Window *win, SDL_Renderer *rend, SDL_Texture *texture);

#define CELL(grid, row, col) (grid)[(row) * WIDTH + (col)]

int main()
{

    SDL_Window *win = NULL;
    SDL_Renderer *rend = NULL;
    SDL_Texture *texture = NULL;
    cell *grid = NULL;
    cell *temp_grid = NULL;

    if(!init_sdl(&win, &rend, &texture)) return 1;
    if(!init_grid(&grid, &temp_grid)) return 1;

    ViewState view = {1, 0, 0};
    InputState input = {0, 0, 0};
    SimState sim = {100, 0, true};
    SDL_Event ev;

    while(sim.running)
    {
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

bool init_sdl(SDL_Window **win, SDL_Renderer **rend, SDL_Texture **texture)
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0) 
    {
        fprintf(stderr, "SDL failed to initialise: %s\n", SDL_GetError());
        return false;
    }

    *win = SDL_CreateWindow("CA-SDL GoL", 
        SDL_WINDOWPOS_CENTERED, 
        SDL_WINDOWPOS_CENTERED, 
        WIDTH, HEIGHT, 
        SDL_WINDOW_SHOWN);

    if (*win == NULL)
    {
        fprintf(stderr,"Unable to create window.\n");
        return false;
    }

    *rend = SDL_CreateRenderer(*win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    if (*rend == NULL)
    {
        fprintf(stderr,"Unable to create renderer.\n");
        return false;
    }

    *texture = SDL_CreateTexture(*rend, 
                SDL_PIXELFORMAT_ARGB8888, 
                SDL_TEXTUREACCESS_STREAMING, 
                WIDTH, 
                HEIGHT);

    if (*texture == NULL)
    {
        fprintf(stderr,"Unable to create texture.\n");
        return false;
    }

    return true;
}

bool init_grid(cell **grid, cell **temp_grid)
{
    *grid = malloc(HEIGHT * WIDTH * sizeof(cell));
    if(*grid == NULL)
    {
        fprintf(stderr, "Failed to allocate grid.\n");
        return false;
    }

    *temp_grid = malloc(HEIGHT * WIDTH * sizeof(cell));
    if(*temp_grid == NULL)
    {
        fprintf(stderr, "Failed to allocate temp_grid.\n");
        free(*grid);
        return false;
    }

    srand(time(0));
    for (int i = 0; i < HEIGHT; i++)
    {
        for (int j = 0; j < WIDTH; j++)
        {
            int value = rand() % 3;
            CELL(*grid, i, j).alive = (value == 0);
            CELL(*grid, i, j).state = (CELL(*grid, i, j).alive) ? 0 : 1;
        }
    }

    return true;
}

void process_input(SDL_Event *ev, ViewState *view, SimState *sim, InputState *input)
{
    while (SDL_PollEvent(ev) != 0)
    {
        if (ev->type == SDL_QUIT)
        {
            sim->running = false;
        }
        else if (ev->type == SDL_MOUSEBUTTONDOWN)
        {
            // activate mouse_control
            input->mouse_held = true;

            // store x / y position
            input->mouse_pos_x = ev->button.x;
            input->mouse_pos_y = ev->button.y;
        }
        else if (ev->type == SDL_MOUSEBUTTONUP)
        {
            // de-activate mouse_control
            input->mouse_held = false;
        }
        else if (ev->type == SDL_MOUSEMOTION && input->mouse_held) 
        {
            pan_view(ev->motion.y, ev->motion.x, view, input);
        }
        else if (ev->type == SDL_MOUSEWHEEL)
        {
            // get direction and update zoom accordingly 
            view->zoom += ev->wheel.y;
            if (view->zoom < 1)
                view->zoom = 1;
            if (view->zoom > HEIGHT / 4)
            {
                view->zoom = HEIGHT / 4;
            }
            
            // position render area around scroll position
            pan_view(ev->wheel.y, ev->wheel.x, view, input);
        }
        else if (ev->type == SDL_KEYUP)
        {
            // if key = arrow up / arrow down, adjust delay value 
            if (ev->key.keysym.scancode == SDL_SCANCODE_UP)
            {
                sim->refresh_rate_ms += 10;
            }
            else if (ev->key.keysym.scancode == SDL_SCANCODE_DOWN)
            {
                if (sim->refresh_rate_ms > 10)
                {
                    sim->refresh_rate_ms -= 10;
                }
            }
        }
    }
}

void update_grid(cell *grid, cell *temp_grid)
{
    for (int i = 0; i < HEIGHT; i++)
    {
        for (int j = 0; j < WIDTH; j++)
        {
            // will contain neighboring values
            int neighborhood_sum = 0;

            // gather neighborhood values
            for (int k = i - 1; k <= i + 1; k++)
            {
                for (int z = j - 1; z <= j + 1; z++)
                {
                    if (k >= 0 && k < HEIGHT && z >= 0 && z < WIDTH)
                    {
                        neighborhood_sum += CELL(grid, k, z).alive;
                    }
                }
            }

            // calculate cell value based on rules
            neighborhood_sum -= CELL(grid, i, j).alive;
            CELL(temp_grid, i, j) = calculate_cell(neighborhood_sum, CELL(grid, i, j));
        }
    }

    memcpy(grid, temp_grid, HEIGHT * WIDTH * sizeof(cell));
}

void render_grid(cell *grid, SDL_Renderer *rend, SDL_Texture *texture, ViewState *view)
{
    uint32_t *pixels;
    int pitch = WIDTH * 4;

    // stores initial and previous row/col/argb to avoid redundant calls when zoomed in
    int last_row = 0;
    int last_col = 0;
    argbt argb = calculate_argb(CELL(grid, view->grid_pos_y, view->grid_pos_x));
    
    SDL_LockTexture(texture, NULL, (void **)&pixels, &pitch);

    for (int i = 0; i < HEIGHT * WIDTH; i++)
    {
        int row_index = i / WIDTH;
        int col_index = i % WIDTH;

        int row = view->grid_pos_y + (row_index / view->zoom);
        int col = view->grid_pos_x + (col_index / view->zoom);

        //only calculate argb if its a new grid index, otherwise use previous values
        if (row != last_row || col != last_col)
        {
            argb = calculate_argb(CELL(grid, row, col));
            last_row = row;
            last_col = col;
        }

        // bit shift uint8_t argb values into uint32_t pixel
        *(pixels + i) = (argb.alpha << 24) + (argb.red << 16) + (argb.green << 8) + argb.blue;
    }
    
    SDL_UnlockTexture(texture);
    
    SDL_RenderClear(rend);
    SDL_RenderCopy(rend, texture, NULL, NULL);
    SDL_RenderPresent(rend);
}

cell calculate_cell(int sum, cell c)
{
    if (sum >= 2 && sum <= 3 && c.alive == 1)
    {
        //stasis
        c.state = 1;
        return c;
    }
    else if (sum == 3 && c.alive == 0)
    {
        //birth
        c.alive = 1;
        c.state = 0;
        return c;
    }
    else if (c.alive == 1)
    {
        //death
        c.alive = 0;
        c.state = 2;
        return c;
    }
    else
    {
        //nothingness
        c.alive = 0;
        c.state = 1;
        return c;
    }
}

argbt calculate_argb(cell c)
{
    argbt argb;

    if (c.alive == 0 && c.state == 2)
    {
        // death 
        argb.alpha  = 255;
        argb.red    = 255;
        argb.green  = 0;
        argb.blue   = 0;
    }
    else if (c.alive == 1 && c.state == 0)
    {
        // birth 
        argb.alpha  = 255;
        argb.red    = 255;
        argb.green  = 0;
        argb.blue   = 255;
    }
    else if (c.alive == 1 && c.state == 1)
    {
        // stasis 
        argb.alpha  = 255;
        argb.red    = 0;
        argb.green  = 0;
        argb.blue   = 255;
    }
    else
    {
        // nothingness 
        argb.alpha  = 0;
        argb.red    = 0;
        argb.green  = 0;
        argb.blue   = 0;
    }

    return argb;
}

void pan_view(int event_y, int event_x, ViewState *view, InputState *input)
{
    int new_y = view->grid_pos_y;
    int new_x = view->grid_pos_x;

    new_y += (event_y - input->mouse_pos_y);
    new_x += (event_x - input->mouse_pos_x);

    if (new_y >= 0 && (new_y + (HEIGHT / view->zoom)) <= HEIGHT ) // CAP rendering area at HEIGHT
    {
        view->grid_pos_y = new_y;
        input->mouse_pos_y = event_y;
    }
    if (new_x >= 0 && (new_x + (WIDTH / view->zoom))<= WIDTH - 1) // CAP rendering area at WIDTH
    {
        view->grid_pos_x = new_x;
        input->mouse_pos_x = event_x;
    }
}

void cleanup(cell *grid, cell *temp_grid, SDL_Window *win, SDL_Renderer *rend, SDL_Texture *texture)
{
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(rend);
    SDL_DestroyWindow(win);
    SDL_Quit();
    free(grid);
    free(temp_grid);
}