#include "touch_driver.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#define TAG "TOUCH"

// CST816S pins
#define INT_PIN   5
#define RST_PIN   13

// Touch registers
#define REG_FINGERS 0x02
#define REG_XH      0x03
#define REG_XL      0x04
#define REG_YH      0x05
#define REG_YL      0x06

static SemaphoreHandle_t touch_sem = NULL;
static i2c_master_dev_handle_t i2c_dev = NULL;
static lv_indev_t *s_indev = NULL;  

// Current touch state
static volatile int16_t latest_x = 0;
static volatile int16_t latest_y = 0;
static volatile bool touch_down = false;

// ------------------- ISR -------------------
static void IRAM_ATTR gpio_isr_handler(void *args) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xSemaphoreGiveFromISR(touch_sem, &xHigherPriorityTaskWoken);
    if (xHigherPriorityTaskWoken) {
        portYIELD_FROM_ISR();
    }
}

// ------------------- Touch task -------------------
static void touch_task(void *args) {
    uint8_t num_fingers, xh, xl, yh, yl;
    while (1) {
        // Wait for ISR to tell us something happened
        if (xSemaphoreTake(touch_sem, portMAX_DELAY) == pdTRUE) {
            do {
                // Read number of fingers
                uint8_t reg = REG_FINGERS;
                if (i2c_master_transmit_receive(i2c_dev, &reg, 1, &num_fingers, 1, 1000) != ESP_OK) {
                    ESP_LOGW(TAG, "Failed to read fingers");
                    break;
                }
                
                if (num_fingers > 0) {
                    // Read coordinates
                    uint8_t buf[4];
                    reg = REG_XH;
                    if (i2c_master_transmit_receive(i2c_dev, &reg, 1, buf, 4, 1000) == ESP_OK) {
                        xh = buf[0];
                        xl = buf[1];
                        yh = buf[2];
                        yl = buf[3];
                        
                        latest_x = ((xh & 0x0F) << 8) | xl;
                        latest_y = ((yh & 0x0F) << 8) | yl;
                        touch_down = true;
                        
                        ESP_LOGD(TAG, "Touch: x=%d, y=%d", latest_x, latest_y);
                    }
                } else {
                    touch_down = false;
                }
                vTaskDelay(pdMS_TO_TICKS(10)); // Small debounce
            } while (num_fingers > 0);
        }
    }
}

// ------------------- LVGL callback -------------------
void touch_read_callback(lv_indev_t *indev, lv_indev_data_t *data) {
    (void)indev;
    
    data->point.x = latest_x;
    data->point.y = latest_y;
    data->state = touch_down ? LV_INDEV_STATE_PR : LV_INDEV_STATE_REL;
}

// ------------------- Init -------------------
void touch_init(i2c_master_dev_handle_t i2c_handle) {
    i2c_dev = i2c_handle;
    
    // Create semaphore
    touch_sem = xSemaphoreCreateBinary();
    if (!touch_sem) {
        ESP_LOGE(TAG, "Failed to create semaphore");
        return;
    }
    
    // Reset CST816S
    gpio_reset_pin(RST_PIN);
    gpio_set_direction(RST_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(RST_PIN, 1);
    vTaskDelay(pdMS_TO_TICKS(50));
    gpio_set_level(RST_PIN, 0);
    vTaskDelay(pdMS_TO_TICKS(10));
    gpio_set_level(RST_PIN, 1);
    vTaskDelay(pdMS_TO_TICKS(50));
    
    // Configure INT pin
    gpio_reset_pin(INT_PIN);
    gpio_set_direction(INT_PIN, GPIO_MODE_INPUT);
    gpio_set_pull_mode(INT_PIN, GPIO_PULLUP_ONLY);
    gpio_set_intr_type(INT_PIN, GPIO_INTR_NEGEDGE);
    gpio_install_isr_service(0);
    gpio_isr_handler_add(INT_PIN, gpio_isr_handler, NULL);
    gpio_intr_enable(INT_PIN);
    
    // Start background task with smaller stack
    xTaskCreate(touch_task, "touch_task", 2048, NULL, 8, NULL);
    
    // *** THIS IS THE MISSING PART ***
    // Create LVGL input device (v9.3 API)
    s_indev = lv_indev_create();
    if (s_indev) {
        lv_indev_set_type(s_indev, LV_INDEV_TYPE_POINTER);
        lv_indev_set_read_cb(s_indev, touch_read_callback);
        ESP_LOGI(TAG, "LVGL touch input device created successfully");
    } else {
        ESP_LOGE(TAG, "Failed to create LVGL input device");
    }
}

lv_indev_t* touch_get_indev(void)
{
    return s_indev;
}