#pragma once

#include <stdint.h>

struct StoredAlarm {
  uint8_t hour;
  uint8_t minute;
  uint8_t enabled;
  uint8_t weekday_mask;
  uint8_t ringtone;
};

struct StoredAlarms {
  static constexpr uint8_t kMaxAlarms = 4;

  uint8_t count;
  StoredAlarm alarms[kMaxAlarms];
};

class AlarmStorage {
 public:
  virtual ~AlarmStorage() = default;

  // Returns false when no valid saved alarms are available. A migrated record is returned in its latest shape.
  virtual bool load(StoredAlarms& alarms, bool& migrated) = 0;
  virtual bool save(const StoredAlarms& alarms) = 0;
};

class InMemoryAlarmStorage : public AlarmStorage {
 public:
  InMemoryAlarmStorage();

  bool load(StoredAlarms& alarms, bool& migrated) override;
  bool save(const StoredAlarms& alarms) override;

 private:
  StoredAlarms alarms_;
  bool has_saved_alarms_;
};

#if defined(ARDUINO)
class PreferencesAlarmStorage : public AlarmStorage {
 public:
  bool load(StoredAlarms& alarms, bool& migrated) override;
  bool save(const StoredAlarms& alarms) override;
};
#endif
