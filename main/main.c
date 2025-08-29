#include <stdio.h>
#include "esp_log.h"
#include "lvgl.h"
#include "lcd_driver.h"
#include "lvgl_tick.h"
#include "screen.h"
#include "wifi.h"
#include "time_sync.h"
#include "touch_driver.h"
#include "driver/i2c_master.h"


// I2C config
#define I2C_NUM                     I2C_NUM_0
#define I2C_SDA                     6
#define I2C_SCL                     7
#define I2C_FREQ_HZ                 400000
#define CST816S_I2C_ADDR            0x15


// LVGL buffer config
#define DISP_HOR_RES                240
#define DISP_VER_RES                240
#define DISP_BUF_FACTOR             10
#define BYTES_PER_PIXEL             2
#define DISP_BUF_SIZE               ((DISP_HOR_RES * DISP_VER_RES) / DISP_BUF_FACTOR * BYTES_PER_PIXEL)

void my_flush_cb();

static i2c_master_bus_handle_t i2c_bus_handle = NULL;
static i2c_master_dev_handle_t i2c_device_handle = NULL;

void init_i2c();

void app_main(void)
{
   // Needed before anything can be done with lvgl
   lv_init(); 
   ESP_LOGI("LVGL", "LVGL initialized.");
   
   // Initializes lcd screen(GC9A01)
   esp_lcd_panel_handle_t panel_handle = lcd_init(); 
   
 
   // Checks if lvgl is intialized correctly and outputs and error message if not
   if (!lv_is_initialized())
   {
      ESP_LOGE("LVGL", "LVGL failed to initialize.");
      return;
   }
    // Init I2C
      init_i2c();

   // Creates a display in lvlg the correct size of our actual display
   lv_display_t * display1 = lv_display_create(240, 240);

   display_context_t * ctx = malloc(sizeof(display_context_t));
   ctx -> panel = panel_handle;

   lv_display_set_user_data(display1, ctx);

   // Buffer setup
   uint8_t *buf1 = heap_caps_malloc(DISP_BUF_SIZE, MALLOC_CAP_DMA);
   uint8_t *buf2 = heap_caps_malloc(DISP_BUF_SIZE, MALLOC_CAP_DMA);
   lv_display_set_buffers(display1, buf1, buf2, DISP_BUF_SIZE, LV_DISPLAY_RENDER_MODE_PARTIAL);

   lv_display_set_flush_cb(display1, my_flush_cb);

   // Starts millisecond timer for lvgl tick
   init_lvgl_tick(); 

   // Initialize SNTP for time sync
   ESP_LOGI("SNTP", "Initializing SNTP...");
   if(time_sync_init() == ESP_OK)
   {
      ESP_LOGI("SNTP", "SNTP initialized succesfully");
   }
   else
   {
      ESP_LOGE("SNTP", "SNTP failed to intialize");
   }
   
   // Initializes the lvgl screens
   init_screens();

   while(1)
   {
      // LVGL periodic handler
      lv_timer_handler();
      
      // Small delay
      vTaskDelay(pdMS_TO_TICKS(10));
   } 
  
  
     
}

// Flushes lvgl setup to screen
void my_flush_cb(lv_display_t * disp, const lv_area_t * area, uint8_t * px_buf)
{
   disp_update(disp, area, px_buf);

   lv_display_flush_ready(disp);
}

// I2C init function
void init_i2c(void)
{
  
  i2c_master_bus_config_t i2c_bus_config = {
   .clk_source = I2C_CLK_SRC_DEFAULT,
   .i2c_port = I2C_NUM,
   .scl_io_num = I2C_SCL,
   .sda_io_num = I2C_SDA,
   .glitch_ignore_cnt = 7,
   .flags.enable_internal_pullup = true
  };
  ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_bus_config, &i2c_bus_handle));
  ESP_LOGI("I2C", "I2C Bus Initialized");

  i2c_device_config_t i2c_device_config = {
   .dev_addr_length = I2C_ADDR_BIT_LEN_7,
   .device_address = CST816S_I2C_ADDR,
   .scl_speed_hz = I2C_FREQ_HZ
  };
  ESP_ERROR_CHECK(i2c_master_bus_add_device(i2c_bus_handle, &i2c_device_config, &i2c_device_handle));
  ESP_LOGI("I2C", "I2C Device Added");
  vTaskDelay(pdMS_TO_TICKS(10));
  ESP_LOGI("I2C", "I2C Device Initialized");
}
