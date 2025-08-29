/******************************************************************************
 * i2c_scan - Simple I2C bus scanner for ESP32 (esp-idf, i2c_master.h)
 *
 * Scans addresses 0x03–0x77 and logs found devices to ESP_LOGI.
 *
 * Usage:
 *   1. Ensure i2c_bus_handle is initialized (via your i2c init function).
 *   2. Include this header: #include "i2c_scan.h"
 *   3. Call scan_i2c() when needed.
 *
 * Notes:
 *   - Debugging tool; optional in normal operation.
 *   - Relies on a global i2c_bus_handle defined elsewhere (e.g., main.c).
 ******************************************************************************/
#include "i2c_scan.h"
#include "esp_log.h"
#include "driver/i2c_master.h"
#include "esp_err.h"

extern i2c_master_bus_handle_t i2c_bus_handle;

void scan_i2c(void) {
    ESP_LOGI("I2C", "Starting I2C scan...");

    for (uint8_t addr = 3; addr < 0x78; addr++) {
        if (i2c_master_probe(i2c_bus_handle, addr, 1000) == ESP_OK) {
            ESP_LOGI("I2C", "Device found at 0x%02X", addr);
        }
    }

    ESP_LOGI("I2C", "I2C scan complete");
}
