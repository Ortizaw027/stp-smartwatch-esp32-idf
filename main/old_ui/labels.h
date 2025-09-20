#pragma once
#include "lvgl.h"


//Label creation functions
lv_obj_t* create_time_label(lv_obj_t *parent);
lv_obj_t* create_date_label(lv_obj_t *parent);

//Update function
void update_time_label(lv_obj_t *time_label, lv_obj_t *date_label);

