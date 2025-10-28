#pragma once
#include "esphome/components/cover/cover.h"
#include "esphome/core/component.h"
#include "knx_ip.h"
namespace esphome { namespace knx_ip {
class KNXCover : public cover::Cover, public Component, public KNXEntity {
 public:
  void setup() override;
  cover::CoverTraits get_traits() override;
  void set_move_ga_id(const std::string &ga_id) { move_ga_id_ = ga_id; }
  void set_position_ga_id(const std::string &ga_id) { position_ga_id_ = ga_id; }
  void set_stop_ga_id(const std::string &ga_id) { stop_ga_id_ = ga_id; }
  void on_knx_telegram(const std::string &ga, const std::vector<uint8_t> &data) override;
 protected:
  void control(const cover::CoverCall &call) override;
  std::string move_ga_id_, position_ga_id_, stop_ga_id_;
};
}}