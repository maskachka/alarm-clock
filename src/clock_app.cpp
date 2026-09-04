#include "clock_app.h"

#include "clock_formatter.h"

namespace
{
  constexpr uint32_t kRefreshPeriodMs = 50;
  constexpr uint32_t kBlinkCycleMs = 1000;
  constexpr uint32_t kBlinkVisibleMs = 850;
  constexpr uint8_t kClockCharacterCount = 5;
  constexpr int32_t kMainContentWidth = 296;
  constexpr int32_t kClockRowHeight = 72;
  constexpr int32_t kClockDigitSlotWidth = 32;
  constexpr int32_t kClockSeparatorSlotWidth = 16;
  constexpr int32_t kClockLabelHeight = 56;
  constexpr int32_t kClockDisplayWidth = (4 * kClockDigitSlotWidth) + kClockSeparatorSlotWidth;
  constexpr int32_t kBottomRowWidth = 296;
  constexpr int32_t kBottomRowHeight = 42;
  constexpr int32_t kAlarmCheckboxSize = 24;
  constexpr int32_t kEditorDialogWidth = 288;
  constexpr int32_t kEditorDialogHeight = 220;
  constexpr int32_t kEditorRollerWidth = 104;
  constexpr int32_t kEditorRollerHeight = 108;

  struct ButtonStyle
  {
    uint32_t background_hex;
    uint32_t text_hex;
    int32_t radius;
  };

  struct ButtonSize
  {
    int32_t width;
    int32_t height;
  };

  struct ButtonElements
  {
    lv_obj_t *button;
    lv_obj_t *label;
  };

  constexpr ButtonStyle kPrimaryButtonStyle{0x2D8CF0, 0xFFFFFF, 14};
  constexpr ButtonStyle kSecondaryButtonStyle{0xD9E2EC, 0x1F2933, 14};
  constexpr ButtonSize kAlarmControlButtonSize{68, 38};
  constexpr ButtonSize kEditorActionButtonSize{96, 42};
  constexpr char kHourOptions[] =
      "00\n01\n02\n03\n04\n05\n06\n07\n08\n09\n10\n11\n12\n13\n14\n15\n16\n17\n18\n19\n20\n21\n22\n23";
  constexpr char kMinuteOptions[] =
      "00\n01\n02\n03\n04\n05\n06\n07\n08\n09\n10\n11\n12\n13\n14\n15\n16\n17\n18\n19\n20\n21\n22\n23\n24\n25\n26\n"
      "27\n28\n29\n30\n31\n32\n33\n34\n35\n36\n37\n38\n39\n40\n41\n42\n43\n44\n45\n46\n47\n48\n49\n50\n51\n52\n53\n"
      "54\n55\n56\n57\n58\n59";

  constexpr int32_t slotWidthForCharacterIndex(uint8_t index)
  {
    return index == 2 ? kClockSeparatorSlotWidth : kClockDigitSlotWidth;
  }

  ButtonElements createButton(lv_obj_t *parent, const char *text, const ButtonStyle &style, const ButtonSize &size,
                               lv_event_cb_t click_handler, void *event_user_data)
  {
    lv_obj_t *button = lv_button_create(parent);
    lv_obj_set_size(button, size.width, size.height);
    lv_obj_set_style_radius(button, style.radius, 0);
    lv_obj_set_style_bg_color(button, lv_color_hex(style.background_hex), 0);
    lv_obj_set_style_text_color(button, lv_color_hex(style.text_hex), 0);
    lv_obj_add_event_cb(button, click_handler, LV_EVENT_CLICKED, event_user_data);

    lv_obj_t *label = lv_label_create(button);
    lv_label_set_text(label, text);
    lv_obj_set_style_text_color(label, lv_color_hex(style.text_hex), 0);
    lv_obj_center(label);

    return {button, label};
  }
}

ClockApp::ClockApp(ClockService &clock_service, AlarmService &alarm_service, AlarmBuzzer &alarm_buzzer)
    : clock_service_(clock_service),
      alarm_buzzer_(alarm_buzzer),
      controller_(alarm_service),
      clock_character_labels_{},
      primary_button_(nullptr),
      primary_button_label_(nullptr),
      alarm_time_label_(nullptr),
      alarm_checkbox_(nullptr),
      alarm_editor_overlay_(nullptr),
      alarm_hour_roller_(nullptr),
      alarm_minute_roller_(nullptr),
      refresh_timer_(nullptr),
      buzzer_active_(false) {}

