#include "dpt.h"
#include <cstring>
#include <algorithm>
#include <cmath>

namespace esphome {
namespace knx_tp {

// DPT 1.xxx - Boolean
bool DPT::decode_dpt1(const std::vector<uint8_t> &data) {
  if (data.empty()) return false;
  return (data[0] & 0x01) != 0;
}

std::vector<uint8_t> DPT::encode_dpt1(bool value) {
  return {value ? (uint8_t)0x01 : (uint8_t)0x00};
}

// DPT 5.xxx - 8-bit unsigned
uint8_t DPT::decode_dpt5(const std::vector<uint8_t> &data) {
  if (data.empty()) return 0;
  return data[0];
}

std::vector<uint8_t> DPT::encode_dpt5(uint8_t value) {
  return {value};
}

// DPT 5.001 - Percentage
float DPT::decode_dpt5_percentage(const std::vector<uint8_t> &data) {
  if (data.empty()) return 0.0f;
  return (data[0] * 100.0f) / 255.0f;
}

std::vector<uint8_t> DPT::encode_dpt5_percentage(float value) {
  value = clamp(value, 0.0f, 100.0f);
  uint8_t scaled = static_cast<uint8_t>(std::round(value * 255.0f / 100.0f));
  return {scaled};
}

// DPT 5.003 - Angle
float DPT::decode_dpt5_angle(const std::vector<uint8_t> &data) {
  if (data.empty()) return 0.0f;
  return (data[0] * 360.0f) / 255.0f;
}

std::vector<uint8_t> DPT::encode_dpt5_angle(float value) {
  // Validate input: check for NaN and Infinity to prevent infinite loops
  if (!std::isfinite(value)) {
    return {0};  // Return 0 degrees for invalid values
  }

  // Normalize angle to 0-360 range
  while (value < 0.0f) value += 360.0f;
  while (value >= 360.0f) value -= 360.0f;
  uint8_t scaled = static_cast<uint8_t>(std::round(value * 255.0f / 360.0f));
  return {scaled};
}

// DPT 9.xxx - 2-byte float
float DPT::decode_dpt9(const std::vector<uint8_t> &data) {
  // Strict validation: need exactly 2 bytes for DPT 9
  if (data.size() < 2 || data.empty()) return 0.0f;

  // Safe cast to uint16_t with explicit cast
  uint16_t raw = (static_cast<uint16_t>(data[0]) << 8) | static_cast<uint16_t>(data[1]);
  
  // Extract mantissa (11 bits) and exponent (4 bits)
  int16_t mantissa = raw & 0x7FF;
  uint8_t exponent = (raw >> 11) & 0x0F;
  
  // Check sign bit
  if (raw & 0x8000) {
    // Negative value: two's complement
    mantissa = -(~(mantissa - 1) & 0x7FF);
  }
  
  // Calculate value: mantissa * 2^exponent * 0.01
  return (0.01f * mantissa) * (1 << exponent);
}

std::vector<uint8_t> DPT::encode_dpt9(float value) {
  // Validate input: check for NaN and Infinity
  if (!std::isfinite(value)) {
    return {0x00, 0x00};  // Return zero for invalid values
  }

  // Clamp to DPT 9 valid range BEFORE multiplication to prevent overflow
  // DPT 9 range: -671088.64 to 670760.96
  if (value < -671088.64f) value = -671088.64f;
  if (value > 670760.96f) value = 670760.96f;

  // Use int32_t to avoid overflow during scaling
  int32_t mantissa32 = static_cast<int32_t>(value * 100.0f);
  uint8_t exponent = 0;

  // Scale mantissa to fit in 11 bits (-2048 to 2047)
  // Use division instead of shift for portability with negative numbers
  while ((mantissa32 < -2048 || mantissa32 > 2047) && exponent < 15) {
    mantissa32 = mantissa32 / 2;
    exponent++;
  }

  // Safe cast to int16_t after scaling
  int16_t mantissa = static_cast<int16_t>(mantissa32);

  // Final clamp (should not be needed after division loop)
  if (mantissa < -2048) mantissa = -2048;
  if (mantissa > 2047) mantissa = 2047;
  
  // Encode: MEEEEMMM MMMMMMMM
  // M = sign bit, E = exponent, M = mantissa
  uint16_t raw = ((exponent & 0x0F) << 11) | (mantissa & 0x7FF);
  if (mantissa < 0) {
    raw |= 0x8000;  // Set sign bit
  }
  
  return {
    static_cast<uint8_t>(raw >> 8),
    static_cast<uint8_t>(raw & 0xFF)
  };
}

// DPT 14.xxx - 4-byte float
float DPT::decode_dpt14(const std::vector<uint8_t> &data) {
  if (data.size() < 4) return 0.0f;
  
  // IEEE 754 single precision float
  uint32_t raw = (data[0] << 24) | (data[1] << 16) | (data[2] << 8) | data[3];
  float value;
  memcpy(&value, &raw, sizeof(float));
  return value;
}

std::vector<uint8_t> DPT::encode_dpt14(float value) {
  // IEEE 754 single precision float
  uint32_t raw;
  memcpy(&raw, &value, sizeof(float));
  
  return {
    static_cast<uint8_t>(raw >> 24),
    static_cast<uint8_t>((raw >> 16) & 0xFF),
    static_cast<uint8_t>((raw >> 8) & 0xFF),
    static_cast<uint8_t>(raw & 0xFF)
  };
}

// DPT 16.001 - Character string (max 14 characters per KNX spec)
std::string DPT::decode_dpt16(const std::vector<uint8_t> &data) {
  constexpr size_t MAX_DPT16_LENGTH = 14;

  std::string result;
  result.reserve(std::min(data.size(), MAX_DPT16_LENGTH));

  size_t count = 0;
  for (uint8_t byte : data) {
    if (byte == 0 || count >= MAX_DPT16_LENGTH) break;  // Null terminator or max length

    // Validate printable ASCII characters (0x20-0x7E)
    if (byte >= 0x20 && byte <= 0x7E) {
      result += static_cast<char>(byte);
      count++;
    }
  }

  return result;
}

std::vector<uint8_t> DPT::encode_dpt16(const std::string &value) {
  // DPT 16.001 max length is 14 characters per KNX specification
  constexpr size_t MAX_DPT16_LENGTH = 14;

  std::vector<uint8_t> result;
  result.reserve(std::min(value.length(), MAX_DPT16_LENGTH) + 1);

  size_t count = 0;
  for (char c : value) {
    if (count >= MAX_DPT16_LENGTH) break;  // Enforce max length

    // Only encode printable ASCII characters (0x20-0x7E)
    if (c >= 0x20 && c <= 0x7E) {
      result.push_back(static_cast<uint8_t>(c));
      count++;
    }
  }

  result.push_back(0);  // Null terminator
  return result;
}

// DPT 20.102 - HVAC Mode
DPT::HVACMode DPT::decode_dpt20_102(const std::vector<uint8_t> &data) {
  if (data.empty()) return HVACMode::AUTO;

  uint8_t mode = data[0];
  if (mode <= static_cast<uint8_t>(HVACMode::FROST_PROTECTION)) {
    return static_cast<HVACMode>(mode);
  }

  return HVACMode::AUTO;
}

std::vector<uint8_t> DPT::encode_dpt20_102(HVACMode mode) {
  return {static_cast<uint8_t>(mode)};
}

// DPT 10.001 - Time of Day (3 bytes)
// Format: Byte 0: day_of_week(3 bits) + hour(5 bits)
//         Byte 1: reserved(2 bits) + minute(6 bits)
//         Byte 2: reserved(2 bits) + second(6 bits)
DPT::TimeOfDay DPT::decode_dpt10(const std::vector<uint8_t> &data) {
  TimeOfDay time = {0, 0, 0, 0};

  if (data.size() < 3) return time;

  // Byte 0: bits 7-5 = day of week, bits 4-0 = hour
  time.day_of_week = (data[0] >> 5) & 0x07;
  time.hour = data[0] & 0x1F;

  // Byte 1: bits 5-0 = minute
  time.minute = data[1] & 0x3F;

  // Byte 2: bits 5-0 = second
  time.second = data[2] & 0x3F;

  return time;
}

std::vector<uint8_t> DPT::encode_dpt10(const TimeOfDay &time) {
  // Clamp values to valid ranges
  uint8_t dow = time.day_of_week & 0x07;  // 0-7
  uint8_t hour = time.hour > 23 ? 23 : time.hour;
  uint8_t minute = time.minute > 59 ? 59 : time.minute;
  uint8_t second = time.second > 59 ? 59 : time.second;

  return {
    static_cast<uint8_t>((dow << 5) | hour),
    minute,
    second
  };
}

// DPT 11.001 - Date (3 bytes)
// Format: Byte 0: day (1-31)
//         Byte 1: month (1-12)
//         Byte 2: year (0-99)
//         Year encoding: 0-89 = 2000-2089, 90-99 = 1990-1999
DPT::Date DPT::decode_dpt11(const std::vector<uint8_t> &data) {
  Date date = {1, 1, 2000};

  if (data.size() < 3) return date;

  date.day = data[0];
  date.month = data[1];
  uint8_t year_byte = data[2];

  // Convert year byte to full year
  if (year_byte < 90) {
    date.year = 2000 + year_byte;
  } else {
    date.year = 1900 + year_byte;
  }

  return date;
}

std::vector<uint8_t> DPT::encode_dpt11(const Date &date) {
  // Clamp values
  uint8_t day = date.day;
  if (day < 1) day = 1;
  if (day > 31) day = 31;

  uint8_t month = date.month;
  if (month < 1) month = 1;
  if (month > 12) month = 12;

  // Convert full year to year byte
  uint8_t year_byte;
  if (date.year >= 2000 && date.year <= 2089) {
    year_byte = date.year - 2000;
  } else if (date.year >= 1990 && date.year <= 1999) {
    year_byte = date.year - 1900;
  } else {
    // Default to 2000 if out of range
    year_byte = 0;
  }

  return {day, month, year_byte};
}

// DPT 19.001 - Date and Time (8 bytes)
// Format: Bytes 0-1: year (1990-2089)
//         Byte 2: month (1-12)
//         Byte 3: day (1-31)
//         Byte 4: day_of_week(3 bits) + hour(5 bits)
//         Byte 5: minute (0-59)
//         Byte 6: second (0-59)
//         Byte 7: fault/quality flags
DPT::DateTime DPT::decode_dpt19(const std::vector<uint8_t> &data) {
  DateTime dt = {2000, 1, 1, 0, 0, 0, 0, 0, false, false, false, false, false, false, false, false};

  // Strict validation: DPT 19.001 requires exactly 8 bytes
  if (data.size() != 8 || data.empty()) return dt;

  // Defensive: verify each index is accessible
  for (size_t i = 0; i < 8; i++) {
    if (i >= data.size()) return dt;  // Safety check
  }

  // Bytes 0-1: year (safe cast)
  dt.year = (static_cast<uint16_t>(data[0]) << 8) | static_cast<uint16_t>(data[1]);

  // Byte 2: month
  dt.month = data[2];

  // Byte 3: day
  dt.day = data[3];

  // Byte 4: day_of_week (bits 7-5) + hour (bits 4-0)
  dt.day_of_week = (data[4] >> 5) & 0x07;
  dt.hour = data[4] & 0x1F;

  // Byte 5: minute
  dt.minute = data[5];

  // Byte 6: second
  dt.second = data[6];

  // Byte 7: flags
  dt.fault = (data[7] & 0x80) != 0;
  dt.working_day = (data[7] & 0x40) != 0;
  dt.no_wd = (data[7] & 0x20) != 0;
  dt.no_year = (data[7] & 0x10) != 0;
  dt.no_date = (data[7] & 0x08) != 0;
  dt.no_dow = (data[7] & 0x04) != 0;
  dt.no_time = (data[7] & 0x02) != 0;
  dt.summer_time = (data[7] & 0x01) != 0;
  dt.quality = (data[7] & 0x80) >> 7;

  // Validate ranges
  if (dt.year < 1990 || dt.year > 2089) dt.year = 2000;
  if (dt.month < 1 || dt.month > 12) dt.month = 1;
  if (dt.day < 1 || dt.day > 31) dt.day = 1;
  if (dt.day_of_week > 7) dt.day_of_week = 0;
  if (dt.hour > 23) dt.hour = 0;
  if (dt.minute > 59) dt.minute = 0;
  if (dt.second > 59) dt.second = 0;

  return dt;
}

std::vector<uint8_t> DPT::encode_dpt19(const DateTime &dt) {
  // Clamp values
  uint16_t year = dt.year;
  if (year < 1990) year = 1990;
  if (year > 2089) year = 2089;

  uint8_t month = dt.month;
  if (month < 1) month = 1;
  if (month > 12) month = 12;

  uint8_t day = dt.day;
  if (day < 1) day = 1;
  if (day > 31) day = 31;

  uint8_t dow = dt.day_of_week & 0x07;
  uint8_t hour = dt.hour > 23 ? 23 : dt.hour;
  uint8_t minute = dt.minute > 59 ? 59 : dt.minute;
  uint8_t second = dt.second > 59 ? 59 : dt.second;

  // Build flags byte
  uint8_t flags = 0;
  if (dt.fault) flags |= 0x80;
  if (dt.working_day) flags |= 0x40;
  if (dt.no_wd) flags |= 0x20;
  if (dt.no_year) flags |= 0x10;
  if (dt.no_date) flags |= 0x08;
  if (dt.no_dow) flags |= 0x04;
  if (dt.no_time) flags |= 0x02;
  if (dt.summer_time) flags |= 0x01;

  return {
    static_cast<uint8_t>(year >> 8),
    static_cast<uint8_t>(year & 0xFF),
    month,
    day,
    static_cast<uint8_t>((dow << 5) | hour),
    minute,
    second,
    flags
  };
}

// Helper functions
float DPT::clamp(float value, float min, float max) {
  if (value < min) return min;
  if (value > max) return max;
  return value;
}

}  // namespace knx_tp
}  // namespace esphome
