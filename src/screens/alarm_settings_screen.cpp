#include "screens/alarm_settings_screen.h"

#include "alarm_service.h"
#include "ui/button.h"
#include "ui/icons.h"
#include "ui/page_header.h"
#include "ui/theme.h"

namespace {
constexpr int32_t kPagePadding = 14;
constexpr int32_t kRollerWidth = 104;
constexpr int32_t kRollerHeight = 82;
constexpr Ui::ButtonSize kHeaderSaveButtonSize{60, 32};
constexpr Ui::ButtonSize kDayButtonSize{34, 30};
constexpr char kHourOptions[] =
    "00\n01\n02\n03\n04\n05\n06\n07\n08\n09\n10\n11\n12\n13\n14\n15\n16\n17\n18\n19\n20\n21\n22\n23";
constexpr char kMinuteOptions[] =
    "00\n01\n02\n03\n04\n05\n06\n07\n08\n09\n10\n11\n12\n13\n14\n15\n16\n17\n18\n19\n20\n21\n22\n23\n24\n25\n26\n"
    "27\n28\n29\n30\n31\n32\n33\n34\n35\n36\n37\n38\n39\n40\n41\n42\n43\n44\n45\n46\n47\n48\n49\n50\n51\n52\n53\n"
    "54\n55\n56\n57\n58\n59";
constexpr const char* kWeekdayLabels[] = {"S", "M", "T", "W", "T", "F", "S"};
}  // namespace

AlarmSettingsScreen::AlarmSettingsScreen(AlarmSettingsScreenListener& listener)
    : listener_(listener),
      root_(nullptr),
      title_(nullptr),
      hour_roller_(nullptr),
      minute_roller_(nullptr),
      every_day_switch_(nullptr),
      weekday_buttons_{} {}

