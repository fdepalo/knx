#include "group_address.h"
#include <algorithm>
#include <cstdlib>
#include <cstdio>

namespace esphome {
namespace knx_ip {

void GroupAddress::set_address(const std::string &address) {
  // Parse address format: "main.middle.sub" or "main/middle/sub"
  std::string addr = address;
  std::replace(addr.begin(), addr.end(), '/', '.');

  size_t first_dot = addr.find('.');
  size_t second_dot = addr.find('.', first_dot + 1);

  if (first_dot == std::string::npos || second_dot == std::string::npos) {
    this->address_ = 0;
    return;
  }

  // Parse without exceptions (not enabled in ESP-IDF by default)
  std::string main_str = addr.substr(0, first_dot);
  std::string middle_str = addr.substr(first_dot + 1, second_dot - first_dot - 1);
  std::string sub_str = addr.substr(second_dot + 1);

  // Simple integer parsing without exceptions
  char *endptr;
  long main_val = strtol(main_str.c_str(), &endptr, 10);
  if (*endptr != '\0' || main_val < 0 || main_val > 31) {
    this->address_ = 0;
    return;
  }

  long middle_val = strtol(middle_str.c_str(), &endptr, 10);
  if (*endptr != '\0' || middle_val < 0 || middle_val > 7) {
    this->address_ = 0;
    return;
  }

  long sub_val = strtol(sub_str.c_str(), &endptr, 10);
  if (*endptr != '\0' || sub_val < 0 || sub_val > 255) {
    this->address_ = 0;
    return;
  }

  // Encode: AAAAA/BBB/CCCCCCCC -> area (5 bits), line (3 bits), device (8 bits)
  this->address_ = ((main_val & 0x1F) << 11) | ((middle_val & 0x07) << 8) | (sub_val & 0xFF);
}

std::string GroupAddress::get_address() const {
  int area = (this->address_ >> 11) & 0x1F;
  int line = (this->address_ >> 8) & 0x07;
  int device = this->address_ & 0xFF;

  char addr[16];
  snprintf(addr, sizeof(addr), "%d/%d/%d", area, line, device);
  return std::string(addr);
}

}  // namespace knx_ip
}  // namespace esphome
