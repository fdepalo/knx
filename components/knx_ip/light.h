#pragma once
#include "esphome/components/light/light_output.h"
#include "esphome/core/component.h"
#include "knx_ip.h"
namespace esphome { namespace knx_ip {
class KNXLight : public light::LightOutput, public Component, public KNXEntity {
 public:
  void setup() override;
  light::LightTraits get_traits() override;
  void set_switch_ga_id(const std::string &ga_id) { switch_ga_id_ = ga_id; }
  void set_brightness_ga_id(const std::string &ga_id) { brightness_ga_id_ = ga_id; }
  void set_state_ga_id(const std::string &ga_id) { state_ga_id_ = ga_id; }
  void write_state(light::LightState *state) override;
  void on_knx_telegram(const std::string &ga, const std::vector<uint8_t> &data) override;
 protected:
  std::string switch_ga_id_, brightness_ga_id_, state_ga_id_;
};
}}