#ifndef CONFIG_H
#define CONFIG_H

#ifndef GRID_HEIGHT
#define GRID_HEIGHT 1024
#endif

#ifndef GRID_WIDTH
#define GRID_WIDTH 2048
#endif

#define MIN_ZOOM               1
#define MAX_ZOOM_DIVISOR       4
#define INITIAL_REFRESH_RATE_MS 100
#define REFRESH_RATE_STEP_MS   10
#define MIN_REFRESH_RATE_MS    10
#define MAX_REFRESH_RATE_MS    1000
#define INITIAL_ALIVE_PROBABILITY 3
#define BYTES_PER_PIXEL        4

typedef enum {
    CELL_STATE_BIRTH   = 0,
    CELL_STATE_STASIS  = 1,
    CELL_STATE_DEATH   = 2,
    CELL_STATE_NOTHING = 3
} CellState;

#define COLOR_BIRTH   0xFFFF00FFu
#define COLOR_STASIS  0xFF0000FFu
#define COLOR_DEATH   0xFFFF0000u
#define COLOR_NOTHING 0x00000000u

#endif
