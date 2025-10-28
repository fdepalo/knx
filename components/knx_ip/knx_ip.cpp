#include "knx_ip.h"
#include "esphome/core/log.h"
#include "esphome/core/application.h"

#ifdef USE_TIME
#include "esphome/components/time/real_time_clock.h"
#endif

// Include Thelsing KNX stack for IP
#include <esp32_idf_platform.h>
#include <knx/bau57B0.h>

namespace esphome {
namespace knx_ip {

static constexpr const char* TAG = "knx_ip";

void KNXIPComponent::setup() {
  ESP_LOGCONFIG(TAG, "Setting up KNX IP...");

  // Parse physical address
  this->physical_address_int_ = this->parse_physical_address_(this->physical_address_);

  // Initialize Thelsing KNX platform for ESP-IDF
  this->platform_ = new Esp32IdfPlatform();

  // Initialize IP BAU (Bau57B0 for IP vs Bau07B0 for TP)
  this->bau_ = new Bau57B0(*this->platform_);

  // Configure physical address
  this->bau_->deviceObject().individualAddress(this->physical_address_int_);

  // Configure IP parameters
  if (this->routing_mode_) {
    ESP_LOGCONFIG(TAG, "  Mode: Routing (multicast)");
    ESP_LOGCONFIG(TAG, "  Multicast: %s:%d", this->multicast_address_.c_str(), this->gateway_port_);

    // Routing mode uses multicast
    // The platform will automatically setup multicast on 224.0.23.12:3671
    // unless overridden
  } else {
    ESP_LOGCONFIG(TAG, "  Mode: Tunneling");
    ESP_LOGCONFIG(TAG, "  Gateway: %s:%d", this->gateway_ip_.c_str(), this->gateway_port_);

    // Tunneling mode connects to specific gateway
    // This requires KNX_TUNNELING flag and additional configuration
    #ifndef KNX_TUNNELING
    ESP_LOGW(TAG, "Tunneling mode requested but KNX_TUNNELING not defined!");
    ESP_LOGW(TAG, "Add -DKNX_TUNNELING to build flags");
    #endif
  }

  // Configure group objects for each registered group address
  auto& groupObjectTable = this->bau_->groupObjectTable();
  uint16_t go_index = 1;  // Group object indices start at 1

  for (auto *ga : this->group_addresses_) {
    uint16_t ga_int = ga->get_address_int();
    ESP_LOGD(TAG, "Configuring group object %u for GA %s (%u)",
             go_index, ga->get_address().c_str(), ga_int);
    go_index++;
  }

  // Enable the KNX device
  this->bau_->enabled(true);
  this->connected_ = true;

  ESP_LOGCONFIG(TAG, "KNX IP setup complete");
}

void KNXIPComponent::loop() {
  if (!this->connected_) {
    return;
  }

  // Must call loop frequently to handle network traffic
  // This processes incoming/outgoing KNX/IP frames
  this->bau_->loop();

  // Handle time broadcast if configured
  #ifdef USE_TIME
  if (this->time_source_ != nullptr && !this->time_broadcast_ga_id_.empty()) {
    uint32_t now = millis();
    if (now - this->last_time_broadcast_ >= this->time_broadcast_interval_) {
      this->broadcast_time_();
      this->last_time_broadcast_ = now;
    }
  }
  #endif
}

void KNXIPComponent::dump_config() {
  ESP_LOGCONFIG(TAG, "KNX IP:");
  ESP_LOGCONFIG(TAG, "  Physical Address: %s", this->physical_address_.c_str());
  ESP_LOGCONFIG(TAG, "  Mode: %s", this->routing_mode_ ? "Routing" : "Tunneling");

  if (this->routing_mode_) {
    ESP_LOGCONFIG(TAG, "  Multicast: %s:%d", this->multicast_address_.c_str(), this->gateway_port_);
  } else {
    ESP_LOGCONFIG(TAG, "  Gateway: %s:%d", this->gateway_ip_.c_str(), this->gateway_port_);
  }

  ESP_LOGCONFIG(TAG, "  Group Addresses: %d", this->group_addresses_.size());
  for (auto *ga : this->group_addresses_) {
    ESP_LOGCONFIG(TAG, "    - %s: %s", ga->get_id().c_str(), ga->get_address().c_str());
  }

  ESP_LOGCONFIG(TAG, "  Entities: %d", this->entities_.size());

  if (this->time_source_ != nullptr) {
    ESP_LOGCONFIG(TAG, "  Time Broadcast: enabled (interval: %dms)", this->time_broadcast_interval_);
  }
}

void KNXIPComponent::set_physical_address(const std::string &address) {
  this->physical_address_ = address;
}

void KNXIPComponent::register_group_address(GroupAddress *ga) {
  this->group_addresses_.push_back(ga);
}

void KNXIPComponent::register_entity(KNXEntity *entity) {
  this->entities_.push_back(entity);
  entity->set_knx_component(this);
  ESP_LOGD(TAG, "Registered entity (total: %d)", this->entities_.size());
}

GroupAddress *KNXIPComponent::get_group_address(const std::string &id) {
  for (auto *ga : this->group_addresses_) {
    if (ga->get_id() == id) {
      return ga;
    }
  }
  return nullptr;
}

void KNXIPComponent::send_telegram(const std::string &dest_addr, const std::vector<uint8_t> &data) {
  if (!this->bau_) {
    ESP_LOGW(TAG, "BAU not initialized, cannot send telegram");
    return;
  }

  ESP_LOGD(TAG, "Sending telegram to %s with %d bytes", dest_addr.c_str(), data.size());

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
      offset += snprintf(data_hex + offset, sizeof(data_hex) - offset, "%02X ", data[i]);
    }
    ESP_LOGV(TAG, "  Data: %s", data_hex);
  }
}

