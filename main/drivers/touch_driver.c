#include <stdio.h>
#include "touch_driver.h"
#include "lvgl.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "driver/i2c_master.h"

// Pin definitions
#define INT_PIN        5
#define RST_PIN        13

static SemaphoreHandle_t touch_sem = NULL;


// ISR triggers when INT pin goes low
static void IRAM_ATTR gpio_isr_handler(void *args){
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    //Signals the task a touch event occurred
    xSemaphoreGiveFromISR(touch_sem, &xHigherPriorityTaskWoken);

    if(xHigherPriorityTaskWoken){
        portYIELD_FROM_ISR();
    }
}

void touch_read(void *args){
    i2c_master_dev_handle_t i2c_handle = (i2c_master_dev_handle_t)args;
    uint8_t gesture;
    uint8_t xh, xl, yh, yl;
    uint16_t x, y;

    while(1){
        // Wait for ISR semaphore (touch event)
        if(xSemaphoreTake(touch_sem, portMAX_DELAY) == pdTRUE){
            // Small delay to let the gesture register stabilize
            vTaskDelay(pdMS_TO_TICKS(10));

            // --- Read Gesture Register (0x01) ---
            uint8_t reg_gesture = 0x01;
            esp_err_t ret = i2c_master_transmit_receive(i2c_handle, &reg_gesture, 1, &gesture, 1, 1000);
            if(ret != ESP_OK){
                ESP_LOGW("TOUCH", "Failed to read gesture register");
                continue;
            }

            // --- Handle Gestures ---
            switch(gesture){
                case 0x01: // Slide Up
                    ESP_LOGI("TOUCH", "Swipe Up");
                    break;
                case 0x02: // Slide Down
                    ESP_LOGI("TOUCH", "Swipe Down");
                    break;
                case 0x03: // Swipe Left
                    ESP_LOGI("TOUCH", "Swipe Left");
                    break;
                case 0x04: // Swipe Right
                    ESP_LOGI("TOUCH", "Swipe Right");
                    break;
                case 0x05: // Single Click / Tap
                case 0x0B: // Double Click
                    {
                        // --- Read X and Y coordinates ---
                        uint8_t reg_xh = 0x03;
                        uint8_t reg_xl = 0x04;
                        uint8_t reg_yh = 0x05;
                        uint8_t reg_yl = 0x06;

                        if(i2c_master_transmit_receive(i2c_handle, &reg_xh, 1, &xh, 1, 1000) == ESP_OK &&
                           i2c_master_transmit_receive(i2c_handle, &reg_xl, 1, &xl, 1, 1000) == ESP_OK &&
                           i2c_master_transmit_receive(i2c_handle, &reg_yh, 1, &yh, 1, 1000) == ESP_OK &&
                           i2c_master_transmit_receive(i2c_handle, &reg_yl, 1, &yl, 1, 1000) == ESP_OK){

                            x = ((xh & 0x0F) << 8) | xl;
                            y = ((yh & 0x0F) << 8) | yl;

                            if(gesture == 0x05)
                                ESP_LOGI("TOUCH", "Single Tap at X: %d Y: %d", x, y);
                            else
                                ESP_LOGI("TOUCH", "Double Tap at X: %d Y: %d", x, y);
                        } else {
                            ESP_LOGW("TOUCH", "Failed to read coordinates");
                        }
                    }
                    break;
                case 0x0C: // Long Press
                    ESP_LOGI("TOUCH", "Long Press Detected");
                    break;
                default:
                    break;
            }
        }
    }
}



//Initializes CST816S touch driver
void touch_init(i2c_master_dev_handle_t i2c_handle){

    //Create binary semaphore
    touch_sem = xSemaphoreCreateBinary();
    if(touch_sem == NULL){
        ESP_LOGE("TOUCH", "Failed to create semaphore");
    } 

    //Reset CST816S
    gpio_reset_pin(RST_PIN);
    gpio_set_direction(RST_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(RST_PIN, 1);
    vTaskDelay(pdMS_TO_TICKS(50));
    gpio_set_level(RST_PIN, 0);
    vTaskDelay(pdMS_TO_TICKS(10));
    gpio_set_level(RST_PIN, 1);
    vTaskDelay(pdMS_TO_TICKS(50));

    //Configure INT pin and attach ISR
    gpio_reset_pin(INT_PIN);
    gpio_set_direction(INT_PIN, GPIO_MODE_INPUT);
    gpio_set_pull_mode(INT_PIN, GPIO_PULLUP_ONLY);
    gpio_set_intr_type(INT_PIN, GPIO_INTR_NEGEDGE);
    gpio_install_isr_service(0);
    gpio_isr_handler_add(INT_PIN, gpio_isr_handler, NULL);
    gpio_intr_enable(INT_PIN);

    // Start FreeRTOS task to process touch events
    xTaskCreate(touch_read, "touch_task", 4096, (void *)i2c_handle, 10, NULL);
}
