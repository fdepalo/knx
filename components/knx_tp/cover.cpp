#include "cover.h"
#include "dpt.h"
#include "esphome/core/log.h"
namespace esphome { namespace knx_tp {
static constexpr const char* TAG = "knx_tp.cover";
void KNXCover::setup() { if (knx_) knx_->register_entity(this); }
cover::CoverTraits KNXCover::get_traits() {
  auto t = cover::CoverTraits();
  t.set_supports_stop(true);
  t.set_supports_position(!position_ga_id_.empty());
  return t;
}
void KNXCover::control(const cover::CoverCall &call) {
  if (call.get_position().has_value() && knx_ && !position_ga_id_.empty()) {
    position = *call.get_position();
    knx_->send_group_write(position_ga_id_, DPT::encode_dpt5_percentage(position * 100.0f));
  }
  publish_state();
}
void KNXCover::on_knx_telegram(const std::string &ga, const std::vector<uint8_t> &data) {
  // Null pointer check: ensure KNX component is initialized
  if (!knx_) {
    ESP_LOGE(TAG, "KNX component is nullptr in on_knx_telegram");
    return;
  }

  auto pos_ga = knx_->get_group_address(position_ga_id_);
  if (pos_ga && pos_ga->get_address() == ga) {
    position = DPT::decode_dpt5_percentage(data) / 100.0f;
    publish_state();
  }
}
}}