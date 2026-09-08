#pragma once

#include <stdint.h>

#include "app_settings_storage.h"

class AppSettingsService {
 public:
  static constexpr uint8_t kDefaultAlarmVolume = 70;
  static constexpr uint8_t kMaxAlarmVolume = 100;

  AppSettingsService();
  explicit AppSettingsService(AppSettingsStorage& storage);
  void load();
  uint8_t alarmVolume() const;
  void setAlarmVolume(uint8_t volume);

 private:
  void persist() const;
  AppSettingsStorage* storage_;
  uint8_t alarm_volume_;
};
