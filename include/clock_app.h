#pragma once

#include <lvgl.h>

#include "alarm_buzzer.h"
#include "alarm_service.h"
#include "clock_service.h"

class ClockApp {
public:
  ClockApp(ClockService &clock_service, AlarmService &alarm_service, AlarmBuzzer &alarm_buzzer);

  void build();

private:
  static void onPrimaryButtonPressed(lv_event_t *event);
  static void onApplyAlarmPressed(lv_event_t *event);
  static void onDisableAlarmPressed(lv_event_t *event);
  static void onCancelAlarmPressed(lv_event_t *event);
  static void onRefreshTimer(lv_timer_t *timer);

  void refresh();
  void showAlarmEditor();
  void hideAlarmEditor();
  void syncEditorToAlarm();
  void updateAlarmButtonLabel();
  void stopAlarm();
  void applyAlarmFromEditor();

  ClockService &clock_service_;
  AlarmService &alarm_service_;
  AlarmBuzzer &alarm_buzzer_;
  lv_obj_t *clock_label_;
  lv_obj_t *primary_button_label_;
  lv_obj_t *alarm_editor_overlay_;
  lv_obj_t *alarm_hour_roller_;
  lv_obj_t *alarm_minute_roller_;
  lv_timer_t *refresh_timer_;
  bool buzzer_active_;
};
