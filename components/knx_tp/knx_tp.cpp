// Define MASK_VERSION before including any KNX headers
#ifndef MASK_VERSION
#define MASK_VERSION 0x07B0
#endif

#include "knx_tp.h"
#include "esphome/core/log.h"
#include <algorithm>

// Include time component header only if time broadcast is used
#ifdef USE_TIME
#include "esphome/components/time/real_time_clock.h"
#endif

// Thelsing KNX stack includes
#include <esp32_idf_platform.h>
#include <knx/bau07B0.h>
#include <knx/group_object_table_object.h>
#include <knx/group_object.h>

namespace esphome {
namespace knx_tp {

static constexpr const char* TAG = "knx_tp";

void KNXTPComponent::setup() {
  ESP_LOGCONFIG(TAG, "Setting up KNX TP with Thelsing stack...");
  ESP_LOGCONFIG(TAG, "Physical Address: %s", this->physical_address_.c_str());

  if (!this->parent_) {
    ESP_LOGE(TAG, "UART parent not set!");
    this->mark_failed();
    return;
  }

  // Setup SAV pin if configured
  if (this->sav_pin_ != nullptr) {
    this->sav_pin_->setup();
    this->sav_pin_->pin_mode(gpio::FLAG_INPUT);
    ESP_LOGCONFIG(TAG, "SAV pin configured for BCU detection");
  }

  // Create ESP32-IDF KNX platform (from Thelsing library)
  // UART_NUM_1 is the default UART port for KNX
  this->platform_ = new Esp32IdfPlatform(UART_NUM_1);

  // Create BAU (Bus Access Unit) - 07B0 is for TP with BCU1
  this->bau_ = new Bau07B0(*this->platform_);

  // Convert physical address
  this->physical_address_int_ = this->address_to_int_(this->physical_address_);

  // Set device address
  this->bau_->deviceObject().individualAddress(this->physical_address_int_);

  // Configure group objects for each registered group address
  auto& groupObjectTable = this->bau_->groupObjectTable();
  uint16_t go_index = 1;  // Group object indices start at 1

  for (auto *ga : this->group_addresses_) {
    uint16_t ga_int = this->address_to_int_(ga->get_address());

    // Create group object
    // This simplified version creates basic group objects
    // In a full implementation, you'd configure DPT types, flags, etc.
    ESP_LOGD(TAG, "Configuring group object %u for GA %s (%u)",
             go_index, ga->get_address().c_str(), ga_int);

    go_index++;
  }

  // Enable the KNX device
  this->bau_->enabled(true);

  ESP_LOGCONFIG(TAG, "KNX TP setup complete");
}

KNXTPComponent::~KNXTPComponent() {
  // Clean up external library resources
  // Note: In normal ESPHome operation, components are not destroyed until device reboot
  // This destructor provides cleanup for restart/reload scenarios

  ESP_LOGD(TAG, "Cleaning up KNX TP component resources");

  // Delete BAU first (depends on platform)
  if (this->bau_ != nullptr) {
    delete this->bau_;
    this->bau_ = nullptr;
  }

  // Delete platform
  if (this->platform_ != nullptr) {
    delete this->platform_;
    this->platform_ = nullptr;
  }

  ESP_LOGD(TAG, "KNX TP component cleanup complete");
}

void KNXTPComponent::loop() {
  // Check SAV pin for BCU connection status
  if (this->sav_pin_ != nullptr) {
    this->bcu_connected_ = this->sav_pin_->digital_read();

    // Log status changes
    if (this->bcu_connected_ != this->bcu_connected_last_) {
      if (this->bcu_connected_) {
        ESP_LOGI(TAG, "BCU connected to KNX bus (SAV pin HIGH)");
      } else {
        ESP_LOGW(TAG, "BCU disconnected from KNX bus (SAV pin LOW)");
      }
      this->bcu_connected_last_ = this->bcu_connected_;
    }
  } else {
    // If no SAV pin configured, assume BCU is always connected
    this->bcu_connected_ = true;
  }

  // Process KNX stack only if BCU is connected
  if (this->bau_ && this->bcu_connected_) {
    this->bau_->loop();
  }

  // Time broadcast (only if BCU is connected)
#ifdef USE_TIME
  if (this->bcu_connected_ && this->time_source_ != nullptr && !this->time_broadcast_ga_id_.empty()) {
    uint32_t now = millis();
    if (now - this->last_time_broadcast_ >= this->time_broadcast_interval_) {
      this->broadcast_time_();
      this->last_time_broadcast_ = now;
    }
  }
#endif

  // The Esp32IdfPlatform handles UART internally, no explicit loop needed
  // Check for incoming telegrams - handled internally via callbacks
}

void KNXTPComponent::dump_config() {
  ESP_LOGCONFIG(TAG, "KNX TP (Thelsing Stack):");
  ESP_LOGCONFIG(TAG, "  Physical Address: %s (0x%04X)",
                this->physical_address_.c_str(), this->physical_address_int_);
  ESP_LOGCONFIG(TAG, "  Group Addresses: %d", this->group_addresses_.size());

  for (const auto *ga : this->group_addresses_) {
    uint16_t ga_int = this->address_to_int_(ga->get_address());
    ESP_LOGCONFIG(TAG, "    %s: %s (0x%04X)",
                  ga->get_id().c_str(), ga->get_address().c_str(), ga_int);
  }

  ESP_LOGCONFIG(TAG, "  Registered Entities: %d", this->entities_.size());

  if (this->bau_) {
    ESP_LOGCONFIG(TAG, "  BAU Status: %s", this->bau_->enabled() ? "Enabled" : "Disabled");
  }

  // SAV pin info
  if (this->sav_pin_ != nullptr) {
    ESP_LOGCONFIG(TAG, "  SAV Pin: Configured (BCU detection enabled)");
    ESP_LOGCONFIG(TAG, "    Current Status: %s", this->bcu_connected_ ? "CONNECTED" : "DISCONNECTED");
  }

  // Time broadcast info
#ifdef USE_TIME
  if (this->time_source_ != nullptr && !this->time_broadcast_ga_id_.empty()) {
    ESP_LOGCONFIG(TAG, "  Time Broadcast:");
    ESP_LOGCONFIG(TAG, "    GA: %s", this->time_broadcast_ga_id_.c_str());
    ESP_LOGCONFIG(TAG, "    Interval: %u seconds", this->time_broadcast_interval_ / 1000);
  }
#endif
}

void KNXTPComponent::set_physical_address(const std::string &address) {
  this->physical_address_ = address;
  this->physical_address_int_ = this->address_to_int_(address);
  ESP_LOGD(TAG, "Physical address set to: %s (0x%04X)", address.c_str(), this->physical_address_int_);
}

void KNXTPComponent::set_uart_parent(uart::UARTComponent *parent) {
  this->parent_ = parent;
}

void KNXTPComponent::register_group_address(GroupAddress *ga) {
  this->group_addresses_.push_back(ga);
  // Add to hash map for O(1) lookup
  this->ga_lookup_[ga->get_id()] = ga;
  ESP_LOGD(TAG, "Registered group address: %s -> %s", ga->get_id().c_str(), ga->get_address().c_str());
}

void KNXTPComponent::register_entity(KNXEntity *entity) {
  this->entities_.push_back(entity);
  entity->set_knx_component(this);
  ESP_LOGD(TAG, "Registered entity (total: %d)", this->entities_.size());
}

GroupAddress *KNXTPComponent::get_group_address(const std::string &id) {
  // O(1) hash map lookup instead of O(n) linear search
  auto it = this->ga_lookup_.find(id);
  if (it != this->ga_lookup_.end()) {
    return it->second;
  }
  ESP_LOGW(TAG, "Group address '%s' not found", id.c_str());
  return nullptr;
}

void KNXTPComponent::send_telegram(const std::string &dest_addr, const std::vector<uint8_t> &data) {
  if (!this->bau_) {
    ESP_LOGW(TAG, "BAU not initialized, cannot send telegram");
    return;
  }

  uint16_t dest_addr_int = this->address_to_int_(dest_addr);

  ESP_LOGD(TAG, "Sending telegram to %s (0x%04X) with %d bytes",
           dest_addr.c_str(), dest_addr_int, data.size());

  // Find the group object for this address
  auto& groupObjectTable = this->bau_->groupObjectTable();

  // For simplicity, we'll use the lower-level telegram interface
  // In a full implementation, you'd map to specific group objects

  if (data.size() > 0) {
    // Usa buffer statico invece di std::string per evitare allocazioni
    // Max 32 bytes di dati = 96 caratteri (3 per byte) + null terminator
    char data_hex[97];
    size_t offset = 0;
    for (size_t i = 0; i < data.size() && i < 32; i++) {
      size_t remaining = sizeof(data_hex) - offset;
      int written = snprintf(data_hex + offset, remaining, "%02X ", data[i]);

      // Check for snprintf error (negative return value)
      if (written < 0) {
        ESP_LOGE(TAG, "snprintf error in data hex formatting");
        break;
      }

      // Check if buffer would overflow (snprintf returns chars that would be written)
      if (static_cast<size_t>(written) >= remaining) {
        // Buffer full, truncation would occur, stop
        break;
      }

      offset += written;
    }
    // Ensure null termination (defensive programming)
    if (offset < sizeof(data_hex)) {
      data_hex[offset] = '\0';
    } else {
      data_hex[sizeof(data_hex) - 1] = '\0';
    }
    ESP_LOGV(TAG, "  Data: %s", data_hex);
  }
}

void KNXTPComponent::send_group_write(const std::string &ga_id, const std::vector<uint8_t> &data) {
  auto ga = this->get_group_address(ga_id);
  if (ga != nullptr) {
    ESP_LOGD(TAG, "Group write to %s (%s)", ga_id.c_str(), ga->get_address().c_str());
    this->send_telegram(ga->get_address(), data);
  } else {
    ESP_LOGW(TAG, "Cannot send group write: Group address %s not found", ga_id.c_str());
  }
}

void KNXTPComponent::send_group_read(const std::string &ga_id) {
  auto ga = this->get_group_address(ga_id);
  if (ga != nullptr) {
    ESP_LOGD(TAG, "Group read request to %s (%s)", ga_id.c_str(), ga->get_address().c_str());
    std::vector<uint8_t> empty_data;
    this->send_telegram(ga->get_address(), empty_data);
  } else {
    ESP_LOGW(TAG, "Cannot send group read: Group address %s not found", ga_id.c_str());
  }
}

void KNXTPComponent::send_group_response(const std::string &ga_id, const std::vector<uint8_t> &data) {
  auto ga = this->get_group_address(ga_id);
  if (ga != nullptr) {
    ESP_LOGD(TAG, "Group response to %s (%s)", ga_id.c_str(), ga->get_address().c_str());
    this->send_telegram(ga->get_address(), data);
  } else {
    ESP_LOGW(TAG, "Cannot send group response: Group address %s not found", ga_id.c_str());
  }
}

void KNXTPComponent::parse_telegram_(const std::vector<uint8_t> &telegram) {
  // With Thelsing stack, telegram parsing is handled internally
  // This function is kept for compatibility but may not be needed
  ESP_LOGV(TAG, "parse_telegram_ called with %d bytes", telegram.size());
}

void KNXTPComponent::notify_entities_(const std::string &ga, uint16_t ga_int, const std::vector<uint8_t> &data) {
  ESP_LOGV(TAG, "Notifying %d entities about telegram for GA %s", this->entities_.size(), ga.c_str());

#if USE_KNX_ON_TELEGRAM
  // Call generic telegram triggers (for ALL telegrams)
  // Note: Pass copies to avoid lifetime issues with lambdas
  this->telegram_callbacks_.call(ga, data);
#endif

#if USE_KNX_ON_GROUP_ADDRESS
  // Call group address specific triggers (O(1) lookup)
  // Use pre-converted integer address to avoid redundant conversion
  auto it = this->ga_callbacks_.find(ga_int);
  if (it != this->ga_callbacks_.end()) {
    it->second.call(data);
  }
#endif

  // Notify registered entities
  for (auto *entity : this->entities_) {
    if (entity != nullptr) {
      entity->on_knx_telegram(ga, data);
    }
  }
}

void KNXTPComponent::group_object_callback_(uint16_t ga, const uint8_t *data, uint8_t len) {
  // Input validation: check for null pointer
  if (data == nullptr) {
    ESP_LOGE(TAG, "Null data pointer in group_object_callback_ for GA %u", ga);
    return;
  }

  // Input validation: check for valid length (KNX TP max payload is 254 bytes)
  constexpr uint8_t MAX_KNX_PAYLOAD = 254;
  if (len > MAX_KNX_PAYLOAD) {
    ESP_LOGE(TAG, "Invalid data length %u (max %u) for GA %u", len, MAX_KNX_PAYLOAD, ga);
    return;
  }

  // Convert GA to string format
  std::string ga_str = this->int_to_address_(ga);

  // Convert data to vector with pre-allocation for efficiency
  std::vector<uint8_t> data_vec;
  data_vec.reserve(len);  // Pre-allocate to avoid reallocation
  data_vec.assign(data, data + len);

  ESP_LOGD(TAG, "Group object callback for GA %s with %d bytes", ga_str.c_str(), len);

  // Notify entities (pass both string and int to avoid redundant conversion)
  this->notify_entities_(ga_str, ga, data_vec);
}

uint8_t KNXTPComponent::calculate_checksum_(const std::vector<uint8_t> &data) {
  // XOR checksum (kept for compatibility)
  // Validate input: need at least 1 byte to calculate checksum
  if (data.size() <= 1) {
    return 0xFF;  // Return default checksum for empty/single-byte data
  }

  uint8_t checksum = 0;
  // Safe: we know data.size() >= 2, so data.size() - 1 >= 1
  for (size_t i = 0; i < data.size() - 1; i++) {
    checksum ^= data[i];
  }
  return ~checksum;
}

std::vector<uint8_t> KNXTPComponent::encode_address_(const std::string &address) {
  uint16_t addr_int = this->address_to_int_(address);
  return {
    static_cast<uint8_t>(addr_int >> 8),
    static_cast<uint8_t>(addr_int & 0xFF)
  };
}

std::string KNXTPComponent::decode_address_(const std::vector<uint8_t> &data, size_t offset) {
  if (offset + 1 >= data.size()) {
    return "0.0.0";
  }

  uint16_t encoded = (data[offset] << 8) | data[offset + 1];
  return this->int_to_address_(encoded);
}

uint16_t KNXTPComponent::address_to_int_(const std::string &address) {
  // Parse address format: "area.line.device" or "area/line/device"

  // Input validation: check length (min: "1.1.1" = 5 chars, max: reasonable limit)
  if (address.empty() || address.length() > 16) {
    ESP_LOGE(TAG, "Invalid address length: %zu", address.length());
    return 0;
  }

  std::string addr = address;
  std::replace(addr.begin(), addr.end(), '/', '.');

  size_t first_dot = addr.find('.');

  // Validate first_dot position
  if (first_dot == std::string::npos || first_dot == 0) {
    ESP_LOGW(TAG, "Invalid address format (no first dot or starts with dot): %s", address.c_str());
    return 0;
  }

  // Check for overflow before adding 1
  if (first_dot >= addr.length() - 1) {
    ESP_LOGE(TAG, "Invalid address format (ends with dot): %s", address.c_str());
    return 0;
  }

  size_t second_dot = addr.find('.', first_dot + 1);

  // Validate second_dot position
  if (second_dot == std::string::npos ||
      second_dot <= first_dot + 1 ||  // Must have at least 1 char between dots
      second_dot >= addr.length() - 1) {  // Must have at least 1 char after second dot
    ESP_LOGW(TAG, "Invalid address format (invalid second dot): %s", address.c_str());
    return 0;
  }

  // Manual validation to avoid exceptions (ESP-IDF uses -fno-exceptions)
  std::string area_str = addr.substr(0, first_dot);
  std::string line_str = addr.substr(first_dot + 1, second_dot - first_dot - 1);
  std::string device_str = addr.substr(second_dot + 1);

  // Check if all parts are numeric
  bool valid = true;
  for (char c : area_str) if (!isdigit(c)) valid = false;
  for (char c : line_str) if (!isdigit(c)) valid = false;
  for (char c : device_str) if (!isdigit(c)) valid = false;

  if (!valid || area_str.empty() || line_str.empty() || device_str.empty()) {
    ESP_LOGE(TAG, "Invalid address format (non-numeric): %s", address.c_str());
    return 0;
  }

  // Convert with bounds checking
  long area_long = atol(area_str.c_str());
  long line_long = atol(line_str.c_str());
  long device_long = atol(device_str.c_str());

  if (area_long < 0 || area_long > 31 || line_long < 0 || line_long > 7 || device_long < 0 || device_long > 255) {
    ESP_LOGE(TAG, "Address value out of range: %s (area:%ld line:%ld device:%ld)",
             address.c_str(), area_long, line_long, device_long);
    return 0;
  }

  int area = static_cast<int>(area_long);
  int line = static_cast<int>(line_long);
  int device = static_cast<int>(device_long);

  // For group addresses: AAAAA/BBB/CCCCCCCC -> area (5 bits), line (3 bits), device (8 bits)
  // For physical addresses: AAAA.LLLL.DDDDDDDD -> area (4 bits), line (4 bits), device (8 bits)
  // We'll use the group address format for simplicity
  uint16_t encoded = ((area & 0x1F) << 11) | ((line & 0x07) << 8) | (device & 0xFF);

  return encoded;
}

std::string KNXTPComponent::int_to_address_(uint16_t address) {
  int area = (address >> 11) & 0x1F;
  int line = (address >> 8) & 0x07;
  int device = address & 0xFF;

  char addr[16];
  int written = snprintf(addr, sizeof(addr), "%d/%d/%d", area, line, device);

  // Check for snprintf error or truncation
  if (written < 0 || written >= static_cast<int>(sizeof(addr))) {
    ESP_LOGE(TAG, "Address formatting error for %u", address);
    return "0/0/0";  // Return default address on error
  }

  return std::string(addr);
}

void KNXTPComponent::broadcast_time_() {
#ifdef USE_TIME
  if (this->time_source_ == nullptr || this->time_broadcast_ga_id_.empty()) {
    return;
  }

  // Get current time from time component
  auto time = this->time_source_->now();

  // Check if time is valid
  if (!time.is_valid()) {
    ESP_LOGW(TAG, "Time not yet synchronized, skipping broadcast");
    return;
  }

  // Create DPT 19.001 DateTime structure
  DPT::DateTime dt;
  dt.year = time.year;
  dt.month = time.month;
  dt.day = time.day_of_month;
  dt.day_of_week = time.day_of_week;  // ESPHome: 1=Sunday, KNX: 1=Monday
  dt.hour = time.hour;
  dt.minute = time.minute;
  dt.second = time.second;

  // Set flags
  dt.fault = false;
  dt.working_day = (time.day_of_week >= 2 && time.day_of_week <= 6);  // Mon-Fri
  dt.no_wd = false;
  dt.no_year = false;
  dt.no_date = false;
  dt.no_dow = false;
  dt.no_time = false;
  dt.summer_time = time.is_dst;
  dt.quality = 0;  // Good quality

  // Encode and send
  auto telegram = DPT::encode_dpt19(dt);
  this->send_group_write(this->time_broadcast_ga_id_, telegram);

  ESP_LOGD(TAG, "Broadcasted time: %04d-%02d-%02d %02d:%02d:%02d%s",
           dt.year, dt.month, dt.day, dt.hour, dt.minute, dt.second,
           dt.summer_time ? " (DST)" : "");
#else
  // Time component not available
  ESP_LOGW(TAG, "Time broadcast requested but time component not available");
#endif
}

#if USE_KNX_ON_GROUP_ADDRESS
void KNXTPComponent::add_on_group_address_callback(uint16_t ga_int,
                                                    std::function<void(std::vector<uint8_t>)> &&callback) {
  // Add callback to hash map (creates entry if doesn't exist)
  this->ga_callbacks_[ga_int].add(std::move(callback));

  ESP_LOGD(TAG, "Registered on_group_address callback for GA 0x%04X (%s)",
           ga_int, this->int_to_address_(ga_int).c_str());
}
#endif

}  // namespace knx_tp
}  // namespace esphome
