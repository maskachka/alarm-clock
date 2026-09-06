#include "clock_app.h"

#include <cstdio>

#include "ui/theme.h"

namespace {
constexpr uint32_t kRefreshPeriodMs = 50;
}

ClockApp::ClockApp(ClockService& clock_service, AlarmService& alarm_service, AlarmBuzzer& alarm_buzzer)
    : clock_service_(clock_service),
      alarm_service_(alarm_service),
      alarm_buzzer_(alarm_buzzer),
      controller_(alarm_service),
      clock_screen_(*this),
      alarm_settings_screen_(*this),
      confirmation_dialog_view_(*this),
      alarm_list_screen_(*this),
      refresh_timer_(nullptr),
      buzzer_active_(false),
      creating_alarm_(false) {}

void ClockApp::build() {
  alarm_service_.load();
  alarm_buzzer_.begin();

  lv_obj_t* screen = lv_screen_active();
  UiTheme::applyToScreen(screen);
  clock_screen_.build(screen);
  alarm_list_screen_.build(screen);
  alarm_settings_screen_.build(screen);
  confirmation_dialog_view_.build(screen);

  refresh_timer_ = lv_timer_create(onRefreshTimer, kRefreshPeriodMs, this);
  controller_.initialize();
  applyControllerState();
  refresh();
}

void ClockApp::onRefreshTimer(lv_timer_t* timer) {
  auto* app = static_cast<ClockApp*>(lv_timer_get_user_data(timer));
  if (app != nullptr) {
    app->refresh();
  }
}

void ClockApp::onSettingsRequested() {
  clock_screen_.hide();
  alarm_list_screen_.render(alarm_service_);
  alarm_list_screen_.show();
}

void ClockApp::onAlarmAddRequested() {
  if (alarm_service_.count() == AlarmService::kMaxAlarms) return;
  creating_alarm_ = true;
  alarm_settings_screen_.setTitle("Add new alarm");
  applyControllerEffects(controller_.openNewAlarmSettings());
  applyControllerState();
}

void ClockApp::onAlarmDeleteRequested(uint8_t index) {
  if (index >= alarm_service_.count()) return;
  char message[96];
  snprintf(message, sizeof(message), "Are you sure you want to delete\nalarm %u at %02u:%02u?",
           static_cast<unsigned>(index + 1), static_cast<unsigned>(alarm_service_.hour(index)),
           static_cast<unsigned>(alarm_service_.minute(index)));
  confirmation_dialog_view_.show({ConfirmationAction::DeleteAlarm, index, message, "Delete", true});
}
void ClockApp::onDismissRequested() {
  confirmation_dialog_view_.show(
      {ConfirmationAction::DismissRingingAlarms, 0, "Dismiss all currently\nringing alarms?", "Dismiss", false});
}
void ClockApp::onConfirmationConfirmed(ConfirmationAction action, uint8_t index) {
  if (action == ConfirmationAction::DeleteAlarm)
    alarm_service_.removeAlarm(index);
  else
    applyControllerEffects(controller_.dismissAllRinging());
  confirmation_dialog_view_.hide();
  applyControllerState();
}
void ClockApp::onConfirmationCancelled() { confirmation_dialog_view_.hide(); }

void ClockApp::onAlarmSelected(uint8_t index) {
  char title[32];
  snprintf(title, sizeof(title), "Edit alarm %u", static_cast<unsigned>(index + 1));
  alarm_settings_screen_.setTitle(title);
  applyControllerEffects(controller_.openAlarmSettings(index));
  applyControllerState();
}

void ClockApp::onAlarmToggled(uint8_t index) {
  controller_.selectAlarm(index);
  applyControllerEffects(controller_.onToggleAlarmPressed());
  applyControllerState();
}

void ClockApp::onAlarmListBackRequested() {
  alarm_list_screen_.hide();
  clock_screen_.show();
}

void ClockApp::onAlarmSettingsApplied(uint8_t hour, uint8_t minute, uint8_t weekday_mask) {
  if (creating_alarm_) {
    alarm_service_.addAlarm(hour, minute);
    alarm_service_.setWeekdayMask(alarm_service_.count() - 1, weekday_mask);
    creating_alarm_ = false;
    applyControllerEffects(controller_.onCancelAlarmPressed());
  } else {
    applyControllerEffects(controller_.onApplyAlarmPressed(hour, minute, weekday_mask));
  }
  applyControllerState();
}

void ClockApp::onAlarmSettingsCancelled() {
  creating_alarm_ = false;
  applyControllerEffects(controller_.onCancelAlarmPressed());
  applyControllerState();
}

void ClockApp::onAlarmSettingsBackRequested() { onAlarmSettingsCancelled(); }

void ClockApp::refresh() {
  ClockTime now;
  if (!clock_service_.getCurrentTime(now)) {
    applyControllerEffects(controller_.refresh(false, ClockTime{0, 0, 0, 0}));
    applyControllerState();
    return;
  }

  applyControllerEffects(controller_.refresh(true, now));
  applyControllerState();
}

void ClockApp::applyControllerState() {
  const ClockAppState& state = controller_.state();
  clock_screen_.render(state);
  alarm_list_screen_.render(alarm_service_);

  if (state.alarm_settings_visible) {
    alarm_settings_screen_.show();
  } else {
    alarm_settings_screen_.hide();
  }
}

void ClockApp::applyControllerEffects(const ClockAppEffects& effects) {
  if (effects.sync_alarm_settings_selection) {
    alarm_settings_screen_.setSelection(effects.settings_hour, effects.settings_minute, effects.settings_weekday_mask);
  }

  if (effects.start_buzzer) {
    alarm_buzzer_.start();
    buzzer_active_ = true;
  }

  if (effects.update_buzzer) {
    alarm_buzzer_.update();
  }

  if (effects.stop_buzzer && buzzer_active_) {
    alarm_buzzer_.stop();
    buzzer_active_ = false;
  }
}
