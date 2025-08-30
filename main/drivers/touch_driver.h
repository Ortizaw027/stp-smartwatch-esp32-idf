#pragma once

#include "driver/i2c_master.h"
#include "esp_err.h"

// Init function
void touch_init(i2c_master_dev_handle_t i2c_handle);

