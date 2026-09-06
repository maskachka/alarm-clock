#pragma once

#include <lvgl.h>

#include "alarm_service.h"

class AlarmListScreenListener {
 public:
  virtual ~AlarmListScreenListener() = default;
  virtual void onAlarmAddRequested() = 0;
  virtual void onDismissRequested() = 0;
  virtual void onAlarmDeleteRequested(uint8_t index) = 0;
  virtual void onAlarmSelected(uint8_t index) = 0;
  virtual void onAlarmToggled(uint8_t index) = 0;
  virtual void onAlarmListBackRequested() = 0;
};

class AlarmListScreen {
 public:
  explicit AlarmListScreen(AlarmListScreenListener& listener);
  void build(lv_obj_t* parent);
  void render(const AlarmService& alarms);
  void show();
  void hide();

 private:
  struct Row {
    AlarmListScreen* screen;
    uint8_t index;
    lv_obj_t* root;
    lv_obj_t* time;
    lv_obj_t* enabled_switch;
  };

  static void onAdd(lv_event_t* event);
  static void onDismiss(lv_event_t* event);
  static void onDelete(lv_event_t* event);
  static void onSet(lv_event_t* event);
  static void onToggle(lv_event_t* event);
  static void onBack(lv_event_t* event);

  AlarmListScreenListener& listener_;
  lv_obj_t* root_;
  lv_obj_t* dismiss_row_;
  lv_obj_t* dismiss_button_;
  Row rows_[AlarmService::kMaxAlarms];
};
