#include "time_sync.h"
#include "esp_log.h"
#include "esp_sntp.h"
#include "wifi.h"
#include <stdbool.h>
#include <sys/time.h>
#include <string.h>

static const char *TAG = "TIME SYNC";
static bool s_time_synced = false;

// Check if time has been synchronized
bool time_is_synced(void)
{
    return s_time_synced;
}

// SNTP callback — called when time is synchronized
static void time_sync_cb(struct timeval *tv) {
    if (!s_time_synced) {
        s_time_synced = true;
        ESP_LOGI(TAG, "Time synchronized from SNTP");

        // Switch SNTP to hourly updates after first sync
        esp_sntp_set_sync_interval(3600 * 1000); // 1 hour in milliseconds
    }
}

// Initialize SNTP without blocking
esp_err_t time_sync_init(void)
{
    // Make sure Wi-Fi is connected first
    if (connect_wifi() != ESP_OK) {
        ESP_LOGE(TAG, "Wi-Fi not connected. Cannot sync time.");
        return ESP_FAIL;
    }

    // Set timezone immediately (no blocking)
    setenv("TZ", "EST5EDT,M3.2.0/2,M11.1.0", 1);
    tzset();

    // Initialize SNTP
    esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
    esp_sntp_setservername(0, "pool.ntp.org");
    esp_sntp_set_time_sync_notification_cb(time_sync_cb);
    esp_sntp_init();

    // Fast retry until first sync (10 seconds)
    esp_sntp_set_sync_interval(10 * 1000); // 10 seconds in milliseconds

    ESP_LOGI(TAG, "SNTP initialized, time will sync asynchronously");

    return ESP_OK;
}

// Returns current timestamp
time_t time_sync_get_current_time(void)
{
    time_t now;
    time(&now);
    return now;
}

// Returns current local time struct
struct tm time_sync_get_local_time(void)
{
    time_t now = time_sync_get_current_time();
    struct tm local_time;
    localtime_r(&now, &local_time);
    return local_time;
}
