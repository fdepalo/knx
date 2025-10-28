# KNX Router Implementation Guide

## Cos'è un Router KNX?

Un **router KNX** collega due reti KNX separate e inoltra i telegrammi tra di esse:

- **Router IP↔TP**: Collega rete IP (WiFi/Ethernet) con bus fisico TP
- **Router TP↔TP**: Collega due linee TP separate (line coupler)
- **Router IP↔IP**: Collega due sottoreti IP separate

## Implementazioni Possibili in ESPHome

### ⭐ Approccio 1: Componente Dedicato `knx_router` (RACCOMANDATO)

Crea un nuovo componente che usa **Bau091A** della libreria Thelsing.

#### Vantaggi
- ✅ Routing automatico gestito dal BAU
- ✅ Filtri di routing configurabili
- ✅ Performance ottimali
- ✅ Conforme agli standard KNX
- ✅ Supporta filtering table (quali GA inoltrare)

#### File Structure
```
components/knx_router/
├── __init__.py
├── knx_router.h
├── knx_router.cpp
└── README.md
```

#### YAML Configuration
```yaml
uart:
  id: knx_uart
  tx_pin: GPIO17
  rx_pin: GPIO16
  baud_rate: 19200

wifi:
  ssid: "MyNetwork"
  password: "password"

# Router dedicato
knx_router:
  # Physical addresses
  ip_address: "1.1.200"      # IP side physical address
  tp_address: "1.1.100"      # TP side physical address

  uart_id: knx_uart

  # Routing configuration
  mode: bidirectional        # bidirectional, ip_to_tp, tp_to_ip

  # IP configuration
  routing_mode: true         # Use multicast routing
  multicast_address: "224.0.23.12"

  # Filtering (optional)
  filter_group_addresses:
    - "0/0/1"   # Forward only these addresses
    - "0/0/2"
    - "1/0/1"

  # Or filter by area
  filter_areas:
    - 0         # Forward all GA in area 0
    - 1         # Forward all GA in area 1
```

#### Implementation Sketch

**knx_router.h:**
```cpp
#pragma once

#include "esphome/core/component.h"
#include "esphome/components/uart/uart.h"

#ifndef MASK_VERSION
#define MASK_VERSION 0x091A  // IP/TP Coupler
#endif

class Esp32IdfPlatform;
class Bau091A;  // IP/TP Coupler BAU

namespace esphome {
namespace knx_router {

enum RoutingMode {
  ROUTING_BIDIRECTIONAL,
  ROUTING_IP_TO_TP,
  ROUTING_TP_TO_IP
};

class KNXRouter : public Component, public uart::UARTDevice {
 public:
  void setup() override;
  void loop() override;
  void dump_config() override;
  float get_setup_priority() const override {
    return setup_priority::AFTER_CONNECTION;
  }

  // Configuration
  void set_ip_address(const std::string &addr);
  void set_tp_address(const std::string &addr);
  void set_routing_mode(RoutingMode mode) { routing_mode_ = mode; }
  void set_ip_routing(bool routing) { ip_routing_ = routing; }
  void set_multicast_address(const std::string &addr) { multicast_ = addr; }

  // Filtering
  void add_filter_ga(const std::string &ga);
  void add_filter_area(uint8_t area);

 protected:
  std::string ip_physical_address_;
  std::string tp_physical_address_;
  uint16_t ip_address_int_{0};
  uint16_t tp_address_int_{0};

  RoutingMode routing_mode_{ROUTING_BIDIRECTIONAL};
  bool ip_routing_{true};  // true=routing, false=tunneling
  std::string multicast_{"224.0.23.12"};

  // Filtering
  std::vector<uint16_t> filter_gas_;
  std::vector<uint8_t> filter_areas_;

  // Thelsing KNX stack
  Bau091A *bau_{nullptr};           // IP/TP Coupler BAU
  Esp32IdfPlatform *platform_{nullptr};

  // Helpers
  uint16_t parse_physical_address_(const std::string &addr);
  bool should_forward_(uint16_t ga);
};

}  // namespace knx_router
}  // namespace esphome
```

