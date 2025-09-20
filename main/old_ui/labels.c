#include "labels.h"
#include "time_sync.h"
#include "fonts.h"
#include <stdio.h>


lv_obj_t* create_time_label(lv_obj_t *parent) {
    lv_obj_t *label = lv_label_create(parent);
    lv_label_set_text(label, "Syncing time...");
    lv_obj_set_width(label, lv_pct(100));
    lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_text_color(label, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_font(label, &inter_48, 0);
    lv_obj_align(label, LV_ALIGN_CENTER, 0, -10);
    return label;
}

lv_obj_t* create_date_label(lv_obj_t *parent) {
    lv_obj_t *label = lv_label_create(parent);
    lv_label_set_text(label, "");
    lv_obj_set_width(label, lv_pct(100));
    lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_text_color(label, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_font(label, &inter_20, 0);
    lv_obj_align_to(label, parent, LV_ALIGN_CENTER, 0, 30);
    return label;
}

void update_time_label(lv_obj_t *time_label, lv_obj_t *date_label) {
    if(!time_is_synced()) {
        lv_label_set_text(time_label, "Syncing time...");
        lv_label_set_text(date_label, "");
        return;
    }

    struct tm t = time_sync_get_local_time();
    static char time_str[8];
    static char date_str[32];

    int hour = t.tm_hour % 12;
    if(hour == 0) hour = 12;

    snprintf(time_str, sizeof(time_str), "%02d:%02d", hour, t.tm_min);
    strftime(date_str, sizeof(date_str), "%a, %d %b", &t);

    lv_label_set_text(time_label, time_str);
    lv_label_set_text(date_label, date_str);
}
