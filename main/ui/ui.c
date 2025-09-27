#include "lvgl.h"
#include "esp_log.h"
#include <stdio.h>
#include "stopwatch.h"
#include "fonts.h"
#include "time_sync.h"
#include "step_counter.h"

static const char* TAG = "UI";

// --- Stopwatch LVGL objects ---
static lv_obj_t *stopwatch_label = NULL;
static lv_obj_t *btn_start_stop = NULL;
static lv_obj_t *btn_reset = NULL;

// --- Watch LVGL objects ---
static lv_obj_t *watch_label = NULL;
static lv_obj_t *date_label = NULL;
static char watch_time_str[64];
static char watch_date_str[32];

// --- Battery LVGL objects ---
static lv_obj_t *battery_cont;
static lv_obj_t *battery_icon;
static lv_obj_t *battery_label;

// --- Step counter LVGL objects ---
static lv_obj_t *step_label = NULL;
static lv_obj_t *btn_step_reset = NULL;

// --- Start/Stop button callback ---
static void start_stop_cb(lv_event_t * e) {
    stopwatch_toggle();
    lv_obj_t *btn = lv_event_get_target(e);
    if (stopwatch_running()) {
        lv_label_set_text(lv_obj_get_child(btn, 0), "Stop");
    } else {
        lv_label_set_text(lv_obj_get_child(btn, 0), "Start");
    }
}

// --- Reset button callback ---
static void reset_cb(lv_event_t * e) {
    stopwatch_reset();
    if(stopwatch_label) lv_label_set_text(stopwatch_label, "00:00:00");
    if(btn_start_stop) lv_label_set_text(lv_obj_get_child(btn_start_stop, 0), "Start");
}

// --- Step counter reset button callback ---
static void step_reset_cb(lv_event_t * e) {
    step_counter_reset();
    if(step_label) lv_label_set_text_fmt(step_label, "Steps: %d", step_counter_get_count());
}

// --- LVGL timer to update stopwatch display ---
static void stopwatch_update_timer_cb(lv_timer_t *timer) {
    if(stopwatch_label) lv_label_set_text(stopwatch_label, stopwatch_get_time_str());
}

// --- LVGL timer to update step counter display ---
static void step_update_timer_cb(lv_timer_t *timer) {
    if(step_label) {
        lv_label_set_text_fmt(step_label, "Steps: %d", step_counter_get_count());
    }
}

// --- LVGL timer to update watch label ---
static void lv_update_watch_cb(lv_timer_t *timer)
{
    if (!watch_label || !date_label) return;

    if(!time_is_synced()) {
        lv_label_set_text(watch_label, "Syncing...");
        lv_label_set_text(date_label, "");
        return;
    }

    struct tm t = time_sync_get_local_time();

    // Format time
    int hour = t.tm_hour % 12;
    if(hour == 0) hour = 12;
    snprintf(watch_time_str, sizeof(watch_time_str), "%02d:%02d", hour, t.tm_min);

    // Format date
    strftime(watch_date_str, sizeof(watch_date_str), "%a, %d %b", &t);

    lv_label_set_text(watch_label, watch_time_str);
    lv_label_set_text(date_label, watch_date_str);
}

