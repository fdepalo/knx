#pragma once

#include "esphome/core/automation.h"
#include "esphome/core/component.h"
#include "dpt.h"
#include <string>
#include <vector>

namespace esphome {
namespace knx_tp {

/**
 * Trigger for generic telegram (called for ALL telegrams)
 * Variables: group_address (string), data (vector<uint8_t>)
 */
class TelegramTrigger : public Trigger<std::string, std::vector<uint8_t>> {
 public:
  explicit TelegramTrigger() {}
};

/**
 * Trigger for specific group address (called only for matching GA)
 * Variables: data (vector<uint8_t>)
 */
class GroupAddressTrigger : public Trigger<std::vector<uint8_t>> {
 public:
  explicit GroupAddressTrigger() {}
};

// ============================================================================
// KNX DPT Decode Actions
// These actions simplify DPT decoding in automations
// ============================================================================

/**
 * Action: Decode DPT 1 (Boolean)
 * Usage:
 *   - knx_tp.decode_dpt1:
 *       then:
 *         - if:
 *             condition:
 *               lambda: 'return x;'
 *             then:
 *               - switch.turn_on: my_switch
 */
template<typename... Ts>
class DecodeDPT1Action : public Action<Ts...> {
 public:
  void play(Ts... x) override {
    // Access the 'data' variable from trigger context
    // Note: This assumes 'data' is available in the parent automation
  }
};

/**
 * Lambda helper functions for DPT decoding
 * These can be used directly in lambda expressions
 *
 * Usage example:
 *   using namespace esphome::knx_tp::dpt_helpers;
 *   float temp = decode_dpt9(data);  // DPT 9.001 = temperature
 *   float hum = decode_dpt9(data);   // DPT 9.007 = humidity
 *   bool state = decode_dpt1(data);  // DPT 1.001 = switch
 */
namespace dpt_helpers {

/**
 * Decode DPT 1.xxx - Boolean (1 bit)
 * Usage: DPT 1.001 (switch), DPT 1.002 (bool), etc.
 * @param data Raw KNX telegram data
 * @return true or false
 */
inline bool decode_dpt1(const std::vector<uint8_t> &data) {
  return DPT::decode_dpt1(data);
}

/**
 * Decode DPT 5.xxx - 8-bit unsigned value (0-255)
 * Usage: DPT 5.004 (percent 0-255), DPT 5.010 (counter), etc.
 * @param data Raw KNX telegram data
 * @return Value 0-255
 */
inline uint8_t decode_dpt5(const std::vector<uint8_t> &data) {
  return DPT::decode_dpt5(data);
}

/**
 * Decode DPT 5.001 - Percentage (0-100%)
 * @param data Raw KNX telegram data
 * @return Percentage 0.0-100.0
 */
inline float decode_dpt5_percentage(const std::vector<uint8_t> &data) {
  return DPT::decode_dpt5_percentage(data);
}

/**
 * Decode DPT 5.003 - Angle (0-360°)
 * @param data Raw KNX telegram data
 * @return Angle 0.0-360.0 degrees
 */
inline float decode_dpt5_angle(const std::vector<uint8_t> &data) {
  return DPT::decode_dpt5_angle(data);
}

/**
 * Decode DPT 9.xxx - 2-byte float (-671088.64 to 670760.96)
 * Usage: DPT 9.001 (temperature), DPT 9.004 (brightness),
 *        DPT 9.005 (speed), DPT 9.006 (pressure), DPT 9.007 (humidity), etc.
 * @param data Raw KNX telegram data
 * @return Float value (user must interpret based on DPT subtype)
 */
inline float decode_dpt9(const std::vector<uint8_t> &data) {
  return DPT::decode_dpt9(data);
}

/**
 * Decode DPT 14.xxx - 4-byte float (IEEE 754)
 * Usage: DPT 14.000 to DPT 14.079 (various physical units)
 * @param data Raw KNX telegram data
 * @return Float value (user must interpret based on DPT subtype)
 */
inline float decode_dpt14(const std::vector<uint8_t> &data) {
  return DPT::decode_dpt14(data);
}

/**
 * Decode DPT 16.001 - Character string (ASCII, 14 bytes max)
 * @param data Raw KNX telegram data
 * @return String value
 */
inline std::string decode_dpt16(const std::vector<uint8_t> &data) {
  return DPT::decode_dpt16(data);
}

/**
 * Decode DPT 10.001 - Time of Day
 * @param data Raw KNX telegram data
 * @return TimeOfDay struct with day_of_week, hour, minute, second
 */
inline DPT::TimeOfDay decode_dpt10(const std::vector<uint8_t> &data) {
  return DPT::decode_dpt10(data);
}

/**
 * Decode DPT 11.001 - Date
 * @param data Raw KNX telegram data
 * @return Date struct with day, month, year
 */
inline DPT::Date decode_dpt11(const std::vector<uint8_t> &data) {
  return DPT::decode_dpt11(data);
}

/**
 * Decode DPT 19.001 - Date and Time
 * @param data Raw KNX telegram data
 * @return DateTime struct
 */
inline DPT::DateTime decode_dpt19(const std::vector<uint8_t> &data) {
  return DPT::decode_dpt19(data);
}

/**
 * Decode DPT 20.102 - HVAC Mode
 * @param data Raw KNX telegram data
 * @return HVACMode enum value
 */
inline DPT::HVACMode decode_dpt20_102(const std::vector<uint8_t> &data) {
  return DPT::decode_dpt20_102(data);
}

}  // namespace dpt_helpers

}  // namespace knx_tp
}  // namespace esphome
