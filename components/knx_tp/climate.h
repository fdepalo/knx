#pragma once

#include "esphome/components/climate/climate.h"
#include "esphome/core/component.h"
#include "knx_tp.h"

namespace esphome {
namespace knx_tp {

class KNXClimate : public climate::Climate, public Component, public KNXEntity {
 public:
  void setup() override;
  void dump_config() override;
  void control(const climate::ClimateCall &call) override;
  climate::ClimateTraits traits() override;

  // Configuration setters
  void set_temperature_ga_id(const std::string &ga_id) { temperature_ga_id_ = ga_id; }
  void set_setpoint_ga_id(const std::string &ga_id) { setpoint_ga_id_ = ga_id; }
  void set_mode_ga_id(const std::string &ga_id) { mode_ga_id_ = ga_id; }
  void set_action_ga_id(const std::string &ga_id) { action_ga_id_ = ga_id; }

  // Preset GA setters
  void set_preset_comfort_ga_id(const std::string &ga_id) { preset_comfort_ga_id_ = ga_id; }
  void set_preset_eco_ga_id(const std::string &ga_id) { preset_eco_ga_id_ = ga_id; }
  void set_preset_away_ga_id(const std::string &ga_id) { preset_away_ga_id_ = ga_id; }
  void set_preset_sleep_ga_id(const std::string &ga_id) { preset_sleep_ga_id_ = ga_id; }

  void on_knx_telegram(const std::string &ga, const std::vector<uint8_t> &data) override;

 protected:
  // Group address IDs
  std::string temperature_ga_id_;
  std::string setpoint_ga_id_;
  std::string mode_ga_id_;
  std::string action_ga_id_;
  std::string preset_comfort_ga_id_;
  std::string preset_eco_ga_id_;
  std::string preset_away_ga_id_;
  std::string preset_sleep_ga_id_;

  // Helper methods
  void send_temperature_(float temp);
  void send_mode_(climate::ClimateMode mode);
  void send_preset_(climate::ClimatePreset preset);
  climate::ClimateMode hvac_mode_to_climate_mode_(uint8_t hvac_mode);
  uint8_t climate_mode_to_hvac_mode_(climate::ClimateMode mode);
};

}  // namespace knx_tp
}  // namespace esphome
