#pragma once
#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/core/component.h"
#include "knx_ip.h"
namespace esphome { namespace knx_ip {

enum class TextSensorDPT {
  DPT_16,    // Character string (default)
  DPT_10,    // Time of Day
  DPT_11,    // Date
  DPT_19     // Date and Time
};

class KNXTextSensor : public text_sensor::TextSensor, public Component, public KNXEntity {
 public:
  void setup() override;
  void dump_config() override;
  void set_state_ga_id(const std::string &ga_id) { state_ga_id_ = ga_id; }
  void set_dpt_type(TextSensorDPT dpt) { dpt_type_ = dpt; }
  void on_knx_telegram(const std::string &ga, const std::vector<uint8_t> &data) override;
 protected:
  std::string state_ga_id_;
  TextSensorDPT dpt_type_ = TextSensorDPT::DPT_16;
};
}}