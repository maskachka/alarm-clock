#include "alarm_service.h"

AlarmService::AlarmService() : alarms_{}, storage_(nullptr), count_(1), last_checked_minute_(-1) {
  alarms_[0] = {7, 0, kEveryDayMask, kDefaultAlarmRingtone, false, false, false, false, 0, 0};
}

AlarmService::AlarmService(AlarmStorage& storage) : alarms_{}, storage_(&storage), count_(1), last_checked_minute_(-1) {
  alarms_[0] = {7, 0, kEveryDayMask, kDefaultAlarmRingtone, false, false, false, false, 0, 0};
}
void AlarmService::load() {
  if (storage_ == nullptr) return;
  StoredAlarms stored = {};
  bool migrated = false;
  if (!storage_->load(stored, migrated)) return;

  count_ = stored.count;
  for (uint8_t i = 0; i < count_; ++i) {
    alarms_[i] = {static_cast<uint8_t>(stored.alarms[i].hour % 24),
                  static_cast<uint8_t>(stored.alarms[i].minute % 60),
                  static_cast<uint8_t>(stored.alarms[i].weekday_mask & kEveryDayMask),
                  stored.alarms[i].ringtone < static_cast<uint8_t>(AlarmRingtone::Count)
                      ? static_cast<AlarmRingtone>(stored.alarms[i].ringtone)
                      : kDefaultAlarmRingtone,
                  stored.alarms[i].enabled != 0,
                  false,
                  false,
                  false,
                  0,
                  0};
  }
  last_checked_minute_ = -1;
  if (migrated) persist();
}
uint8_t AlarmService::count() const { return count_; }
bool AlarmService::addAlarm(uint8_t h, uint8_t m) {
  if (count_ == kMaxAlarms) return false;
  alarms_[count_++] = {static_cast<uint8_t>(h % 24),
                       static_cast<uint8_t>(m % 60),
                       kEveryDayMask,
                       kDefaultAlarmRingtone,
                       false,
                       false,
                       false,
                       false,
                       0,
                       0};
  persist();
  return true;
}
bool AlarmService::removeAlarm(uint8_t index) {
  if (!isValidIndex(index)) return false;
  for (uint8_t i = index + 1; i < count_; ++i) alarms_[i - 1] = alarms_[i];
  --count_;
  persist();
  return true;
}
void AlarmService::setAlarm(uint8_t i, uint8_t h, uint8_t m) {
  if (isValidIndex(i)) {
    Alarm& a = alarm(i);
    a.hour = h % 24;
    a.minute = m % 60;
    persist();
  }
}
void AlarmService::setWeekdayMask(uint8_t i, uint8_t weekday_mask) {
  if (!isValidIndex(i)) return;
  alarm(i).weekday_mask = weekday_mask & kEveryDayMask;
  persist();
}
void AlarmService::setEnabled(uint8_t i, bool enabled) {
  if (isValidIndex(i)) {
    Alarm& a = alarm(i);
    a.enabled = enabled;
    if (!enabled) {
      a.ringing = false;
      a.snoozing = false;
    }
    persist();
  }
}
void AlarmService::dismiss(uint8_t i) {
  if (isValidIndex(i)) {
    Alarm& a = alarm(i);
    a.ringing = false;
    a.triggered_for_current_minute = true;
    persist();
  }
}
bool AlarmService::isEnabled(uint8_t i) const { return isValidIndex(i) && alarm(i).enabled; }
bool AlarmService::isRinging(uint8_t i) const { return isValidIndex(i) && alarm(i).ringing; }
bool AlarmService::hasRingingAlarm() const {
  for (uint8_t i = 0; i < count_; ++i)
    if (alarms_[i].ringing) return true;
  return false;
}
uint8_t AlarmService::dismissAllRinging() {
  uint8_t dismissed = 0;
  for (uint8_t i = 0; i < count_; ++i)
    if (alarms_[i].ringing) {
      alarms_[i].ringing = false;
      alarms_[i].triggered_for_current_minute = true;
      alarms_[i].snoozing = false;
      ++dismissed;
    }
  return dismissed;
}
uint8_t AlarmService::hour(uint8_t i) const { return isValidIndex(i) ? alarm(i).hour : 0; }
uint8_t AlarmService::minute(uint8_t i) const { return isValidIndex(i) ? alarm(i).minute : 0; }
uint8_t AlarmService::weekdayMask(uint8_t i) const { return isValidIndex(i) ? alarm(i).weekday_mask : 0; }
AlarmRingtone AlarmService::ringtone(uint8_t i) const {
  return isValidIndex(i) ? alarm(i).ringtone : kDefaultAlarmRingtone;
}
void AlarmService::setRingtone(uint8_t i, AlarmRingtone ringtone) {
  if (!isValidIndex(i) || ringtone >= AlarmRingtone::Count) return;
  alarm(i).ringtone = ringtone;
  persist();
}
AlarmRingtone AlarmService::ringingRingtone() const {
  for (uint8_t i = count_; i > 0; --i)
    if (alarms_[i - 1].ringing) return alarms_[i - 1].ringtone;
  return kDefaultAlarmRingtone;
}
bool AlarmService::nextOccurrence(const ClockTime& now, NextOccurrence& occurrence) const {
  if (now.weekday > 6) return false;

  const uint16_t current_minute = static_cast<uint16_t>(now.hour) * 60 + now.minute;
  bool found = false;
  for (uint8_t i = 0; i < count_; ++i) {
    const Alarm& candidate = alarms_[i];
    if (!candidate.enabled || candidate.weekday_mask == 0) continue;

    if (candidate.snoozing) {
      uint8_t days_from_now = static_cast<uint8_t>((candidate.snooze_weekday + 7 - now.weekday) % 7);
      const uint16_t current_minute = static_cast<uint16_t>(now.hour) * 60 + now.minute;
      if (days_from_now == 0 && candidate.snooze_minute < current_minute) days_from_now = 7;
      const uint16_t minutes_until =
          static_cast<uint16_t>(days_from_now) * 24 * 60 + candidate.snooze_minute - current_minute;
      if (!found || minutes_until < occurrence.minutes_until) {
        occurrence = {i,
                      static_cast<uint8_t>(candidate.snooze_minute / 60),
                      static_cast<uint8_t>(candidate.snooze_minute % 60),
                      candidate.snooze_weekday,
                      days_from_now,
                      minutes_until};
        found = true;
      }
      continue;
    }

    const uint16_t alarm_minute = static_cast<uint16_t>(candidate.hour) * 60 + candidate.minute;
    for (uint8_t days_from_now = 0; days_from_now <= 7; ++days_from_now) {
      const uint8_t weekday = static_cast<uint8_t>((now.weekday + days_from_now) % 7);
      if ((candidate.weekday_mask & (1u << weekday)) == 0) continue;
      if (days_from_now == 0 && alarm_minute < current_minute) continue;

      const uint16_t minutes_until = static_cast<uint16_t>(days_from_now) * 24 * 60 + alarm_minute - current_minute;
      if (!found || minutes_until < occurrence.minutes_until) {
        occurrence = {i, candidate.hour, candidate.minute, weekday, days_from_now, minutes_until};
        found = true;
      }
      break;
    }
  }
  return found;
}
bool AlarmService::nextSnoozeOccurrence(const ClockTime& now, NextOccurrence& occurrence) const {
  if (now.weekday > 6) return false;

  const uint16_t current_minute = static_cast<uint16_t>(now.hour) * 60 + now.minute;
  bool found = false;
  for (uint8_t i = 0; i < count_; ++i) {
    const Alarm& candidate = alarms_[i];
    if (!candidate.enabled || !candidate.snoozing) continue;

    uint8_t days_from_now = static_cast<uint8_t>((candidate.snooze_weekday + 7 - now.weekday) % 7);
    if (days_from_now == 0 && candidate.snooze_minute < current_minute) days_from_now = 7;
    const uint16_t minutes_until =
        static_cast<uint16_t>(days_from_now) * 24 * 60 + candidate.snooze_minute - current_minute;
    if (!found || minutes_until < occurrence.minutes_until) {
      occurrence = {i,
                    static_cast<uint8_t>(candidate.snooze_minute / 60),
                    static_cast<uint8_t>(candidate.snooze_minute % 60),
                    candidate.snooze_weekday,
                    days_from_now,
                    minutes_until};
      found = true;
    }
  }
  return found;
}
uint8_t AlarmService::snoozeAllRinging(const ClockTime& now, uint16_t duration_minutes) {
  if (now.weekday > 6 || duration_minutes == 0) return 0;

  const uint16_t current_minute = static_cast<uint16_t>(now.hour) * 60 + now.minute;
  const uint16_t target = static_cast<uint16_t>(current_minute + duration_minutes);
  const uint8_t snooze_weekday = static_cast<uint8_t>((now.weekday + target / (24 * 60)) % 7);
  const uint16_t snooze_minute = target % (24 * 60);
  uint8_t snoozed = 0;
  for (uint8_t i = 0; i < count_; ++i) {
    Alarm& a = alarms_[i];
    if (!a.ringing) continue;
    a.ringing = false;
    a.triggered_for_current_minute = true;
    a.snoozing = true;
    a.snooze_weekday = snooze_weekday;
    a.snooze_minute = snooze_minute;
    ++snoozed;
  }
  return snoozed;
}
bool AlarmService::update(const ClockTime& now) {
  const int16_t current = static_cast<int16_t>(now.hour) * 60 + now.minute;
  if (current != last_checked_minute_) {
    last_checked_minute_ = current;
    for (uint8_t i = 0; i < count_; ++i) alarms_[i].triggered_for_current_minute = false;
  }
  bool started = false;
  for (uint8_t i = 0; i < count_; ++i) {
    Alarm& a = alarms_[i];
    if (!a.enabled) {
      a.ringing = false;
      a.snoozing = false;
      continue;
    }
    if (a.snoozing) {
      const uint16_t current_minute = static_cast<uint16_t>(now.hour) * 60 + now.minute;
      if (now.weekday == a.snooze_weekday && current_minute >= a.snooze_minute) {
        for (uint8_t j = 0; j < count_; ++j) alarms_[j].ringing = false;
        a.snoozing = false;
        a.ringing = true;
        a.triggered_for_current_minute = true;
        started = true;
      } else {
        a.ringing = false;
      }
      continue;
    }
    const uint8_t weekday_bit = now.weekday < 7 ? static_cast<uint8_t>(1u << now.weekday) : 0;
    if (!a.triggered_for_current_minute && now.hour == a.hour && now.minute == a.minute &&
        (a.weekday_mask & weekday_bit) != 0) {
      for (uint8_t j = 0; j < count_; ++j) alarms_[j].ringing = false;
      a.ringing = true;
      a.triggered_for_current_minute = true;
      started = true;
    }
  }
  return started;
}
bool AlarmService::isValidIndex(uint8_t i) const { return i < count_; }
AlarmService::Alarm& AlarmService::alarm(uint8_t i) { return alarms_[i]; }
const AlarmService::Alarm& AlarmService::alarm(uint8_t i) const { return alarms_[i]; }
void AlarmService::persist() const {
  if (storage_ == nullptr) return;
  StoredAlarms stored = {};
  stored.count = count_;
  for (uint8_t i = 0; i < count_; ++i) {
    stored.alarms[i] = {alarms_[i].hour, alarms_[i].minute, static_cast<uint8_t>(alarms_[i].enabled),
                        alarms_[i].weekday_mask, static_cast<uint8_t>(alarms_[i].ringtone)};
  }
  storage_->save(stored);
}
