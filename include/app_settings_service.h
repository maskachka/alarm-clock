#pragma once

#include <stdint.h>

#include "app_settings_storage.h"

class AppSettingsService {
 public:
  static constexpr uint8_t kMinAlarmVolume = 1;
  static constexpr uint8_t kMaxAlarmVolume = 100;
  static constexpr uint8_t kDefaultAlarmVolume = kMinAlarmVolume;

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
