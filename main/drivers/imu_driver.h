#pragma once
#include "driver/i2c_master.h"


// Stores latest IMU data
typedef struct
{
    int16_t ax, ay, az;
    int16_t gx, gy, gz;
}imu_data_t;

// Pass in the I2C device handle
void imu_init(i2c_master_dev_handle_t dev);

// Update imu data
void imu_update(i2c_master_dev_handle_t dev);

// Getter for functions that need to use IMU data
imu_data_t imu_get_data(void);
