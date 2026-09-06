#pragma once

#include <stdint.h>

#include "alarm_service.h"
#include "clock_time.h"

struct ClockAppState {
  char clock_text[6];
  char alarm_text[6];
  char primary_button_text[64];
  bool alarm_enabled;
  bool alarm_settings_visible;
};

struct ClockAppEffects {
  bool start_buzzer;
  bool stop_buzzer;
  bool update_buzzer;
  bool sync_alarm_settings_selection;
  uint8_t settings_hour;
  uint8_t settings_minute;
  uint8_t settings_weekday_mask;
};

class ClockAppController {
 public:
  explicit ClockAppController(AlarmService& alarm_service);

  void initialize();
  void selectAlarm(uint8_t index);
  ClockAppEffects openNewAlarmSettings();
  ClockAppEffects openAlarmSettings(uint8_t index);
  ClockAppEffects dismissAllRinging();
  ClockAppEffects refresh(bool has_time, const ClockTime& now);
  ClockAppEffects onPrimaryButtonPressed();
  ClockAppEffects onToggleAlarmPressed();
  ClockAppEffects onApplyAlarmPressed(uint8_t selected_hour, uint8_t selected_minute, uint8_t weekday_mask);
  ClockAppEffects onCancelAlarmPressed();

  const ClockAppState& state() const;

 private:
  static ClockAppEffects makeNoEffects();

  void setClockUnavailable();
  void setClockText(const ClockTime& time_value);
  void updateButtonLabels();
  ClockAppEffects makeAlarmSettingsSyncEffects() const;

  AlarmService& alarm_service_;
  ClockAppState state_;
  bool buzzer_active_;
  uint8_t active_alarm_index_;
};
