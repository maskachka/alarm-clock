#pragma once

#include <lvgl.h>

#include "alarm_ringtone.h"

class AlarmRingtoneSettingsScreenListener {
 public:
  virtual ~AlarmRingtoneSettingsScreenListener() = default;
  virtual void onAlarmRingtoneSaved(AlarmRingtone ringtone) = 0;
  virtual void onAlarmRingtoneSettingsBackRequested() = 0;
  virtual void onAlarmRingtonePreviewStarted(AlarmRingtone ringtone) = 0;
  virtual void onAlarmRingtonePreviewStopped() = 0;
};

class AlarmRingtoneSettingsScreen {
 public:
  explicit AlarmRingtoneSettingsScreen(AlarmRingtoneSettingsScreenListener& listener);

  void build(lv_obj_t* parent);
  void show();
  void hide();
  void setSelection(AlarmRingtone ringtone);
  void setPreviewing(AlarmRingtone ringtone);

 private:
  struct RingtoneRow {
    AlarmRingtoneSettingsScreen* screen;
    AlarmRingtone ringtone;
    lv_obj_t* selector_label;
    lv_obj_t* preview_label;
  };

  static void onBack(lv_event_t* event);
  static void onSave(lv_event_t* event);
  static void onRingtoneSelected(lv_event_t* event);
  static void onPreviewPressed(lv_event_t* event);
  void setSelectedRow(AlarmRingtone ringtone);

  AlarmRingtoneSettingsScreenListener& listener_;
  lv_obj_t* root_;
  AlarmRingtone selected_ringtone_;
  AlarmRingtone previewing_ringtone_;
  RingtoneRow rows_[static_cast<uint8_t>(AlarmRingtone::Count)];
};
