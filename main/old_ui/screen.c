#include "screen.h"
#include "labels.h"
#include "time_sync.h"
#include "lvgl.h"
#include <stdio.h>

// -------------------------- Screen state machine --------------------------
static screen_t current_screen = SCREEN_HOME;
static lv_obj_t *screens[SCREEN_COUNT];

// -------------------------- Home screen widgets --------------------------
static lv_obj_t *time_label = NULL;
static lv_obj_t *date_label = NULL;

// Timer callback to update home screen time
static void timer_cb(lv_timer_t *t) {
    update_time_label(time_label, date_label);
}

// Update home screen immediately
void update_home_screen_time(void) {
    update_time_label(time_label, date_label);
}

// -------------------------- Screen creation --------------------------
void create_home_screen(void) {
    lv_obj_t *scr = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x2104), LV_PART_MAIN);

    time_label = create_time_label(scr);
    date_label = create_date_label(scr);

    update_time_label(time_label, date_label);

    // Update every minute
    lv_timer_create(timer_cb, 60000, NULL);

    screens[SCREEN_HOME] = scr;
}

void create_stopwatch_screen(void) {
    lv_obj_t *scr = lv_obj_create(NULL);
    // TODO: add stopwatch widgets
    screens[SCREEN_STOPWATCH] = scr;
}

void create_compass_screen(void) {
    lv_obj_t *scr = lv_obj_create(NULL);
    // TODO: add compass widgets
    screens[SCREEN_COMPASS] = scr;
}

// -------------------------- Screen switching --------------------------
void switch_to_screen(screen_t new_screen) {
    if (new_screen >= 0 && new_screen < SCREEN_COUNT) {
        lv_scr_load(screens[new_screen]);
        current_screen = new_screen;
    }
}

// Swipe detection
void handle_swipe(int delta_x) {
    if (delta_x < -50 && current_screen < SCREEN_COUNT - 1)
        switch_to_screen(current_screen + 1);
    else if (delta_x > 50 && current_screen > 0)
        switch_to_screen(current_screen - 1);
}

// -------------------------- Initialization --------------------------
void init_screens(void) {
    create_home_screen();
    create_stopwatch_screen();
    create_compass_screen();
    switch_to_screen(SCREEN_HOME);
}
