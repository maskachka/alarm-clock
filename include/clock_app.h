#pragma once

#include <lvgl.h>

#include "alarm_buzzer.h"
#include "clock_app_controller.h"
#include "clock_service.h"
#include "app_settings_service.h"
#include "screens/alarm_settings_screen.h"
#include "screens/alarm_delete_confirmation_view.h"
#include "screens/alarm_list_screen.h"
#include "screens/alarm_ringing_overlay.h"
#include "screens/alarm_ringtone_settings_screen.h"
#include "screens/clock_screen.h"
#include "screens/settings_screen.h"
#include "screens/volume_settings_screen.h"

class ClockApp : private ClockScreenListener,
                 private AlarmSettingsScreenListener,
                 private AlarmListScreenListener,
                 private ConfirmationDialogListener,
                 private AlarmRingingOverlayListener,
                 private AlarmRingtoneSettingsScreenListener,
                 private SettingsScreenListener,
                 private VolumeSettingsScreenListener {
 public:
  ClockApp(ClockService& clock_service, AlarmService& alarm_service, AlarmBuzzer& alarm_buzzer,
           AppSettingsService& app_settings);

  void build();

 private:
  static void onClockTimer(lv_timer_t* timer);
  static void onBuzzerTimer(lv_timer_t* timer);

  void onSettingsRequested() override;
  void onAlarmAddRequested() override;
  void onAlarmDeleteRequested(uint8_t index) override;
  void onConfirmationConfirmed(ConfirmationAction action, uint8_t index) override;
  void onConfirmationCancelled() override;
  void onAlarmSettingsApplied(uint8_t hour, uint8_t minute, uint8_t weekday_mask) override;
  void onAlarmSettingsCancelled() override;
  void onAlarmSettingsBackRequested() override;
  void onAlarmSelected(uint8_t index) override;
  void onAlarmRingtoneSettingsRequested(uint8_t index) override;
  void onAlarmToggled(uint8_t index) override;
  void onAlarmListBackRequested() override;
  void onAlarmDismissRequested() override;
  void onAlarmSnoozeRequested() override;
  void onAlarmRingtoneSaved(AlarmRingtone ringtone) override;
  void onAlarmRingtoneSettingsBackRequested() override;
  void onAlarmRingtonePreviewStarted(AlarmRingtone ringtone) override;
  void onAlarmRingtonePreviewStopped() override;
  void onAlarmsSettingsRequested() override;
  void onVolumeSettingsRequested() override;
  void onSettingsBackRequested() override;
  void onVolumeSaved(uint8_t volume) override;
  void onVolumeSettingsBackRequested() override;

  void pollClockAndAlarms();
  void updateBuzzer();
  void applyControllerState();
  void applyControllerEffects(const ClockAppEffects& effects);
  void stopRingtonePreview();
  bool renderedStateMatchesCurrent() const;
  bool renderedAlarmsMatchCurrent() const;
  void rememberRenderedState();

  struct RenderedAlarm {
    uint8_t hour;
    uint8_t minute;
    bool enabled;
  };

  ClockService& clock_service_;
  AlarmService& alarm_service_;
  AlarmBuzzer& alarm_buzzer_;
  AppSettingsService& app_settings_;
  ClockAppController controller_;
  ClockScreen clock_screen_;
  AlarmSettingsScreen alarm_settings_screen_;
  ConfirmationDialogView confirmation_dialog_view_;
  AlarmRingingOverlay alarm_ringing_overlay_;
  AlarmRingtoneSettingsScreen alarm_ringtone_settings_screen_;
  AlarmListScreen alarm_list_screen_;
  SettingsScreen settings_screen_;
  VolumeSettingsScreen volume_settings_screen_;
  lv_timer_t* clock_timer_;
  lv_timer_t* buzzer_timer_;
  ClockAppState rendered_state_;
  RenderedAlarm rendered_alarms_[AlarmService::kMaxAlarms];
  uint8_t rendered_alarm_count_;
  bool has_rendered_state_;
  bool rendered_ringing_overlay_visible_;
  bool buzzer_active_;
  bool ringtone_preview_active_;
  bool creating_alarm_;
};
