#pragma once
#include <stdint.h>

// Initializes or resets the step counter state
void step_counter_init(void);

// Updates step counter based on latest IMU data
// Call this periodically after imu_update
void step_counter_update(void);

// Returns the current step count
int step_counter_get_count(void);
