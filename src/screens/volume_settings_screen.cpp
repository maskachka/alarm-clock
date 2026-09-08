#include "screens/volume_settings_screen.h"

#include <stdio.h>

#include "ui/button.h"
#include "ui/icons.h"
#include "ui/page_header.h"
#include "ui/theme.h"

VolumeSettingsScreen::VolumeSettingsScreen(VolumeSettingsScreenListener& listener)
    : listener_(listener), root_(nullptr), slider_(nullptr), value_(nullptr) {}
void VolumeSettingsScreen::build(lv_obj_t* parent) {
  root_ = lv_obj_create(parent);
  lv_obj_remove_style_all(root_);
  lv_obj_set_size(root_, LV_PCT(100), LV_PCT(100));
  lv_obj_set_style_pad_all(root_, 12, 0);
  lv_obj_set_style_pad_row(root_, 18, 0);
  lv_obj_set_flex_flow(root_, LV_FLEX_FLOW_COLUMN);
  lv_obj_add_flag(root_, LV_OBJ_FLAG_HIDDEN);
  constexpr Ui::ButtonSize kSaveButtonSize{60, 32};
  Ui::createPageHeader(root_, {"Alarm volume", UiIcon::kBackLabel, onBack, this, "Save", &UiTheme::kPrimaryButtonStyle,
                               kSaveButtonSize, onSave, this});
  value_ = lv_label_create(root_);
  lv_obj_set_style_text_font(value_, &lv_font_montserrat_28, 0);
  lv_obj_set_style_text_color(value_, lv_color_hex(UiTheme::kTextPrimary), 0);
  slider_ = lv_slider_create(root_);
  lv_obj_set_width(slider_, LV_PCT(100));
  lv_slider_set_range(slider_, 0, 100);
  lv_obj_add_event_cb(slider_, onSliderChanged, LV_EVENT_VALUE_CHANGED, this);
  setVolume(70);
}
void VolumeSettingsScreen::setVolume(uint8_t volume) {
  lv_slider_set_value(slider_, volume > 100 ? 100 : volume, LV_ANIM_OFF);
  updateLabel();
}
void VolumeSettingsScreen::show() {
  if (lv_obj_has_flag(root_, LV_OBJ_FLAG_HIDDEN)) {
    lv_obj_scroll_to_y(root_, 0, LV_ANIM_OFF);
    lv_obj_clear_flag(root_, LV_OBJ_FLAG_HIDDEN);
  }
}
void VolumeSettingsScreen::hide() { lv_obj_add_flag(root_, LV_OBJ_FLAG_HIDDEN); }
void VolumeSettingsScreen::onSave(lv_event_t* event) {
  auto* screen = static_cast<VolumeSettingsScreen*>(lv_event_get_user_data(event));
  if (screen != nullptr) screen->listener_.onVolumeSaved(lv_slider_get_value(screen->slider_));
}
void VolumeSettingsScreen::onBack(lv_event_t* event) {
  auto* screen = static_cast<VolumeSettingsScreen*>(lv_event_get_user_data(event));
  if (screen != nullptr) screen->listener_.onVolumeSettingsBackRequested();
}
void VolumeSettingsScreen::onSliderChanged(lv_event_t* event) {
  auto* screen = static_cast<VolumeSettingsScreen*>(lv_event_get_user_data(event));
  if (screen != nullptr) screen->updateLabel();
}
void VolumeSettingsScreen::updateLabel() {
  char text[8];
  snprintf(text, sizeof(text), "%d%%", lv_slider_get_value(slider_));
  lv_label_set_text(value_, text);
}
