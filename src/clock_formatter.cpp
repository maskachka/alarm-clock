#include "clock_formatter.h"

#include <stdio.h>

namespace ClockFormatter {
namespace {
constexpr const char* kWeekdayNames[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
constexpr const char* kMonthNames[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
                                       "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
}  // namespace

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

size_t formatShortDate(const ClockTime& time_value, char* out_buffer, size_t buffer_size) {
  if (out_buffer == nullptr || buffer_size == 0) return 0;
  if (time_value.weekday > 6 || time_value.month == 0 || time_value.month > 12 || time_value.day == 0 ||
      time_value.day > 31) {
    out_buffer[0] = '\0';
    return 0;
  }

  const int written = snprintf(out_buffer, buffer_size, "%s, %s %u", kWeekdayNames[time_value.weekday],
                               kMonthNames[time_value.month - 1], time_value.day);
  if (written < 0) {
    out_buffer[0] = '\0';
    return 0;
  }
  return static_cast<size_t>(written);
}

}  // namespace ClockFormatter
