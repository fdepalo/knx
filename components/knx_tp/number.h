#pragma once
#include "esphome/components/number/number.h"
#include "esphome/core/component.h"
#include "knx_tp.h"
namespace esphome { namespace knx_tp {
class KNXNumber : public number::Number, public Component, public KNXEntity {
 public:
  void setup() override;
  void dump_config() override;
  void set_command_ga_id(const std::string &ga_id) { command_ga_id_ = ga_id; }
  void set_state_ga_id(const std::string &ga_id) { state_ga_id_ = ga_id; }
  void on_knx_telegram(const std::string &ga, const std::vector<uint8_t> &data) override;
 protected:
  void control(float value) override;
  std::string command_ga_id_, state_ga_id_;
};
}}