#include <unity.h>

#include "app_settings_service.h"

void testAppSettingsDefaultsToOnePercentVolume() {
  AppSettingsService settings;
  TEST_ASSERT_EQUAL_UINT8(1, settings.alarmVolume());
}

void testAppSettingsRestoresPersistedVolume() {
  InMemoryAppSettingsStorage storage;
  AppSettingsService settings(storage);
  settings.setAlarmVolume(35);

  AppSettingsService restored(storage);
  restored.load();

  TEST_ASSERT_EQUAL_UINT8(35, restored.alarmVolume());
}

void testAppSettingsClampsVolumeToOneHundredPercent() {
  AppSettingsService settings;
  settings.setAlarmVolume(120);
  TEST_ASSERT_EQUAL_UINT8(100, settings.alarmVolume());
}

void testAppSettingsClampsVolumeToOnePercent() {
  AppSettingsService settings;
  settings.setAlarmVolume(0);
  TEST_ASSERT_EQUAL_UINT8(1, settings.alarmVolume());
}
