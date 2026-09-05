#pragma once

#include <stdint.h>

#include <lvgl.h>

#include "clock_service.h"

class ClockDisplayController {
 public:
  explicit ClockDisplayController(ClockService& clock_service);

  void attachLabel(lv_obj_t* label);
  void begin(uint32_t update_period_ms = 1000);
  void refresh();

 private:
  static void onTimer(lv_timer_t* timer);

  ClockService& clock_service_;
  lv_obj_t* label_;
  lv_timer_t* timer_;
};
