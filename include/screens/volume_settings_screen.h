#pragma once

#include <lvgl.h>

class VolumeSettingsScreenListener {
 public:
  virtual ~VolumeSettingsScreenListener() = default;
  virtual void onVolumeSaved(uint8_t volume) = 0;
  virtual void onVolumeSettingsBackRequested() = 0;
};

class VolumeSettingsScreen {
 public:
  explicit VolumeSettingsScreen(VolumeSettingsScreenListener& listener);
  void build(lv_obj_t* parent);
  void setVolume(uint8_t volume);
  void show();
  void hide();

 private:
  static void onSave(lv_event_t* event);
  static void onBack(lv_event_t* event);
  static void onSliderChanged(lv_event_t* event);
  void updateLabel();
  VolumeSettingsScreenListener& listener_;
  lv_obj_t* root_;
  lv_obj_t* slider_;
  lv_obj_t* value_;
};
