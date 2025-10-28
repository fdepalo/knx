#include "light.h"
#include "dpt.h"
#include "esphome/core/log.h"
namespace esphome { namespace knx_tp {
static constexpr const char* TAG = "knx_tp.light";
void KNXLight::setup() { if (knx_) knx_->register_entity(this); }
light::LightTraits KNXLight::get_traits() {
  auto t = light::LightTraits();
  if (brightness_ga_id_.empty()) t.set_supported_color_modes({light::ColorMode::ON_OFF});
  else t.set_supported_color_modes({light::ColorMode::BRIGHTNESS});
  return t;
}
void KNXLight::write_state(light::LightState *state) {
  bool binary; float brightness;
  state->current_values_as_binary(&binary);
  state->current_values_as_brightness(&brightness);
  if (knx_) {
    knx_->send_group_write(switch_ga_id_, DPT::encode_dpt1(binary));
    if (!brightness_ga_id_.empty())
      knx_->send_group_write(brightness_ga_id_, DPT::encode_dpt5_percentage(brightness * 100.0f));
  }
}
void KNXLight::on_knx_telegram(const std::string &ga, const std::vector<uint8_t> &data) {}
}}