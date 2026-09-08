#include "app_settings_storage.h"

#if defined(ARDUINO)
#include <Preferences.h>
#endif

InMemoryAppSettingsStorage::InMemoryAppSettingsStorage() : settings_{}, has_saved_settings_(false) {}
bool InMemoryAppSettingsStorage::load(StoredAppSettings& settings) {
  if (!has_saved_settings_) return false;
  settings = settings_;
  return true;
}
bool InMemoryAppSettingsStorage::save(const StoredAppSettings& settings) {
  settings_ = settings;
  has_saved_settings_ = true;
  return true;
}

#if defined(ARDUINO)
namespace {
constexpr const char* kStorageNamespace = "alarm-clock";
constexpr const char* kAlarmVolumeKey = "alarm-volume";
}  // namespace
bool PreferencesAppSettingsStorage::load(StoredAppSettings& settings) {
  Preferences preferences;
  if (!preferences.begin(kStorageNamespace, true)) return false;
  if (!preferences.isKey(kAlarmVolumeKey)) {
    preferences.end();
    return false;
  }
  settings.alarm_volume = preferences.getUChar(kAlarmVolumeKey, 0);
  preferences.end();
  return true;
}
bool PreferencesAppSettingsStorage::save(const StoredAppSettings& settings) {
  Preferences preferences;
  if (!preferences.begin(kStorageNamespace, false)) return false;
  preferences.putUChar(kAlarmVolumeKey, settings.alarm_volume);
  preferences.end();
  return true;
}
#endif
