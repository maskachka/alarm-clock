#pragma once

#include <stdint.h>

#include "clock_time.h"

inline ClockTime makeTime(uint8_t hour, uint8_t minute, uint8_t second = 0, uint8_t weekday = 0, uint8_t day = 0,
                          uint8_t month = 0) {
  return {hour, minute, second, weekday, day, month};
}
