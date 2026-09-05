#include "clock_app.h"

#include "ui/theme.h"

namespace
{
constexpr uint32_t kRefreshPeriodMs = 50;
}

ClockApp::ClockApp(ClockService &clock_service, AlarmService &alarm_service, AlarmBuzzer &alarm_buzzer)
    : clock_service_(clock_service), alarm_buzzer_(alarm_buzzer), controller_(alarm_service), clock_screen_(*this),
      alarm_editor_view_(*this), refresh_timer_(nullptr), buzzer_active_(false) {}

void ClockApp::build()
{
  alarm_buzzer_.begin();

  lv_obj_t *screen = lv_screen_active();
  UiTheme::applyToScreen(screen);
  clock_screen_.build(screen);
  alarm_editor_view_.build(screen);

  refresh_timer_ = lv_timer_create(onRefreshTimer, kRefreshPeriodMs, this);
  controller_.initialize();
  applyControllerState();
  refresh();
}

void ClockApp::onRefreshTimer(lv_timer_t *timer)
{
  auto *app = static_cast<ClockApp *>(lv_timer_get_user_data(timer));
  if(app != nullptr) {
    app->refresh();
  }
}

void ClockApp::onClockScreenPrimaryAction()
{
  applyControllerEffects(controller_.onPrimaryButtonPressed());
  applyControllerState();
}

void ClockApp::onClockScreenAlarmToggled()
{
  applyControllerEffects(controller_.onToggleAlarmPressed());
  applyControllerState();
}

void ClockApp::onAlarmEditorApplied(uint8_t hour, uint8_t minute)
{
  applyControllerEffects(controller_.onApplyAlarmPressed(hour, minute));
  applyControllerState();
}

void ClockApp::onAlarmEditorCancelled()
{
  applyControllerEffects(controller_.onCancelAlarmPressed());
  applyControllerState();
}

void ClockApp::refresh()
{
  ClockTime now;
  if(!clock_service_.getCurrentTime(now)) {
    applyControllerEffects(controller_.refresh(false, ClockTime{0, 0, 0}));
    applyControllerState();
    return;
  }

  applyControllerEffects(controller_.refresh(true, now));
  applyControllerState();
}

void ClockApp::applyControllerState()
{
  const ClockAppState &state = controller_.state();
  clock_screen_.render(state);

  if(state.editor_visible) {
    alarm_editor_view_.show();
  }
  else {
    alarm_editor_view_.hide();
  }
}

void ClockApp::applyControllerEffects(const ClockAppEffects &effects)
{
  if(effects.sync_editor_selection) {
    alarm_editor_view_.setSelection(effects.editor_hour, effects.editor_minute);
  }

  if(effects.start_buzzer) {
    alarm_buzzer_.start();
    buzzer_active_ = true;
  }

  if(effects.update_buzzer) {
    alarm_buzzer_.update();
  }

  if(effects.stop_buzzer && buzzer_active_) {
    alarm_buzzer_.stop();
    buzzer_active_ = false;
  }
}