// --- Tileview event callback ---
static void tileview_event_cb(lv_event_t * e) {
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

// --- Create Tile 1 step counter ---
static void create_tile1_step_counter(lv_obj_t *tile1) {
    ESP_LOGI(TAG, "Creating tile1 step counter...");

    step_label = lv_label_create(tile1);
    lv_label_set_text(step_label, "Steps: 0");
    lv_obj_set_style_text_font(step_label, &inter_48, 0);
    lv_obj_set_style_text_color(step_label, lv_color_white(), 0);
    lv_obj_align(step_label, LV_ALIGN_CENTER, 0, -30);

    btn_step_reset = lv_button_create(tile1);
    lv_obj_set_size(btn_step_reset, 120, 50);
    lv_obj_align(btn_step_reset, LV_ALIGN_CENTER, 0, 40);
    lv_obj_add_event_cb(btn_step_reset, step_reset_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t *btn_label = lv_label_create(btn_step_reset);
    lv_label_set_text(btn_label, "Reset");
    lv_obj_center(btn_label);

    // Timer to refresh step count
    lv_timer_create(step_update_timer_cb, 1000, NULL);

    ESP_LOGI(TAG, "Tile1 step counter created successfully");
}

// --- Create Tile 3 stopwatch ---
static void create_tile3_stopwatch(lv_obj_t *tile3) {
    ESP_LOGI(TAG, "Creating tile3 stopwatch...");
    stopwatch_label = lv_label_create(tile3);
    lv_label_set_text(stopwatch_label, "00:00:00");
    lv_obj_set_style_text_font(stopwatch_label, &inter_48, 0);
    lv_obj_set_style_text_color(stopwatch_label, lv_color_white(), 0);
    lv_obj_set_style_text_align(stopwatch_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(stopwatch_label, LV_ALIGN_CENTER, 0, -30);

    lv_obj_t *btn_container = lv_obj_create(tile3);
    lv_obj_set_size(btn_container, lv_pct(80), 50);
    lv_obj_align(btn_container, LV_ALIGN_CENTER, 0, 30);
    lv_obj_set_flex_flow(btn_container, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(btn_container, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(btn_container, LV_OBJ_FLAG_SCROLLABLE);

    btn_start_stop = lv_button_create(btn_container);
    lv_obj_set_size(btn_start_stop, 100, 50);
    lv_obj_add_event_cb(btn_start_stop, start_stop_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t *btn_label_ss = lv_label_create(btn_start_stop);
    lv_label_set_text(btn_label_ss, "Start");
    lv_obj_set_style_text_font(btn_label_ss, &inter_20, 0);
    lv_obj_center(btn_label_ss);

    btn_reset = lv_button_create(btn_container);
    lv_obj_set_size(btn_reset, 100, 50);
    lv_obj_add_event_cb(btn_reset, reset_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t *btn_label_reset = lv_label_create(btn_reset);
    lv_label_set_text(btn_label_reset, "Reset");
    lv_obj_set_style_text_font(btn_label_reset, &inter_20, 0);
    lv_obj_center(btn_label_reset);

    lv_timer_create(stopwatch_update_timer_cb, 50, NULL);
    ESP_LOGI(TAG, "Tile3 stopwatch created successfully");
}

// --- Battery indicator ---
static void create_battery_status(void) {
    battery_cont = lv_obj_create(lv_scr_act());
    lv_obj_set_size(battery_cont, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_clear_flag(battery_cont, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_opa(battery_cont, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_opa(battery_cont, LV_OPA_TRANSP, 0);

    lv_obj_align(battery_cont, LV_ALIGN_TOP_MID, 0, 2);

    battery_icon = lv_label_create(battery_cont);
    lv_label_set_text(battery_icon, LV_SYMBOL_BATTERY_FULL);
    lv_obj_set_style_text_color(battery_icon, lv_color_white(), 0);

    battery_label = lv_label_create(battery_cont);
    lv_label_set_text(battery_label, "%");
    lv_obj_set_style_text_color(battery_label, lv_color_white(), 0);
    lv_obj_align_to(battery_label, battery_icon, LV_ALIGN_OUT_RIGHT_MID, 5, 0);
}

// --- Update battery percentage on screen ---
void ui_update_battery(int percentage)
{
    if(!battery_label || !battery_icon) return;

    // Update text
    char buf[8];
    snprintf(buf, sizeof(buf), "%d%%", percentage);
    lv_label_set_text(battery_label, buf);

    // Update icon based on level
    if(percentage > 80) {
        lv_label_set_text(battery_icon, LV_SYMBOL_BATTERY_FULL);
    } else if(percentage > 60) {
        lv_label_set_text(battery_icon, LV_SYMBOL_BATTERY_3);
    } else if(percentage > 40) {
        lv_label_set_text(battery_icon, LV_SYMBOL_BATTERY_2);
    } else if(percentage > 20) {
        lv_label_set_text(battery_icon, LV_SYMBOL_BATTERY_1);
    } else {
        lv_label_set_text(battery_icon, LV_SYMBOL_BATTERY_EMPTY);
    }
}

// --- Initialize UI ---
void ui_init(void) {
    ESP_LOGI(TAG, "Creating tileview...");
    lv_obj_t *tileview = lv_tileview_create(lv_scr_act());
    lv_obj_add_event_cb(tileview, tileview_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_set_scrollbar_mode(tileview, LV_SCROLLBAR_MODE_OFF);

    // Create tiles
    lv_obj_t *tile1 = lv_tileview_add_tile(tileview, 0, 0, LV_DIR_RIGHT);
    lv_obj_t *tile2 = lv_tileview_add_tile(tileview, 1, 0, LV_DIR_LEFT | LV_DIR_RIGHT);
    lv_obj_t *tile3 = lv_tileview_add_tile(tileview, 2, 0, LV_DIR_LEFT);

    // Start on tile2 (middle)
    lv_tileview_set_tile_by_index(tileview, 1, 0, LV_ANIM_OFF);

    lv_obj_remove_flag(tileview, LV_OBJ_FLAG_SCROLL_MOMENTUM);
    lv_obj_set_scroll_snap_x(tileview, LV_SCROLL_SNAP_CENTER);

    // Tile colors
    lv_obj_set_style_bg_color(tile1, lv_color_hex(0x2104), 0);
    lv_obj_set_style_bg_opa(tile1, LV_OPA_COVER, 0);
    lv_obj_set_style_bg_color(tile2, lv_color_hex(0x2104), 0);
    lv_obj_set_style_bg_opa(tile2, LV_OPA_COVER, 0);
    lv_obj_set_style_bg_color(tile3, lv_color_hex(0x2104), 0);
    lv_obj_set_style_bg_opa(tile3, LV_OPA_COVER, 0);

    // --- Tile1 step counter ---
    create_tile1_step_counter(tile1);

    // --- Tile2 watch ---
    watch_label = lv_label_create(tile2);
    lv_label_set_text(watch_label, "Syncing...");
    lv_obj_set_style_text_font(watch_label, &inter_48, 0);
    lv_obj_set_style_text_color(watch_label, lv_color_white(), 0);
    lv_obj_align(watch_label, LV_ALIGN_CENTER, 0, -10);

    date_label = lv_label_create(tile2);
    lv_label_set_text(date_label, "");
    lv_obj_set_style_text_font(date_label, &inter_14, 0);
    lv_obj_set_style_text_color(date_label, lv_color_white(), 0);
    lv_obj_align(date_label, LV_ALIGN_CENTER, 0, 40);

    lv_timer_create(lv_update_watch_cb, 1000, NULL);

    // --- Tile3 stopwatch ---
    create_tile3_stopwatch(tile3);

    // --- Battery indicator ---
    create_battery_status();

    ESP_LOGI(TAG, "UI initialized successfully");
}
