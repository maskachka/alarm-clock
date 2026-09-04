#pragma once

#include <lvgl.h>

#include "alarm_buzzer.h"
#include "clock_app_controller.h"
#include "alarm_service.h"
#include "clock_service.h"

class ClockApp {
public:
  ClockApp(ClockService &clock_service, AlarmService &alarm_service, AlarmBuzzer &alarm_buzzer);

  void build();

private:
  static void onPrimaryButtonPressed(lv_event_t *event);
  static void onToggleAlarmPressed(lv_event_t *event);
  static void onApplyAlarmPressed(lv_event_t *event);
  static void onCancelAlarmPressed(lv_event_t *event);
  static void onRefreshTimer(lv_timer_t *timer);

  void refresh();
  void applyControllerState();
  void applyControllerEffects(const ClockAppEffects &effects);
  void applyAlarmFromEditor();
  void setClockDisplayText(const char *text);

  ClockService &clock_service_;
  AlarmBuzzer &alarm_buzzer_;
  ClockAppController controller_;
  lv_obj_t *clock_character_labels_[5];
  lv_obj_t *primary_button_;
  lv_obj_t *primary_button_label_;
  lv_obj_t *alarm_time_label_;
  lv_obj_t *alarm_checkbox_;
  lv_obj_t *alarm_editor_overlay_;
  lv_obj_t *alarm_hour_roller_;
  lv_obj_t *alarm_minute_roller_;
  lv_timer_t *refresh_timer_;
  bool buzzer_active_;
};
