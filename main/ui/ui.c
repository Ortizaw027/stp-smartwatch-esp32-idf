#include "lvgl.h"
#include "esp_log.h"
#include <stdio.h>
#include "stopwatch.h"
#include "fonts.h"  // inter_14, inter_20, inter_32, inter_48

static const char* TAG = "UI";

// Stopwatch LVGL objects
static lv_obj_t *stopwatch_label = NULL;
static lv_obj_t *btn_start_stop = NULL;
static lv_obj_t *btn_reset = NULL;

// --- Start/Stop button callback ---
static void start_stop_cb(lv_event_t * e)
{
    stopwatch_toggle();

    lv_obj_t *btn = lv_event_get_target(e);
    if (stopwatch_running()) {
        lv_label_set_text(lv_obj_get_child(btn, 0), "Stop");
    } else {
        lv_label_set_text(lv_obj_get_child(btn, 0), "Start");
    }
}

// --- Reset button callback ---
static void reset_cb(lv_event_t * e)
{
    stopwatch_reset();
    if(stopwatch_label) lv_label_set_text(stopwatch_label, "00:00:00");
    if(btn_start_stop) lv_label_set_text(lv_obj_get_child(btn_start_stop, 0), "Start");
}

// --- LVGL timer to update stopwatch display ---
static void stopwatch_update_timer_cb(lv_timer_t *timer)
{
    if(stopwatch_label)
        lv_label_set_text(stopwatch_label, stopwatch_get_time_str());
}

// --- Tileview event callback ---
static void tileview_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * tileview = lv_event_get_target(e);
    
    if(code == LV_EVENT_VALUE_CHANGED) {
        lv_obj_t * current_tile = lv_tileview_get_tile_active(tileview);
        if (current_tile) {
            lv_coord_t x = lv_obj_get_x(current_tile);
            lv_coord_t tile_width = lv_obj_get_width(current_tile);
            int tile_index = (tile_width > 0) ? (-x / tile_width) : 0;
            ESP_LOGI(TAG, "Tileview changed to tile index: %d", tile_index);
        }
    }
}

