#include "hal/adc_types.h"
#include "esp_log.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "ui.h"

static const char *TAG = "BATTERY";

// ADC handles
static adc_oneshot_unit_handle_t adc1_handle;
static adc_cali_handle_t cali_handle = NULL;

// Battery monitor task handle
static TaskHandle_t battery_task_handle = NULL;

// Voltage divider scaling factor (measured)
#define BATTERY_DIVIDER_SCALE 3.5f

// ADC channel
#define BATTERY_ADC_CHANNEL ADC_CHANNEL_0

// Initialize ADC
void battery_adc_init(void)
{
    adc_oneshot_unit_init_cfg_t init_cfg = {
        .unit_id  = ADC_UNIT_1,
        .ulp_mode = ADC_ULP_MODE_DISABLE,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_cfg, &adc1_handle));

    adc_oneshot_chan_cfg_t chan_cfg = {
        .atten    = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, BATTERY_ADC_CHANNEL, &chan_cfg));

    adc_cali_curve_fitting_config_t cali_cfg = {
        .unit_id  = ADC_UNIT_1,
        .atten    = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
    ESP_ERROR_CHECK(adc_cali_create_scheme_curve_fitting(&cali_cfg, &cali_handle));

    ESP_LOGI(TAG, "Battery ADC initialized (GPIO1 -> ADC1_CH0)");
}

// Read battery voltage in volts
float battery_adc_read_voltage(void)
{
    int raw = 0;
    int voltage_mv = 0;
    ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, BATTERY_ADC_CHANNEL, &raw));
    ESP_ERROR_CHECK(adc_cali_raw_to_voltage(cali_handle, raw, &voltage_mv));
    
    float battery_voltage = (voltage_mv * BATTERY_DIVIDER_SCALE) / 1000.0f;
    
    ESP_LOGI(TAG, "ADC raw: %d, Calibrated: %d mV, Battery: %.2f V",
             raw, voltage_mv, battery_voltage);

    // Return in volts
    return battery_voltage;
}

// Convert voltage to percentage
static int battery_voltage_to_percent(float voltage)
{
    const float min_v = 3.0f;  // 0%
    const float max_v = 4.2f;  // 100%
    
    if (voltage <= min_v) return 0;
    if (voltage >= max_v) return 100;
    
    return (int)((voltage - min_v) / (max_v - min_v) * 100.0f);
}

// Battery monitor task
static void battery_monitor_task(void *arg)
{
    int last_percent = -1;
    
    while (1)
    {
        float voltage = battery_adc_read_voltage();
        int percent = battery_voltage_to_percent(voltage);
        
        if (percent != last_percent) {
        // Only update if percentage decreased or this is first reading(Battery percentage was flucuating a lot)
        if (last_percent == -1 || percent < last_percent) {
            last_percent = percent;
            ui_update_battery(percent);
    }
}
        
        // 30 sec interval
        vTaskDelay(pdMS_TO_TICKS(30000));
    }
}

// Start battery monitor
void battery_monitor_start(void)
{
    if (!battery_task_handle) {
        xTaskCreate(battery_monitor_task, "battery_monitor", 4096, NULL, 5, &battery_task_handle);
    }
}