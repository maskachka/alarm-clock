#pragma once

#include <lvgl.h>

class AlarmRingingOverlayListener {
 public:
  virtual ~AlarmRingingOverlayListener() = default;

  virtual void onAlarmDismissRequested() = 0;
  virtual void onAlarmSnoozeRequested() = 0;
};

class AlarmRingingOverlay {
 public:
  explicit AlarmRingingOverlay(AlarmRingingOverlayListener& listener);

  void build(lv_obj_t* parent);
  void show();
  void hide();

 private:
  static void onDismiss(lv_event_t* event);
  static void onSnooze(lv_event_t* event);

  AlarmRingingOverlayListener& listener_;
  lv_obj_t* root_;
};