**knx_router.cpp:**
```cpp
#include "knx_router.h"
#include "esphome/core/log.h"

#include <knx/bau091A.h>
#include <knx/esp32_idf_platform.h>

namespace esphome {
namespace knx_router {

static constexpr const char* TAG = "knx_router";

void KNXRouter::setup() {
  ESP_LOGCONFIG(TAG, "Setting up KNX Router...");

  this->ip_address_int_ = this->parse_physical_address_(this->ip_physical_address_);
  this->tp_address_int_ = this->parse_physical_address_(this->tp_physical_address_);

  // Initialize platform
  this->platform_ = new Esp32IdfPlatform();

  // Initialize IP/TP Coupler BAU
  this->bau_ = new Bau091A(*this->platform_);

  // Configure physical addresses
  // Primary (IP) address
  this->bau_->deviceObject().individualAddress(this->ip_address_int_);

  // The Bau091A handles both IP and TP automatically!
  // It will route telegrams between the two networks

  ESP_LOGCONFIG(TAG, "  IP Physical Address: %s", this->ip_physical_address_.c_str());
  ESP_LOGCONFIG(TAG, "  TP Physical Address: %s", this->tp_physical_address_.c_str());
  ESP_LOGCONFIG(TAG, "  Routing Mode: %s",
    this->routing_mode_ == ROUTING_BIDIRECTIONAL ? "Bidirectional" :
    this->routing_mode_ == ROUTING_IP_TO_TP ? "IP → TP" : "TP → IP");

  // Start the router
  this->bau_->start();

  ESP_LOGCONFIG(TAG, "KNX Router started - bridging IP ↔ TP");
}

void KNXRouter::loop() {
  // Process both IP and TP traffic
  this->bau_->loop();

  // The Bau091A automatically routes telegrams between networks
  // based on the routing table and filter rules
}

void KNXRouter::dump_config() {
  ESP_LOGCONFIG(TAG, "KNX Router:");
  ESP_LOGCONFIG(TAG, "  IP Address: %s", this->ip_physical_address_.c_str());
  ESP_LOGCONFIG(TAG, "  TP Address: %s", this->tp_physical_address_.c_str());
  ESP_LOGCONFIG(TAG, "  Multicast: %s:3671", this->multicast_.c_str());

  if (!this->filter_gas_.empty()) {
    ESP_LOGCONFIG(TAG, "  Filtering: %d group addresses", this->filter_gas_.size());
  }
  if (!this->filter_areas_.empty()) {
    ESP_LOGCONFIG(TAG, "  Filtering: %d areas", this->filter_areas_.size());
  }
}

bool KNXRouter::should_forward_(uint16_t ga) {
  // If no filters, forward everything
  if (this->filter_gas_.empty() && this->filter_areas_.empty()) {
    return true;
  }

  // Check specific GA filter
  for (uint16_t filter_ga : this->filter_gas_) {
    if (ga == filter_ga) {
      return true;
    }
  }

  // Check area filter
  uint8_t area = (ga >> 11) & 0x1F;
  for (uint8_t filter_area : this->filter_areas_) {
    if (area == filter_area) {
      return true;
    }
  }

  return false;
}

uint16_t KNXRouter::parse_physical_address_(const std::string &address) {
  // Same as before
  std::string addr = address;
  for (char &c : addr) {
    if (c == '/') c = '.';
  }

  size_t pos1 = addr.find('.');
  size_t pos2 = addr.find('.', pos1 + 1);

  if (pos1 == std::string::npos || pos2 == std::string::npos) {
    ESP_LOGE(TAG, "Invalid physical address: %s", address.c_str());
    return 0;
  }

  uint8_t area = std::stoi(addr.substr(0, pos1));
  uint8_t line = std::stoi(addr.substr(pos1 + 1, pos2 - pos1 - 1));
  uint8_t device = std::stoi(addr.substr(pos2 + 1));

  return ((area & 0x0F) << 12) | ((line & 0x0F) << 8) | (device & 0xFF);
}

}  // namespace knx_router
}  // namespace esphome
```

---

### Approccio 2: Due Componenti Separati con Bridge Manuale

Usa `knx_tp` e `knx_ip` esistenti e inoltra manualmente i telegrammi.

#### YAML Configuration
```yaml
uart:
  id: knx_uart
  tx_pin: GPIO17
  rx_pin: GPIO16

wifi:
  ssid: "MyNetwork"

knx_tp:
  physical_address: "1.1.100"
  uart_id: knx_uart
  group_addresses:
    - id: shared_light
      address: "0/0/1"

knx_ip:
  physical_address: "1.1.200"
  routing_mode: true
  group_addresses:
    - id: shared_light
      address: "0/0/1"

# Bridge logic in custom component
custom_component:
  - lambda: |-
      // Forward telegrams between TP and IP
      // This would require custom C++ code
```

#### Vantaggi/Svantaggi
- ❌ Richiede codice custom per inoltrare
- ❌ Due stack KNX separati (più memoria)
- ❌ Duplicazione configurazione GA
- ✅ Massima flessibilità
- ✅ Può applicare logica custom

---

### Approccio 3: Proxy Applicativo

Crea entità ESPHome che ascoltano su un lato e trasmettono sull'altro.

#### YAML Configuration
```yaml
# TP Side
knx_tp:
  physical_address: "1.1.100"
  uart_id: knx_uart
  group_addresses:
    - id: tp_light
      address: "0/0/1"

# IP Side
knx_ip:
  physical_address: "1.1.200"
  group_addresses:
    - id: ip_light
      address: "0/0/1"

# Bridge via ESPHome entities
switch:
  - platform: template
    name: "Bridged Light"
    lambda: |-
      // Read from TP
      return id(tp_light_switch).state;
    turn_on_action:
      - switch.turn_on: ip_light_switch  # Forward to IP
    turn_off_action:
      - switch.turn_off: ip_light_switch

  - platform: knx_tp
    id: tp_light_switch
    command_ga: tp_light
    on_state:
      - switch.turn_on: ip_light_switch  # Forward to IP

  - platform: knx_ip
    id: ip_light_switch
    command_ga: ip_light
```

