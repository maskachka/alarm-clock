#pragma once

#include <lvgl.h>

#include "clock_app_controller.h"

class ClockScreenListener
{
public:
  virtual ~ClockScreenListener() = default;

  virtual void onClockScreenPrimaryAction() = 0;
  virtual void onClockScreenAlarmToggled() = 0;
};

class ClockScreen
{
public:
  explicit ClockScreen(ClockScreenListener &listener);

  void build(lv_obj_t *parent);
  void render(const ClockAppState &state);

private:
  static void onPrimaryButtonPressed(lv_event_t *event);
  static void onToggleAlarmPressed(lv_event_t *event);

  void setClockText(const char *text);

  ClockScreenListener &listener_;
  lv_obj_t *clock_character_labels_[5];
  lv_obj_t *primary_button_label_;
  lv_obj_t *alarm_time_label_;
  lv_obj_t *alarm_checkbox_;
};
