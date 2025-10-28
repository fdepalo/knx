#include "binary_sensor.h"
#include "dpt.h"
#include "esphome/core/log.h"

namespace esphome {
namespace knx_tp {

static constexpr const char* TAG = "knx_tp.binary_sensor";

void KNXBinarySensor::setup() {
  ESP_LOGCONFIG(TAG, "Setting up KNX Binary Sensor '%s'...", this->get_name().c_str());
  
  if (this->knx_ != nullptr) {
    this->knx_->register_entity(this);
    ESP_LOGD(TAG, "Registered with KNX component");
  } else {
    ESP_LOGE(TAG, "KNX component is nullptr!");
  }
}

void KNXBinarySensor::dump_config() {
  LOG_BINARY_SENSOR("", "KNX Binary Sensor", this);
  ESP_LOGCONFIG(TAG, "  State GA: %s", this->state_ga_id_.c_str());
  
  if (this->invert_) {
    ESP_LOGCONFIG(TAG, "  Inverted: YES");
  }
  
  if (this->auto_reset_time_ms_ > 0) {
    ESP_LOGCONFIG(TAG, "  Auto-reset time: %u ms", this->auto_reset_time_ms_);
  }
}

void KNXBinarySensor::on_knx_telegram(const std::string &ga, const std::vector<uint8_t> &data) {
  // Check if this telegram is for our group address
  auto our_ga = this->knx_->get_group_address(this->state_ga_id_);
  if (our_ga == nullptr || our_ga->get_address() != ga) {
    return;  // Not for us
  }
  
  ESP_LOGD(TAG, "'%s': Received telegram on GA %s", this->get_name().c_str(), ga.c_str());
  
  // Decode DPT 1.001 (boolean)
  bool state = DPT::decode_dpt1(data);
  
  ESP_LOGD(TAG, "'%s': Decoded state: %s", this->get_name().c_str(), state ? "ON" : "OFF");
  
  update_state_from_knx(state);
}

void KNXBinarySensor::update_state_from_knx(bool state) {
  // Apply inversion if configured
  if (this->invert_) {
    state = !state;
    ESP_LOGD(TAG, "'%s': State inverted to: %s", 
             this->get_name().c_str(), state ? "ON" : "OFF");
  }
  
  // Publish the state
  this->publish_state(state);
  
  ESP_LOGI(TAG, "'%s': State changed to %s", 
           this->get_name().c_str(), state ? "ON" : "OFF");
  
  // Handle auto-reset (useful for motion sensors, buttons, etc.)
  if (this->auto_reset_time_ms_ > 0 && state) {
    ESP_LOGD(TAG, "'%s': Scheduling auto-reset in %u ms", 
             this->get_name().c_str(), this->auto_reset_time_ms_);
    
    this->set_timeout("auto_reset", this->auto_reset_time_ms_, [this]() {
      ESP_LOGD(TAG, "'%s': Auto-reset triggered", this->get_name().c_str());
      this->publish_state(false);
      ESP_LOGI(TAG, "'%s': Auto-reset to OFF", this->get_name().c_str());
    });
  }
}

}  // namespace knx_tp
}  // namespace esphome
