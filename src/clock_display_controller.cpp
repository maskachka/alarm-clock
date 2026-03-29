#include "clock_display_controller.h"

#include "clock_formatter.h"

ClockDisplayController::ClockDisplayController(ClockService &clock_service)
    : clock_service_(clock_service), label_(nullptr), timer_(nullptr) {}

void ClockDisplayController::attachLabel(lv_obj_t *label) {
  label_ = label;
}

void ClockDisplayController::begin(uint32_t update_period_ms) {
  if (timer_ != nullptr) {
    lv_timer_delete(timer_);
    timer_ = nullptr;
  }

  timer_ = lv_timer_create(onTimer, update_period_ms, this);
  refresh();
}

void ClockDisplayController::refresh() {
  if (label_ == nullptr) {
    return;
  }

  ClockTime now;
  if (!clock_service_.getCurrentTime(now)) {
    lv_label_set_text(label_, "--:--:--");
    return;
  }

  char clock_text[9];
  ClockFormatter::formatHHMMSS(now, clock_text, sizeof(clock_text));
  lv_label_set_text(label_, clock_text);
}

void ClockDisplayController::onTimer(lv_timer_t *timer) {
  auto *controller = static_cast<ClockDisplayController *>(timer->user_data);
  if (controller == nullptr) {
    return;
  }

  controller->refresh();
}
