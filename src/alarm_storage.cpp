#include "alarm_storage.h"

#include <string.h>

#if defined(ARDUINO)
#include <Preferences.h>
#endif

InMemoryAlarmStorage::InMemoryAlarmStorage() : alarms_{}, has_saved_alarms_(false) {}

bool InMemoryAlarmStorage::load(StoredAlarms& alarms, bool& migrated) {
  migrated = false;
  if (!has_saved_alarms_) return false;
  alarms = alarms_;
  return true;
}

bool InMemoryAlarmStorage::save(const StoredAlarms& alarms) {
  if (alarms.count > StoredAlarms::kMaxAlarms) return false;
  alarms_ = alarms;
  has_saved_alarms_ = true;
  return true;
}

#if defined(ARDUINO)
namespace {
struct StoredAlarmV1 {
  uint8_t hour;
  uint8_t minute;
  uint8_t enabled;
};

struct StoredAlarmV2 {
  uint8_t hour;
  uint8_t minute;
  uint8_t enabled;
  uint8_t weekday_mask;
};

constexpr uint8_t kStorageVersion = 3;
constexpr uint8_t kEveryDayMask = 0x7f;
constexpr uint8_t kDefaultRingtone = 0;
constexpr const char* kStorageNamespace = "alarm-clock";
constexpr const char* kVersionKey = "version";
constexpr const char* kCountKey = "count";
constexpr const char* kAlarmsKey = "alarms";
}  // namespace

bool PreferencesAlarmStorage::load(StoredAlarms& alarms, bool& migrated) {
  migrated = false;
  Preferences preferences;
  if (!preferences.begin(kStorageNamespace, true)) return false;
  if (!preferences.isKey(kCountKey)) {
    preferences.end();
    return false;
  }

  const uint8_t count = preferences.getUChar(kCountKey, 0);
  const uint8_t version = preferences.getUChar(kVersionKey, 1);
  const size_t record_size = version == kStorageVersion ? sizeof(StoredAlarm)
                             : version == 2             ? sizeof(StoredAlarmV2)
                             : version == 1             ? sizeof(StoredAlarmV1)
                                                        : 0;
  if (count > StoredAlarms::kMaxAlarms || record_size == 0 ||
      (count > 0 && preferences.getBytesLength(kAlarmsKey) != count * record_size)) {
    preferences.end();
    return false;
  }

  alarms = {};
  alarms.count = count;
  if (count > 0 && version == kStorageVersion) {
    preferences.getBytes(kAlarmsKey, alarms.alarms, count * sizeof(StoredAlarm));
  } else if (count > 0 && version == 2) {
    StoredAlarmV2 legacy[StoredAlarms::kMaxAlarms] = {};
    preferences.getBytes(kAlarmsKey, legacy, count * sizeof(StoredAlarmV2));
    for (uint8_t i = 0; i < count; ++i) {
      alarms.alarms[i] = {legacy[i].hour, legacy[i].minute, legacy[i].enabled, legacy[i].weekday_mask,
                          kDefaultRingtone};
    }
  } else if (count > 0) {
    StoredAlarmV1 legacy[StoredAlarms::kMaxAlarms] = {};
    preferences.getBytes(kAlarmsKey, legacy, count * sizeof(StoredAlarmV1));
    for (uint8_t i = 0; i < count; ++i) {
      alarms.alarms[i] = {legacy[i].hour, legacy[i].minute, legacy[i].enabled, kEveryDayMask, kDefaultRingtone};
    }
  }
  preferences.end();
  migrated = version < kStorageVersion;
  return true;
}

bool PreferencesAlarmStorage::save(const StoredAlarms& alarms) {
  if (alarms.count > StoredAlarms::kMaxAlarms) return false;
  Preferences preferences;
  if (!preferences.begin(kStorageNamespace, false)) return false;
  preferences.putUChar(kCountKey, alarms.count);
  if (alarms.count == 0)
    preferences.remove(kAlarmsKey);
  else
    preferences.putBytes(kAlarmsKey, alarms.alarms, alarms.count * sizeof(StoredAlarm));
  preferences.putUChar(kVersionKey, kStorageVersion);
  preferences.end();
  return true;
}
#endif
