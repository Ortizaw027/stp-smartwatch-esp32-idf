#include <stdio.h>
#include "esp_log.h"
#include "lvgl.h"
#include "lcd_driver.h"
#include "lvgl_tick.h"
#include "ui.h"
#include "wifi.h"
#include "time_sync.h"
#include "touch_driver.h"
#include "driver/i2c_master.h"
#include "imu_driver.h"
#include "step_counter.h"
#include "i2c_scan.h"

// I2C config
#define I2C_NUM       I2C_NUM_0
#define I2C_SDA       6
#define I2C_SCL       7
#define I2C_FREQ_HZ   100000
#define CST816S_I2C_ADDR 0x15
#define QMI8658_I2C_ADDR 0x6B

// LVGL buffer config
#define DISP_HOR_RES     240
#define DISP_VER_RES     240
#define DISP_BUF_FACTOR  10
#define BYTES_PER_PIXEL  2
#define DISP_BUF_SIZE    ((DISP_HOR_RES * DISP_VER_RES) / DISP_BUF_FACTOR * BYTES_PER_PIXEL)

// IMU update interval (in main loop cycles)
#define IMU_UPDATE_INTERVAL 10  // 10 * 5ms = 50ms updates

static const char* TAG = "MAIN";

static i2c_master_bus_handle_t i2c_bus_handle = NULL;
static i2c_master_dev_handle_t touch_device_handle = NULL;
static i2c_master_dev_handle_t imu_device_handle = NULL;

void my_flush_cb(lv_display_t *disp, const lv_area_t *area, uint8_t *px_buf);
void init_i2c(void);

void app_main(void)
{
    ESP_LOGI(TAG, "Starting app_main, free heap: %" PRIu32, (uint32_t)esp_get_free_heap_size());

    // Initialize SNTP 
    ESP_LOGI(TAG, "Initializing SNTP...");
    if(time_sync_init() == ESP_OK) {
        ESP_LOGI(TAG, "SNTP initialized successfully");
    } else {
        ESP_LOGE(TAG, "SNTP failed to initialize");
    }

    // Initialize LVGL
    lv_init();
    ESP_LOGI(TAG, "LVGL initialized.");

    // Initialize LCD
    esp_lcd_panel_handle_t panel_handle = lcd_init();
    ESP_LOGI(TAG, "LCD initialized.");
    
    // Initialize I2C after time sync
    init_i2c();
    scan_i2c(i2c_bus_handle);
   
    // Allocate LVGL display buffers on the heap
    uint8_t *buf1 = heap_caps_malloc(DISP_BUF_SIZE, MALLOC_CAP_DMA);
    uint8_t *buf2 = heap_caps_malloc(DISP_BUF_SIZE, MALLOC_CAP_DMA);
    if (!buf1 || !buf2) {
        ESP_LOGE(TAG, "Failed to allocate LVGL buffers");
        return;
    }
    ESP_LOGI(TAG, "LVGL buffers allocated, size: %d bytes each", DISP_BUF_SIZE);

    // Create LVGL display
    lv_display_t *display = lv_display_create(DISP_HOR_RES, DISP_VER_RES);
    if (!display) {
        ESP_LOGE(TAG, "Failed to create LVGL display");
        return;
    }

    display_context_t *ctx = heap_caps_malloc(sizeof(display_context_t), MALLOC_CAP_INTERNAL);
    ctx->panel = panel_handle;
    lv_display_set_user_data(display, ctx);

    lv_display_set_buffers(display, buf1, buf2, DISP_BUF_SIZE, LV_DISPLAY_RENDER_MODE_PARTIAL);
    lv_display_set_flush_cb(display, my_flush_cb);

    // Initialize LVGL millisecond tick
    init_lvgl_tick();

    // Initialize touch after LVGL display is ready
    touch_init(touch_device_handle);

    // Initialize IMU 
    imu_init(imu_device_handle);

    // Initialize step counter
    step_counter_init();

    // Retrieve the touch input device from the driver and enable it for LVGL
    lv_indev_t *touch_indev = touch_get_indev();
    if (touch_indev) {
        lv_indev_enable(touch_indev, true);
        ESP_LOGI(TAG, "Touch input device pointer retrieved and enabled");
    } else {
        ESP_LOGE(TAG, "Failed to enable touch input — driver returned NULL");
    }

    // Initialize UI 
    ui_init();

    // Counter for IMU updates to reduce I2C bus load
    int imu_counter = 0;

    // Main loop
    while (1) {
        // Handle LVGL tasks
        lv_timer_handler(); 
       
        // Update IMU less frequently to prevent I2C bus congestion
        imu_counter++;
        if (imu_counter >= IMU_UPDATE_INTERVAL) {
            // Only update IMU every 50ms instead of every 5ms
            imu_update(imu_device_handle);

            // Updates the step counter
            step_counter_update();
            
            imu_counter = 0;
        }

        // 5 ms delay
        vTaskDelay(pdMS_TO_TICKS(5));          
    }
}

// Flush callback for LVGL
void my_flush_cb(lv_display_t *disp, const lv_area_t *area, uint8_t *px_buf)
{
    disp_update(disp, area, px_buf);
    lv_display_flush_ready(disp);
}

// I2C initialization
void init_i2c(void)
{
    i2c_master_bus_config_t bus_cfg = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = I2C_NUM,
        .scl_io_num = I2C_SCL,
        .sda_io_num = I2C_SDA,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true
    };
    ESP_ERROR_CHECK(i2c_new_master_bus(&bus_cfg, &i2c_bus_handle));
    ESP_LOGI(TAG, "I2C bus initialized");

    // Touch device I2C config
    i2c_device_config_t touch_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = CST816S_I2C_ADDR,
        .scl_speed_hz = I2C_FREQ_HZ
    };
    ESP_ERROR_CHECK(i2c_master_bus_add_device(i2c_bus_handle, &touch_cfg, &touch_device_handle));
    ESP_LOGI(TAG, "I2C touch device added");

    // IMU device I2C config
    i2c_device_config_t imu_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = QMI8658_I2C_ADDR,
        .scl_speed_hz = I2C_FREQ_HZ
    };
    ESP_ERROR_CHECK(i2c_master_bus_add_device(i2c_bus_handle, &imu_cfg, &imu_device_handle));
    ESP_LOGI(TAG, "I2C IMU device added");

    vTaskDelay(pdMS_TO_TICKS(10));
    ESP_LOGI(TAG, "I2C devices ready");
}