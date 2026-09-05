#include "screens/clock_screen.h"

#include "ui/button.h"
#include "ui/theme.h"

namespace
{
constexpr uint32_t kBlinkCycleMs = 1000;
constexpr uint32_t kBlinkVisibleMs = 850;
constexpr uint8_t kClockCharacterCount = 5;
constexpr int32_t kClockRowHeight = 72;
constexpr int32_t kClockDigitSlotWidth = 32;
constexpr int32_t kClockSeparatorSlotWidth = 16;
constexpr int32_t kClockLabelHeight = 56;
constexpr int32_t kClockDisplayWidth = (4 * kClockDigitSlotWidth) + kClockSeparatorSlotWidth;
constexpr int32_t kBottomRowWidth = 296;
constexpr int32_t kBottomRowHeight = 42;
constexpr int32_t kAlarmCheckboxSize = 24;
constexpr Ui::ButtonSize kAlarmControlButtonSize{68, 38};

int32_t slotWidthForCharacterIndex(uint8_t index)
{
  return index == 2 ? kClockSeparatorSlotWidth : kClockDigitSlotWidth;
}
} // namespace

ClockScreen::ClockScreen(ClockScreenListener &listener)
    : listener_(listener), clock_character_labels_{}, primary_button_label_(nullptr), alarm_time_label_(nullptr),
      alarm_checkbox_(nullptr) {}

