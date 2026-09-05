#pragma once

#include <stddef.h>

#include "clock_time.h"

namespace ClockFormatter {

// Formats time as HH:MM. Returns number of chars written (excluding null terminator).
size_t formatHHMM(const ClockTime& time_value, char* out_buffer, size_t buffer_size);

// Formats time as HH:MM:SS. Returns number of chars written (excluding null terminator).
size_t formatHHMMSS(const ClockTime& time_value, char* out_buffer, size_t buffer_size);

}  // namespace ClockFormatter
