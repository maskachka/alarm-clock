#include "screens/clock_screen.h"
#include "ui/theme.h"
namespace {
constexpr int32_t kDigitWidth = 32, kSeparatorWidth = 16;
}
ClockScreen::ClockScreen(ClockScreenListener& l) : listener_(l), root_(nullptr), digits_{} {}
void ClockScreen::build(lv_obj_t* parent) {
  root_ = lv_obj_create(parent);
  lv_obj_remove_style_all(root_);
  lv_obj_set_size(root_, LV_PCT(100), LV_PCT(100));
  lv_obj_set_style_pad_top(root_, 28, 0);
  lv_obj_set_style_pad_row(root_, 24, 0);
  lv_obj_set_flex_flow(root_, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(root_, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  lv_obj_t* row = lv_obj_create(root_);
  lv_obj_remove_style_all(row);
  lv_obj_set_size(row, 144, 72);
  lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
  lv_obj_set_flex_align(row, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  for (uint8_t i = 0; i < 5; ++i) {
    digits_[i] = lv_label_create(row);
    lv_obj_set_size(digits_[i], i == 2 ? kSeparatorWidth : kDigitWidth, 56);
    lv_obj_set_style_text_font(digits_[i], &lv_font_montserrat_48, 0);
    lv_obj_set_style_text_color(digits_[i], lv_color_hex(UiTheme::kTextPrimary), 0);
    lv_obj_set_style_text_align(digits_[i], LV_TEXT_ALIGN_CENTER, 0);
  }
  lv_obj_t* settings = lv_label_create(root_);
  lv_label_set_text(settings, "Settings");
  lv_obj_set_style_text_font(settings, &lv_font_montserrat_20, 0);
  lv_obj_set_style_text_color(settings, lv_color_hex(UiTheme::kAccent), 0);
  lv_obj_add_flag(settings, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_add_event_cb(settings, onSettingsPressed, LV_EVENT_CLICKED, this);
}
void ClockScreen::render(const ClockAppState& s) { setClockText(s.clock_text); }
void ClockScreen::show() { lv_obj_clear_flag(root_, LV_OBJ_FLAG_HIDDEN); }
void ClockScreen::hide() { lv_obj_add_flag(root_, LV_OBJ_FLAG_HIDDEN); }
void ClockScreen::onSettingsPressed(lv_event_t* e) {
  auto* s = static_cast<ClockScreen*>(lv_event_get_user_data(e));
  if (s) s->listener_.onSettingsRequested();
}
void ClockScreen::setClockText(const char* t) {
  if (!t) return;
  bool colon = (lv_tick_get() % 1000) < 850;
  for (uint8_t i = 0; i < 5; ++i) {
    char c = t[i] == '\0' ? ' ' : t[i];
    if (i == 2 && c == ':' && !colon) c = ' ';
    char x[2] = {c, '\0'};
    lv_label_set_text(digits_[i], x);
  }
}
