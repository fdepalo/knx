#include "number.h"
#include "dpt.h"
#include "esphome/core/log.h"
namespace esphome { namespace knx_tp {
static constexpr const char* TAG = "knx_tp.number";
void KNXNumber::setup() { if (knx_) knx_->register_entity(this); }
void KNXNumber::dump_config() { LOG_NUMBER("", "KNX Number", this); }
void KNXNumber::control(float value) {
  if (knx_) knx_->send_group_write(command_ga_id_, DPT::encode_dpt9(value));
  publish_state(value);
}
void KNXNumber::on_knx_telegram(const std::string &ga, const std::vector<uint8_t> &data) {
  // Null pointer check: ensure KNX component is initialized
  if (!knx_) {
    ESP_LOGE(TAG, "KNX component is nullptr in on_knx_telegram");
    return;
  }

  auto state_ga = knx_->get_group_address(state_ga_id_);
  if (state_ga && state_ga->get_address() == ga) {
    float value = DPT::decode_dpt9(data);
    publish_state(value);
  }
}
}}