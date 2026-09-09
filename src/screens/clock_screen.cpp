#include "screens/clock_screen.h"

#include <cstring>

#include "ui/icons.h"
#include "ui/theme.h"
namespace {
constexpr int32_t kDigitWidth = 32, kSeparatorWidth = 16;
}
ClockScreen::ClockScreen(ClockScreenListener& l)
    : listener_(l),
      root_(nullptr),
      date_(nullptr),
      next_alarm_(nullptr),
      digits_{},
      colon_blink_timer_(nullptr),
      clock_text_{},
      colon_visible_(true) {}
void ClockScreen::build(lv_obj_t* parent) {
  root_ = lv_obj_create(parent);
  lv_obj_remove_style_all(root_);
  lv_obj_set_size(root_, LV_PCT(100), LV_PCT(100));
  lv_obj_set_style_pad_top(root_, 28, 0);
  lv_obj_set_style_pad_row(root_, 24, 0);
  lv_obj_set_flex_flow(root_, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(root_, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  lv_obj_t* clock_line = lv_obj_create(root_);
  lv_obj_remove_style_all(clock_line);
  lv_obj_set_size(clock_line, LV_SIZE_CONTENT, 72);
  lv_obj_set_style_pad_column(clock_line, 8, 0);
  lv_obj_set_flex_flow(clock_line, LV_FLEX_FLOW_ROW);
  lv_obj_set_flex_align(clock_line, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  date_ = lv_label_create(clock_line);
  lv_obj_set_style_text_font(date_, &lv_font_montserrat_16, 0);
  lv_obj_set_style_text_color(date_, lv_color_hex(UiTheme::kTextPrimary), 0);
  lv_obj_add_flag(date_, LV_OBJ_FLAG_HIDDEN);
  lv_obj_t* row = lv_obj_create(clock_line);
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
  next_alarm_ = lv_label_create(root_);
  lv_obj_set_width(next_alarm_, LV_PCT(100));
  lv_label_set_long_mode(next_alarm_, LV_LABEL_LONG_WRAP);
  lv_obj_set_style_text_align(next_alarm_, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_set_style_text_font(next_alarm_, &lv_font_montserrat_14, 0);
  lv_obj_set_style_text_color(next_alarm_, lv_color_hex(UiTheme::kTextPrimary), 0);
  lv_obj_t* settings = lv_obj_create(root_);
  lv_obj_remove_style_all(settings);
  lv_obj_set_size(settings, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
  lv_obj_set_style_pad_column(settings, 6, 0);
  lv_obj_set_flex_flow(settings, LV_FLEX_FLOW_ROW);
  lv_obj_set_flex_align(settings, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  lv_obj_add_flag(settings, LV_OBJ_FLAG_FLOATING);
  lv_obj_align(settings, LV_ALIGN_BOTTOM_LEFT, 14, -14);
  lv_obj_add_flag(settings, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_add_event_cb(settings, onSettingsPressed, LV_EVENT_CLICKED, this);

  lv_obj_t* settings_icon = lv_label_create(settings);
  lv_label_set_text(settings_icon, UiIcon::kSettings);
  lv_obj_set_style_text_font(settings_icon, &lv_font_montserrat_20, 0);
  lv_obj_set_style_text_color(settings_icon, lv_color_hex(UiTheme::kAccent), 0);

  lv_obj_t* settings_label = lv_label_create(settings);
  lv_label_set_text(settings_label, "Settings");
  lv_obj_set_style_text_font(settings_label, &lv_font_montserrat_20, 0);
  lv_obj_set_style_text_color(settings_label, lv_color_hex(UiTheme::kAccent), 0);
  colon_blink_timer_ = lv_timer_create(onColonBlinkTimer, 500, this);
}
void ClockScreen::render(const ClockAppState& s) {
  setDateText(s.date_text);
  setClockText(s.clock_text);
  setNextAlarmText(s.next_alarm_text);
}
void ClockScreen::show() {
  if (lv_obj_has_flag(root_, LV_OBJ_FLAG_HIDDEN)) {
    lv_obj_scroll_to_y(root_, 0, LV_ANIM_OFF);
    lv_obj_clear_flag(root_, LV_OBJ_FLAG_HIDDEN);
  }
}
void ClockScreen::hide() { lv_obj_add_flag(root_, LV_OBJ_FLAG_HIDDEN); }
void ClockScreen::onSettingsPressed(lv_event_t* e) {
  auto* s = static_cast<ClockScreen*>(lv_event_get_user_data(e));
  if (s) s->listener_.onSettingsRequested();
}
void ClockScreen::onColonBlinkTimer(lv_timer_t* timer) {
  auto* screen = static_cast<ClockScreen*>(lv_timer_get_user_data(timer));
  if (screen == nullptr || lv_obj_has_flag(screen->root_, LV_OBJ_FLAG_HIDDEN)) return;
  screen->colon_visible_ = !screen->colon_visible_;
  screen->renderClockText();
}
void ClockScreen::setClockText(const char* t) {
  if (!t) return;
  strncpy(clock_text_, t, sizeof(clock_text_));
  clock_text_[sizeof(clock_text_) - 1] = '\0';
  renderClockText();
}

void ClockScreen::renderClockText() {
  for (uint8_t i = 0; i < 5; ++i) {
    char c = clock_text_[i] == '\0' ? ' ' : clock_text_[i];
    if (i == 2 && c == ':' && !colon_visible_) c = ' ';
    char x[2] = {c, '\0'};
    lv_label_set_text(digits_[i], x);
  }
}

void ClockScreen::setDateText(const char* text) {
  lv_label_set_text(date_, text);
  if (text != nullptr && text[0] != '\0')
    lv_obj_clear_flag(date_, LV_OBJ_FLAG_HIDDEN);
  else
    lv_obj_add_flag(date_, LV_OBJ_FLAG_HIDDEN);
}

void ClockScreen::setNextAlarmText(const char* text) { lv_label_set_text(next_alarm_, text); }
