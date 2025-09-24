#include "imu_driver.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"

#define TAG "IMU"

// QMI8658 registers (Typical Sensor Mode)
#define CTRL1  0x02  // Control register 1: oscillator, endianness, auto-increment
#define CTRL2  0x03  // Accelerometer: full scale + ODR
#define CTRL3  0x04  // Gyroscope: full scale + ODR
#define CTRL5  0x06  // Filters (LPF)
#define CTRL7  0x08  // Enable sensors: accelerometer + gyro

#define ACCEL_X_H 0x12


// Example register values for typical sensor mode
#define CTRL1_VAL 0x20
#define CTRL2_VAL 0x16
#define CTRL3_VAL 0x46
#define CTRL5_VAL 0xA5
#define CTRL7_VAL 0x01

// Struct that holds latest IMU data
static imu_data_t latest_data;

void imu_init(i2c_master_dev_handle_t dev) {
    uint8_t data[2];
    
    // Check WHO_AM_I (should return 0x05)
    data[0] = 0x00; // WHO_AM_I register
    i2c_master_transmit(dev, data, 1, -1);
    uint8_t who_am_i;
    i2c_master_receive(dev, &who_am_i, 1, -1);
    ESP_LOGI(TAG, "WHO_AM_I: 0x%02X", who_am_i);
    
    // 2. Configure registers
    data[0] = 0x02; data[1] = 0x60; // CTRL1: auto-increment + little endian
    i2c_master_transmit(dev, data, 2, -1);
    
    data[0] = 0x03; data[1] = 0x15; // CTRL2: ±4g + 235Hz
    i2c_master_transmit(dev, data, 2, -1);
    
    data[0] = 0x04; data[1] = 0x55; // CTRL3: ±512dps + 235Hz  
    i2c_master_transmit(dev, data, 2, -1);
    
    data[0] = 0x08; data[1] = 0x03; // CTRL7: Enable accel + gyro
    i2c_master_transmit(dev, data, 2, -1);
    
    vTaskDelay(pdMS_TO_TICKS(100)); // Wait for startup
    ESP_LOGI(TAG, "IMU configured");
}

void imu_update(i2c_master_dev_handle_t dev) {
    uint8_t start_reg = 0x33; // Start from TEMP_L
    uint8_t raw_data[14];
    
    // Set read pointer
    i2c_master_transmit(dev, &start_reg, 1, -1);
    
    // Read 14 bytes
    i2c_master_receive(dev, raw_data, 14, -1);
    
    // Combine bytes (little endian)
    latest_data.ax = (raw_data[3] << 8) | raw_data[2];
    latest_data.ay = (raw_data[5] << 8) | raw_data[4]; 
    latest_data.az = (raw_data[7] << 8) | raw_data[6];
    latest_data.gx = (raw_data[9] << 8) | raw_data[8];
    latest_data.gy = (raw_data[11] << 8) | raw_data[10];
    latest_data.gz = (raw_data[13] << 8) | raw_data[12];

    // Simple log for debugging raw values
    ESP_LOGI(TAG, "AX: %d, AY: %d, AZ: %d | GX: %d, GY: %d, GZ: %d",
             latest_data.ax, latest_data.ay, latest_data.az,
             latest_data.gx, latest_data.gy, latest_data.gz);
    
}

imu_data_t imu_get_data(void){
    return latest_data; // Returns copy of latest_data
}