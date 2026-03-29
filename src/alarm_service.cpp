#include "alarm_service.h"

AlarmService::AlarmService()
    : hour_(7), minute_(0), last_checked_minute_(-1), enabled_(false), ringing_(false),
      triggered_for_current_minute_(false) {}

void AlarmService::setAlarm(uint8_t hour, uint8_t minute) {
  hour_ = static_cast<uint8_t>(hour % 24);
  minute_ = static_cast<uint8_t>(minute % 60);
}

void AlarmService::setEnabled(bool enabled) {
  enabled_ = enabled;
  if (!enabled_) {
    ringing_ = false;
  }
}

void AlarmService::dismiss() {
  ringing_ = false;
  triggered_for_current_minute_ = true;
}

bool AlarmService::isEnabled() const {
  return enabled_;
}

bool AlarmService::isRinging() const {
  return ringing_;
}

uint8_t AlarmService::hour() const {
  return hour_;
}

uint8_t AlarmService::minute() const {
  return minute_;
}

bool AlarmService::update(const ClockTime &now) {
  const int16_t current_minute = static_cast<int16_t>(now.hour) * 60 + now.minute;
  if (current_minute != last_checked_minute_) {
    last_checked_minute_ = current_minute;
    triggered_for_current_minute_ = false;
  }

  if (!enabled_) {
    ringing_ = false;
    return false;
  }

  if (!triggered_for_current_minute_ && now.hour == hour_ && now.minute == minute_) {
    ringing_ = true;
    triggered_for_current_minute_ = true;
    return true;
  }

  return false;
}
