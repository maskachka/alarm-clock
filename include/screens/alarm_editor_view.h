#pragma once

#include <stdint.h>

#include <lvgl.h>

class AlarmEditorViewListener {
 public:
  virtual ~AlarmEditorViewListener() = default;

  virtual void onAlarmEditorApplied(uint8_t hour, uint8_t minute) = 0;
  virtual void onAlarmEditorCancelled() = 0;
};

class AlarmEditorView {
 public:
  explicit AlarmEditorView(AlarmEditorViewListener& listener);

  void build(lv_obj_t* parent);
  void show();
  void hide();
  void setSelection(uint8_t hour, uint8_t minute);

 private:
  static void onApplyButtonPressed(lv_event_t* event);
  static void onCancelButtonPressed(lv_event_t* event);

  AlarmEditorViewListener& listener_;
  lv_obj_t* overlay_;
  lv_obj_t* hour_roller_;
  lv_obj_t* minute_roller_;
};
