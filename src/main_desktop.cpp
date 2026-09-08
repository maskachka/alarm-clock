#include <stdint.h>
#include <unistd.h>

#include <lvgl.h>
#include <src/drivers/x11/lv_x11.h>

#include "alarm_buzzer.h"
#include "app_settings_service.h"
#include "app_settings_storage.h"
#include "alarm_service.h"
#include "alarm_storage.h"
#include "clock_app.h"
#include "clock_service.h"
#include "display_configuration.h"

static constexpr const char* kTimezone = "PST8PDT,M3.2.0/2,M11.1.0/2";
static constexpr const char* kNtpServer = "pool.ntp.org";
static constexpr int32_t kDesktopWindowWidth = 360;
static constexpr int32_t kDesktopWindowHeight = 240;

int main() {
  lv_init();

  lv_display_t* display = lv_x11_window_create("Alarm Clock UI", kDesktopWindowWidth, kDesktopWindowHeight);
  if (display == nullptr) {
    return 1;
  }

  lv_x11_inputs_create(display, nullptr);

  ClockService clock_service(kTimezone, kNtpServer);
  clock_service.begin(nullptr, nullptr);
  InMemoryAlarmStorage alarm_storage;
  AlarmService alarm_service(alarm_storage);
  InMemoryAppSettingsStorage app_settings_storage;
  AppSettingsService app_settings(app_settings_storage);
  SilentAlarmBuzzer alarm_buzzer;

  ClockApp app(clock_service, alarm_service, alarm_buzzer, app_settings);
  app.build();

  while (true) {
    const uint32_t wait_ms = lv_timer_handler();
    usleep((wait_ms > 0 ? wait_ms : 5U) * 1000U);
  }

  return 0;
}
