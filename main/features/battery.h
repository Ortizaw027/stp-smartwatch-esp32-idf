#pragma once

#include <stdbool.h>

// Initialize the ADC for battery measurement
void battery_adc_init(void);

// Start the FreeRTOS task that monitors battery voltage and updates UI
void battery_monitor_start(void);

// Read the current battery voltage in millivolts
float battery_adc_read_voltage(void);

// Convert a battery voltage (V) to percentage
int battery_voltage_to_percent(float voltage);
