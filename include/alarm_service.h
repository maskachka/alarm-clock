#pragma once

#include <stdint.h>

#include "alarm_ringtone.h"
#include "clock_time.h"

class AlarmService {
 public:
  static constexpr uint8_t kMaxAlarms = 4;
  static constexpr uint8_t kEveryDayMask = 0x7f;

  struct NextOccurrence {
    uint8_t alarm_index;
    uint8_t hour;
    uint8_t minute;
    uint8_t weekday;
    uint8_t days_from_now;
    uint16_t minutes_until;
  };

  AlarmService();

  void load();

  uint8_t count() const;
  bool addAlarm(uint8_t hour, uint8_t minute);
  bool removeAlarm(uint8_t index);
  void setAlarm(uint8_t index, uint8_t hour, uint8_t minute);
  void setWeekdayMask(uint8_t index, uint8_t weekday_mask);
  void setEnabled(uint8_t index, bool enabled);
  void dismiss(uint8_t index);
  bool isEnabled(uint8_t index) const;
  bool isRinging(uint8_t index) const;
  bool hasRingingAlarm() const;
  uint8_t dismissAllRinging();
  uint8_t hour(uint8_t index) const;
  uint8_t minute(uint8_t index) const;
  uint8_t weekdayMask(uint8_t index) const;
  AlarmRingtone ringtone(uint8_t index) const;
  void setRingtone(uint8_t index, AlarmRingtone ringtone);
  AlarmRingtone ringingRingtone() const;
  bool nextOccurrence(const ClockTime& now, NextOccurrence& occurrence) const;
  bool nextSnoozeOccurrence(const ClockTime& now, NextOccurrence& occurrence) const;
  uint8_t snoozeAllRinging(const ClockTime& now, uint16_t duration_minutes = 10);

  bool update(const ClockTime& now);

 private:
  struct Alarm {
    uint8_t hour;
    uint8_t minute;
    uint8_t weekday_mask;
    AlarmRingtone ringtone;
    bool enabled;
    bool ringing;
    bool triggered_for_current_minute;
    bool snoozing;
    uint8_t snooze_weekday;
    uint16_t snooze_minute;
  };

  bool isValidIndex(uint8_t index) const;
  Alarm& alarm(uint8_t index);
  const Alarm& alarm(uint8_t index) const;
  void persist() const;

  Alarm alarms_[kMaxAlarms];
  uint8_t count_;
  int16_t last_checked_minute_;
};
