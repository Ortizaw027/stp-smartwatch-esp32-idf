#pragma once
#include "lvgl.h"

// Enum for screen IDs
typedef enum {
    SCREEN_HOME,
    SCREEN_STOPWATCH,
    SCREEN_COMPASS,
    SCREEN_COUNT
} screen_t;

// Functions
void init_screens(void);
void switch_to_screen(screen_t new_screen);
void handle_swipe(int delta_x);

// Home screen helpers
void update_home_screen_time(void);
