#pragma once

#include <stdint.h>

#include "alarm_service.h"
#include "clock_time.h"

struct ClockAppState {
  char clock_text[6];
  char alarm_text[6];
  char primary_button_text[64];
  bool alarm_enabled;
  bool editor_visible;
};

struct ClockAppEffects {
  bool start_buzzer;
  bool stop_buzzer;
  bool update_buzzer;
  bool sync_editor_selection;
  uint8_t editor_hour;
  uint8_t editor_minute;
};

class ClockAppController {
public:
  explicit ClockAppController(AlarmService &alarm_service);

  void initialize();
  ClockAppEffects refresh(bool has_time, const ClockTime &now);
  ClockAppEffects onPrimaryButtonPressed();
  ClockAppEffects onToggleAlarmPressed();
  ClockAppEffects onApplyAlarmPressed(uint8_t selected_hour, uint8_t selected_minute);
  ClockAppEffects onCancelAlarmPressed();

  const ClockAppState &state() const;

private:
  static ClockAppEffects makeNoEffects();

  void setClockUnavailable();
  void setClockText(const ClockTime &time_value);
  void updateButtonLabels();
  ClockAppEffects makeEditorSyncEffects() const;

  AlarmService &alarm_service_;
  ClockAppState state_;
  bool buzzer_active_;
};
