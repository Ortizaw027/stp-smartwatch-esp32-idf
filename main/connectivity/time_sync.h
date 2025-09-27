#pragma once
#include "esp_err.h"
#include <stdbool.h>
#include <time.h>

// For instant time update on startup
bool time_is_synced(void); 

// Initializes SNTP and syncs time
esp_err_t time_sync_init(void);         

// Returns current timestamp
time_t time_sync_get_current_time(void); 

// Returns struct tm for local time
struct tm time_sync_get_local_time(void); 
