#pragma once
#include "lvgl.h"
#include "driver/i2c_master.h"

void touch_init(i2c_master_dev_handle_t i2c_handle);
void touch_read_callback(lv_indev_t *indev, lv_indev_data_t *data);
lv_indev_t* touch_get_indev(void);