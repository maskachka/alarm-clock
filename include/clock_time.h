#pragma once

#include <stdint.h>

struct ClockTime {
  uint8_t hour;
  uint8_t minute;
  uint8_t second;
  // Matches the C library convention: Sunday is 0, Saturday is 6.
  uint8_t weekday;
  uint8_t day;
  // One-based month number: January is 1, December is 12.
  uint8_t month;
};