void KNXIPComponent::send_group_write(const std::string &ga_id, const std::vector<uint8_t> &data) {
  auto ga = this->get_group_address(ga_id);
  if (ga != nullptr) {
    ESP_LOGD(TAG, "Group write to %s (%s)", ga_id.c_str(), ga->get_address().c_str());
    this->send_telegram(ga->get_address(), data);
  } else {
    ESP_LOGW(TAG, "Cannot send group write: Group address %s not found", ga_id.c_str());
  }
}

void KNXIPComponent::send_group_read(const std::string &ga_id) {
  auto ga = this->get_group_address(ga_id);
  if (ga != nullptr) {
    ESP_LOGD(TAG, "Group read request to %s (%s)", ga_id.c_str(), ga->get_address().c_str());
    std::vector<uint8_t> empty_data;
    this->send_telegram(ga->get_address(), empty_data);
  } else {
    ESP_LOGW(TAG, "Cannot send group read: Group address %s not found", ga_id.c_str());
  }
}

void KNXIPComponent::send_group_response(const std::string &ga_id, const std::vector<uint8_t> &data) {
  auto ga = this->get_group_address(ga_id);
  if (ga != nullptr) {
    ESP_LOGD(TAG, "Group response to %s (%s)", ga_id.c_str(), ga->get_address().c_str());
    this->send_telegram(ga->get_address(), data);
  } else {
    ESP_LOGW(TAG, "Cannot send group response: Group address %s not found", ga_id.c_str());
  }
}

void KNXIPComponent::broadcast_time_() {
  #ifdef USE_TIME
  if (this->time_source_ == nullptr || this->time_broadcast_ga_id_.empty()) {
    return;
  }

  auto now = this->time_source_->now();
  if (!now.is_valid()) {
    return;
  }

  // Encode as DPT 19.001 (Date and Time)
  DPT::DateTime dt;
  dt.year = now.year;
  dt.month = now.month;
  dt.day = now.day_of_month;
  dt.day_of_week = now.day_of_week;
  dt.hour = now.hour;
  dt.minute = now.minute;
  dt.second = now.second;
  dt.quality = 0;  // No fault
  dt.fault = false;
  dt.working_day = (now.day_of_week >= 1 && now.day_of_week <= 5);
  dt.no_wd = false;
  dt.no_year = false;
  dt.no_date = false;
  dt.no_dow = false;
  dt.no_time = false;
  dt.summer_time = false;  // Could check DST if needed

  std::vector<uint8_t> data = DPT::encode_dpt19(dt);
  this->send_group_write(this->time_broadcast_ga_id_, data);

  ESP_LOGD(TAG, "Broadcast time: %04d-%02d-%02d %02d:%02d:%02d",
           dt.year, dt.month, dt.day, dt.hour, dt.minute, dt.second);
  #endif
}

void KNXIPComponent::notify_entities_(const std::string &ga, const std::vector<uint8_t> &data) {
  for (auto *entity : this->entities_) {
    entity->on_knx_telegram(ga, data);
  }
}

