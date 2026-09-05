#include "screens/alarm_editor_view.h"

#include "ui/button.h"
#include "ui/theme.h"

namespace {
constexpr int32_t kEditorDialogWidth = 288;
constexpr int32_t kEditorDialogHeight = 220;
constexpr int32_t kEditorRollerWidth = 104;
constexpr int32_t kEditorRollerHeight = 96;
constexpr Ui::ButtonSize kEditorActionButtonSize{96, 42};
constexpr char kHourOptions[] =
    "00\n01\n02\n03\n04\n05\n06\n07\n08\n09\n10\n11\n12\n13\n14\n15\n16\n17\n18\n19\n20\n21\n22\n23";
constexpr char kMinuteOptions[] =
    "00\n01\n02\n03\n04\n05\n06\n07\n08\n09\n10\n11\n12\n13\n14\n15\n16\n17\n18\n19\n20\n21\n22\n23\n24\n25\n26\n"
    "27\n28\n29\n30\n31\n32\n33\n34\n35\n36\n37\n38\n39\n40\n41\n42\n43\n44\n45\n46\n47\n48\n49\n50\n51\n52\n53\n"
    "54\n55\n56\n57\n58\n59";
}  // namespace

AlarmEditorView::AlarmEditorView(AlarmEditorViewListener& listener)
    : listener_(listener), overlay_(nullptr), hour_roller_(nullptr), minute_roller_(nullptr) {}

void AlarmEditorView::build(lv_obj_t* parent) {
  overlay_ = lv_obj_create(parent);
  lv_obj_remove_style_all(overlay_);
  lv_obj_set_size(overlay_, LV_PCT(100), LV_PCT(100));
  lv_obj_set_style_bg_color(overlay_, lv_color_hex(UiTheme::kSurfaceRaised), 0);
  lv_obj_set_style_bg_opa(overlay_, LV_OPA_COVER, 0);
  lv_obj_set_style_pad_all(overlay_, 0, 0);
  lv_obj_set_style_border_width(overlay_, 0, 0);
  lv_obj_set_style_radius(overlay_, 0, 0);
  lv_obj_add_flag(overlay_, LV_OBJ_FLAG_HIDDEN);

  lv_obj_t* dialog = lv_obj_create(overlay_);
  lv_obj_set_size(dialog, kEditorDialogWidth, kEditorDialogHeight);
  lv_obj_center(dialog);
  lv_obj_set_style_bg_color(dialog, lv_color_hex(UiTheme::kSurfaceRaised), 0);
  lv_obj_set_style_border_width(dialog, 0, 0);
  lv_obj_set_style_radius(dialog, 24, 0);
  lv_obj_set_style_pad_top(dialog, 8, 0);
  lv_obj_set_style_pad_bottom(dialog, 8, 0);
  lv_obj_set_style_pad_left(dialog, 18, 0);
  lv_obj_set_style_pad_right(dialog, 18, 0);
  lv_obj_set_style_pad_row(dialog, 16, 0);
  lv_obj_set_flex_flow(dialog, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(dialog, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

  lv_obj_t* title = lv_label_create(dialog);
  lv_label_set_text(title, "Set Alarm Time");
  lv_obj_set_style_text_font(title, &lv_font_montserrat_20, 0);
  lv_obj_set_style_text_color(title, lv_color_hex(UiTheme::kTextPrimary), 0);

  lv_obj_t* rollers_row = lv_obj_create(dialog);
  lv_obj_remove_style_all(rollers_row);
  lv_obj_set_size(rollers_row, LV_SIZE_CONTENT, kEditorRollerHeight);
  lv_obj_set_style_pad_column(rollers_row, 12, 0);
  lv_obj_set_flex_flow(rollers_row, LV_FLEX_FLOW_ROW);
  lv_obj_set_flex_align(rollers_row, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

  hour_roller_ = lv_roller_create(rollers_row);
  lv_roller_set_options(hour_roller_, kHourOptions, LV_ROLLER_MODE_INFINITE);
  lv_obj_set_size(hour_roller_, kEditorRollerWidth, kEditorRollerHeight);
  lv_obj_set_style_text_font(hour_roller_, &lv_font_montserrat_20, LV_PART_MAIN);
  lv_obj_set_style_text_font(hour_roller_, &lv_font_montserrat_20, LV_PART_SELECTED);
  lv_obj_set_style_text_line_space(hour_roller_, 1, LV_PART_MAIN);

  lv_obj_t* colon_label = lv_label_create(rollers_row);
  lv_label_set_text(colon_label, ":");
  lv_obj_set_style_text_font(colon_label, &lv_font_montserrat_28, 0);
  lv_obj_set_style_text_color(colon_label, lv_color_hex(UiTheme::kTextPrimary), 0);

  minute_roller_ = lv_roller_create(rollers_row);
  lv_roller_set_options(minute_roller_, kMinuteOptions, LV_ROLLER_MODE_INFINITE);
  lv_obj_set_size(minute_roller_, kEditorRollerWidth, kEditorRollerHeight);
  lv_obj_set_style_text_font(minute_roller_, &lv_font_montserrat_20, LV_PART_MAIN);
  lv_obj_set_style_text_font(minute_roller_, &lv_font_montserrat_20, LV_PART_SELECTED);
  lv_obj_set_style_text_line_space(minute_roller_, 1, LV_PART_MAIN);

  lv_obj_t* actions_row = lv_obj_create(dialog);
  lv_obj_remove_style_all(actions_row);
  lv_obj_set_size(actions_row, LV_PCT(100), LV_SIZE_CONTENT);
  lv_obj_set_flex_flow(actions_row, LV_FLEX_FLOW_ROW);
  lv_obj_set_flex_align(actions_row, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

  Ui::createButton(actions_row, "Set", UiTheme::kPrimaryButtonStyle, kEditorActionButtonSize, onApplyButtonPressed,
                   this);
  Ui::createButton(actions_row, "Cancel", UiTheme::kSecondaryButtonStyle, kEditorActionButtonSize,
                   onCancelButtonPressed, this);
}

void AlarmEditorView::show() { lv_obj_clear_flag(overlay_, LV_OBJ_FLAG_HIDDEN); }

void AlarmEditorView::hide() { lv_obj_add_flag(overlay_, LV_OBJ_FLAG_HIDDEN); }

void AlarmEditorView::setSelection(uint8_t hour, uint8_t minute) {
  lv_roller_set_selected(hour_roller_, hour, LV_ANIM_OFF);
  lv_roller_set_selected(minute_roller_, minute, LV_ANIM_OFF);
}

void AlarmEditorView::onApplyButtonPressed(lv_event_t* event) {
  auto* view = static_cast<AlarmEditorView*>(lv_event_get_user_data(event));
  if (view != nullptr) {
    view->listener_.onAlarmEditorApplied(static_cast<uint8_t>(lv_roller_get_selected(view->hour_roller_)),
                                         static_cast<uint8_t>(lv_roller_get_selected(view->minute_roller_)));
  }
}

void AlarmEditorView::onCancelButtonPressed(lv_event_t* event) {
  auto* view = static_cast<AlarmEditorView*>(lv_event_get_user_data(event));
  if (view != nullptr) {
    view->listener_.onAlarmEditorCancelled();
  }
}
