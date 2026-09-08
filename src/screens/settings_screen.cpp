#include "screens/settings_screen.h"

#include "ui/icons.h"
#include "ui/page_header.h"
#include "ui/theme.h"

namespace {
lv_obj_t* createLink(lv_obj_t* parent, const char* text, lv_event_cb_t callback, void* user_data) {
  lv_obj_t* link = lv_label_create(parent);
  lv_label_set_text(link, text);
  lv_obj_set_width(link, LV_PCT(100));
  lv_obj_set_style_pad_all(link, 10, 0);
  lv_obj_set_style_text_font(link, &lv_font_montserrat_20, 0);
  lv_obj_set_style_text_color(link, lv_color_hex(UiTheme::kAccent), 0);
  lv_obj_add_flag(link, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_add_event_cb(link, callback, LV_EVENT_CLICKED, user_data);
  return link;
}
}  // namespace

SettingsScreen::SettingsScreen(SettingsScreenListener& listener) : listener_(listener), root_(nullptr) {}
void SettingsScreen::build(lv_obj_t* parent) {
  root_ = lv_obj_create(parent);
  lv_obj_remove_style_all(root_);
  lv_obj_set_size(root_, LV_PCT(100), LV_PCT(100));
  lv_obj_set_style_pad_all(root_, 12, 0);
  lv_obj_set_style_pad_row(root_, 12, 0);
  lv_obj_set_flex_flow(root_, LV_FLEX_FLOW_COLUMN);
  lv_obj_add_flag(root_, LV_OBJ_FLAG_HIDDEN);
  Ui::createPageHeader(root_, {"Settings", UiIcon::kBackLabel, onBack, this, nullptr, nullptr, {}, nullptr, nullptr});
  createLink(root_, "Alarms", onAlarms, this);
  createLink(root_, "Alarm volume", onVolume, this);
}
void SettingsScreen::show() {
  if (lv_obj_has_flag(root_, LV_OBJ_FLAG_HIDDEN)) {
    lv_obj_scroll_to_y(root_, 0, LV_ANIM_OFF);
    lv_obj_clear_flag(root_, LV_OBJ_FLAG_HIDDEN);
  }
}
void SettingsScreen::hide() { lv_obj_add_flag(root_, LV_OBJ_FLAG_HIDDEN); }
void SettingsScreen::onAlarms(lv_event_t* event) {
  auto* screen = static_cast<SettingsScreen*>(lv_event_get_user_data(event));
  if (screen != nullptr) screen->listener_.onAlarmsSettingsRequested();
}
void SettingsScreen::onVolume(lv_event_t* event) {
  auto* screen = static_cast<SettingsScreen*>(lv_event_get_user_data(event));
  if (screen != nullptr) screen->listener_.onVolumeSettingsRequested();
}
void SettingsScreen::onBack(lv_event_t* event) {
  auto* screen = static_cast<SettingsScreen*>(lv_event_get_user_data(event));
  if (screen != nullptr) screen->listener_.onSettingsBackRequested();
}