void ClockApp::build()
{
  alarm_buzzer_.begin();

  lv_obj_set_style_bg_color(lv_screen_active(), lv_color_hex(0xF3F0E8), 0);
  lv_obj_set_style_text_color(lv_screen_active(), lv_color_hex(0x1F2933), 0);

  lv_obj_t *main_column = lv_obj_create(lv_screen_active());
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

  for (uint8_t i = 0; i < kClockCharacterCount; ++i)
  {
    const int32_t slot_width = slotWidthForCharacterIndex(i);
    clock_character_labels_[i] = lv_label_create(clock_row);
    lv_obj_set_size(clock_character_labels_[i], slot_width, kClockLabelHeight);
    lv_obj_set_style_bg_opa(clock_character_labels_[i], LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(clock_character_labels_[i], 0, 0);
    lv_obj_set_style_pad_all(clock_character_labels_[i], 0, 0);
    lv_obj_set_style_text_font(clock_character_labels_[i], &lv_font_montserrat_48, 0);
    lv_obj_set_style_text_color(clock_character_labels_[i], lv_color_hex(0x1F2933), 0);
    lv_obj_set_style_text_align(clock_character_labels_[i], LV_TEXT_ALIGN_CENTER, 0);
  }
  setClockDisplayText("--:--");

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
  lv_obj_set_style_text_color(alarm_label, lv_color_hex(0x1F2933), 0);

  alarm_time_label_ = lv_label_create(alarm_group);
  lv_label_set_text(alarm_time_label_, "--:--");
  lv_obj_set_style_text_font(alarm_time_label_, &lv_font_montserrat_20, 0);
  lv_obj_set_style_text_color(alarm_time_label_, lv_color_hex(0x1F2933), 0);

  const ButtonElements primary_button = createButton(alarm_group, "", kPrimaryButtonStyle, kAlarmControlButtonSize,
                                                      onPrimaryButtonPressed, this);
  primary_button_ = primary_button.button;
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
  lv_obj_set_style_text_color(on_label, lv_color_hex(0x1F2933), 0);

  alarm_checkbox_ = lv_checkbox_create(enabled_group);
  lv_checkbox_set_text(alarm_checkbox_, "");
  lv_obj_add_event_cb(alarm_checkbox_, onToggleAlarmPressed, LV_EVENT_VALUE_CHANGED, this);
  lv_obj_set_style_pad_all(alarm_checkbox_, 0, 0);
  lv_obj_set_style_pad_column(alarm_checkbox_, 0, 0);
  lv_obj_set_style_text_color(alarm_checkbox_, lv_color_hex(0x1F2933), 0);
  lv_obj_set_size(alarm_checkbox_, kAlarmCheckboxSize, kAlarmCheckboxSize);
  lv_obj_set_style_width(alarm_checkbox_, kAlarmCheckboxSize, LV_PART_INDICATOR);
  lv_obj_set_style_height(alarm_checkbox_, kAlarmCheckboxSize, LV_PART_INDICATOR);
  lv_obj_set_style_radius(alarm_checkbox_, 6, LV_PART_INDICATOR);
  lv_obj_set_style_border_color(alarm_checkbox_, lv_color_hex(0x1F2933), LV_PART_INDICATOR);
  lv_obj_set_style_border_width(alarm_checkbox_, 2, LV_PART_INDICATOR);
  lv_obj_set_style_bg_color(alarm_checkbox_, lv_color_hex(0xF7F3EA), LV_PART_INDICATOR);
  lv_obj_set_style_bg_color(alarm_checkbox_, lv_color_hex(0x2D8CF0), LV_PART_INDICATOR | LV_STATE_CHECKED);
  lv_obj_set_style_bg_opa(alarm_checkbox_, LV_OPA_COVER, LV_PART_INDICATOR | LV_STATE_CHECKED);

  alarm_editor_overlay_ = lv_obj_create(lv_screen_active());
  lv_obj_remove_style_all(alarm_editor_overlay_);
  lv_obj_set_size(alarm_editor_overlay_, LV_PCT(100), LV_PCT(100));
  lv_obj_set_style_bg_color(alarm_editor_overlay_, lv_color_hex(0xF7F3EA), 0);
  lv_obj_set_style_bg_opa(alarm_editor_overlay_, LV_OPA_COVER, 0);
  lv_obj_set_style_pad_all(alarm_editor_overlay_, 0, 0);
  lv_obj_set_style_border_width(alarm_editor_overlay_, 0, 0);
  lv_obj_set_style_radius(alarm_editor_overlay_, 0, 0);
  lv_obj_add_flag(alarm_editor_overlay_, LV_OBJ_FLAG_HIDDEN);

  lv_obj_t *editor_dialog = lv_obj_create(alarm_editor_overlay_);
  lv_obj_set_size(editor_dialog, kEditorDialogWidth, kEditorDialogHeight);
  lv_obj_center(editor_dialog);
  lv_obj_set_style_bg_color(editor_dialog, lv_color_hex(0xF7F3EA), 0);
  lv_obj_set_style_border_width(editor_dialog, 0, 0);
  lv_obj_set_style_radius(editor_dialog, 24, 0);
  lv_obj_set_style_pad_top(editor_dialog, 8, 0);
  lv_obj_set_style_pad_bottom(editor_dialog, 18, 0);
  lv_obj_set_style_pad_left(editor_dialog, 18, 0);
  lv_obj_set_style_pad_right(editor_dialog, 18, 0);
  lv_obj_set_style_pad_row(editor_dialog, 16, 0);
  lv_obj_set_flex_flow(editor_dialog, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(editor_dialog, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

  lv_obj_t *editor_title = lv_label_create(editor_dialog);
  lv_label_set_text(editor_title, "Set Alarm Time");
  lv_obj_set_style_text_font(editor_title, &lv_font_montserrat_20, 0);
  lv_obj_set_style_text_color(editor_title, lv_color_hex(0x1F2933), 0);

  lv_obj_t *rollers_row = lv_obj_create(editor_dialog);
  lv_obj_remove_style_all(rollers_row);
  lv_obj_set_size(rollers_row, LV_SIZE_CONTENT, kEditorRollerHeight);
  lv_obj_set_style_pad_column(rollers_row, 12, 0);
  lv_obj_set_flex_flow(rollers_row, LV_FLEX_FLOW_ROW);
  lv_obj_set_flex_align(rollers_row, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

  alarm_hour_roller_ = lv_roller_create(rollers_row);
  lv_roller_set_options(alarm_hour_roller_, kHourOptions, LV_ROLLER_MODE_INFINITE);
  lv_obj_set_size(alarm_hour_roller_, kEditorRollerWidth, kEditorRollerHeight);
  lv_obj_set_style_text_font(alarm_hour_roller_, &lv_font_montserrat_20, LV_PART_MAIN);
  lv_obj_set_style_text_font(alarm_hour_roller_, &lv_font_montserrat_20, LV_PART_SELECTED);
  lv_obj_set_style_text_line_space(alarm_hour_roller_, 1, LV_PART_MAIN);

  lv_obj_t *colon_label = lv_label_create(rollers_row);
  lv_label_set_text(colon_label, ":");
  lv_obj_set_style_text_font(colon_label, &lv_font_montserrat_28, 0);
  lv_obj_set_style_text_color(colon_label, lv_color_hex(0x1F2933), 0);

  alarm_minute_roller_ = lv_roller_create(rollers_row);
  lv_roller_set_options(alarm_minute_roller_, kMinuteOptions, LV_ROLLER_MODE_INFINITE);
  lv_obj_set_size(alarm_minute_roller_, kEditorRollerWidth, kEditorRollerHeight);
  lv_obj_set_style_text_font(alarm_minute_roller_, &lv_font_montserrat_20, LV_PART_MAIN);
  lv_obj_set_style_text_font(alarm_minute_roller_, &lv_font_montserrat_20, LV_PART_SELECTED);
  lv_obj_set_style_text_line_space(alarm_minute_roller_, 1, LV_PART_MAIN);

  lv_obj_t *actions_row = lv_obj_create(editor_dialog);
  lv_obj_remove_style_all(actions_row);
  lv_obj_set_size(actions_row, LV_PCT(100), LV_SIZE_CONTENT);
  lv_obj_set_flex_flow(actions_row, LV_FLEX_FLOW_ROW);
  lv_obj_set_flex_align(actions_row, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

  createButton(actions_row, "Set", kPrimaryButtonStyle, kEditorActionButtonSize, onApplyAlarmPressed, this);
  createButton(actions_row, "Cancel", kSecondaryButtonStyle, kEditorActionButtonSize, onCancelAlarmPressed, this);

  refresh_timer_ = lv_timer_create(onRefreshTimer, kRefreshPeriodMs, this);
  controller_.initialize();
  applyControllerState();
  refresh();
}

void ClockApp::onPrimaryButtonPressed(lv_event_t *event)
{
  auto *app = static_cast<ClockApp *>(lv_event_get_user_data(event));
  if (app == nullptr)
  {
    return;
  }

  app->applyControllerEffects(app->controller_.onPrimaryButtonPressed());
  app->applyControllerState();
}

void ClockApp::onToggleAlarmPressed(lv_event_t *event)
{
  auto *app = static_cast<ClockApp *>(lv_event_get_user_data(event));
  if (app == nullptr)
  {
    return;
  }

  app->applyControllerEffects(app->controller_.onToggleAlarmPressed());
  app->applyControllerState();
}

void ClockApp::onApplyAlarmPressed(lv_event_t *event)
{
  auto *app = static_cast<ClockApp *>(lv_event_get_user_data(event));
  if (app == nullptr)
  {
    return;
  }

  app->applyAlarmFromEditor();
}

void ClockApp::onCancelAlarmPressed(lv_event_t *event)
{
  auto *app = static_cast<ClockApp *>(lv_event_get_user_data(event));
  if (app == nullptr)
  {
    return;
  }

  app->applyControllerEffects(app->controller_.onCancelAlarmPressed());
  app->applyControllerState();
}

void ClockApp::onRefreshTimer(lv_timer_t *timer)
{
  auto *app = static_cast<ClockApp *>(lv_timer_get_user_data(timer));
  if (app == nullptr)
  {
    return;
  }

  app->refresh();
}

void ClockApp::refresh()
{
  ClockTime now;
  if (!clock_service_.getCurrentTime(now))
  {
    applyControllerEffects(controller_.refresh(false, ClockTime{0, 0, 0}));
    applyControllerState();
    return;
  }

  applyControllerEffects(controller_.refresh(true, now));
  applyControllerState();
}

void ClockApp::applyAlarmFromEditor()
{
  const uint16_t selected_hour = lv_roller_get_selected(alarm_hour_roller_);
  const uint16_t selected_minute = lv_roller_get_selected(alarm_minute_roller_);
  applyControllerEffects(
      controller_.onApplyAlarmPressed(static_cast<uint8_t>(selected_hour), static_cast<uint8_t>(selected_minute)));
  applyControllerState();
}

void ClockApp::applyControllerState()
{
  const ClockAppState &state = controller_.state();
  setClockDisplayText(state.clock_text);
  lv_label_set_text(alarm_time_label_, state.alarm_text);
  lv_label_set_text(primary_button_label_, state.primary_button_text);
  if (state.alarm_enabled)
  {
    lv_obj_add_state(alarm_checkbox_, LV_STATE_CHECKED);
  }
  else
  {
    lv_obj_remove_state(alarm_checkbox_, LV_STATE_CHECKED);
  }

  if (state.editor_visible)
  {
    lv_obj_clear_flag(alarm_editor_overlay_, LV_OBJ_FLAG_HIDDEN);
  }
  else
  {
    lv_obj_add_flag(alarm_editor_overlay_, LV_OBJ_FLAG_HIDDEN);
  }
}

void ClockApp::applyControllerEffects(const ClockAppEffects &effects)
{
  if (effects.sync_editor_selection)
  {
    lv_roller_set_selected(alarm_hour_roller_, effects.editor_hour, LV_ANIM_OFF);
    lv_roller_set_selected(alarm_minute_roller_, effects.editor_minute, LV_ANIM_OFF);
  }

  if (effects.start_buzzer)
  {
    alarm_buzzer_.start();
    buzzer_active_ = true;
  }

  if (effects.update_buzzer)
  {
    alarm_buzzer_.update();
  }

  if (effects.stop_buzzer && buzzer_active_)
  {
    alarm_buzzer_.stop();
    buzzer_active_ = false;
  }
}

void ClockApp::setClockDisplayText(const char *text)
{
  if (text == nullptr)
  {
    return;
  }

  const bool blink_separator_visible = (lv_tick_get() % kBlinkCycleMs) < kBlinkVisibleMs;

  for (uint8_t i = 0; i < kClockCharacterCount; ++i)
  {
    if (clock_character_labels_[i] == nullptr)
    {
      continue;
    }

    char current_character = text[i] == '\0' ? ' ' : text[i];
    if (i == 2 && current_character == ':' && !blink_separator_visible)
    {
      current_character = ' ';
    }

    char slot_text[2] = {current_character, '\0'};
    lv_label_set_text(clock_character_labels_[i], slot_text);
  }
}