uint16_t KNXIPComponent::parse_physical_address_(const std::string &address) {
  // Parse format: "area.line.device" or "area/line/device"
  // Example: "1.1.200" -> 0x1100 + 200 = 0x11C8

  std::string addr = address;
  // Replace / with .
  for (char &c : addr) {
    if (c == '/') c = '.';
  }

  size_t pos1 = addr.find('.');
  size_t pos2 = addr.find('.', pos1 + 1);

  if (pos1 == std::string::npos || pos2 == std::string::npos) {
    ESP_LOGE(TAG, "Invalid physical address format: %s", address.c_str());
    return 0;
  }

  // Manual validation to avoid exceptions (ESP-IDF uses -fno-exceptions)
  std::string area_str = addr.substr(0, pos1);
  std::string line_str = addr.substr(pos1 + 1, pos2 - pos1 - 1);
  std::string device_str = addr.substr(pos2 + 1);

  // Check if all parts are numeric
  bool valid = true;
  for (char c : area_str) if (!isdigit(c)) valid = false;
  for (char c : line_str) if (!isdigit(c)) valid = false;
  for (char c : device_str) if (!isdigit(c)) valid = false;

  if (!valid || area_str.empty() || line_str.empty() || device_str.empty()) {
    ESP_LOGE(TAG, "Invalid physical address (non-numeric): %s", address.c_str());
    return 0;
  }

  // Convert with bounds checking
  long area_long = atol(area_str.c_str());
  long line_long = atol(line_str.c_str());
  long device_long = atol(device_str.c_str());

  if (area_long < 0 || area_long > 15 || line_long < 0 || line_long > 15 || device_long < 0 || device_long > 255) {
    ESP_LOGE(TAG, "Physical address out of range: %s", address.c_str());
    return 0;
  }

  uint8_t area = static_cast<uint8_t>(area_long);
  uint8_t line = static_cast<uint8_t>(line_long);
  uint8_t device = static_cast<uint8_t>(device_long);

  // Physical address format: AAAA LLLL DDDDDDDD (4+4+8 bits)
  return ((area & 0x0F) << 12) | ((line & 0x0F) << 8) | (device & 0xFF);
}

std::vector<uint8_t> KNXIPComponent::encode_address_(const std::string &address) {
  // Parse group address: "main/middle/sub" or "main.middle.sub"
  // Returns 2 bytes

  std::string addr = address;
  for (char &c : addr) {
    if (c == '/') c = '.';
  }

  size_t pos1 = addr.find('.');
  size_t pos2 = addr.find('.', pos1 + 1);

  if (pos1 == std::string::npos || pos2 == std::string::npos) {
    ESP_LOGE(TAG, "Invalid group address format: %s", address.c_str());
    return {0, 0};
  }

  // Manual validation to avoid exceptions (ESP-IDF uses -fno-exceptions)
  std::string main_str = addr.substr(0, pos1);
  std::string middle_str = addr.substr(pos1 + 1, pos2 - pos1 - 1);
  std::string sub_str = addr.substr(pos2 + 1);

  // Check if all parts are numeric
  bool valid = true;
  for (char c : main_str) if (!isdigit(c)) valid = false;
  for (char c : middle_str) if (!isdigit(c)) valid = false;
  for (char c : sub_str) if (!isdigit(c)) valid = false;

  if (!valid || main_str.empty() || middle_str.empty() || sub_str.empty()) {
    ESP_LOGE(TAG, "Invalid group address (non-numeric): %s", address.c_str());
    return {0, 0};
  }

  // Convert with bounds checking
  long main_long = atol(main_str.c_str());
  long middle_long = atol(middle_str.c_str());
  long sub_long = atol(sub_str.c_str());

  if (main_long < 0 || main_long > 31 || middle_long < 0 || middle_long > 7 || sub_long < 0 || sub_long > 255) {
    ESP_LOGE(TAG, "Group address out of range: %s", address.c_str());
    return {0, 0};
  }

  uint8_t main = static_cast<uint8_t>(main_long);
  uint8_t middle = static_cast<uint8_t>(middle_long);
  uint8_t sub = static_cast<uint8_t>(sub_long);

  // Group address format: MMMM MMMM MSSS SSSS (5+3+8 bits)
  uint16_t ga_int = ((main & 0x1F) << 11) | ((middle & 0x07) << 8) | (sub & 0xFF);

  return {static_cast<uint8_t>(ga_int >> 8), static_cast<uint8_t>(ga_int & 0xFF)};
}

}  // namespace knx_ip
}  // namespace esphome
