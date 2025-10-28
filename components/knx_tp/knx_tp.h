#pragma once

#include "esphome/core/component.h"
#include "esphome/core/hal.h"
#include "esphome/components/uart/uart.h"
#include "esphome/core/automation.h"
#include "group_address.h"
#include "dpt.h"
#include <vector>
#include <string>
#include <unordered_map>

// Define MASK_VERSION before including KNX headers
#ifndef MASK_VERSION
#define MASK_VERSION 0x07B0
#endif

// Compile-time flags to enable/disable trigger features
// Can be overridden with build flags: -DUSE_KNX_ON_TELEGRAM=0
#ifndef USE_KNX_ON_TELEGRAM
#define USE_KNX_ON_TELEGRAM 1  // Default: enabled
#endif

#ifndef USE_KNX_ON_GROUP_ADDRESS
#define USE_KNX_ON_GROUP_ADDRESS 1  // Default: enabled
#endif

// Forward declarations for Thelsing KNX stack
class Esp32IdfPlatform;
class Bau07B0;  // BAU class is in global namespace, not knx::

namespace esphome {

// Forward declaration for time component (optional dependency)
namespace time {
class RealTimeClock;
}

namespace knx_tp {

class KNXEntity;

/**
 * Main KNX TP Component
 * Handles KNX Twisted Pair communication and entity management
 * Integrates with Thelsing KNX stack
 */
class KNXTPComponent : public Component, public uart::UARTDevice {
 public:
  void setup() override;
  void loop() override;
  void dump_config() override;
  float get_setup_priority() const override { return setup_priority::AFTER_CONNECTION; }

  // Destructor for cleanup of external library resources
  ~KNXTPComponent();

  // Configuration
  void set_physical_address(const std::string &address);
  void register_group_address(GroupAddress *ga);
  void register_entity(KNXEntity *entity);
  void set_uart_parent(uart::UARTComponent *parent);

  // SAV pin configuration (BCU connection detection)
  void set_sav_pin(GPIOPin *pin) { sav_pin_ = pin; }
  bool is_bcu_connected() const { return bcu_connected_; }

  // Time broadcast configuration
  void set_time_source(time::RealTimeClock *time_source) { time_source_ = time_source; }
  void set_time_broadcast_ga(const std::string &ga_id) { time_broadcast_ga_id_ = ga_id; }
  void set_time_broadcast_interval(uint32_t interval_ms) { time_broadcast_interval_ = interval_ms; }

  // Communication
  void send_telegram(const std::string &dest_addr, const std::vector<uint8_t> &data);
  void send_group_write(const std::string &ga_id, const std::vector<uint8_t> &data);
  void send_group_read(const std::string &ga_id);
  void send_group_response(const std::string &ga_id, const std::vector<uint8_t> &data);

  // Accessors
  GroupAddress *get_group_address(const std::string &id);
  std::string get_physical_address() const { return physical_address_; }

  // Thelsing KNX stack integration
  Bau07B0* get_bau() { return bau_; }

#if USE_KNX_ON_TELEGRAM
  // Register generic telegram trigger (called for ALL telegrams)
  void add_on_telegram_callback(std::function<void(std::string, std::vector<uint8_t>)> &&callback) {
    this->telegram_callbacks_.add(std::move(callback));
  }
#endif

#if USE_KNX_ON_GROUP_ADDRESS
  // Register group address specific trigger (called only for matching GA)
  void add_on_group_address_callback(uint16_t ga_int, std::function<void(std::vector<uint8_t>)> &&callback);
#endif

 protected:
  std::string physical_address_;
  std::vector<GroupAddress *> group_addresses_;
  std::unordered_map<std::string, GroupAddress *> ga_lookup_;  // O(1) lookup by ID
  std::vector<KNXEntity *> entities_;

  // Thelsing KNX stack objects
  Bau07B0 *bau_{nullptr};  // BAU is in global namespace
  Esp32IdfPlatform *platform_{nullptr};
  uint16_t physical_address_int_{0};

  // SAV pin for BCU connection detection
  GPIOPin *sav_pin_{nullptr};
  bool bcu_connected_{false};
  bool bcu_connected_last_{false};

  // Time broadcast
  time::RealTimeClock *time_source_{nullptr};
  std::string time_broadcast_ga_id_;
  uint32_t time_broadcast_interval_{60000};  // Default: 60 seconds
  uint32_t last_time_broadcast_{0};
  void broadcast_time_();

  // Telegram processing
  void parse_telegram_(const std::vector<uint8_t> &telegram);
  void notify_entities_(const std::string &ga, uint16_t ga_int, const std::vector<uint8_t> &data);
  void group_object_callback_(uint16_t ga, const uint8_t *data, uint8_t len);

  // Utilities
  uint8_t calculate_checksum_(const std::vector<uint8_t> &data);
  std::vector<uint8_t> encode_address_(const std::string &address);
  std::string decode_address_(const std::vector<uint8_t> &data, size_t offset);
  uint16_t address_to_int_(const std::string &address);
  std::string int_to_address_(uint16_t address);

#if USE_KNX_ON_TELEGRAM
  // Generic telegram triggers (called for every telegram)
  CallbackManager<void(std::string, std::vector<uint8_t>)> telegram_callbacks_;
#endif

#if USE_KNX_ON_GROUP_ADDRESS
  // Group address specific triggers (O(1) lookup via hash map)
  std::unordered_map<uint16_t, CallbackManager<void(std::vector<uint8_t>)>> ga_callbacks_;
#endif
};

/**
 * Base class for all KNX entities
 * Nota: std::vector<uint8_t> per telegrammi usa Small Buffer Optimization (SBO)
 * per telegrammi < 16 bytes, quindi non alloca heap per la maggior parte dei casi
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

  void set_knx_component(KNXTPComponent *knx) { knx_ = knx; }

 protected:
  KNXTPComponent *knx_{nullptr};
};

}  // namespace knx_tp
}  // namespace esphome
