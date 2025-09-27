#pragma once

#include "lvgl.h"

// Initialize the main UI
void ui_init(void);

// Update the battery percentage (0-100)
void ui_update_battery(int percentage);
