#include "clock_app_controller.h"

#include <stdio.h>
#include <string.h>

#include "clock_formatter.h"

ClockAppController::ClockAppController(AlarmService& alarm_service)
    : alarm_service_(alarm_service),
      buzzer_active_(false),
      current_time_{},
      has_current_time_(false),
      active_alarm_index_(0) {
  state_.clock_text[0] = '\0';
  state_.date_text[0] = '\0';
  state_.next_alarm_text[0] = '\0';
  state_.alarm_text[0] = '\0';
  state_.primary_button_text[0] = '\0';
  state_.alarm_enabled = false;
  state_.alarm_settings_visible = false;
  state_.ringtone_settings_visible = false;
}

void ClockAppController::selectAlarm(uint8_t index) {
  if (index < alarm_service_.count()) active_alarm_index_ = index;
  updateButtonLabels();
}

ClockAppEffects ClockAppController::openNewAlarmSettings() {
  ClockAppEffects effects = makeNoEffects();
  state_.alarm_settings_visible = true;
  state_.ringtone_settings_visible = false;
  effects.sync_alarm_settings_selection = true;
  effects.settings_hour = 7;
  effects.settings_minute = 0;
  effects.settings_weekday_mask = AlarmService::kEveryDayMask;
  return effects;
}
ClockAppEffects ClockAppController::openAlarmSettings(uint8_t index) {
  selectAlarm(index);
  state_.alarm_settings_visible = true;
  state_.ringtone_settings_visible = false;
  return makeAlarmSettingsSyncEffects();
}
ClockAppEffects ClockAppController::openAlarmRingtoneSettings(uint8_t index) {
  selectAlarm(index);
  state_.alarm_settings_visible = false;
  state_.ringtone_settings_visible = true;
  return makeNoEffects();
}
ClockAppEffects ClockAppController::dismissAllRinging() {
  ClockAppEffects effects = makeNoEffects();
  if (alarm_service_.dismissAllRinging() > 0 && buzzer_active_) {
    effects.stop_buzzer = true;
    buzzer_active_ = false;
  }
  if (has_current_time_) setNextAlarmText(current_time_);
  updateButtonLabels();
  return effects;
}
ClockAppEffects ClockAppController::snoozeAllRinging() {
  ClockAppEffects effects = makeNoEffects();
  if (has_current_time_ && alarm_service_.snoozeAllRinging(current_time_) > 0 && buzzer_active_) {
    effects.stop_buzzer = true;
    buzzer_active_ = false;
  }
  if (has_current_time_) setNextAlarmText(current_time_);
  updateButtonLabels();
  return effects;
}

void ClockAppController::initialize() {
  setClockUnavailable();
  state_.alarm_settings_visible = false;
  state_.ringtone_settings_visible = false;
  buzzer_active_ = false;
  has_current_time_ = false;
  updateButtonLabels();
}

ClockAppEffects ClockAppController::refresh(bool has_time, const ClockTime& now) {
  ClockAppEffects effects = makeNoEffects();

  if (!has_time) {
    has_current_time_ = false;
    setClockUnavailable();
    return effects;
  }

  current_time_ = now;
  has_current_time_ = true;

  setClockText(now);
  setNextAlarmText(now);

  if (alarm_service_.update(now)) {
    effects.start_buzzer = true;
    buzzer_active_ = true;
  }

  if (alarm_service_.hasRingingAlarm()) {
    if (!buzzer_active_) {
      effects.start_buzzer = true;
      buzzer_active_ = true;
    }
    effects.update_buzzer = true;
  } else if (buzzer_active_) {
    effects.stop_buzzer = true;
    buzzer_active_ = false;
  }

  updateButtonLabels();
  return effects;
}

ClockAppEffects ClockAppController::onPrimaryButtonPressed() {
  ClockAppEffects effects = makeNoEffects();

  if (alarm_service_.hasRingingAlarm()) {
    return dismissAllRinging();
  }

  state_.alarm_settings_visible = true;
  return makeAlarmSettingsSyncEffects();
}

ClockAppEffects ClockAppController::onToggleAlarmPressed() {
  ClockAppEffects effects = makeNoEffects();

  if (alarm_service_.isEnabled(active_alarm_index_)) {
    alarm_service_.setEnabled(active_alarm_index_, false);
    if (buzzer_active_ && !alarm_service_.hasRingingAlarm()) {
      effects.stop_buzzer = true;
      buzzer_active_ = false;
    }
  } else {
    alarm_service_.setEnabled(active_alarm_index_, true);
  }

  updateButtonLabels();
  return effects;
}

ClockAppEffects ClockAppController::onApplyAlarmPressed(uint8_t selected_hour, uint8_t selected_minute,
                                                        uint8_t weekday_mask) {
  alarm_service_.setAlarm(active_alarm_index_, selected_hour, selected_minute);
  alarm_service_.setWeekdayMask(active_alarm_index_, weekday_mask);
  state_.alarm_settings_visible = false;
  updateButtonLabels();
  return makeNoEffects();
}

