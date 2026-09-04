#include "clock_app_controller.h"

#include <stdio.h>
#include <string.h>

#include "clock_formatter.h"

ClockAppController::ClockAppController(AlarmService &alarm_service) : alarm_service_(alarm_service), buzzer_active_(false) {
  state_.clock_text[0] = '\0';
  state_.alarm_text[0] = '\0';
  state_.primary_button_text[0] = '\0';
  state_.alarm_enabled = false;
  state_.editor_visible = false;
}

void ClockAppController::initialize() {
  setClockUnavailable();
  state_.editor_visible = false;
  buzzer_active_ = false;
  updateButtonLabels();
}

ClockAppEffects ClockAppController::refresh(bool has_time, const ClockTime &now) {
  ClockAppEffects effects = makeNoEffects();

  if (!has_time) {
    setClockUnavailable();
    return effects;
  }

  setClockText(now);

  if (alarm_service_.update(now) && !buzzer_active_) {
    effects.start_buzzer = true;
    buzzer_active_ = true;
  }

  if (alarm_service_.isRinging()) {
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

  if (alarm_service_.isRinging()) {
    if (buzzer_active_) {
      effects.stop_buzzer = true;
      buzzer_active_ = false;
    }
    alarm_service_.dismiss();
    updateButtonLabels();
    return effects;
  }

  state_.editor_visible = true;
  return makeEditorSyncEffects();
}

ClockAppEffects ClockAppController::onToggleAlarmPressed() {
  ClockAppEffects effects = makeNoEffects();

  if (alarm_service_.isEnabled()) {
    alarm_service_.setEnabled(false);
    if (buzzer_active_) {
      effects.stop_buzzer = true;
      buzzer_active_ = false;
    }
  } else {
    alarm_service_.setEnabled(true);
  }

  updateButtonLabels();
  return effects;
}

ClockAppEffects ClockAppController::onApplyAlarmPressed(uint8_t selected_hour, uint8_t selected_minute) {
  alarm_service_.setAlarm(selected_hour, selected_minute);
  state_.editor_visible = false;
  updateButtonLabels();
  return makeNoEffects();
}

ClockAppEffects ClockAppController::onCancelAlarmPressed() {
  state_.editor_visible = false;
  return makeNoEffects();
}

const ClockAppState &ClockAppController::state() const {
  return state_;
}

ClockAppEffects ClockAppController::makeNoEffects() {
  ClockAppEffects effects = {};
  effects.start_buzzer = false;
  effects.stop_buzzer = false;
  effects.update_buzzer = false;
  effects.sync_editor_selection = false;
  effects.editor_hour = 0;
  effects.editor_minute = 0;
  return effects;
}

void ClockAppController::setClockUnavailable() {
  strncpy(state_.clock_text, "--:--", sizeof(state_.clock_text));
  state_.clock_text[sizeof(state_.clock_text) - 1] = '\0';
}

void ClockAppController::setClockText(const ClockTime &time_value) {
  ClockFormatter::formatHHMM(time_value, state_.clock_text, sizeof(state_.clock_text));
}

void ClockAppController::updateButtonLabels() {
  const ClockTime alarm_time = {alarm_service_.hour(), alarm_service_.minute(), 0};
  ClockFormatter::formatHHMM(alarm_time, state_.alarm_text, sizeof(state_.alarm_text));

  if (alarm_service_.isRinging()) {
    snprintf(state_.primary_button_text, sizeof(state_.primary_button_text), "Dismiss");
  } else {
    snprintf(state_.primary_button_text, sizeof(state_.primary_button_text), "Set");
  }

  state_.alarm_enabled = alarm_service_.isEnabled();
}

ClockAppEffects ClockAppController::makeEditorSyncEffects() const {
  ClockAppEffects effects = makeNoEffects();
  effects.sync_editor_selection = true;
  effects.editor_hour = alarm_service_.hour();
  effects.editor_minute = alarm_service_.minute();
  return effects;
}
