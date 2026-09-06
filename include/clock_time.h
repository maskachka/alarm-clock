#pragma once

#include <stdint.h>

struct ClockTime {
  uint8_t hour;
  uint8_t minute;
  uint8_t second;
  // Matches the C library convention: Sunday is 0, Saturday is 6.
  uint8_t weekday;
};