// --- Create Tile 3 stopwatch ---
static void create_tile3_stopwatch(lv_obj_t *tile3)
{
    ESP_LOGI(TAG, "Creating tile3 stopwatch...");

    // Stopwatch label, centered higher
    stopwatch_label = lv_label_create(tile3);
    lv_label_set_text(stopwatch_label, "00:00:00");
    lv_obj_set_style_text_font(stopwatch_label, &inter_48, 0);
    lv_obj_set_style_text_color(stopwatch_label, lv_color_white(), 0);
    lv_obj_set_style_text_align(stopwatch_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(stopwatch_label, LV_ALIGN_CENTER, 0, -30); // slightly above center

    // Container for buttons, below label
    lv_obj_t *btn_container = lv_obj_create(tile3);
    lv_obj_set_size(btn_container, lv_pct(80), 50);
    lv_obj_align(btn_container, LV_ALIGN_CENTER, 0, 30); // below label
    lv_obj_set_flex_flow(btn_container, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(btn_container, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(btn_container, LV_OBJ_FLAG_SCROLLABLE);

    // Start/Stop button
    btn_start_stop = lv_button_create(btn_container);
    lv_obj_set_size(btn_start_stop, 100, 50);
    lv_obj_add_event_cb(btn_start_stop, start_stop_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t *btn_label_ss = lv_label_create(btn_start_stop);
    lv_label_set_text(btn_label_ss, "Start");
    lv_obj_set_style_text_font(btn_label_ss, &inter_20, 0);
    lv_obj_center(btn_label_ss);

    // Reset button
    btn_reset = lv_button_create(btn_container);
    lv_obj_set_size(btn_reset, 100, 50);
    lv_obj_add_event_cb(btn_reset, reset_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t *btn_label_reset = lv_label_create(btn_reset);
    lv_label_set_text(btn_label_reset, "Reset");
    lv_obj_set_style_text_font(btn_label_reset, &inter_20, 0);
    lv_obj_center(btn_label_reset);

    // LVGL timer updates label every 50ms
    lv_timer_create(stopwatch_update_timer_cb, 50, NULL);

    ESP_LOGI(TAG, "Tile3 stopwatch created successfully");
}

// --- Initialize UI ---
void ui_init(void)
{
    ESP_LOGI(TAG, "Creating tileview...");

    lv_obj_t *tileview = lv_tileview_create(lv_screen_active());
    lv_obj_add_event_cb(tileview, tileview_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

    // Disable visual scrollbar
    lv_obj_set_scrollbar_mode(tileview, LV_SCROLLBAR_MODE_OFF);

    // Create tiles
    lv_obj_t *tile1 = lv_tileview_add_tile(tileview, 0, 0, LV_DIR_RIGHT);
    lv_obj_t *tile2 = lv_tileview_add_tile(tileview, 1, 0, LV_DIR_LEFT | LV_DIR_RIGHT);
    lv_obj_t *tile3 = lv_tileview_add_tile(tileview, 2, 0, LV_DIR_LEFT);

    // Start on middle tile
    lv_tileview_set_tile_by_index(tileview, 1, 0, LV_ANIM_OFF);

    // Tileview config
    lv_obj_remove_flag(tileview, LV_OBJ_FLAG_SCROLL_MOMENTUM);
    lv_obj_set_scroll_snap_x(tileview, LV_SCROLL_SNAP_CENTER);

    // Tile colors
    lv_obj_set_style_bg_color(tile1, lv_color_hex(0xFF0000), 0);
    lv_obj_set_style_bg_opa(tile1, LV_OPA_COVER, 0);
    lv_obj_set_style_bg_color(tile2, lv_color_hex(0x00FF00), 0);
    lv_obj_set_style_bg_opa(tile2, LV_OPA_COVER, 0);
    lv_obj_set_style_bg_color(tile3, lv_color_hex(0x0000FF), 0);
    lv_obj_set_style_bg_opa(tile3, LV_OPA_COVER, 0);

    // --- Tile 1 content ---
    lv_obj_t *label1 = lv_label_create(tile1);
    lv_label_set_text(label1, "TILE 1\nRED\nSwipe right ->");
    lv_obj_set_style_text_color(label1, lv_color_white(), 0);
    lv_obj_set_style_text_align(label1, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_center(label1);

    lv_obj_t *btn1 = lv_button_create(tile1);
    lv_obj_set_size(btn1, 100, 40);
    lv_obj_align(btn1, LV_ALIGN_BOTTOM_MID, 0, -20);
    lv_obj_add_event_cb(btn1, NULL, LV_EVENT_CLICKED, NULL);
    lv_obj_t *btn_label1 = lv_label_create(btn1);
    lv_label_set_text(btn_label1, "CLICK ME");
    lv_obj_center(btn_label1);

    // --- Tile 2 content ---
    lv_obj_t *label2 = lv_label_create(tile2);
    lv_label_set_text(label2, "TILE 2\nGREEN\n<- Swipe left or right ->");
    lv_obj_set_style_text_color(label2, lv_color_black(), 0);
    lv_obj_set_style_text_align(label2, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_center(label2);

    lv_obj_t *btn2 = lv_button_create(tile2);
    lv_obj_set_size(btn2, 120, 40);
    lv_obj_align(btn2, LV_ALIGN_BOTTOM_MID, 0, -20);
    lv_obj_add_event_cb(btn2, NULL, LV_EVENT_CLICKED, NULL);
    lv_obj_t *btn_label2 = lv_label_create(btn2);
    lv_label_set_text(btn_label2, "TEST TOUCH");
    lv_obj_set_style_text_color(btn_label2, lv_color_white(), 0);
    lv_obj_center(btn_label2);

    // --- Tile 3 stopwatch ---
    create_tile3_stopwatch(tile3);

    ESP_LOGI(TAG, "UI initialized successfully");
}
