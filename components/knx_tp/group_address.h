#pragma once

#include <string>

namespace esphome {
namespace knx_tp {

/**
 * Represents a KNX Group Address
 * Group addresses are used for multicast communication in KNX
 */
class GroupAddress {
 public:
  GroupAddress() = default;
  
  void set_id(const std::string &id) { id_ = id; }
  std::string get_id() const { return id_; }

  void set_address(const std::string &address);
  void set_address(uint16_t address) { address_ = address; }
  std::string get_address() const;
  uint16_t get_address_int() const { return address_; }

  /**
   * Get address components
   */
  uint8_t get_main_group() const { return (address_ >> 11) & 0x1F; }
  uint8_t get_middle_group() const { return (address_ >> 8) & 0x07; }
  uint8_t get_sub_group() const { return address_ & 0xFF; }

 protected:
  std::string id_;          // Manteniamo solo l'ID come stringa (necessario per lookup)
  uint16_t address_{0};     // Indirizzo in formato intero (risparmio ~29 bytes)
};

}  // namespace knx_tp
}  // namespace esphome
