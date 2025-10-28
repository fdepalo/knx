#pragma once

#include "esphome/components/switch/switch.h"
#include "esphome/core/component.h"
#include "knx_tp.h"

namespace esphome {
namespace knx_tp {

/**
 * KNX Switch
 * Controls boolean actuators on KNX bus (lights, relays, etc.)
 */
class KNXSwitch : public switch_::Switch, public Component, public KNXEntity {
 public:
  void setup() override;
  void dump_config() override;
  float get_setup_priority() const override { return setup_priority::DATA; }
  
  void set_command_ga_id(const std::string &ga_id) { command_ga_id_ = ga_id; }
  void set_state_ga_id(const std::string &ga_id) { state_ga_id_ = ga_id; }
  void set_invert(bool invert) { invert_ = invert; }
  
  void on_knx_telegram(const std::string &ga, const std::vector<uint8_t> &data) override;

 protected:
  void write_state(bool state) override;
  
  std::string command_ga_id_;
  std::string state_ga_id_;
  bool invert_{false};
};

}  // namespace knx_tp
}  // namespace esphome
