#include "alarm_service.h"

AlarmService::AlarmService() : alarms_{}, count_(1), last_checked_minute_(-1) {
  alarms_[0] = {7, 0, false, false, false};
}
uint8_t AlarmService::count() const { return count_; }
void AlarmService::setAlarm(uint8_t i, uint8_t h, uint8_t m) {
  if (isValidIndex(i)) {
    Alarm& a = alarm(i);
    a.hour = h % 24;
    a.minute = m % 60;
  }
}
void AlarmService::setEnabled(uint8_t i, bool enabled) {
  if (isValidIndex(i)) {
    Alarm& a = alarm(i);
    a.enabled = enabled;
    if (!enabled) a.ringing = false;
  }
}
void AlarmService::dismiss(uint8_t i) {
  if (isValidIndex(i)) {
    Alarm& a = alarm(i);
    a.ringing = false;
    a.triggered_for_current_minute = true;
  }
}
bool AlarmService::isEnabled(uint8_t i) const { return isValidIndex(i) && alarm(i).enabled; }
bool AlarmService::isRinging(uint8_t i) const { return isValidIndex(i) && alarm(i).ringing; }
bool AlarmService::hasRingingAlarm() const {
  for (uint8_t i = 0; i < count_; ++i)
    if (alarms_[i].ringing) return true;
  return false;
}
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