void AlarmSettingsScreen::build(lv_obj_t* parent) {
  root_ = lv_obj_create(parent);
  lv_obj_remove_style_all(root_);
  lv_obj_set_size(root_, LV_PCT(100), LV_PCT(100));
  lv_obj_set_style_bg_color(root_, lv_color_hex(UiTheme::kSurfaceRaised), 0);
  lv_obj_set_style_bg_opa(root_, LV_OPA_COVER, 0);
  lv_obj_set_style_pad_all(root_, kPagePadding, 0);
  lv_obj_set_style_pad_row(root_, 10, 0);
  lv_obj_set_flex_flow(root_, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(root_, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  lv_obj_set_scroll_dir(root_, LV_DIR_VER);
  lv_obj_add_flag(root_, LV_OBJ_FLAG_HIDDEN);

  const Ui::PageHeaderElements header =
      Ui::createPageHeader(root_, {"Alarm settings", UiIcon::kBackLabel, onBackPressed, this, "Save",
                                   &UiTheme::kPrimaryButtonStyle, kHeaderSaveButtonSize, onApplyButtonPressed, this});
  title_ = header.title;

  lv_obj_t* rollers_row = lv_obj_create(root_);
  lv_obj_remove_style_all(rollers_row);
  lv_obj_set_size(rollers_row, LV_SIZE_CONTENT, kRollerHeight);
  lv_obj_set_style_pad_column(rollers_row, 12, 0);
  lv_obj_set_flex_flow(rollers_row, LV_FLEX_FLOW_ROW);
  lv_obj_set_flex_align(rollers_row, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

  hour_roller_ = lv_roller_create(rollers_row);
  lv_roller_set_options(hour_roller_, kHourOptions, LV_ROLLER_MODE_INFINITE);
  lv_obj_set_size(hour_roller_, kRollerWidth, kRollerHeight);
  lv_obj_set_style_text_font(hour_roller_, &lv_font_montserrat_20, LV_PART_MAIN);
  lv_obj_set_style_text_font(hour_roller_, &lv_font_montserrat_20, LV_PART_SELECTED);
  lv_obj_set_style_text_line_space(hour_roller_, 1, LV_PART_MAIN);

  lv_obj_t* colon_label = lv_label_create(rollers_row);
  lv_label_set_text(colon_label, ":");
  lv_obj_set_style_text_font(colon_label, &lv_font_montserrat_28, 0);
  lv_obj_set_style_text_color(colon_label, lv_color_hex(UiTheme::kTextPrimary), 0);

  minute_roller_ = lv_roller_create(rollers_row);
  lv_roller_set_options(minute_roller_, kMinuteOptions, LV_ROLLER_MODE_INFINITE);
  lv_obj_set_size(minute_roller_, kRollerWidth, kRollerHeight);
  lv_obj_set_style_text_font(minute_roller_, &lv_font_montserrat_20, LV_PART_MAIN);
  lv_obj_set_style_text_font(minute_roller_, &lv_font_montserrat_20, LV_PART_SELECTED);
  lv_obj_set_style_text_line_space(minute_roller_, 1, LV_PART_MAIN);

  lv_obj_t* repeat_header = lv_obj_create(root_);
  lv_obj_remove_style_all(repeat_header);
  lv_obj_set_size(repeat_header, LV_PCT(100), LV_SIZE_CONTENT);
  lv_obj_set_style_pad_bottom(repeat_header, 4, 0);
  lv_obj_set_flex_flow(repeat_header, LV_FLEX_FLOW_ROW);
  lv_obj_set_flex_align(repeat_header, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  lv_obj_t* repeat_label = lv_label_create(repeat_header);
  lv_label_set_text(repeat_label, "Repeat every day");
  lv_obj_set_style_text_font(repeat_label, &lv_font_montserrat_16, 0);
  every_day_switch_ = lv_switch_create(repeat_header);
  lv_obj_add_event_cb(every_day_switch_, onEveryDayChanged, LV_EVENT_VALUE_CHANGED, this);

  lv_obj_t* weekdays_row = lv_obj_create(root_);
  lv_obj_remove_style_all(weekdays_row);
  lv_obj_set_size(weekdays_row, LV_PCT(100), LV_SIZE_CONTENT);
  lv_obj_set_style_pad_column(weekdays_row, 5, 0);
  lv_obj_set_flex_flow(weekdays_row, LV_FLEX_FLOW_ROW);
  lv_obj_set_flex_align(weekdays_row, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  for (uint8_t weekday = 0; weekday < 7; ++weekday) {
    Ui::ButtonElements day = Ui::createButton(weekdays_row, kWeekdayLabels[weekday], UiTheme::kSecondaryButtonStyle,
                                              kDayButtonSize, onWeekdayPressed, this);
    weekday_buttons_[weekday] = day.button;
    lv_obj_add_flag(day.button, LV_OBJ_FLAG_CHECKABLE);
    lv_obj_set_style_bg_color(day.button, lv_color_hex(UiTheme::kAccent), LV_STATE_CHECKED);
    lv_obj_set_style_text_color(day.button, lv_color_hex(UiTheme::kTextOnAccent), LV_STATE_CHECKED);
  }
}

void AlarmSettingsScreen::show() {
  if (lv_obj_has_flag(root_, LV_OBJ_FLAG_HIDDEN)) {
    lv_obj_scroll_to_y(root_, 0, LV_ANIM_OFF);
    lv_obj_clear_flag(root_, LV_OBJ_FLAG_HIDDEN);
  }
}
void AlarmSettingsScreen::hide() { lv_obj_add_flag(root_, LV_OBJ_FLAG_HIDDEN); }

void AlarmSettingsScreen::setTitle(const char* title) { lv_label_set_text(title_, title); }

void AlarmSettingsScreen::setSelection(uint8_t hour, uint8_t minute, uint8_t weekday_mask) {
  lv_roller_set_selected(hour_roller_, hour, LV_ANIM_OFF);
  lv_roller_set_selected(minute_roller_, minute, LV_ANIM_OFF);
  for (uint8_t weekday = 0; weekday < 7; ++weekday) setWeekdaySelected(weekday, (weekday_mask & (1u << weekday)) != 0);
  if (weekday_mask == AlarmService::kEveryDayMask)
    lv_obj_add_state(every_day_switch_, LV_STATE_CHECKED);
  else
    lv_obj_clear_state(every_day_switch_, LV_STATE_CHECKED);
}

void AlarmSettingsScreen::onApplyButtonPressed(lv_event_t* event) {
  auto* view = static_cast<AlarmSettingsScreen*>(lv_event_get_user_data(event));
  if (view != nullptr)
    view->listener_.onAlarmSettingsApplied(static_cast<uint8_t>(lv_roller_get_selected(view->hour_roller_)),
                                           static_cast<uint8_t>(lv_roller_get_selected(view->minute_roller_)),
                                           view->selectedWeekdayMask());
}
void AlarmSettingsScreen::onBackPressed(lv_event_t* event) {
  auto* view = static_cast<AlarmSettingsScreen*>(lv_event_get_user_data(event));
  if (view != nullptr) view->listener_.onAlarmSettingsBackRequested();
}
void AlarmSettingsScreen::onEveryDayChanged(lv_event_t* event) {
  auto* view = static_cast<AlarmSettingsScreen*>(lv_event_get_user_data(event));
  if (view == nullptr) return;

  const bool every_day_selected = lv_obj_has_state(view->every_day_switch_, LV_STATE_CHECKED);
  for (uint8_t weekday = 0; weekday < 7; ++weekday) {
    view->setWeekdaySelected(weekday, every_day_selected);
  }
}
void AlarmSettingsScreen::onWeekdayPressed(lv_event_t* event) {
  auto* view = static_cast<AlarmSettingsScreen*>(lv_event_get_user_data(event));
  if (view == nullptr) return;
  if (view->selectedWeekdayMask() == AlarmService::kEveryDayMask)
    lv_obj_add_state(view->every_day_switch_, LV_STATE_CHECKED);
  else
    lv_obj_clear_state(view->every_day_switch_, LV_STATE_CHECKED);
}
uint8_t AlarmSettingsScreen::selectedWeekdayMask() const {
  uint8_t mask = 0;
  for (uint8_t weekday = 0; weekday < 7; ++weekday)
    if (lv_obj_has_state(weekday_buttons_[weekday], LV_STATE_CHECKED)) mask |= static_cast<uint8_t>(1u << weekday);
  return mask;
}
void AlarmSettingsScreen::setWeekdaySelected(uint8_t weekday, bool selected) {
  if (selected)
    lv_obj_add_state(weekday_buttons_[weekday], LV_STATE_CHECKED);
  else
    lv_obj_clear_state(weekday_buttons_[weekday], LV_STATE_CHECKED);
}
