#include "clock_formatter.h"

#include <stdio.h>

namespace ClockFormatter {

size_t formatHHMM(const ClockTime& time_value, char* out_buffer, size_t buffer_size) {
  if (out_buffer == nullptr || buffer_size == 0) {
    return 0;
  }

  const int written = snprintf(out_buffer, buffer_size, "%02u:%02u", time_value.hour, time_value.minute);
  if (written < 0) {
    out_buffer[0] = '\0';
    return 0;
  }

  return static_cast<size_t>(written);
}

size_t formatHHMMSS(const ClockTime& time_value, char* out_buffer, size_t buffer_size) {
  if (out_buffer == nullptr || buffer_size == 0) {
    return 0;
  }

  const int written =
      snprintf(out_buffer, buffer_size, "%02u:%02u:%02u", time_value.hour, time_value.minute, time_value.second);

  if (written < 0) {
    out_buffer[0] = '\0';
    return 0;
  }

  return static_cast<size_t>(written);
}

}  // namespace ClockFormatter
