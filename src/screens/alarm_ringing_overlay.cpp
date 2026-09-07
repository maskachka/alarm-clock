#include "screens/alarm_ringing_overlay.h"

#include "ui/button.h"
#include "ui/theme.h"

namespace {
constexpr Ui::ButtonSize kActionButtonSize{96, 42};
}

AlarmRingingOverlay::AlarmRingingOverlay(AlarmRingingOverlayListener& listener) : listener_(listener), root_(nullptr) {}

void AlarmRingingOverlay::build(lv_obj_t* parent) {
  root_ = lv_obj_create(parent);
  lv_obj_remove_style_all(root_);
  lv_obj_set_size(root_, LV_PCT(100), LV_PCT(100));
  lv_obj_set_style_bg_color(root_, lv_color_hex(UiTheme::kSurfaceRaised), 0);
  lv_obj_set_style_bg_opa(root_, LV_OPA_COVER, 0);
  lv_obj_add_flag(root_, LV_OBJ_FLAG_HIDDEN);

  lv_obj_t* dialog = lv_obj_create(root_);
  lv_obj_set_size(dialog, 288, 160);
  lv_obj_center(dialog);
  lv_obj_set_style_pad_all(dialog, 18, 0);
  lv_obj_set_style_pad_row(dialog, 16, 0);
  lv_obj_set_flex_flow(dialog, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(dialog, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

  lv_obj_t* title = lv_label_create(dialog);
  lv_label_set_text(title, "Alarm ringing");
  lv_obj_set_style_text_font(title, &lv_font_montserrat_20, 0);
  lv_obj_set_style_text_color(title, lv_color_hex(UiTheme::kTextPrimary), 0);

  lv_obj_t* actions = lv_obj_create(dialog);
  lv_obj_remove_style_all(actions);
  lv_obj_set_size(actions, LV_PCT(100), LV_SIZE_CONTENT);
  lv_obj_set_flex_flow(actions, LV_FLEX_FLOW_ROW);
  lv_obj_set_flex_align(actions, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  Ui::createButton(actions, "Dismiss", UiTheme::kPrimaryButtonStyle, kActionButtonSize, onDismiss, this);
  Ui::createButton(actions, "Snooze", UiTheme::kSecondaryButtonStyle, kActionButtonSize, onSnooze, this);
}

void AlarmRingingOverlay::show() {
  if (lv_obj_has_flag(root_, LV_OBJ_FLAG_HIDDEN)) lv_obj_clear_flag(root_, LV_OBJ_FLAG_HIDDEN);
}

void AlarmRingingOverlay::hide() { lv_obj_add_flag(root_, LV_OBJ_FLAG_HIDDEN); }

void AlarmRingingOverlay::onDismiss(lv_event_t* event) {
  auto* overlay = static_cast<AlarmRingingOverlay*>(lv_event_get_user_data(event));
  if (overlay != nullptr) overlay->listener_.onAlarmDismissRequested();
}

void AlarmRingingOverlay::onSnooze(lv_event_t* event) {
  auto* overlay = static_cast<AlarmRingingOverlay*>(lv_event_get_user_data(event));
  if (overlay != nullptr) overlay->listener_.onAlarmSnoozeRequested();
}
