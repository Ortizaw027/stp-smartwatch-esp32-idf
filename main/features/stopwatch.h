#pragma once
#include <stdbool.h>
#include <stdint.h>

// Start the stopwatch
void stopwatch_start(void);

// Stop the stopwatch
void stopwatch_stop(void);

// Toggle running state
void stopwatch_toggle(void);

// Reset stopwatch to zero
void stopwatch_reset(void);

// Returns true if running
bool stopwatch_running(void);

// Returns elapsed time in microseconds
int64_t stopwatch_get_elapsed_us(void);

// Returns elapsed time in milliseconds
int64_t stopwatch_get_elapsed_ms(void);

// Returns formatted string "mm:ss:hh" (minutes:seconds:hundredths) for UI
const char* stopwatch_get_time_str(void);
