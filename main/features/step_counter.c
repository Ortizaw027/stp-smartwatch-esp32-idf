#include <math.h>
#include "esp_log.h"
#include "esp_timer.h"
#include "imu_driver.h"
#include "step_counter.h"
#include "time_sync.h"

#define TAG "STEP_COUNTER"
#define HIGH_THRESHOLD 14000   // Higher threshold - less sensitive
#define LOW_THRESHOLD  10500   // Lower threshold - bigger movement needed
#define MIN_STEP_INTERVAL_MS 300  // Minimum 300ms between steps

static int step_count = 0;
static bool above_threshold = false;
static uint32_t last_step_time = 0;
static int last_reset_day = -1;  // Track last reset day for 24hr reset

void step_counter_init(void) {
    step_count = 0;
    above_threshold = false;
    last_step_time = 0;
    last_reset_day = -1;
    ESP_LOGI(TAG, "Step counter initialized");
}

void step_counter_update(void) {
    // Check for daily reset first
    if(time_is_synced()) {
        struct tm t = time_sync_get_local_time();
        if(last_reset_day != -1 && last_reset_day != t.tm_mday) {
            // New day - reset step counter
            step_count = 0;
            last_reset_day = t.tm_mday;
            ESP_LOGI(TAG, "Steps auto-reset for new day: %d", t.tm_mday);
        } else if(last_reset_day == -1) {
            // First time syncing - initialize the day
            last_reset_day = t.tm_mday;
            ESP_LOGI(TAG, "Day tracking initialized: %d", t.tm_mday);
        }
    }
    
    imu_data_t d = imu_get_data();
   
    // Calculate total acceleration magnitude
    int magnitude = abs(d.ax) + abs(d.ay) + abs(d.az);
    
    // Get current time in milliseconds
    uint32_t current_time = esp_timer_get_time() / 1000;
   
    // Two-threshold step detection with timing
    if (!above_threshold && magnitude > HIGH_THRESHOLD) {
        above_threshold = true;
    }
    else if (above_threshold && magnitude < LOW_THRESHOLD) {
        // Check if enough time has passed since last step
        if (current_time - last_step_time > MIN_STEP_INTERVAL_MS) {
            above_threshold = false;
            step_count++;
            last_step_time = current_time;
            ESP_LOGI(TAG, "Steps: %d", step_count);
        } else {
            // Too soon - probably same step or wrist movement
            above_threshold = false;
        }
    }
}

int step_counter_get_count(void) {
    return step_count;
}

void step_counter_reset(void) {
    step_count = 0;
    ESP_LOGI(TAG, "Steps manually reset");
}