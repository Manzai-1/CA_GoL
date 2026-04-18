#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>


#define HEIGHT 256
#define WIDTH 256


// zoom variables, default 1 cell rendered as 1px 
int zoom = 1;
int grid_pos_y = 0;
int grid_pos_x = 0;

// user input variables
char mouse = 0;
int mouse_pos_y = 0;
int mouse_pos_x = 0;
int delay = 100;



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

cell calculate_cell(int sum, cell c);
argbt calculate_argb(cell c);
void update_render_area(int event_y, int event_x);

#define CELL(grid, row, col) (grid)[(row) * WIDTH + (col)]

int main()
{
    // ----------------------------------- INIT GRAPHICS

    if (SDL_Init(SDL_INIT_VIDEO) != 0) 
    {
        fprintf(stderr, "SDL failed to initialise: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window *win = NULL;
    SDL_Renderer *rend = NULL;
    SDL_Texture *texture = NULL;

    //define
    win = SDL_CreateWindow("CA-SDL GoL", 
                SDL_WINDOWPOS_CENTERED, 
                SDL_WINDOWPOS_CENTERED, 
                WIDTH, HEIGHT, 
                SDL_WINDOW_SHOWN);

    if (win == NULL)
    {
        printf("Unable to create window.\n");
        return 1;
    }

    rend = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    if (rend == NULL)
    {
        printf("Unable to create renderer.\n");
        return 1;
    }
  
    texture = SDL_CreateTexture(rend, 
                    SDL_PIXELFORMAT_ARGB8888, 
                    SDL_TEXTUREACCESS_STREAMING, 
                    WIDTH, 
                    HEIGHT);

    if (texture == NULL)
    {
        printf("Unable to create texture.\n");
        return 1;
    }



    // ----------------------------------- INIT CA 

    cell *grid = malloc(HEIGHT * WIDTH * sizeof(cell));
    cell *temp_grid = malloc(HEIGHT * WIDTH * sizeof(cell));
    int generations = 0;

    // initialize grid to random 0 / 1
    srand(time(0));
    for (int i = 0; i < HEIGHT; i++)
    {
        for (int j = 0; j < WIDTH; j++)
        {
            int value = rand() % 3;
            if (value == 0)
                CELL(grid, i, j).alive = 1;
            else
                CELL(grid, i, j).alive = 0;
        }
    }


    int running = 1;
    SDL_Event ev;

    // game loop
    while(running == 1)
    {
        // while event quee is not empty
        while (SDL_PollEvent(&ev) != 0)
        {
            if (ev.type == SDL_QUIT)
            {
                // ----------------------------------- release resources
                SDL_DestroyTexture(texture);
                SDL_DestroyRenderer(rend);
                SDL_DestroyWindow(win);
                running = 0;
                SDL_Quit();
                printf("\nGenerations: %i\n", generations);
                return 0;
            }
            else if (ev.type == SDL_MOUSEBUTTONDOWN)
            {
                // activate mouse_control
                mouse = 1;

                // store x / y position
                mouse_pos_x = ev.button.x;
                mouse_pos_y = ev.button.y;
            }
            else if (ev.type == SDL_MOUSEBUTTONUP)
            {
                // de-activate mouse_control
                mouse = 0;
            }
            else if (ev.type == SDL_MOUSEMOTION && mouse == 1) 
            {
                update_render_area(ev.motion.y, ev.motion.x);
            }
            else if (ev.type == SDL_MOUSEWHEEL)
            {
                // get direction and update zoom accordingly 
                zoom += ev.wheel.y;
                if (zoom < 1)
                    zoom = 1;
                if (zoom > HEIGHT / 4)
                {
                    zoom = HEIGHT / 4;
                }
                
                // position render area around scroll position
                update_render_area(ev.wheel.y, ev.wheel.x);
            }
            else if (ev.type == SDL_KEYUP)
            {
                // if key = arrow up / arrow down, adjust delay value 
                if (ev.key.keysym.scancode == SDL_SCANCODE_UP)
                {
                    delay += 10;
                }
                else if (ev.key.keysym.scancode == SDL_SCANCODE_DOWN)
                {
                    if (delay > 10)
                    {
                        delay -= 10;
                    }
                }
            }
        }

        // ----------------------------------- calculate next generation into temp grid
        // loop through grid and add value to temp grid based on grid neighborhood
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

        // ----------------------------------- copy over next generation to current grid 
        // make faster with memcopy?? 
        for (int i = 0; i < HEIGHT; i++)
        {
            for (int j = 0; j < WIDTH; j++)
            {
                CELL(grid, i, j) = CELL(temp_grid, i, j);
            }
        }

        // ----------------------------------- display current grid 
        // pixels
        uint32_t *pixels;
        int pitch = WIDTH * 4;

        // stores initial and previous row/col/argb to avoid redundant calls when zoomed in
        int last_row = 0;
        int last_col = 0;
        argbt argb = calculate_argb(CELL(grid, grid_pos_y, grid_pos_x));
        
        SDL_LockTexture(texture, NULL, (void **)&pixels, &pitch);

        for (int i = 0; i < HEIGHT * WIDTH; i++)
        {
            int row_index = i / WIDTH;
            int col_index = i % WIDTH;

            int row = grid_pos_y + (row_index / zoom);
            int col = grid_pos_x + (col_index / zoom);

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
    
        SDL_Delay(delay);
        generations++;
    }

    SDL_Quit();
    free(grid);
    free(temp_grid);
    printf("Memory freed\n");
    return 0;
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

void update_render_area(int event_y, int event_x)
{
    int new_y = grid_pos_y;
    int new_x = grid_pos_x;

    new_y += (event_y - mouse_pos_y);
    new_x += (event_x - mouse_pos_x);

    if (new_y >= 0 && (new_y + (HEIGHT / zoom)) <= HEIGHT ) // CAP rendering area at HEIGHT
    {
        grid_pos_y = new_y;
        mouse_pos_y = event_y;
    }
    if (new_x >= 0 && (new_x + (WIDTH / zoom))<= WIDTH - 1) // CAP rendering area at WIDTH
    {
        grid_pos_x = new_x;
        mouse_pos_x = event_x;
    }
}
