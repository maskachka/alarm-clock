/*  Rui Santos & Sara Santos - Random Nerd Tutorials
    THIS EXAMPLE WAS TESTED WITH THE FOLLOWING HARDWARE:
    1) ESP32-2432S028R 2.8 inch 240x320 also known as the Cheap Yellow Display (CYD):
   https://makeradvisor.com/tools/cyd-cheap-yellow-display-esp32-2432s028r/ SET UP INSTRUCTIONS:
   https://RandomNerdTutorials.com/cyd-lvgl/ 2) REGULAR ESP32 Dev Board + 2.8 inch 240x320 TFT Display:
   https://makeradvisor.com/tools/2-8-inch-ili9341-tft-240x320/ and
   https://makeradvisor.com/tools/esp32-dev-board-wi-fi-bluetooth/ SET UP INSTRUCTIONS:
   https://RandomNerdTutorials.com/esp32-tft-lvgl/ Permission is hereby granted, free of charge, to any person obtaining
   a copy of this software and associated documentation files. The above copyright notice and this permission notice
   shall be included in all copies or substantial portions of the Software.
*/

/*  Install the "lvgl" library version 9.2 by kisvegabor to interface with the TFT Display - https://lvgl.io/
    *** IMPORTANT: lv_conf.h available on the internet will probably NOT work with the examples available at Random Nerd
   Tutorials ***
    *** YOU MUST USE THE lv_conf.h FILE PROVIDED IN THE LINK BELOW IN ORDER TO USE THE EXAMPLES FROM RANDOM NERD
   TUTORIALS *** FULL INSTRUCTIONS AVAILABLE ON HOW CONFIGURE THE LIBRARY: https://RandomNerdTutorials.com/cyd-lvgl/ or
   https://RandomNerdTutorials.com/esp32-tft-lvgl/   */
#include <lvgl.h>
#include <string.h>

/*  Install the "TFT_eSPI" library by Bodmer to interface with the TFT Display - https://github.com/Bodmer/TFT_eSPI
    *** IMPORTANT: User_Setup.h available on the internet will probably NOT work with the examples available at Random
   Nerd Tutorials ***
    *** YOU MUST USE THE User_Setup.h FILE PROVIDED IN THE LINK BELOW IN ORDER TO USE THE EXAMPLES FROM RANDOM NERD
   TUTORIALS *** FULL INSTRUCTIONS AVAILABLE ON HOW CONFIGURE THE LIBRARY: https://RandomNerdTutorials.com/cyd-lvgl/ or
   https://RandomNerdTutorials.com/esp32-tft-lvgl/   */
#include <TFT_eSPI.h>

// Install the "XPT2046_Touchscreen" library by Paul Stoffregen to use the Touchscreen -
// https://github.com/PaulStoffregen/XPT2046_Touchscreen - Note: this library doesn't require further configuration
#include <XPT2046_Touchscreen.h>

#include "alarm_buzzer.h"
#include "alarm_service.h"
#include "clock_app.h"
#include "clock_service.h"
#include "buzzer_configuration.h"
#include "display_configuration.h"
#include "touchscreen_configuration.h"

SPIClass touchscreenSPI = SPIClass(VSPI);
XPT2046_Touchscreen touchscreen(XPT2046_CS, XPT2046_IRQ);

int x, y, z;

#define DRAW_BUF_SIZE (SCREEN_WIDTH * SCREEN_HEIGHT / 10 * (LV_COLOR_DEPTH / 8))
uint32_t draw_buf[DRAW_BUF_SIZE / 4];

#ifndef WIFI_SSID
#define WIFI_SSID ""
#endif

#ifndef WIFI_PASSWORD
#define WIFI_PASSWORD ""
#endif

static constexpr const char* kTimezone = "PST8PDT,M3.2.0/2,M11.1.0/2";
static constexpr const char* kNtpServer = "pool.ntp.org";
static constexpr unsigned long kBuzzerSelfTestMs = 350;
static constexpr bool kRunStartupBuzzerSelfTest = false;

ClockService g_clock_service(kTimezone, kNtpServer);
AlarmService g_alarm_service;
Esp32PassiveBuzzer g_alarm_buzzer(BUZZER_PIN);
ClockApp g_clock_app(g_clock_service, g_alarm_service, g_alarm_buzzer);

void touchscreen_read(lv_indev_t* indev, lv_indev_data_t* data) {
  LV_UNUSED(indev);

  if (touchscreen.tirqTouched() && touchscreen.touched()) {
    TS_Point p = touchscreen.getPoint();
    x = map(p.x, 200, 3700, 1, SCREEN_WIDTH);
    y = map(p.y, 240, 3800, 1, SCREEN_HEIGHT);
    z = p.z;
    LV_UNUSED(z);

    data->state = LV_INDEV_STATE_PRESSED;
    data->point.x = x;
    data->point.y = y;
  } else {
    data->state = LV_INDEV_STATE_RELEASED;
  }
}

void runBuzzerSelfTest() {
  if (BUZZER_PIN < 0) {
    Serial.println("Buzzer self-test skipped: buzzer disabled.");
    return;
  }

  Serial.printf("Running buzzer self-test on GPIO %d.\n", BUZZER_PIN);
  g_alarm_buzzer.start();
  delay(kBuzzerSelfTestMs);
  g_alarm_buzzer.update();
  delay(kBuzzerSelfTestMs);
  g_alarm_buzzer.stop();
}

void setup() {
  String lvgl_arduino =
      String("LVGL Library Version: ") + lv_version_major() + "." + lv_version_minor() + "." + lv_version_patch();
  Serial.begin(115200);
  Serial.println(lvgl_arduino);
  Serial.printf("Build config: WIFI_SSID length=%u\n", static_cast<unsigned>(strlen(WIFI_SSID)));

  g_clock_service.begin(WIFI_SSID, WIFI_PASSWORD);
  if (!g_clock_service.isSynchronized()) {
    Serial.println("Clock sync pending. Define WIFI_SSID/WIFI_PASSWORD build flags to enable NTP.");
  }
  Serial.printf("Buzzer config: GPIO %d\n", BUZZER_PIN);

  lv_init();

  touchscreenSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  touchscreen.begin(touchscreenSPI);
  touchscreen.setRotation(2);

  lv_display_t* disp = lv_tft_espi_create(SCREEN_WIDTH, SCREEN_HEIGHT, draw_buf, sizeof(draw_buf));
  lv_display_set_rotation(disp, LV_DISPLAY_ROTATION_90);

  lv_indev_t* indev = lv_indev_create();
  lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
  lv_indev_set_read_cb(indev, touchscreen_read);

  g_clock_app.build();
  if (kRunStartupBuzzerSelfTest) {
    runBuzzerSelfTest();
  }
}

void loop() {
  lv_timer_handler();
  lv_tick_inc(5);
  delay(5);
}
