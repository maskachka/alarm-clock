#pragma once

#include <lvgl.h>

class SettingsScreenListener {
 public:
  virtual ~SettingsScreenListener() = default;
  virtual void onAlarmsSettingsRequested() = 0;
  virtual void onVolumeSettingsRequested() = 0;
  virtual void onSettingsBackRequested() = 0;
};

class SettingsScreen {
 public:
  explicit SettingsScreen(SettingsScreenListener& listener);
  void build(lv_obj_t* parent);
  void show();
  void hide();

 private:
  static void onAlarms(lv_event_t* event);
  static void onVolume(lv_event_t* event);
  static void onBack(lv_event_t* event);
  SettingsScreenListener& listener_;
  lv_obj_t* root_;
};
