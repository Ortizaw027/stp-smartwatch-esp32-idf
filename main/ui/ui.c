#include "lvgl.h"
#include "esp_log.h"
#include <stdio.h>

static const char* TAG = "UI";

// Timer callback function for button reset (C function)
static void btn_reset_timer_cb(lv_timer_t * timer)
{
    lv_obj_t * btn = (lv_obj_t*)lv_timer_get_user_data(timer);
    lv_obj_t * label = lv_obj_get_child(btn, 0);
    lv_label_set_text(label, "CLICK ME");
    lv_timer_delete(timer);
}

static void btn_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * btn = lv_event_get_target(e);
    
    if(code == LV_EVENT_CLICKED) {
        ESP_LOGI(TAG, "Button was clicked!");
        
        // Visual feedback
        lv_obj_t * label = lv_obj_get_child(btn, 0);
        lv_label_set_text(label, "CLICKED!");
        
        // Reset text after delay using proper C function
        lv_timer_t * timer = lv_timer_create_basic();
        lv_timer_set_cb(timer, btn_reset_timer_cb);
        lv_timer_set_period(timer, 1000);
        lv_timer_set_user_data(timer, btn);
    }
}

static void tileview_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * tileview = lv_event_get_target(e);
    
    if(code == LV_EVENT_VALUE_CHANGED) {
        lv_obj_t * current_tile = lv_tileview_get_tile_active(tileview);
        if (current_tile) {
            // Simple way to track which tile is active
            lv_coord_t x = lv_obj_get_x(current_tile);
            lv_coord_t tile_width = lv_obj_get_width(current_tile);
            int tile_index = (tile_width > 0) ? (-x / tile_width) : 0;
            ESP_LOGI(TAG, "Tileview changed to tile index: %d", tile_index);
        }
    }
}