void ClockScreen::build(lv_obj_t *parent)
{
  lv_obj_t *main_column = lv_obj_create(parent);
  lv_obj_remove_style_all(main_column);
  lv_obj_set_size(main_column, LV_PCT(100), LV_PCT(100));
  lv_obj_set_style_pad_top(main_column, 28, 0);
  lv_obj_set_style_pad_bottom(main_column, 20, 0);
  lv_obj_set_style_pad_left(main_column, 12, 0);
  lv_obj_set_style_pad_right(main_column, 12, 0);
  lv_obj_set_style_pad_row(main_column, 24, 0);
  lv_obj_set_flex_flow(main_column, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(main_column, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

  lv_obj_t *clock_row = lv_obj_create(main_column);
  lv_obj_remove_style_all(clock_row);
  lv_obj_set_size(clock_row, kClockDisplayWidth, kClockRowHeight);
  lv_obj_set_flex_flow(clock_row, LV_FLEX_FLOW_ROW);
  lv_obj_set_flex_align(clock_row, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

  for(uint8_t i = 0; i < kClockCharacterCount; ++i) {
    const int32_t slot_width = slotWidthForCharacterIndex(i);
    clock_character_labels_[i] = lv_label_create(clock_row);
    lv_obj_set_size(clock_character_labels_[i], slot_width, kClockLabelHeight);
    lv_obj_set_style_bg_opa(clock_character_labels_[i], LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(clock_character_labels_[i], 0, 0);
    lv_obj_set_style_pad_all(clock_character_labels_[i], 0, 0);
    lv_obj_set_style_text_font(clock_character_labels_[i], &lv_font_montserrat_48, 0);
    lv_obj_set_style_text_color(clock_character_labels_[i], lv_color_hex(UiTheme::kTextPrimary), 0);
    lv_obj_set_style_text_align(clock_character_labels_[i], LV_TEXT_ALIGN_CENTER, 0);
  }

  lv_obj_t *controls_row = lv_obj_create(main_column);
  lv_obj_remove_style_all(controls_row);
  lv_obj_set_size(controls_row, kBottomRowWidth, kBottomRowHeight);
  lv_obj_set_flex_flow(controls_row, LV_FLEX_FLOW_ROW);
  lv_obj_set_flex_align(controls_row, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

  lv_obj_t *alarm_group = lv_obj_create(controls_row);
  lv_obj_remove_style_all(alarm_group);
  lv_obj_set_size(alarm_group, LV_SIZE_CONTENT, LV_PCT(100));
  lv_obj_set_style_pad_column(alarm_group, 10, 0);
  lv_obj_set_flex_flow(alarm_group, LV_FLEX_FLOW_ROW);
  lv_obj_set_flex_align(alarm_group, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

  lv_obj_t *alarm_label = lv_label_create(alarm_group);
  lv_label_set_text(alarm_label, "Alarm:");
  lv_obj_set_style_text_font(alarm_label, &lv_font_montserrat_20, 0);
  lv_obj_set_style_text_color(alarm_label, lv_color_hex(UiTheme::kTextPrimary), 0);

  alarm_time_label_ = lv_label_create(alarm_group);
  lv_label_set_text(alarm_time_label_, "--:--");
  lv_obj_set_style_text_font(alarm_time_label_, &lv_font_montserrat_20, 0);
  lv_obj_set_style_text_color(alarm_time_label_, lv_color_hex(UiTheme::kTextPrimary), 0);

  const Ui::ButtonElements primary_button =
      Ui::createButton(alarm_group, "", UiTheme::kPrimaryButtonStyle, kAlarmControlButtonSize,
                       onPrimaryButtonPressed, this);
  primary_button_label_ = primary_button.label;

  lv_obj_t *enabled_group = lv_obj_create(controls_row);
  lv_obj_remove_style_all(enabled_group);
  lv_obj_set_size(enabled_group, LV_SIZE_CONTENT, LV_PCT(100));
  lv_obj_set_style_pad_column(enabled_group, 10, 0);
  lv_obj_set_flex_flow(enabled_group, LV_FLEX_FLOW_ROW);
  lv_obj_set_flex_align(enabled_group, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

  lv_obj_t *on_label = lv_label_create(enabled_group);
  lv_label_set_text(on_label, "On:");
  lv_obj_set_style_text_font(on_label, &lv_font_montserrat_20, 0);
  lv_obj_set_style_text_color(on_label, lv_color_hex(UiTheme::kTextPrimary), 0);

  alarm_checkbox_ = lv_checkbox_create(enabled_group);
  lv_checkbox_set_text(alarm_checkbox_, "");
  lv_obj_add_event_cb(alarm_checkbox_, onToggleAlarmPressed, LV_EVENT_VALUE_CHANGED, this);
  lv_obj_set_style_pad_all(alarm_checkbox_, 0, 0);
  lv_obj_set_style_pad_column(alarm_checkbox_, 0, 0);
  lv_obj_set_style_text_color(alarm_checkbox_, lv_color_hex(UiTheme::kTextPrimary), 0);
  lv_obj_set_size(alarm_checkbox_, kAlarmCheckboxSize, kAlarmCheckboxSize);
  lv_obj_set_style_width(alarm_checkbox_, kAlarmCheckboxSize, LV_PART_INDICATOR);
  lv_obj_set_style_height(alarm_checkbox_, kAlarmCheckboxSize, LV_PART_INDICATOR);
  lv_obj_set_style_radius(alarm_checkbox_, 6, LV_PART_INDICATOR);
  lv_obj_set_style_border_color(alarm_checkbox_, lv_color_hex(UiTheme::kTextPrimary), LV_PART_INDICATOR);
  lv_obj_set_style_border_width(alarm_checkbox_, 2, LV_PART_INDICATOR);
  lv_obj_set_style_bg_color(alarm_checkbox_, lv_color_hex(UiTheme::kSurfaceRaised), LV_PART_INDICATOR);
  lv_obj_set_style_bg_color(alarm_checkbox_, lv_color_hex(UiTheme::kAccent), LV_PART_INDICATOR | LV_STATE_CHECKED);
  lv_obj_set_style_bg_opa(alarm_checkbox_, LV_OPA_COVER, LV_PART_INDICATOR | LV_STATE_CHECKED);
}

void ClockScreen::render(const ClockAppState &state)
{
  setClockText(state.clock_text);
  lv_label_set_text(alarm_time_label_, state.alarm_text);
  lv_label_set_text(primary_button_label_, state.primary_button_text);

  if(state.alarm_enabled) {
    lv_obj_add_state(alarm_checkbox_, LV_STATE_CHECKED);
  }
  else {
    lv_obj_remove_state(alarm_checkbox_, LV_STATE_CHECKED);
  }
}

void ClockScreen::onPrimaryButtonPressed(lv_event_t *event)
{
  auto *screen = static_cast<ClockScreen *>(lv_event_get_user_data(event));
  if(screen != nullptr) {
    screen->listener_.onClockScreenPrimaryAction();
  }
}

void ClockScreen::onToggleAlarmPressed(lv_event_t *event)
{
  auto *screen = static_cast<ClockScreen *>(lv_event_get_user_data(event));
  if(screen != nullptr) {
    screen->listener_.onClockScreenAlarmToggled();
  }
}

void ClockScreen::setClockText(const char *text)
{
  if(text == nullptr) {
    return;
  }

  const bool blink_separator_visible = (lv_tick_get() % kBlinkCycleMs) < kBlinkVisibleMs;
  for(uint8_t i = 0; i < kClockCharacterCount; ++i) {
    char current_character = text[i] == '\0' ? ' ' : text[i];
    if(i == 2 && current_character == ':' && !blink_separator_visible) {
      current_character = ' ';
    }

    char slot_text[2] = {current_character, '\0'};
    lv_label_set_text(clock_character_labels_[i], slot_text);
  }
}
