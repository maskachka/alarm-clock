#include "ui/button.h"
#include "screens/alarm_list_screen.h"

#include <stdio.h>

#include "ui/icons.h"
#include "ui/page_header.h"
#include "ui/theme.h"

AlarmListScreen::AlarmListScreen(AlarmListScreenListener& l)
    : listener_(l), root_(nullptr), dismiss_row_(nullptr), dismiss_button_(nullptr), rows_{} {}
void AlarmListScreen::build(lv_obj_t* parent) {
  root_ = lv_obj_create(parent);
  lv_obj_remove_style_all(root_);
  lv_obj_set_size(root_, LV_PCT(100), LV_PCT(100));
  lv_obj_set_style_pad_all(root_, 12, 0);
  lv_obj_set_style_pad_row(root_, 12, 0);
  lv_obj_set_flex_flow(root_, LV_FLEX_FLOW_COLUMN);
  lv_obj_add_flag(root_, LV_OBJ_FLAG_HIDDEN);
  constexpr Ui::ButtonSize kAddButtonSize{60, 32};
  Ui::createPageHeader(root_, {"Alarms", UiIcon::kBackLabel, onBack, this, "Add", &UiTheme::kPrimaryButtonStyle,
                               kAddButtonSize, onAdd, this});
  dismiss_row_ = lv_obj_create(root_);
  lv_obj_remove_style_all(dismiss_row_);
  lv_obj_set_size(dismiss_row_, LV_PCT(100), 32);
  lv_obj_set_flex_flow(dismiss_row_, LV_FLEX_FLOW_ROW);
  lv_obj_set_flex_align(dismiss_row_, LV_FLEX_ALIGN_END, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  Ui::ButtonElements dismiss =
      Ui::createButton(dismiss_row_, "Dismiss", UiTheme::kSecondaryButtonStyle, kAddButtonSize, onDismiss, this);
  dismiss_button_ = dismiss.button;
  lv_obj_add_flag(dismiss_row_, LV_OBJ_FLAG_HIDDEN);
  for (uint8_t i = 0; i < AlarmService::kMaxAlarms; ++i) {
    Row& r = rows_[i];
    r.screen = this;
    r.index = i;
    r.root = lv_obj_create(root_);
    lv_obj_remove_style_all(r.root);
    lv_obj_set_size(r.root, LV_PCT(100), 42);
    lv_obj_set_flex_flow(r.root, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(r.root, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_t* group = lv_obj_create(r.root);
    lv_obj_remove_style_all(group);
    lv_obj_set_size(group, LV_SIZE_CONTENT, LV_PCT(100));
    lv_obj_set_style_pad_column(group, 10, 0);
    lv_obj_set_flex_flow(group, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(group, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_t* label = lv_label_create(group);
    char index_text[4];
    snprintf(index_text, sizeof(index_text), "%u:", static_cast<unsigned>(i + 1));
    lv_label_set_text(label, index_text);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_20, 0);
    r.time = lv_label_create(group);
    lv_obj_set_style_text_font(r.time, &lv_font_montserrat_20, 0);
    lv_obj_t* edit = lv_label_create(group);
    lv_label_set_text(edit, "Edit");
    lv_obj_set_style_text_font(edit, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_color(edit, lv_color_hex(UiTheme::kAccent), 0);
    lv_obj_add_flag(edit, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(edit, onSet, LV_EVENT_CLICKED, &r);
    lv_obj_t* remove = lv_label_create(group);
    lv_label_set_text(remove, "Delete");
    lv_obj_set_style_text_font(remove, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_color(remove, lv_color_hex(UiTheme::kAccent), 0);
    lv_obj_add_flag(remove, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(remove, onDelete, LV_EVENT_CLICKED, &r);
    r.enabled_switch = lv_switch_create(r.root);
    lv_obj_add_event_cb(r.enabled_switch, onToggle, LV_EVENT_VALUE_CHANGED, &r);
  }
}
void AlarmListScreen::render(const AlarmService& a) {
  if (a.hasRingingAlarm())
    lv_obj_clear_flag(dismiss_row_, LV_OBJ_FLAG_HIDDEN);
  else
    lv_obj_add_flag(dismiss_row_, LV_OBJ_FLAG_HIDDEN);
  for (uint8_t i = 0; i < AlarmService::kMaxAlarms; ++i) {
    Row& r = rows_[i];
    if (i >= a.count()) {
      lv_obj_add_flag(r.root, LV_OBJ_FLAG_HIDDEN);
      continue;
    }
    lv_obj_clear_flag(r.root, LV_OBJ_FLAG_HIDDEN);
    char t[6];
    snprintf(t, sizeof(t), "%02u:%02u", a.hour(i), a.minute(i));
    lv_label_set_text(r.time, t);
    if (a.isEnabled(i))
      lv_obj_add_state(r.enabled_switch, LV_STATE_CHECKED);
    else
      lv_obj_remove_state(r.enabled_switch, LV_STATE_CHECKED);
  }
}
void AlarmListScreen::show() {
  if (lv_obj_has_flag(root_, LV_OBJ_FLAG_HIDDEN)) {
    lv_obj_scroll_to_y(root_, 0, LV_ANIM_OFF);
    lv_obj_clear_flag(root_, LV_OBJ_FLAG_HIDDEN);
  }
}
void AlarmListScreen::hide() { lv_obj_add_flag(root_, LV_OBJ_FLAG_HIDDEN); }
void AlarmListScreen::onSet(lv_event_t* e) {
  auto* r = static_cast<Row*>(lv_event_get_user_data(e));
  if (r) r->screen->listener_.onAlarmSelected(r->index);
}
void AlarmListScreen::onAdd(lv_event_t* e) {
  auto* s = static_cast<AlarmListScreen*>(lv_event_get_user_data(e));
  if (s) s->listener_.onAlarmAddRequested();
}
void AlarmListScreen::onDismiss(lv_event_t* e) {
  auto* s = static_cast<AlarmListScreen*>(lv_event_get_user_data(e));
  if (s) s->listener_.onDismissRequested();
}
void AlarmListScreen::onDelete(lv_event_t* e) {
  auto* r = static_cast<Row*>(lv_event_get_user_data(e));
  if (r) r->screen->listener_.onAlarmDeleteRequested(r->index);
}
void AlarmListScreen::onToggle(lv_event_t* e) {
  auto* r = static_cast<Row*>(lv_event_get_user_data(e));
  if (r) r->screen->listener_.onAlarmToggled(r->index);
}
void AlarmListScreen::onBack(lv_event_t* e) {
  auto* s = static_cast<AlarmListScreen*>(lv_event_get_user_data(e));
  if (s) s->listener_.onAlarmListBackRequested();
}
