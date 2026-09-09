#include "screens/alarm_ringtone_settings_screen.h"

#include "ui/button.h"
#include "ui/icons.h"
#include "ui/page_header.h"
#include "ui/theme.h"

namespace {
constexpr Ui::ButtonSize kHeaderSaveButtonSize{60, 32};
constexpr Ui::ButtonSize kControlButtonSize{32, 32};
constexpr const char* kRingtoneNames[] = {"Soft Chime", "Gentle Bell", "Rising Wake", "Persistent"};
}  // namespace

AlarmRingtoneSettingsScreen::AlarmRingtoneSettingsScreen(AlarmRingtoneSettingsScreenListener& listener)
    : listener_(listener),
      root_(nullptr),
      selected_ringtone_(kDefaultAlarmRingtone),
      previewing_ringtone_(AlarmRingtone::Count),
      rows_{} {}

void AlarmRingtoneSettingsScreen::build(lv_obj_t* parent) {
  root_ = lv_obj_create(parent);
  lv_obj_remove_style_all(root_);
  lv_obj_set_size(root_, LV_PCT(100), LV_PCT(100));
  lv_obj_set_style_bg_color(root_, lv_color_hex(UiTheme::kSurfaceRaised), 0);
  lv_obj_set_style_bg_opa(root_, LV_OPA_COVER, 0);
  lv_obj_set_style_pad_all(root_, 14, 0);
  lv_obj_set_style_pad_row(root_, 10, 0);
  lv_obj_set_flex_flow(root_, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(root_, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  lv_obj_add_flag(root_, LV_OBJ_FLAG_HIDDEN);

  Ui::createPageHeader(root_, {"Ringtone", UiIcon::kBackLabel, onBack, this, "Save", &UiTheme::kPrimaryButtonStyle,
                               kHeaderSaveButtonSize, onSave, this});
  for (uint8_t i = 0; i < static_cast<uint8_t>(AlarmRingtone::Count); ++i) {
    RingtoneRow& row = rows_[i];
    row = {this, static_cast<AlarmRingtone>(i), nullptr, nullptr};

    lv_obj_t* row_root = lv_obj_create(root_);
    lv_obj_remove_style_all(row_root);
    lv_obj_set_size(row_root, LV_PCT(100), 38);
    lv_obj_set_style_pad_column(row_root, 8, 0);
    lv_obj_set_flex_flow(row_root, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(row_root, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t* name = lv_label_create(row_root);
    lv_label_set_text(name, kRingtoneNames[i]);
    lv_obj_set_flex_grow(name, 1);
    lv_obj_set_style_text_font(name, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_color(name, lv_color_hex(UiTheme::kTextPrimary), 0);
    lv_obj_add_flag(name, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(name, onRingtoneSelected, LV_EVENT_CLICKED, &row);

    Ui::ButtonElements selector =
        Ui::createButton(row_root, "", UiTheme::kSecondaryButtonStyle, kControlButtonSize, onRingtoneSelected, &row);
    row.selector_label = selector.label;

    Ui::ButtonElements preview = Ui::createButton(row_root, UiIcon::kPlay, UiTheme::kSecondaryButtonStyle,
                                                  kControlButtonSize, onPreviewPressed, &row);
    row.preview_label = preview.label;
  }
}

void AlarmRingtoneSettingsScreen::show() {
  if (lv_obj_has_flag(root_, LV_OBJ_FLAG_HIDDEN)) {
    lv_obj_scroll_to_y(root_, 0, LV_ANIM_OFF);
    lv_obj_clear_flag(root_, LV_OBJ_FLAG_HIDDEN);
  }
}

void AlarmRingtoneSettingsScreen::hide() { lv_obj_add_flag(root_, LV_OBJ_FLAG_HIDDEN); }

void AlarmRingtoneSettingsScreen::setSelection(AlarmRingtone ringtone) { setSelectedRow(ringtone); }

void AlarmRingtoneSettingsScreen::setPreviewing(AlarmRingtone ringtone) {
  previewing_ringtone_ = ringtone;
  for (uint8_t i = 0; i < static_cast<uint8_t>(AlarmRingtone::Count); ++i) {
    lv_label_set_text(rows_[i].preview_label,
                      rows_[i].ringtone == previewing_ringtone_ ? UiIcon::kPause : UiIcon::kPlay);
  }
}

void AlarmRingtoneSettingsScreen::onBack(lv_event_t* event) {
  auto* screen = static_cast<AlarmRingtoneSettingsScreen*>(lv_event_get_user_data(event));
  if (screen != nullptr) screen->listener_.onAlarmRingtoneSettingsBackRequested();
}

void AlarmRingtoneSettingsScreen::onSave(lv_event_t* event) {
  auto* screen = static_cast<AlarmRingtoneSettingsScreen*>(lv_event_get_user_data(event));
  if (screen != nullptr) screen->listener_.onAlarmRingtoneSaved(screen->selected_ringtone_);
}

void AlarmRingtoneSettingsScreen::onRingtoneSelected(lv_event_t* event) {
  auto* row = static_cast<RingtoneRow*>(lv_event_get_user_data(event));
  if (row != nullptr) row->screen->setSelectedRow(row->ringtone);
}

void AlarmRingtoneSettingsScreen::onPreviewPressed(lv_event_t* event) {
  auto* row = static_cast<RingtoneRow*>(lv_event_get_user_data(event));
  if (row == nullptr) return;
  if (row->screen->previewing_ringtone_ == row->ringtone) {
    row->screen->listener_.onAlarmRingtonePreviewStopped();
  } else {
    row->screen->listener_.onAlarmRingtonePreviewStarted(row->ringtone);
  }
}

void AlarmRingtoneSettingsScreen::setSelectedRow(AlarmRingtone ringtone) {
  selected_ringtone_ = ringtone < AlarmRingtone::Count ? ringtone : kDefaultAlarmRingtone;
  for (uint8_t i = 0; i < static_cast<uint8_t>(AlarmRingtone::Count); ++i) {
    const bool selected = rows_[i].ringtone == selected_ringtone_;
    lv_label_set_text(rows_[i].selector_label, selected ? UiIcon::kSelected : "");
  }
}
