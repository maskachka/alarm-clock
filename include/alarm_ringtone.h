#pragma once

#include <stdint.h>

enum class AlarmRingtone : uint8_t {
  ClassicChime = 0,
  GentlePulse,
  Sunrise,
  Urgent,
  Count,
};

constexpr AlarmRingtone kDefaultAlarmRingtone = AlarmRingtone::ClassicChime;
