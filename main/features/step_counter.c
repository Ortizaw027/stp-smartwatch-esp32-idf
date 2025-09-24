#include <math.h>
#include "esp_log.h"
#include "imu_driver.h"
#include "step_counter.h"

#define TAG "STEP COUNTER"

// Rolling window size
#define WINDOW_SIZE 20

static int step_count = 0;
static bool above = false;
static int log_counter = 0;

// Rolling window buffer
static int64_t mag_buffer[WINDOW_SIZE];
static int buffer_index = 0;
static int buffer_filled = 0;

// Forward declarations
static int64_t get_min_mag(void);
static int64_t get_max_mag(void);

void step_counter_init(void){
    step_count = 0;
    above = false;
    buffer_index = 0;
    buffer_filled = 0;

    for(int i = 0; i < WINDOW_SIZE; i++) {
        mag_buffer[i] = 0;
    }
}

void step_counter_update(void){
    imu_data_t d = imu_get_data();

    // Squared magnitude
    int64_t mag2 = (int64_t)d.ax * d.ax +
                   (int64_t)d.ay * d.ay +
                   (int64_t)d.az * d.az;

    // Store in circular buffer
    mag_buffer[buffer_index] = mag2;
    buffer_index = (buffer_index + 1) % WINDOW_SIZE;
    if(buffer_filled < WINDOW_SIZE) buffer_filled++;

    // Compute dynamic thresholds once buffer is filled
    int64_t low_th = 0, high_th = 0;
    if(buffer_filled == WINDOW_SIZE){
        int64_t min = get_min_mag();
        int64_t max = get_max_mag();

        low_th  = min + (max - min) / 3;
        high_th = min + 2 * (max - min) / 3;
    } else {
        // Use initial fixed thresholds until buffer is filled
        low_th  = 5000000;
        high_th = 6000000;
    }

    // Log every 50 updates
    if (++log_counter >= 50) {
        ESP_LOGI(TAG, "mag2=%lld, low=%lld, high=%lld, above=%d", mag2, low_th, high_th, above);
        log_counter = 0;
    }

    // Step detection
    if (!above && mag2 > high_th){
        above = true;
    }
    else if (above && mag2 < low_th){
        above = false;
        step_count++;
        ESP_LOGI(TAG, "Step detected. Total %d", step_count);
    }
}

int step_counter_get_count(void){
    return step_count;
}

// --- Helper functions for rolling window ---
static int64_t get_min_mag(void){
    int64_t min = mag_buffer[0];
    for(int i = 1; i < WINDOW_SIZE; i++){
        if(mag_buffer[i] < min) min = mag_buffer[i];
    }
    return min;
}

static int64_t get_max_mag(void){
    int64_t max = mag_buffer[0];
    for(int i = 1; i < WINDOW_SIZE; i++){
        if(mag_buffer[i] > max) max = mag_buffer[i];
    }
    return max;
}