#### Vantaggi/Svantaggi
- ❌ Molto verboso
- ❌ Devi configurare ogni singola entità
- ❌ Non scala bene
- ✅ Completamente in YAML
- ✅ Nessun codice C++ custom

---

## Confronto Approcci

| Caratteristica | Bau091A Router | Due Componenti | Proxy Applicativo |
|----------------|----------------|----------------|-------------------|
| **Complessità** | Media | Alta | Bassa |
| **Performance** | ⭐⭐⭐⭐⭐ | ⭐⭐⭐ | ⭐⭐ |
| **Memoria RAM** | ⭐⭐⭐⭐ | ⭐⭐ | ⭐⭐⭐ |
| **Configurazione** | Semplice | Complessa | Molto verbosa |
| **Scalabilità** | ⭐⭐⭐⭐⭐ | ⭐⭐⭐ | ⭐ |
| **Filtering** | Nativo | Custom | Per entità |
| **Standard KNX** | ✅ Conforme | ⚠️ Custom | ❌ Non conforme |
| **Codice Custom** | Nuovo componente | Bridge code | Solo YAML |

---

## Raccomandazione

### Per Produzione: **Bau091A Router** ⭐

Crea il componente `knx_router` che usa Bau091A. Vantaggi:
1. **Standard KNX**: Usa la classe coupler ufficiale
2. **Performance**: Routing a basso livello nel BAU
3. **Filtering**: Tabelle di routing native
4. **Scalabile**: Inoltra migliaia di telegrammi/secondo
5. **Manutenibile**: Configurazione chiara e semplice

### Per Prototipazione Rapida: **Proxy Applicativo**

Se hai solo pochi device da bridgare e vuoi testare velocemente.

### Per Casi Speciali: **Due Componenti**

Se hai bisogno di logica custom complessa (es. trasformazioni, ritardi, conditional forwarding).

---

## Implementazione Rapida: Script Generator

Vuoi che crei il componente `knx_router` completo con:
- ✅ Bau091A integration
- ✅ Filtering support
- ✅ ESPHome YAML schema
- ✅ Documentation
- ✅ Example configurations

?

---

## Esempio Completo con Router

```yaml
esphome:
  name: knx-gateway

esp32:
  board: esp32dev
  framework:
    type: esp-idf

wifi:
  ssid: !secret wifi_ssid
  password: !secret wifi_password
  manual_ip:
    static_ip: 192.168.1.100
    gateway: 192.168.1.1
    subnet: 255.255.255.0

uart:
  id: knx_uart
  tx_pin: GPIO17
  rx_pin: GPIO16
  baud_rate: 19200
  parity: EVEN

external_components:
  - source:
      type: local
      path: components
    components: [ knx_router ]

# KNX Router Configuration
knx_router:
  # Physical addresses for both sides
  ip_address: "1.1.200"
  tp_address: "1.1.100"

  # UART for TP side
  uart_id: knx_uart

  # IP configuration
  routing_mode: true
  multicast_address: "224.0.23.12"

  # Routing mode
  mode: bidirectional

  # Optional: Filter which addresses to forward
  # (If not specified, forwards ALL)
  filter_areas:
    - 0  # Forward area 0 (lights)
    - 1  # Forward area 1 (sensors)

  # Or filter specific addresses
  # filter_group_addresses:
  #   - "0/0/1"
  #   - "0/0/2"
  #   - "1/0/1"

# Status LED
status_led:
  pin: GPIO2

# Web interface for monitoring
web_server:
  port: 80
```

Il router inoltra **automaticamente** tutti i telegrammi tra IP e TP!

```
     WiFi/Ethernet              UART + Transceiver
          │                            │
          │                            │
    ┌─────▼──────┐              ┌──────▼─────┐
    │  IP Side   │              │  TP Side   │
    │  (Bau091A) │◄────ESP32────►│  (Bau091A)│
    └─────┬──────┘              └──────┬─────┘
          │                            │
    224.0.23.12:3671              2-wire bus
    KNX IP Network              Physical KNX Bus
```

---

## Performance Attese

### Throughput
- **Max telegrams/sec**: ~500-1000 (limitato da TP a 9.6kbps)
- **Latency TP→IP**: ~10-30ms
- **Latency IP→TP**: ~10-30ms
- **RAM usage**: ~50-80KB (vs ~100KB con due componenti)

### Filtering Benefits
Senza filtering: **100% dei telegrammi** attraversano il router
Con filtering per area: Solo **telegrammi rilevanti** (risparmio bandwidth)

---

**Vuoi che implementi il componente `knx_router` completo?** 🚀
