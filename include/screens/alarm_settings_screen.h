#pragma once

#include <stdint.h>

#include <lvgl.h>

class AlarmSettingsScreenListener {
 public:
  virtual ~AlarmSettingsScreenListener() = default;

  virtual void onAlarmSettingsApplied(uint8_t hour, uint8_t minute, uint8_t weekday_mask) = 0;
  virtual void onAlarmSettingsCancelled() = 0;
};

class AlarmSettingsScreen {
 public:
  explicit AlarmSettingsScreen(AlarmSettingsScreenListener& listener);

  void build(lv_obj_t* parent);
  void show();
  void hide();
  void setSelection(uint8_t hour, uint8_t minute, uint8_t weekday_mask);

 private:
  static void onApplyButtonPressed(lv_event_t* event);
  static void onCancelButtonPressed(lv_event_t* event);
  static void onEveryDayChanged(lv_event_t* event);
  static void onWeekdayPressed(lv_event_t* event);

  uint8_t selectedWeekdayMask() const;
  void setWeekdaySelected(uint8_t weekday, bool selected);

  AlarmSettingsScreenListener& listener_;
  lv_obj_t* root_;
  lv_obj_t* hour_roller_;
  lv_obj_t* minute_roller_;
  lv_obj_t* every_day_switch_;
  lv_obj_t* weekday_buttons_[7];
};
