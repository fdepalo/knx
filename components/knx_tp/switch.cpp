#include "switch.h"
#include "dpt.h"
#include "esphome/core/log.h"

namespace esphome {
namespace knx_tp {

static constexpr const char* TAG = "knx_tp.switch";

void KNXSwitch::setup() {
  ESP_LOGCONFIG(TAG, "Setting up KNX Switch '%s'...", this->get_name().c_str());
  
  if (this->knx_ != nullptr) {
    this->knx_->register_entity(this);
    ESP_LOGD(TAG, "Registered with KNX component");
  } else {
    ESP_LOGE(TAG, "KNX component is nullptr!");
  }
}

void KNXSwitch::dump_config() {
  LOG_SWITCH("", "KNX Switch", this);
  ESP_LOGCONFIG(TAG, "  Command GA: %s", this->command_ga_id_.c_str());
  
  if (!this->state_ga_id_.empty()) {
    ESP_LOGCONFIG(TAG, "  State GA: %s", this->state_ga_id_.c_str());
  }
  
  if (this->invert_) {
    ESP_LOGCONFIG(TAG, "  Inverted: YES");
  }
}

void KNXSwitch::write_state(bool state) {
  if (this->knx_ == nullptr) {
    ESP_LOGE(TAG, "'%s': KNX component is nullptr!", this->get_name().c_str());
    return;
  }
  
  // Apply inversion if configured
  bool knx_state = this->invert_ ? !state : state;
  
  ESP_LOGD(TAG, "'%s': Writing state %s (KNX: %s)", 
           this->get_name().c_str(), 
           state ? "ON" : "OFF",
           knx_state ? "ON" : "OFF");
  
  // Encode as DPT 1.001 and send
  std::vector<uint8_t> data = DPT::encode_dpt1(knx_state);
  this->knx_->send_group_write(this->command_ga_id_, data);
  
  // Publish the state locally
  this->publish_state(state);
  
  ESP_LOGI(TAG, "'%s': State set to %s", this->get_name().c_str(), state ? "ON" : "OFF");
}

void KNXSwitch::on_knx_telegram(const std::string &ga, const std::vector<uint8_t> &data) {
  // Check if this telegram is for our state feedback group address
  if (!this->state_ga_id_.empty()) {
    auto state_ga = this->knx_->get_group_address(this->state_ga_id_);
    if (state_ga != nullptr && state_ga->get_address() == ga) {
      // Decode state
      bool knx_state = DPT::decode_dpt1(data);
      
      // Apply inversion if configured
      bool state = this->invert_ ? !knx_state : knx_state;
      
      ESP_LOGD(TAG, "'%s': Received state feedback: %s (KNX: %s)", 
               this->get_name().c_str(), 
               state ? "ON" : "OFF",
               knx_state ? "ON" : "OFF");
      
      this->publish_state(state);
    }
  }
}

}  // namespace knx_tp
}  // namespace esphome