void ui_init(void)
{
    ESP_LOGI(TAG, "Creating basic tileview...");
   
    // Create tileview
    lv_obj_t *tileview = lv_tileview_create(lv_screen_active());
    if (tileview == NULL) {
        ESP_LOGE(TAG, "Failed to create tileview!");
        return;
    }
    
    // Add event callback to track tile changes
    lv_obj_add_event_cb(tileview, tileview_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
   
    ESP_LOGI(TAG, "Tileview created successfully");
   
    // Create first tile (index 0,0)
    lv_obj_t *tile1 = lv_tileview_add_tile(tileview, 0, 0, LV_DIR_RIGHT);
    if (tile1 == NULL) {
        ESP_LOGE(TAG, "Failed to create tile1!");
        return;
    }
   
    // Create second tile (index 1,0)
    lv_obj_t *tile2 = lv_tileview_add_tile(tileview, 1, 0, LV_DIR_LEFT | LV_DIR_RIGHT);
    if (tile2 == NULL) {
        ESP_LOGE(TAG, "Failed to create tile2!");
        return;
    }
   
    // Create third tile (index 2,0)
    lv_obj_t *tile3 = lv_tileview_add_tile(tileview, 2, 0, LV_DIR_LEFT);
    if (tile3 == NULL) {
        ESP_LOGE(TAG, "Failed to create tile3!");
        return;
    }
   
    ESP_LOGI(TAG, "All tiles created successfully");
   
    // Configure tileview for better touch response
    lv_obj_remove_flag(tileview, LV_OBJ_FLAG_SCROLL_MOMENTUM);  // Disable momentum for more responsive feel
    lv_obj_set_scroll_snap_x(tileview, LV_SCROLL_SNAP_CENTER); // Snap to center of tiles
   
    // Set different background colors to distinguish tiles
    lv_obj_set_style_bg_color(tile1, lv_color_hex(0xFF0000), 0); // Red
    lv_obj_set_style_bg_opa(tile1, LV_OPA_COVER, 0);
    
    lv_obj_set_style_bg_color(tile2, lv_color_hex(0x00FF00), 0); // Green  
    lv_obj_set_style_bg_opa(tile2, LV_OPA_COVER, 0);
    
    lv_obj_set_style_bg_color(tile3, lv_color_hex(0x0000FF), 0); // Blue
    lv_obj_set_style_bg_opa(tile3, LV_OPA_COVER, 0);
   
    // Add content to tile 1
    lv_obj_t *label1 = lv_label_create(tile1);
    lv_label_set_text(label1, "TILE 1\nRED\nSwipe right ->");
    lv_obj_set_style_text_color(label1, lv_color_white(), 0);
    lv_obj_set_style_text_align(label1, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_center(label1);
   
    // Add test button to tile 1
    lv_obj_t *btn1 = lv_button_create(tile1);
    lv_obj_set_size(btn1, 100, 40);
    lv_obj_align(btn1, LV_ALIGN_BOTTOM_MID, 0, -20);
    lv_obj_add_event_cb(btn1, btn_event_cb, LV_EVENT_CLICKED, NULL);
    
    // Make button more touch-friendly
    lv_obj_set_style_radius(btn1, 8, 0);
    lv_obj_set_style_shadow_width(btn1, 4, 0);
    lv_obj_set_style_shadow_opa(btn1, LV_OPA_30, 0);
   
    lv_obj_t *btn_label1 = lv_label_create(btn1);
    lv_label_set_text(btn_label1, "CLICK ME");
    lv_obj_center(btn_label1);
   
    // Add content to tile 2
    lv_obj_t *label2 = lv_label_create(tile2);
    lv_label_set_text(label2, "TILE 2\nGREEN\n<- Swipe left or right ->");
    lv_obj_set_style_text_color(label2, lv_color_black(), 0);
    lv_obj_set_style_text_align(label2, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_center(label2);
    
    // Add a second button for testing
    lv_obj_t *btn2 = lv_button_create(tile2);
    lv_obj_set_size(btn2, 120, 40);
    lv_obj_align(btn2, LV_ALIGN_BOTTOM_MID, 0, -20);
    lv_obj_add_event_cb(btn2, btn_event_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_set_style_bg_color(btn2, lv_color_hex(0x404040), 0);
    lv_obj_set_style_radius(btn2, 8, 0);
    
    lv_obj_t *btn_label2 = lv_label_create(btn2);
    lv_label_set_text(btn_label2, "TEST TOUCH");
    lv_obj_set_style_text_color(btn_label2, lv_color_white(), 0);
    lv_obj_center(btn_label2);
   
    // Add content to tile 3
    lv_obj_t *label3 = lv_label_create(tile3);
    lv_label_set_text(label3, "TILE 3\nBLUE\n<- Swipe left");
    lv_obj_set_style_text_color(label3, lv_color_white(), 0);
    lv_obj_set_style_text_align(label3, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_center(label3);
    
    // Add touch debug area
    lv_obj_t *touch_area = lv_obj_create(tile3);
    lv_obj_set_size(touch_area, 150, 60);
    lv_obj_align(touch_area, LV_ALIGN_BOTTOM_MID, 0, -10);
    lv_obj_set_style_bg_color(touch_area, lv_color_hex(0x333333), 0);
    lv_obj_set_style_border_color(touch_area, lv_color_white(), 0);
    lv_obj_set_style_border_width(touch_area, 2, 0);
    lv_obj_set_style_radius(touch_area, 5, 0);
    
    lv_obj_t *touch_label = lv_label_create(touch_area);
    lv_label_set_text(touch_label, "Touch Debug\nArea");
    lv_obj_set_style_text_color(touch_label, lv_color_white(), 0);
    lv_obj_set_style_text_align(touch_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_center(touch_label);
   
    ESP_LOGI(TAG, "Tileview with 3 tiles created successfully");
    ESP_LOGI(TAG, "Try swiping left/right or clicking the buttons");
}