ClockAppEffects ClockAppController::onCancelAlarmPressed() {
  state_.alarm_settings_visible = false;
  return makeNoEffects();
}
ClockAppEffects ClockAppController::onSaveAlarmRingtone(AlarmRingtone ringtone) {
  alarm_service_.setRingtone(active_alarm_index_, ringtone);
  state_.ringtone_settings_visible = false;
  return makeNoEffects();
}

ClockAppEffects ClockAppController::onCancelAlarmRingtoneSettings() {
  state_.ringtone_settings_visible = false;
  return makeNoEffects();
}

const ClockAppState& ClockAppController::state() const { return state_; }

ClockAppEffects ClockAppController::makeNoEffects() {
  ClockAppEffects effects = {};
  effects.start_buzzer = false;
  effects.stop_buzzer = false;
  effects.update_buzzer = false;
  effects.sync_alarm_settings_selection = false;
  effects.settings_hour = 0;
  effects.settings_minute = 0;
  effects.settings_weekday_mask = AlarmService::kEveryDayMask;
  return effects;
}

void ClockAppController::setClockUnavailable() {
  strncpy(state_.clock_text, "--:--", sizeof(state_.clock_text));
  state_.clock_text[sizeof(state_.clock_text) - 1] = '\0';
  state_.date_text[0] = '\0';
  snprintf(state_.next_alarm_text, sizeof(state_.next_alarm_text), "No active alarms");
}

void ClockAppController::setClockText(const ClockTime& time_value) {
  ClockFormatter::formatHHMM(time_value, state_.clock_text, sizeof(state_.clock_text));
  ClockFormatter::formatShortDate(time_value, state_.date_text, sizeof(state_.date_text));
}

void ClockAppController::setNextAlarmText(const ClockTime& time_value) {
  AlarmService::NextOccurrence next = {};
  if (alarm_service_.nextSnoozeOccurrence(time_value, next)) {
    snprintf(state_.next_alarm_text, sizeof(state_.next_alarm_text), "Snoozing for %um...",
             static_cast<unsigned>(next.minutes_until));
    return;
  }
  if (!alarm_service_.nextOccurrence(time_value, next)) {
    snprintf(state_.next_alarm_text, sizeof(state_.next_alarm_text), "No active alarms");
    return;
  }

  const uint8_t duration_days = static_cast<uint8_t>(next.minutes_until / (24 * 60));
  const uint8_t hours = static_cast<uint8_t>((next.minutes_until % (24 * 60)) / 60);
  const uint8_t minutes = static_cast<uint8_t>(next.minutes_until % 60);
  char duration[16];
  if (duration_days == 0) {
    if (hours == 0)
      snprintf(duration, sizeof(duration), "%um", static_cast<unsigned>(minutes));
    else
      snprintf(duration, sizeof(duration), "%uh %um", static_cast<unsigned>(hours), static_cast<unsigned>(minutes));
  } else {
    snprintf(duration, sizeof(duration), "%ud %uh %um", static_cast<unsigned>(duration_days),
             static_cast<unsigned>(hours), static_cast<unsigned>(minutes));
  }

  if (next.days_from_now == 0) {
    snprintf(state_.next_alarm_text, sizeof(state_.next_alarm_text), "Next alarm in %s, at %02u:%02u", duration,
             static_cast<unsigned>(next.hour), static_cast<unsigned>(next.minute));
  } else if (next.days_from_now == 1) {
    snprintf(state_.next_alarm_text, sizeof(state_.next_alarm_text), "Next alarm tomorrow in %s, at %02u:%02u",
             duration, static_cast<unsigned>(next.hour), static_cast<unsigned>(next.minute));
  } else {
    static constexpr const char* kWeekdayNames[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
    snprintf(state_.next_alarm_text, sizeof(state_.next_alarm_text), "Next alarm %s in %s, at %02u:%02u",
             kWeekdayNames[next.weekday], duration, static_cast<unsigned>(next.hour),
             static_cast<unsigned>(next.minute));
  }
}

void ClockAppController::updateButtonLabels() {
  const ClockTime alarm_time = {alarm_service_.hour(active_alarm_index_), alarm_service_.minute(active_alarm_index_),
                                0};
  ClockFormatter::formatHHMM(alarm_time, state_.alarm_text, sizeof(state_.alarm_text));

  if (alarm_service_.isRinging(active_alarm_index_)) {
    snprintf(state_.primary_button_text, sizeof(state_.primary_button_text), "Dismiss");
  } else {
    snprintf(state_.primary_button_text, sizeof(state_.primary_button_text), "Set");
  }

  state_.alarm_enabled = alarm_service_.isEnabled(active_alarm_index_);
}

ClockAppEffects ClockAppController::makeAlarmSettingsSyncEffects() const {
  ClockAppEffects effects = makeNoEffects();
  effects.sync_alarm_settings_selection = true;
  effects.settings_hour = alarm_service_.hour(active_alarm_index_);
  effects.settings_minute = alarm_service_.minute(active_alarm_index_);
  effects.settings_weekday_mask = alarm_service_.weekdayMask(active_alarm_index_);
  return effects;
}
