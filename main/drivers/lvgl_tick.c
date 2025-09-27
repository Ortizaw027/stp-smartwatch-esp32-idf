#include "lvgl.h"
#include "esp_timer.h"
#include "esp_log.h"


static esp_timer_handle_t lvgl_timer ;

static void lvgl_tick_cb(void *arg)
{
    lv_tick_inc(1);
}

void init_lvgl_tick(void)
{
    const esp_timer_create_args_t timer_args ={
        .callback = &lvgl_tick_cb,
        .name = "lvgl_tick"
    };
    ESP_ERROR_CHECK(esp_timer_create(&timer_args, &lvgl_timer));

    // Start the timer periodically(1 milli sec = 1000us)
    ESP_ERROR_CHECK(esp_timer_start_periodic(lvgl_timer, 1000));
}