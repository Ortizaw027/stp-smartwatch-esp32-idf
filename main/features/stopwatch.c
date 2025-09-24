#include "stopwatch.h"
#include "esp_timer.h"
#include <stdio.h>
#include <stdbool.h>

// Internal state
static bool running = false;
static int64_t start_time = 0;        // when stopwatch was last started
static int64_t accumulated_time = 0;  // total before last start
static char time_str[32];             // static buffer for formatted time

void stopwatch_start(void)
{
    if (!running) {
        start_time = esp_timer_get_time();
        running = true;
    }
}

void stopwatch_stop(void)
{
    if (running) {
        int64_t now = esp_timer_get_time();
        accumulated_time += (now - start_time);
        running = false;
    }
}

void stopwatch_toggle(void)
{
    if (running) {
        stopwatch_stop();
    } else {
        stopwatch_start();
    }
}

void stopwatch_reset(void)
{
    running = false;
    accumulated_time = 0;
    start_time = 0;
}

bool stopwatch_running(void)
{
    return running;
}

int64_t stopwatch_get_elapsed_us(void)
{
    if (running) {
        int64_t now = esp_timer_get_time();
        return accumulated_time + (now - start_time);
    } else {
        return accumulated_time;
    }
}

int64_t stopwatch_get_elapsed_ms(void)
{
    return stopwatch_get_elapsed_us() / 1000;
}

const char* stopwatch_get_time_str(void)
{
    int64_t total_us = stopwatch_get_elapsed_us();
    int total_sec = (int)(total_us / 1000000);
    int minutes = total_sec / 60;
    int seconds = total_sec % 60;
    int hundredths = (int)((total_us % 1000000) / 10000);  // 1/100 sec

    snprintf(time_str, sizeof(time_str), "%02d:%02d:%02d", minutes, seconds, hundredths);
    return time_str;
}
