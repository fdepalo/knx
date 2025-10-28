#pragma once

#include "esphome/components/sensor/sensor.h"
#include "esphome/core/component.h"
#include "knx_tp.h"

namespace esphome {
namespace knx_tp {

/**
 * Sensor value types supported by KNX
 */
enum KNXSensorType {
  KNX_SENSOR_TYPE_TEMPERATURE,    // DPT 9.001 - Temperature in °C
  KNX_SENSOR_TYPE_HUMIDITY,       // DPT 9.007 - Humidity in %
  KNX_SENSOR_TYPE_BRIGHTNESS,     // DPT 9.004 - Illuminance in lux
  KNX_SENSOR_TYPE_PRESSURE,       // DPT 9.006 - Pressure in Pa
  KNX_SENSOR_TYPE_PERCENTAGE,     // DPT 5.001 - Percentage 0-100%
  KNX_SENSOR_TYPE_ANGLE,          // DPT 5.003 - Angle 0-360°
  KNX_SENSOR_TYPE_GENERIC_1BYTE,  // DPT 5.xxx - Generic 1-byte unsigned
  KNX_SENSOR_TYPE_GENERIC_2BYTE,  // DPT 9.xxx - Generic 2-byte float
  KNX_SENSOR_TYPE_GENERIC_4BYTE,  // DPT 14.xxx - Generic 4-byte float
};

/**
 * KNX Sensor
 * Receives analog values from KNX bus
 */
class KNXSensor : public sensor::Sensor, public Component, public KNXEntity {
 public:
  void setup() override;
  void dump_config() override;
  float get_setup_priority() const override { return setup_priority::DATA; }
  
  void set_state_ga_id(const std::string &ga_id) { state_ga_id_ = ga_id; }
  void set_sensor_type(KNXSensorType type) { sensor_type_ = type; }
  
  void on_knx_telegram(const std::string &ga, const std::vector<uint8_t> &data) override;

 protected:
  std::string state_ga_id_;
  KNXSensorType sensor_type_{KNX_SENSOR_TYPE_GENERIC_2BYTE};
  
  const char* sensor_type_to_string(KNXSensorType type);
};

}  // namespace knx_tp
}  // namespace esphome
