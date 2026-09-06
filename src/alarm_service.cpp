#include "alarm_service.h"

#if defined(ARDUINO)
#include <Preferences.h>
#endif

namespace {
struct StoredAlarmV1 {
  uint8_t hour;
  uint8_t minute;
  uint8_t enabled;
};

struct StoredAlarm {
  uint8_t hour;
  uint8_t minute;
  uint8_t enabled;
  uint8_t weekday_mask;
};

constexpr uint8_t kStorageVersion = 2;
constexpr const char* kStorageNamespace = "alarm-clock";
constexpr const char* kVersionKey = "version";
constexpr const char* kCountKey = "count";
constexpr const char* kAlarmsKey = "alarms";
}  // namespace

AlarmService::AlarmService() : alarms_{}, count_(1), last_checked_minute_(-1) {
  alarms_[0] = {7, 0, kEveryDayMask, false, false, false};
}
void AlarmService::load() {
#if defined(ARDUINO)
  Preferences preferences;
  if (!preferences.begin(kStorageNamespace, true)) return;
  if (!preferences.isKey(kCountKey)) {
    preferences.end();
    return;
  }

  const uint8_t stored_count = preferences.getUChar(kCountKey, 0);
  const uint8_t stored_version = preferences.getUChar(kVersionKey, 1);
  const size_t record_size = stored_version >= kStorageVersion ? sizeof(StoredAlarm) : sizeof(StoredAlarmV1);
  if (stored_count > kMaxAlarms ||
      (stored_count > 0 && preferences.getBytesLength(kAlarmsKey) != stored_count * record_size)) {
    preferences.end();
    return;
  }

  StoredAlarm stored[kMaxAlarms] = {};
  if (stored_count > 0) {
    if (stored_version >= kStorageVersion) {
      preferences.getBytes(kAlarmsKey, stored, stored_count * sizeof(StoredAlarm));
    } else {
      StoredAlarmV1 legacy[kMaxAlarms] = {};
      preferences.getBytes(kAlarmsKey, legacy, stored_count * sizeof(StoredAlarmV1));
      for (uint8_t i = 0; i < stored_count; ++i) {
        stored[i] = {legacy[i].hour, legacy[i].minute, legacy[i].enabled, kEveryDayMask};
      }
    }
  }
  preferences.end();

  count_ = stored_count;
  for (uint8_t i = 0; i < count_; ++i) {
    alarms_[i] = {static_cast<uint8_t>(stored[i].hour % 24),
                  static_cast<uint8_t>(stored[i].minute % 60),
                  static_cast<uint8_t>(stored[i].weekday_mask & kEveryDayMask),
                  stored[i].enabled != 0,
                  false,
                  false};
  }
  last_checked_minute_ = -1;
  if (stored_version < kStorageVersion) persist();
#endif
}
uint8_t AlarmService::count() const { return count_; }
bool AlarmService::addAlarm(uint8_t h, uint8_t m) {
  if (count_ == kMaxAlarms) return false;
  alarms_[count_++] = {static_cast<uint8_t>(h % 24), static_cast<uint8_t>(m % 60), kEveryDayMask, false, false, false};
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
    if (!enabled) a.ringing = false;
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
      alarms_[i].enabled = false;
      alarms_[i].triggered_for_current_minute = true;
      ++dismissed;
    }
  if (dismissed) persist();
  return dismissed;
}
uint8_t AlarmService::hour(uint8_t i) const { return isValidIndex(i) ? alarm(i).hour : 0; }
uint8_t AlarmService::minute(uint8_t i) const { return isValidIndex(i) ? alarm(i).minute : 0; }
uint8_t AlarmService::weekdayMask(uint8_t i) const { return isValidIndex(i) ? alarm(i).weekday_mask : 0; }
void AlarmService::setAlarm(uint8_t h, uint8_t m) { setAlarm(0, h, m); }
void AlarmService::setEnabled(bool enabled) { setEnabled(0, enabled); }
void AlarmService::dismiss() { dismiss(0); }
bool AlarmService::isEnabled() const { return isEnabled(0); }
bool AlarmService::isRinging() const { return isRinging(0); }
uint8_t AlarmService::hour() const { return hour(0); }
uint8_t AlarmService::minute() const { return minute(0); }
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
      continue;
    }
    const uint8_t weekday_bit = now.weekday < 7 ? static_cast<uint8_t>(1u << now.weekday) : 0;
    if (!a.triggered_for_current_minute && now.hour == a.hour && now.minute == a.minute &&
        (a.weekday_mask & weekday_bit) != 0) {
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
#if defined(ARDUINO)
  Preferences preferences;
  if (!preferences.begin(kStorageNamespace, false)) return;
  preferences.putUChar(kCountKey, count_);
  if (count_ == 0)
    preferences.remove(kAlarmsKey);
  else {
    StoredAlarm stored[kMaxAlarms] = {};
    for (uint8_t i = 0; i < count_; ++i) {
      stored[i] = {alarms_[i].hour, alarms_[i].minute, static_cast<uint8_t>(alarms_[i].enabled),
                   alarms_[i].weekday_mask};
    }
    preferences.putBytes(kAlarmsKey, stored, count_ * sizeof(StoredAlarm));
  }
  preferences.putUChar(kVersionKey, kStorageVersion);
  preferences.end();
#endif
}
