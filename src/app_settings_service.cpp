#include "app_settings_service.h"

AppSettingsService::AppSettingsService() : storage_(nullptr), alarm_volume_(kDefaultAlarmVolume) {}
AppSettingsService::AppSettingsService(AppSettingsStorage& storage)
    : storage_(&storage), alarm_volume_(kDefaultAlarmVolume) {}
void AppSettingsService::load() {
  if (storage_ == nullptr) return;
  StoredAppSettings settings = {};
  if (!storage_->load(settings)) return;
  alarm_volume_ = settings.alarm_volume <= kMaxAlarmVolume ? settings.alarm_volume : kDefaultAlarmVolume;
}
uint8_t AppSettingsService::alarmVolume() const { return alarm_volume_; }
void AppSettingsService::setAlarmVolume(uint8_t volume) {
  alarm_volume_ = volume <= kMaxAlarmVolume ? volume : kMaxAlarmVolume;
  persist();
}
void AppSettingsService::persist() const {
  if (storage_ != nullptr) storage_->save({alarm_volume_});
}
