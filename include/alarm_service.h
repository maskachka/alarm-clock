#pragma once

#include <stdint.h>

#include "clock_time.h"

class AlarmService {
public:
  AlarmService();

  void setAlarm(uint8_t hour, uint8_t minute);
  void setEnabled(bool enabled);
  void dismiss();

  bool isEnabled() const;
  bool isRinging() const;
  uint8_t hour() const;
  uint8_t minute() const;

  bool update(const ClockTime &now);

private:
  uint8_t hour_;
  uint8_t minute_;
  int16_t last_checked_minute_;
  bool enabled_;
  bool ringing_;
  bool triggered_for_current_minute_;
};
