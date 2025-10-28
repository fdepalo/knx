#pragma once

#include "esphome/core/component.h"
#include "esphome/core/hal.h"
#include "group_address.h"
#include "dpt.h"
#include <vector>
#include <string>

// Define MASK_VERSION for KNX-IP before including KNX headers
#ifndef MASK_VERSION
#define MASK_VERSION 0x57B0  // IP device (vs 0x07B0 for TP)
#endif

// Forward declarations for Thelsing KNX stack
class Esp32IdfPlatform;
class Bau57B0;  // IP BAU (vs Bau07B0 for TP)

namespace esphome {

// Forward declaration for time component (optional dependency)
namespace time {
class RealTimeClock;
}

namespace knx_ip {

// Forward declarations (actual definitions in separate headers)
class GroupAddress;
class DPT;
class KNXEntity;

/**
 * Main KNX IP Component
 * Handles KNX/IP communication via network (WiFi/Ethernet)
 * Integrates with Thelsing KNX stack using Bau57B0
 *
 * Key differences from KNX-TP:
 * - Uses IP network instead of UART
 * - Supports routing (multicast) and tunneling modes
 * - No hardware transceiver needed
 * - Default multicast: 224.0.23.12:3671
 */
class KNXIPComponent : public Component {
 public:
  void setup() override;
  void loop() override;
  void dump_config() override;
  float get_setup_priority() const override { return setup_priority::AFTER_CONNECTION; }

  // Note: Destructors removed - ESPHome components live for device lifetime
  // Memory is managed by ESPHome framework and freed on device reboot

  // Configuration
  void set_physical_address(const std::string &address);
  void register_group_address(GroupAddress *ga);
  void register_entity(KNXEntity *entity);

  // IP-specific configuration
  void set_gateway_ip(const std::string &ip) { gateway_ip_ = ip; }
  void set_gateway_port(uint16_t port) { gateway_port_ = port; }
  void set_routing_mode(bool routing) { routing_mode_ = routing; }
  void set_multicast_address(const std::string &addr) { multicast_address_ = addr; }

  // Time broadcast configuration (same as TP)
  void set_time_source(time::RealTimeClock *time_source) { time_source_ = time_source; }
  void set_time_broadcast_ga(const std::string &ga_id) { time_broadcast_ga_id_ = ga_id; }
  void set_time_broadcast_interval(uint32_t interval_ms) { time_broadcast_interval_ = interval_ms; }

  // Communication (same interface as TP for compatibility)
  void send_telegram(const std::string &dest_addr, const std::vector<uint8_t> &data);
  void send_group_write(const std::string &ga_id, const std::vector<uint8_t> &data);
  void send_group_read(const std::string &ga_id);
  void send_group_response(const std::string &ga_id, const std::vector<uint8_t> &data);

  // Accessors
  GroupAddress *get_group_address(const std::string &id);
  std::string get_physical_address() const { return physical_address_; }
  bool is_connected() const { return connected_; }

  // Thelsing KNX stack integration
  Bau57B0* get_bau() { return bau_; }

 protected:
  std::string physical_address_;
  std::vector<GroupAddress *> group_addresses_;
  std::vector<KNXEntity *> entities_;

  // IP-specific configuration
  std::string gateway_ip_;              // Gateway IP for tunneling (optional)
  uint16_t gateway_port_{3671};         // KNX/IP standard port
  bool routing_mode_{true};              // true=routing (multicast), false=tunneling
  std::string multicast_address_{"224.0.23.12"};  // Standard KNX multicast
  bool connected_{false};

  // Thelsing KNX stack objects
  Bau57B0 *bau_{nullptr};               // IP BAU
  Esp32IdfPlatform *platform_{nullptr};
  uint16_t physical_address_int_{0};

  // Time broadcast
  time::RealTimeClock *time_source_{nullptr};
  std::string time_broadcast_ga_id_;
  uint32_t time_broadcast_interval_{60000};  // Default: 60 seconds
  uint32_t last_time_broadcast_{0};
  void broadcast_time_();

  // Telegram processing
  void parse_telegram_(const std::vector<uint8_t> &telegram);
  void notify_entities_(const std::string &ga, const std::vector<uint8_t> &data);
  void group_object_callback_(uint16_t ga, const uint8_t *data, uint8_t len);

  // Utilities
  std::vector<uint8_t> encode_address_(const std::string &address);
  uint16_t parse_physical_address_(const std::string &address);
};

/**
 * Base class for KNX IP entities (sensors, switches, etc.)
 * Same interface as KNX-TP for compatibility
 */
class KNXEntity {
 public:
  virtual ~KNXEntity() = default;

  /**
   * Called when a KNX telegram is received
   * @param ga Group address that triggered this
   * @param data Payload data from the telegram
   */
  virtual void on_knx_telegram(const std::string &ga, const std::vector<uint8_t> &data) = 0;

  void set_knx_component(KNXIPComponent *knx) { knx_ = knx; }

 protected:
  KNXIPComponent *knx_{nullptr};
};

}  // namespace knx_ip
}  // namespace esphome
