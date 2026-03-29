#include "clock_app.h"

#include "clock_formatter.h"

namespace {
constexpr uint32_t kRefreshPeriodMs = 250;
constexpr char kHourOptions[] =
    "00\n01\n02\n03\n04\n05\n06\n07\n08\n09\n10\n11\n12\n13\n14\n15\n16\n17\n18\n19\n20\n21\n22\n23";
constexpr char kMinuteOptions[] =
    "00\n01\n02\n03\n04\n05\n06\n07\n08\n09\n10\n11\n12\n13\n14\n15\n16\n17\n18\n19\n20\n21\n22\n23\n24\n25\n26\n"
    "27\n28\n29\n30\n31\n32\n33\n34\n35\n36\n37\n38\n39\n40\n41\n42\n43\n44\n45\n46\n47\n48\n49\n50\n51\n52\n53\n"
    "54\n55\n56\n57\n58\n59";
}

ClockApp::ClockApp(ClockService &clock_service, AlarmService &alarm_service, AlarmBuzzer &alarm_buzzer)
    : clock_service_(clock_service), alarm_service_(alarm_service), alarm_buzzer_(alarm_buzzer), clock_label_(nullptr),
      primary_button_label_(nullptr), alarm_editor_overlay_(nullptr), alarm_hour_roller_(nullptr),
      alarm_minute_roller_(nullptr), refresh_timer_(nullptr), buzzer_active_(false) {}

void ClockApp::build() {
  alarm_buzzer_.begin();

  lv_obj_set_style_bg_color(lv_screen_active(), lv_color_hex(0xF3F0E8), 0);
  lv_obj_set_style_text_color(lv_screen_active(), lv_color_hex(0x1F2933), 0);

  clock_label_ = lv_label_create(lv_screen_active());
  lv_label_set_text(clock_label_, "--:--:--");
  lv_obj_set_style_text_font(clock_label_, &lv_font_montserrat_48, 0);
  lv_obj_align(clock_label_, LV_ALIGN_CENTER, 0, -36);

  lv_obj_t *primary_button = lv_button_create(lv_screen_active());
  lv_obj_set_size(primary_button, 220, 74);
  lv_obj_align(primary_button, LV_ALIGN_BOTTOM_MID, 0, -20);
  lv_obj_set_style_radius(primary_button, 18, 0);
  lv_obj_set_style_bg_color(primary_button, lv_color_hex(0x2D8CF0), 0);
  lv_obj_add_event_cb(primary_button, onPrimaryButtonPressed, LV_EVENT_CLICKED, this);

  primary_button_label_ = lv_label_create(primary_button);
  lv_obj_set_style_text_align(primary_button_label_, LV_TEXT_ALIGN_CENTER, 0);
  lv_label_set_text(primary_button_label_, "Set Alarm");
  lv_obj_center(primary_button_label_);

  alarm_editor_overlay_ = lv_obj_create(lv_screen_active());
  lv_obj_remove_style_all(alarm_editor_overlay_);
  lv_obj_set_size(alarm_editor_overlay_, LV_PCT(100), LV_PCT(100));
  lv_obj_set_style_bg_color(alarm_editor_overlay_, lv_color_hex(0x000000), 0);
  lv_obj_set_style_bg_opa(alarm_editor_overlay_, LV_OPA_50, 0);
  lv_obj_add_flag(alarm_editor_overlay_, LV_OBJ_FLAG_HIDDEN);

  lv_obj_t *editor_panel = lv_obj_create(alarm_editor_overlay_);
  lv_obj_set_size(editor_panel, 270, 180);
  lv_obj_center(editor_panel);
  lv_obj_set_style_radius(editor_panel, 20, 0);
  lv_obj_set_style_bg_color(editor_panel, lv_color_hex(0xFFFDF7), 0);
  lv_obj_set_style_border_width(editor_panel, 0, 0);
  lv_obj_set_style_pad_all(editor_panel, 16, 0);

  lv_obj_t *editor_title = lv_label_create(editor_panel);
  lv_label_set_text(editor_title, "Alarm Time");
  lv_obj_set_style_text_font(editor_title, &lv_font_montserrat_20, 0);
  lv_obj_align(editor_title, LV_ALIGN_TOP_MID, 0, 0);

  alarm_hour_roller_ = lv_roller_create(editor_panel);
  lv_roller_set_options(alarm_hour_roller_, kHourOptions, LV_ROLLER_MODE_NORMAL);
  lv_obj_set_size(alarm_hour_roller_, 84, 82);
  lv_obj_align(alarm_hour_roller_, LV_ALIGN_LEFT_MID, 20, -8);

  alarm_minute_roller_ = lv_roller_create(editor_panel);
  lv_roller_set_options(alarm_minute_roller_, kMinuteOptions, LV_ROLLER_MODE_NORMAL);
  lv_obj_set_size(alarm_minute_roller_, 84, 82);
  lv_obj_align(alarm_minute_roller_, LV_ALIGN_RIGHT_MID, -20, -8);

  lv_obj_t *colon_label = lv_label_create(editor_panel);
  lv_label_set_text(colon_label, ":");
  lv_obj_set_style_text_font(colon_label, &lv_font_montserrat_28, 0);
  lv_obj_align(colon_label, LV_ALIGN_CENTER, 0, -6);

  lv_obj_t *set_button = lv_button_create(editor_panel);
  lv_obj_set_size(set_button, 72, 40);
  lv_obj_align(set_button, LV_ALIGN_BOTTOM_LEFT, 8, 0);
  lv_obj_add_event_cb(set_button, onApplyAlarmPressed, LV_EVENT_CLICKED, this);
  lv_obj_t *set_button_label = lv_label_create(set_button);
  lv_label_set_text(set_button_label, "Set");
  lv_obj_center(set_button_label);

  lv_obj_t *disable_button = lv_button_create(editor_panel);
  lv_obj_set_size(disable_button, 86, 40);
  lv_obj_align(disable_button, LV_ALIGN_BOTTOM_MID, 0, 0);
  lv_obj_add_event_cb(disable_button, onDisableAlarmPressed, LV_EVENT_CLICKED, this);
  lv_obj_t *disable_button_label = lv_label_create(disable_button);
  lv_label_set_text(disable_button_label, "Disable");
  lv_obj_center(disable_button_label);

  lv_obj_t *cancel_button = lv_button_create(editor_panel);
  lv_obj_set_size(cancel_button, 78, 40);
  lv_obj_align(cancel_button, LV_ALIGN_BOTTOM_RIGHT, -8, 0);
  lv_obj_add_event_cb(cancel_button, onCancelAlarmPressed, LV_EVENT_CLICKED, this);
  lv_obj_t *cancel_button_label = lv_label_create(cancel_button);
  lv_label_set_text(cancel_button_label, "Cancel");
  lv_obj_center(cancel_button_label);

  refresh_timer_ = lv_timer_create(onRefreshTimer, kRefreshPeriodMs, this);
  refresh();
}

