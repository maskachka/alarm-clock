#pragma once
#include <lvgl.h>
#include "clock_app_controller.h"
class ClockScreenListener {
 public:
  virtual ~ClockScreenListener() = default;
  virtual void onSettingsRequested() = 0;
};
class ClockScreen {
 public:
  explicit ClockScreen(ClockScreenListener& listener);
  void build(lv_obj_t* parent);
  void render(const ClockAppState& state);
  void show();
  void hide();

 private:
  static void onSettingsPressed(lv_event_t* event);
  void setClockText(const char* text);
  void setDateText(const char* text);
  void setNextAlarmText(const char* text);
  ClockScreenListener& listener_;
  lv_obj_t* root_;
  lv_obj_t* date_;
  lv_obj_t* next_alarm_;
  lv_obj_t* digits_[5];
};
