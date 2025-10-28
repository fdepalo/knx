#pragma once

#include <vector>
#include <cstdint>
#include <string>

namespace esphome {
namespace knx_ip {

/**
 * KNX Datapoint Type (DPT) encoding and decoding utilities
 * Implements common DPT formats used in KNX communication
 */
class DPT {
 public:
  // DPT 1.xxx - Boolean (1 bit)
  static bool decode_dpt1(const std::vector<uint8_t> &data);
  static std::vector<uint8_t> encode_dpt1(bool value);
  
  // DPT 5.xxx - 8-bit unsigned value (0-255)
  static uint8_t decode_dpt5(const std::vector<uint8_t> &data);
  static std::vector<uint8_t> encode_dpt5(uint8_t value);
  
  // DPT 5.001 - Percentage (0-100%)
  static float decode_dpt5_percentage(const std::vector<uint8_t> &data);
  static std::vector<uint8_t> encode_dpt5_percentage(float value);
  
  // DPT 5.003 - Angle (0-360°)
  static float decode_dpt5_angle(const std::vector<uint8_t> &data);
  static std::vector<uint8_t> encode_dpt5_angle(float value);
  
  // DPT 9.xxx - 2-byte float
  static float decode_dpt9(const std::vector<uint8_t> &data);
  static std::vector<uint8_t> encode_dpt9(float value);
  
  // DPT 14.xxx - 4-byte float
  static float decode_dpt14(const std::vector<uint8_t> &data);
  static std::vector<uint8_t> encode_dpt14(float value);
  
  // DPT 16.001 - Character string (ASCII)
  static std::string decode_dpt16(const std::vector<uint8_t> &data);
  static std::vector<uint8_t> encode_dpt16(const std::string &value);
  
  // DPT 20.102 - HVAC Mode
  enum class HVACMode : uint8_t {
    AUTO = 0,
    COMFORT = 1,
    STANDBY = 2,
    NIGHT = 3,
    FROST_PROTECTION = 4
  };
  static HVACMode decode_dpt20_102(const std::vector<uint8_t> &data);
  static std::vector<uint8_t> encode_dpt20_102(HVACMode mode);

  // DPT 10.001 - Time of Day (3 bytes)
  struct TimeOfDay {
    uint8_t day_of_week;  // 0=no day, 1=Monday, ..., 7=Sunday
    uint8_t hour;         // 0-23
    uint8_t minute;       // 0-59
    uint8_t second;       // 0-59
  };
  static TimeOfDay decode_dpt10(const std::vector<uint8_t> &data);
  static std::vector<uint8_t> encode_dpt10(const TimeOfDay &time);

  // DPT 11.001 - Date (3 bytes)
  struct Date {
    uint8_t day;    // 1-31
    uint8_t month;  // 1-12
    uint16_t year;  // Full year (e.g., 2024)
  };
  static Date decode_dpt11(const std::vector<uint8_t> &data);
  static std::vector<uint8_t> encode_dpt11(const Date &date);

  // DPT 19.001 - Date and Time (8 bytes)
  // Ottimizzato per minimizzare padding (allineamento memoria)
  struct DateTime {
    uint16_t year;        // 1990-2089 (2 bytes)
    uint8_t month;        // 1-12 (1 byte)
    uint8_t day;          // 1-31 (1 byte)
    uint8_t day_of_week;  // 0=no day, 1=Monday, ..., 7=Sunday (1 byte)
    uint8_t hour;         // 0-23 (1 byte)
    uint8_t minute;       // 0-59 (1 byte)
    uint8_t second;       // 0-59 (1 byte)
    uint8_t quality;      // Quality of clock (0=no fault, 1=fault) (1 byte)
    bool fault;           // Fault flag (1 byte)
    bool working_day;     // Working day flag (1 byte)
    bool no_wd;           // No working day valid flag (1 byte)
    bool no_year;         // No year valid flag (1 byte)
    bool no_date;         // No date valid flag (1 byte)
    bool no_dow;          // No day of week valid flag (1 byte)
    bool no_time;         // No time valid flag (1 byte)
    bool summer_time;     // Summer time flag (1 byte)
    // Totale: 16 bytes (vs ~20 prima con padding)
  };
  static DateTime decode_dpt19(const std::vector<uint8_t> &data);
  static std::vector<uint8_t> encode_dpt19(const DateTime &datetime);

 private:
  // Helper functions
  static float clamp(float value, float min, float max);
};

}  // namespace knx_ip
}  // namespace esphome
