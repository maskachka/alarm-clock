#pragma once

#include <stdint.h>

#include "clock_time.h"

class AlarmService {
 public:
  static constexpr uint8_t kMaxAlarms = 4;

  AlarmService();

  void load();

  uint8_t count() const;
  bool addAlarm(uint8_t hour, uint8_t minute);
  bool removeAlarm(uint8_t index);
  void setAlarm(uint8_t index, uint8_t hour, uint8_t minute);
  void setEnabled(uint8_t index, bool enabled);
  void dismiss(uint8_t index);
  bool isEnabled(uint8_t index) const;
  bool isRinging(uint8_t index) const;
  bool hasRingingAlarm() const;
  uint8_t dismissAllRinging();
  uint8_t hour(uint8_t index) const;
  uint8_t minute(uint8_t index) const;

  void setAlarm(uint8_t hour, uint8_t minute);
  void setEnabled(bool enabled);
  void dismiss();

  bool isEnabled() const;
  bool isRinging() const;
  uint8_t hour() const;
  uint8_t minute() const;

  bool update(const ClockTime& now);

 private:
  struct Alarm {
    uint8_t hour;
    uint8_t minute;
    bool enabled;
    bool ringing;
    bool triggered_for_current_minute;
  };

  bool isValidIndex(uint8_t index) const;
  Alarm& alarm(uint8_t index);
  const Alarm& alarm(uint8_t index) const;
  void persist() const;

  Alarm alarms_[kMaxAlarms];
  uint8_t count_;
  int16_t last_checked_minute_;
};
