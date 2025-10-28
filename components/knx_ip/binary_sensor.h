#pragma once

#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/core/component.h"
#include "knx_ip.h"

namespace esphome {
namespace knx_ip {

/**
 * KNX Binary Sensor
 * Receives boolean states from KNX bus (motion, door contacts, buttons, etc.)
 */
class KNXBinarySensor : public binary_sensor::BinarySensor, public Component, public KNXEntity {
 public:
  void setup() override;
  void dump_config() override;
  float get_setup_priority() const override { return setup_priority::DATA; }
  
  void set_state_ga_id(const std::string &ga_id) { state_ga_id_ = ga_id; }
  void set_invert(bool invert) { invert_ = invert; }
  void set_auto_reset_time(uint32_t time_ms) { auto_reset_time_ms_ = time_ms; }
  
  void on_knx_telegram(const std::string &ga, const std::vector<uint8_t> &data) override;

 protected:
  std::string state_ga_id_;
  bool invert_{false};
  uint32_t auto_reset_time_ms_{0};
  
  void update_state_from_knx(bool state);
};

}  // namespace knx_ip
}  // namespace esphome
