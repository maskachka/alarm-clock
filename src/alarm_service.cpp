#include "alarm_service.h"

#if defined(ARDUINO)
#include <Preferences.h>
#endif

namespace {
struct StoredAlarm { uint8_t hour; uint8_t minute; uint8_t enabled; };
constexpr const char* kStorageNamespace = "alarm-clock";
constexpr const char* kCountKey = "count";
constexpr const char* kAlarmsKey = "alarms";
}

AlarmService::AlarmService() : alarms_{}, count_(1), last_checked_minute_(-1) {
  alarms_[0] = {7, 0, false, false, false};
}
void AlarmService::load() {
#if defined(ARDUINO)
  Preferences preferences;
  if (!preferences.begin(kStorageNamespace, true) || !preferences.isKey(kCountKey)) { preferences.end(); return; }
  const uint8_t stored_count = preferences.getUChar(kCountKey, 0);
  if (stored_count > kMaxAlarms || (stored_count > 0 && preferences.getBytesLength(kAlarmsKey) != stored_count * sizeof(StoredAlarm))) { preferences.end(); return; }
  StoredAlarm stored[kMaxAlarms] = {};
  if (stored_count > 0) preferences.getBytes(kAlarmsKey, stored, stored_count * sizeof(StoredAlarm));
  preferences.end();
  count_ = stored_count;
  for (uint8_t i = 0; i < count_; ++i) alarms_[i] = {static_cast<uint8_t>(stored[i].hour % 24), static_cast<uint8_t>(stored[i].minute % 60), stored[i].enabled != 0, false, false};
  last_checked_minute_ = -1;
#endif
}
uint8_t AlarmService::count() const { return count_; }
bool AlarmService::addAlarm(uint8_t h, uint8_t m) {
  if (count_ == kMaxAlarms) return false;
  alarms_[count_++] = {static_cast<uint8_t>(h % 24), static_cast<uint8_t>(m % 60), false, false, false};
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
uint8_t AlarmService::dismissAllRinging() { uint8_t dismissed=0; for(uint8_t i=0;i<count_;++i) if(alarms_[i].ringing){alarms_[i].ringing=false;alarms_[i].enabled=false;alarms_[i].triggered_for_current_minute=true;++dismissed;} if(dismissed) persist(); return dismissed; }
uint8_t AlarmService::hour(uint8_t i) const { return isValidIndex(i) ? alarm(i).hour : 0; }
uint8_t AlarmService::minute(uint8_t i) const { return isValidIndex(i) ? alarm(i).minute : 0; }
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
    if (!a.triggered_for_current_minute && now.hour == a.hour && now.minute == a.minute) {
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
  if (count_ == 0) preferences.remove(kAlarmsKey);
  else { StoredAlarm stored[kMaxAlarms] = {}; for (uint8_t i = 0; i < count_; ++i) stored[i] = {alarms_[i].hour, alarms_[i].minute, static_cast<uint8_t>(alarms_[i].enabled)}; preferences.putBytes(kAlarmsKey, stored, count_ * sizeof(StoredAlarm)); }
  preferences.end();
#endif
}
