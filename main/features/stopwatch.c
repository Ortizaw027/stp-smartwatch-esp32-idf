#include "stopwatch.h"
#include "esp_timer.h"
#include <stdio.h>
#include <stdbool.h>

// Internal state of the stopwatch
static bool running = false;

// Time when the stopwatch was last started
static int64_t start_time = 0;

// Accumulated time before the last start
static int64_t accumulated_time = 0;

// Static buffer for formatted time string
static char time_str[32];

// Start the stopwatch
void stopwatch_start(void)
{
    if (!running) {
        start_time = esp_timer_get_time();
        running = true;
    }
}

// Stop the stopwatch and accumulate time
void stopwatch_stop(void)
{
    if (running) {
        int64_t now = esp_timer_get_time();
        accumulated_time += (now - start_time);
        running = false;
    }
}

// Toggle stopwatch running state
void stopwatch_toggle(void)
{
    if (running) {
        stopwatch_stop();
    } else {
        stopwatch_start();
    }
}

// Reset stopwatch to zero
void stopwatch_reset(void)
{
    running = false;
    accumulated_time = 0;
    start_time = 0;
}

// Check if stopwatch is running
bool stopwatch_running(void)
{
    return running;
}

// Get elapsed time in microseconds
int64_t stopwatch_get_elapsed_us(void)
{
    if (running) {
        int64_t now = esp_timer_get_time();
        return accumulated_time + (now - start_time);
    } else {
        return accumulated_time;
    }
}

// Get elapsed time in milliseconds
int64_t stopwatch_get_elapsed_ms(void)
{
    return stopwatch_get_elapsed_us() / 1000;
}

// Get formatted time string (MM:SS:hundredths)
const char* stopwatch_get_time_str(void)
{
    int64_t total_us = stopwatch_get_elapsed_us();
    int total_sec = (int)(total_us / 1000000);
    int minutes = total_sec / 60;
    int seconds = total_sec % 60;

    // Convert microseconds to hundredths
    int hundredths = (int)((total_us % 1000000) / 10000); 

    snprintf(time_str, sizeof(time_str), "%02d:%02d:%02d", minutes, seconds, hundredths);
    return time_str;
}
