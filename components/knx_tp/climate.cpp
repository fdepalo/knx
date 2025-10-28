#include "climate.h"
#include "dpt.h"
#include "esphome/core/log.h"

namespace esphome {
namespace knx_tp {

static constexpr const char* TAG = "knx_tp.climate";

void KNXClimate::setup() {
  if (this->knx_ != nullptr) {
    this->knx_->register_entity(this);
    ESP_LOGD(TAG, "KNX Climate registered");
  }
}

void KNXClimate::dump_config() {
  LOG_CLIMATE("", "KNX Climate", this);
  ESP_LOGCONFIG(TAG, "  Temperature GA: %s", this->temperature_ga_id_.c_str());
  ESP_LOGCONFIG(TAG, "  Setpoint GA: %s", this->setpoint_ga_id_.c_str());

  if (!this->mode_ga_id_.empty()) {
    ESP_LOGCONFIG(TAG, "  Mode GA: %s", this->mode_ga_id_.c_str());
  }
  if (!this->action_ga_id_.empty()) {
    ESP_LOGCONFIG(TAG, "  Action GA: %s", this->action_ga_id_.c_str());
  }
  if (!this->preset_comfort_ga_id_.empty()) {
    ESP_LOGCONFIG(TAG, "  Preset Comfort GA: %s", this->preset_comfort_ga_id_.c_str());
  }
  if (!this->preset_eco_ga_id_.empty()) {
    ESP_LOGCONFIG(TAG, "  Preset Eco GA: %s", this->preset_eco_ga_id_.c_str());
  }
  if (!this->preset_away_ga_id_.empty()) {
    ESP_LOGCONFIG(TAG, "  Preset Away GA: %s", this->preset_away_ga_id_.c_str());
  }
  if (!this->preset_sleep_ga_id_.empty()) {
    ESP_LOGCONFIG(TAG, "  Preset Sleep GA: %s", this->preset_sleep_ga_id_.c_str());
  }
}

climate::ClimateTraits KNXClimate::traits() {
  auto traits = climate::ClimateTraits();
  traits.set_supports_current_temperature(true);
  traits.set_supports_two_point_target_temperature(false);

  // Supported modes
  traits.set_supported_modes({
    climate::CLIMATE_MODE_OFF,
    climate::CLIMATE_MODE_AUTO,
    climate::CLIMATE_MODE_HEAT,
    climate::CLIMATE_MODE_COOL
  });

  // Action support
  if (!this->action_ga_id_.empty()) {
    traits.set_supports_action(true);
  }

  // Preset support
  std::set<climate::ClimatePreset> presets;
  if (!this->preset_comfort_ga_id_.empty()) presets.insert(climate::CLIMATE_PRESET_COMFORT);
  if (!this->preset_eco_ga_id_.empty()) presets.insert(climate::CLIMATE_PRESET_ECO);
  if (!this->preset_away_ga_id_.empty()) presets.insert(climate::CLIMATE_PRESET_AWAY);
  if (!this->preset_sleep_ga_id_.empty()) presets.insert(climate::CLIMATE_PRESET_SLEEP);

  if (!presets.empty()) {
    traits.set_supported_presets(presets);
  }

  traits.set_visual_min_temperature(10.0f);
  traits.set_visual_max_temperature(30.0f);
  traits.set_visual_temperature_step(0.5f);

  return traits;
}

void KNXClimate::control(const climate::ClimateCall &call) {
  // Handle target temperature change
  if (call.get_target_temperature().has_value()) {
    this->target_temperature = *call.get_target_temperature();
    send_temperature_(this->target_temperature);
    ESP_LOGD(TAG, "Target temperature set to %.1f°C", this->target_temperature);
  }

  // Handle mode change
  if (call.get_mode().has_value()) {
    this->mode = *call.get_mode();
    send_mode_(this->mode);
    ESP_LOGD(TAG, "Mode changed to %d", static_cast<int>(this->mode));
  }

  // Handle preset change
  if (call.get_preset().has_value()) {
    auto preset = *call.get_preset();
    this->preset = preset;
    send_preset_(preset);
    ESP_LOGD(TAG, "Preset changed to %d", static_cast<int>(preset));
  }

  this->publish_state();
}

void KNXClimate::on_knx_telegram(const std::string &ga, const std::vector<uint8_t> &data) {
  // Null pointer check: ensure KNX component is initialized
  if (!this->knx_) {
    ESP_LOGE(TAG, "KNX component is nullptr in on_knx_telegram");
    return;
  }

  auto temp_ga = this->knx_->get_group_address(this->temperature_ga_id_);
  auto setpoint_ga = this->knx_->get_group_address(this->setpoint_ga_id_);

  // Check temperature feedback
  if (temp_ga && temp_ga->get_address() == ga) {
    this->current_temperature = DPT::decode_dpt9(data);
    ESP_LOGD(TAG, "Received current temperature: %.1f°C", this->current_temperature);
    this->publish_state();
    return;
  }

  // Check setpoint feedback
  if (setpoint_ga && setpoint_ga->get_address() == ga) {
    this->target_temperature = DPT::decode_dpt9(data);
    ESP_LOGD(TAG, "Received target temperature: %.1f°C", this->target_temperature);
    this->publish_state();
    return;
  }

  // Check mode feedback
  if (!this->mode_ga_id_.empty()) {
    auto mode_ga = this->knx_->get_group_address(this->mode_ga_id_);
    if (mode_ga && mode_ga->get_address() == ga) {
      auto hvac_mode = DPT::decode_dpt20_102(data);
      this->mode = hvac_mode_to_climate_mode_(static_cast<uint8_t>(hvac_mode));
      ESP_LOGD(TAG, "Received HVAC mode: %d -> Climate mode: %d",
               static_cast<int>(hvac_mode), static_cast<int>(this->mode));
      this->publish_state();
      return;
    }
  }

  // Check action feedback
  if (!this->action_ga_id_.empty()) {
    auto action_ga = this->knx_->get_group_address(this->action_ga_id_);
    if (action_ga && action_ga->get_address() == ga) {
      // Action is typically a boolean: 0=idle, 1=active
      bool active = DPT::decode_dpt1(data);
      if (active) {
        // Determine action based on mode
        if (this->mode == climate::CLIMATE_MODE_HEAT) {
          this->action = climate::CLIMATE_ACTION_HEATING;
        } else if (this->mode == climate::CLIMATE_MODE_COOL) {
          this->action = climate::CLIMATE_ACTION_COOLING;
        } else {
          this->action = climate::CLIMATE_ACTION_IDLE;
        }
      } else {
        this->action = climate::CLIMATE_ACTION_IDLE;
      }
      ESP_LOGD(TAG, "Received action state: %s", active ? "ACTIVE" : "IDLE");
      this->publish_state();
      return;
    }
  }

  // Check preset feedback
  auto check_preset = [&](const std::string &preset_ga_id, climate::ClimatePreset preset_type) {
    if (!preset_ga_id.empty()) {
      auto preset_ga = this->knx_->get_group_address(preset_ga_id);
      if (preset_ga && preset_ga->get_address() == ga) {
        bool active = DPT::decode_dpt1(data);
        if (active) {
          this->preset = preset_type;
          ESP_LOGD(TAG, "Preset activated: %d", static_cast<int>(preset_type));
          this->publish_state();
        }
        return true;
      }
    }
    return false;
  };

  if (check_preset(this->preset_comfort_ga_id_, climate::CLIMATE_PRESET_COMFORT)) return;
  if (check_preset(this->preset_eco_ga_id_, climate::CLIMATE_PRESET_ECO)) return;
  if (check_preset(this->preset_away_ga_id_, climate::CLIMATE_PRESET_AWAY)) return;
  if (check_preset(this->preset_sleep_ga_id_, climate::CLIMATE_PRESET_SLEEP)) return;
}

void KNXClimate::send_temperature_(float temp) {
  if (this->knx_) {
    auto data = DPT::encode_dpt9(temp);
    this->knx_->send_group_write(this->setpoint_ga_id_, data);
    ESP_LOGD(TAG, "Sent setpoint: %.1f°C", temp);
  }
}

void KNXClimate::send_mode_(climate::ClimateMode mode) {
  if (this->knx_ && !this->mode_ga_id_.empty()) {
    uint8_t hvac_mode = climate_mode_to_hvac_mode_(mode);
    auto hvac_mode_enum = static_cast<DPT::HVACMode>(hvac_mode);
    auto data = DPT::encode_dpt20_102(hvac_mode_enum);
    this->knx_->send_group_write(this->mode_ga_id_, data);
    ESP_LOGD(TAG, "Sent HVAC mode: %d", hvac_mode);
  }
}

void KNXClimate::send_preset_(climate::ClimatePreset preset) {
  if (!this->knx_) return;

  // Deactivate all presets first (send OFF to all)
  auto send_preset_off = [&](const std::string &preset_ga_id) {
    if (!preset_ga_id.empty()) {
      this->knx_->send_group_write(preset_ga_id, DPT::encode_dpt1(false));
    }
  };

  send_preset_off(this->preset_comfort_ga_id_);
  send_preset_off(this->preset_eco_ga_id_);
  send_preset_off(this->preset_away_ga_id_);
  send_preset_off(this->preset_sleep_ga_id_);

  // Activate selected preset
  std::string active_preset_ga;
  switch (preset) {
    case climate::CLIMATE_PRESET_COMFORT:
      active_preset_ga = this->preset_comfort_ga_id_;
      break;
    case climate::CLIMATE_PRESET_ECO:
      active_preset_ga = this->preset_eco_ga_id_;
      break;
    case climate::CLIMATE_PRESET_AWAY:
      active_preset_ga = this->preset_away_ga_id_;
      break;
    case climate::CLIMATE_PRESET_SLEEP:
      active_preset_ga = this->preset_sleep_ga_id_;
      break;
    default:
      return;
  }

  if (!active_preset_ga.empty()) {
    this->knx_->send_group_write(active_preset_ga, DPT::encode_dpt1(true));
    ESP_LOGD(TAG, "Sent preset: %d", static_cast<int>(preset));
  }
}

climate::ClimateMode KNXClimate::hvac_mode_to_climate_mode_(uint8_t hvac_mode) {
  // Map KNX HVAC modes (DPT 20.102) to ESPHome climate modes
  switch (hvac_mode) {
    case 0:  // AUTO
      return climate::CLIMATE_MODE_AUTO;
    case 1:  // COMFORT
      return climate::CLIMATE_MODE_HEAT;
    case 2:  // STANDBY
      return climate::CLIMATE_MODE_OFF;
    case 3:  // NIGHT
      return climate::CLIMATE_MODE_HEAT;
    case 4:  // FROST_PROTECTION
      return climate::CLIMATE_MODE_HEAT;
    default:
      return climate::CLIMATE_MODE_AUTO;
  }
}

uint8_t KNXClimate::climate_mode_to_hvac_mode_(climate::ClimateMode mode) {
  // Map ESPHome climate modes to KNX HVAC modes (DPT 20.102)
  switch (mode) {
    case climate::CLIMATE_MODE_AUTO:
      return 0;  // AUTO
    case climate::CLIMATE_MODE_HEAT:
      return 1;  // COMFORT
    case climate::CLIMATE_MODE_COOL:
      return 1;  // COMFORT (same as heat for basic systems)
    case climate::CLIMATE_MODE_OFF:
      return 2;  // STANDBY
    default:
      return 0;  // AUTO
  }
}

}  // namespace knx_tp
}  // namespace esphome