void ClockApp::onPrimaryButtonPressed(lv_event_t *event) {
  auto *app = static_cast<ClockApp *>(lv_event_get_user_data(event));
  if (app == nullptr) {
    return;
  }

  if (app->alarm_service_.isRinging()) {
    app->stopAlarm();
    app->alarm_service_.dismiss();
    app->updateAlarmButtonLabel();
    return;
  }

  app->showAlarmEditor();
}

void ClockApp::onApplyAlarmPressed(lv_event_t *event) {
  auto *app = static_cast<ClockApp *>(lv_event_get_user_data(event));
  if (app == nullptr) {
    return;
  }

  app->applyAlarmFromEditor();
}

void ClockApp::onDisableAlarmPressed(lv_event_t *event) {
  auto *app = static_cast<ClockApp *>(lv_event_get_user_data(event));
  if (app == nullptr) {
    return;
  }

  app->alarm_service_.setEnabled(false);
  app->stopAlarm();
  app->hideAlarmEditor();
  app->updateAlarmButtonLabel();
}

void ClockApp::onCancelAlarmPressed(lv_event_t *event) {
  auto *app = static_cast<ClockApp *>(lv_event_get_user_data(event));
  if (app == nullptr) {
    return;
  }

  app->hideAlarmEditor();
}

void ClockApp::onRefreshTimer(lv_timer_t *timer) {
  auto *app = static_cast<ClockApp *>(lv_timer_get_user_data(timer));
  if (app == nullptr) {
    return;
  }

  app->refresh();
}

void ClockApp::refresh() {
  ClockTime now;
  if (!clock_service_.getCurrentTime(now)) {
    lv_label_set_text(clock_label_, "--:--:--");
    return;
  }

  char clock_text[9];
  ClockFormatter::formatHHMMSS(now, clock_text, sizeof(clock_text));
  lv_label_set_text(clock_label_, clock_text);

  if (alarm_service_.update(now) && !buzzer_active_) {
    alarm_buzzer_.start();
    buzzer_active_ = true;
  }

  if (alarm_service_.isRinging()) {
    if (!buzzer_active_) {
      alarm_buzzer_.start();
      buzzer_active_ = true;
    }
    alarm_buzzer_.update();
  } else if (buzzer_active_) {
    stopAlarm();
  }

  updateAlarmButtonLabel();
}

void ClockApp::showAlarmEditor() {
  syncEditorToAlarm();
  lv_obj_clear_flag(alarm_editor_overlay_, LV_OBJ_FLAG_HIDDEN);
}

void ClockApp::hideAlarmEditor() {
  lv_obj_add_flag(alarm_editor_overlay_, LV_OBJ_FLAG_HIDDEN);
}

void ClockApp::syncEditorToAlarm() {
  lv_roller_set_selected(alarm_hour_roller_, alarm_service_.hour(), LV_ANIM_OFF);
  lv_roller_set_selected(alarm_minute_roller_, alarm_service_.minute(), LV_ANIM_OFF);
}

void ClockApp::updateAlarmButtonLabel() {
  char alarm_text[64];
  ClockTime alarm_time = {alarm_service_.hour(), alarm_service_.minute(), 0};
  char formatted_alarm[6];
  ClockFormatter::formatHHMM(alarm_time, formatted_alarm, sizeof(formatted_alarm));

  if (alarm_service_.isRinging()) {
    lv_snprintf(alarm_text, sizeof(alarm_text), "Dismiss Alarm\n%s", formatted_alarm);
  } else if (alarm_service_.isEnabled()) {
    lv_snprintf(alarm_text, sizeof(alarm_text), "Alarm Set\n%s", formatted_alarm);
  } else {
    lv_snprintf(alarm_text, sizeof(alarm_text), "Set Alarm\n%s (Off)", formatted_alarm);
  }

  lv_label_set_text(primary_button_label_, alarm_text);
}

void ClockApp::stopAlarm() {
  alarm_buzzer_.stop();
  buzzer_active_ = false;
}

void ClockApp::applyAlarmFromEditor() {
  const uint16_t selected_hour = lv_roller_get_selected(alarm_hour_roller_);
  const uint16_t selected_minute = lv_roller_get_selected(alarm_minute_roller_);
  alarm_service_.setAlarm(static_cast<uint8_t>(selected_hour), static_cast<uint8_t>(selected_minute));
  alarm_service_.setEnabled(true);
  hideAlarmEditor();
  updateAlarmButtonLabel();
}
