#pragma once

#include <lvgl.h>

#include "alarm_buzzer.h"
#include "clock_app_controller.h"
#include "clock_service.h"
#include "screens/alarm_editor_view.h"
#include "screens/clock_screen.h"

class ClockApp : private ClockScreenListener, private AlarmEditorViewListener
{
public:
  ClockApp(ClockService &clock_service, AlarmService &alarm_service, AlarmBuzzer &alarm_buzzer);

  void build();

private:
  static void onRefreshTimer(lv_timer_t *timer);

  void onClockScreenPrimaryAction() override;
  void onClockScreenAlarmToggled() override;
  void onAlarmEditorApplied(uint8_t hour, uint8_t minute) override;
  void onAlarmEditorCancelled() override;

  void refresh();
  void applyControllerState();
  void applyControllerEffects(const ClockAppEffects &effects);

  ClockService &clock_service_;
  AlarmBuzzer &alarm_buzzer_;
  ClockAppController controller_;
  ClockScreen clock_screen_;
  AlarmEditorView alarm_editor_view_;
  lv_timer_t *refresh_timer_;
  bool buzzer_active_;
};
