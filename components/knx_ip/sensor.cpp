#include "sensor.h"
#include "dpt.h"
#include "esphome/core/log.h"

namespace esphome {
namespace knx_ip {

static constexpr const char* TAG = "knx_ip.sensor";

void KNXSensor::setup() {
  ESP_LOGCONFIG(TAG, "Setting up KNX Sensor '%s'...", this->get_name().c_str());
  
  if (this->knx_ != nullptr) {
    this->knx_->register_entity(this);
    ESP_LOGD(TAG, "Registered with KNX component");
  } else {
    ESP_LOGE(TAG, "KNX component is nullptr!");
  }
}

void KNXSensor::dump_config() {
  LOG_SENSOR("", "KNX Sensor", this);
  ESP_LOGCONFIG(TAG, "  State GA: %s", this->state_ga_id_.c_str());
  ESP_LOGCONFIG(TAG, "  Sensor Type: %s", this->sensor_type_to_string(this->sensor_type_));
}

void KNXSensor::on_knx_telegram(const std::string &ga, const std::vector<uint8_t> &data) {
  // Check if this telegram is for our group address
  auto our_ga = this->knx_->get_group_address(this->state_ga_id_);
  if (our_ga == nullptr || our_ga->get_address() != ga) {
    return;  // Not for us
  }
  
  ESP_LOGD(TAG, "'%s': Received telegram on GA %s", this->get_name().c_str(), ga.c_str());
  
  if (data.empty()) {
    ESP_LOGW(TAG, "'%s': Received empty data", this->get_name().c_str());
    return;
  }
  
  float value = 0.0f;
  
  // Decode based on sensor type
  switch (this->sensor_type_) {
    case KNX_SENSOR_TYPE_TEMPERATURE:
    case KNX_SENSOR_TYPE_HUMIDITY:
    case KNX_SENSOR_TYPE_BRIGHTNESS:
    case KNX_SENSOR_TYPE_PRESSURE:
    case KNX_SENSOR_TYPE_GENERIC_2BYTE:
      value = DPT::decode_dpt9(data);
      break;

    case KNX_SENSOR_TYPE_PERCENTAGE:
      value = DPT::decode_dpt5_percentage(data);
      break;

    case KNX_SENSOR_TYPE_ANGLE:
      value = DPT::decode_dpt5_angle(data);
      break;

    case KNX_SENSOR_TYPE_GENERIC_1BYTE:
      value = static_cast<float>(DPT::decode_dpt5(data));
      break;

    case KNX_SENSOR_TYPE_GENERIC_4BYTE:
      value = DPT::decode_dpt14(data);
      break;
  }
  
  ESP_LOGD(TAG, "'%s': Decoded value: %.2f", this->get_name().c_str(), value);
  
  // Publish the value
  this->publish_state(value);
  
  ESP_LOGI(TAG, "'%s': New value: %.2f", this->get_name().c_str(), value);
}

const char* KNXSensor::sensor_type_to_string(KNXSensorType type) {
  switch (type) {
    case KNX_SENSOR_TYPE_TEMPERATURE: return "Temperature (DPT 9.001)";
    case KNX_SENSOR_TYPE_HUMIDITY: return "Humidity (DPT 9.007)";
    case KNX_SENSOR_TYPE_BRIGHTNESS: return "Brightness (DPT 9.004)";
    case KNX_SENSOR_TYPE_PRESSURE: return "Pressure (DPT 9.006)";
    case KNX_SENSOR_TYPE_PERCENTAGE: return "Percentage (DPT 5.001)";
    case KNX_SENSOR_TYPE_ANGLE: return "Angle (DPT 5.003)";
    case KNX_SENSOR_TYPE_GENERIC_1BYTE: return "Generic 1-byte (DPT 5.xxx)";
    case KNX_SENSOR_TYPE_GENERIC_2BYTE: return "Generic 2-byte (DPT 9.xxx)";
    case KNX_SENSOR_TYPE_GENERIC_4BYTE: return "Generic 4-byte (DPT 14.xxx)";
    default: return "Unknown";
  }
}

}  // namespace knx_ip
}  // namespace esphome
