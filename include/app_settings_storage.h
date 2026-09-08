#pragma once

#include <stdint.h>

struct StoredAppSettings {
  uint8_t alarm_volume;
};

class AppSettingsStorage {
 public:
  virtual ~AppSettingsStorage() = default;
  virtual bool load(StoredAppSettings& settings) = 0;
  virtual bool save(const StoredAppSettings& settings) = 0;
};

class InMemoryAppSettingsStorage : public AppSettingsStorage {
 public:
  InMemoryAppSettingsStorage();
  bool load(StoredAppSettings& settings) override;
  bool save(const StoredAppSettings& settings) override;

 private:
  StoredAppSettings settings_;
  bool has_saved_settings_;
};

#if defined(ARDUINO)
class PreferencesAppSettingsStorage : public AppSettingsStorage {
 public:
  bool load(StoredAppSettings& settings) override;
  bool save(const StoredAppSettings& settings) override;
};
#endif
