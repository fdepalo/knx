# KNX ESPHome Components Architecture

Questo repository contiene due componenti KNX per ESPHome:

## 📦 Componenti Disponibili

### 1. **knx_tp** - KNX Twisted Pair
Comunicazione fisica via bus TP con transceiver hardware.

**Caratteristiche:**
- Bus fisico a 2 fili
- Richiede transceiver (NCN5120, TPUART, BCU)
- Comunicazione UART a 19200 baud
- Velocità: ~9.6 kbit/s
- Range: fino a 1000m
- BAU: Bau07B0
- MASK_VERSION: 0x07B0

**Usa quando:**
- Hai un bus KNX fisico esistente
- Vuoi controllo diretto dei dispositivi KNX
- Necessiti di affidabilità massima
- La latenza non è critica

### 2. **knx_ip** - KNX/IP (KNXnet/IP)
Comunicazione di rete via WiFi/Ethernet.

**Caratteristiche:**
- Rete IP (WiFi/Ethernet)
- Nessun hardware aggiuntivo necessario
- Supporta routing (multicast) e tunneling
- Velocità: network speed (100+ Mbit/s)
- Range: portata della rete
- BAU: Bau57B0
- MASK_VERSION: 0x57B0

**Usa quando:**
- Non hai accesso fisico al bus KNX
- Vuoi controllo remoto/supervisione
- Hai già una rete IP robusta
- Necessiti di bassa latenza

## 🏗️ Architettura

```
┌──────────────────────────────────────────────────────────┐
│                  ESPHome Application                     │
│         (Home Assistant, Web Server, OTA, ...)           │
└────────────────────────┬─────────────────────────────────┘
                         │
        ┌────────────────┴────────────────┐
        │                                 │
┌───────▼────────┐              ┌────────▼────────┐
│   knx_tp       │              │    knx_ip       │
│  Component     │              │   Component     │
└───────┬────────┘              └────────┬────────┘
        │                                │
        │   Shared Components:           │
        │   ┌────────────────┐          │
        └───┤ DPT encoders   ├──────────┘
            │ GroupAddress   │
            │ KNXEntity      │
            └────────────────┘
                   │
        ┌──────────┴──────────┐
        │                     │
┌───────▼────────┐   ┌────────▼────────┐
│ Thelsing KNX   │   │ Thelsing KNX    │
│   Bau07B0      │   │   Bau57B0       │
│   (TP BAU)     │   │   (IP BAU)      │
└───────┬────────┘   └────────┬────────┘
        │                     │
┌───────▼────────┐   ┌────────▼────────┐
│ TpUartDataLink │   │  IpDataLink     │
│     Layer      │   │     Layer       │
└───────┬────────┘   └────────┬────────┘
        │                     │
┌───────▼────────┐   ┌────────▼────────┐
│  UART (GPIO)   │   │  WiFi/Ethernet  │
│  + Transceiver │   │  (UDP Multicast)│
└───────┬────────┘   └────────┬────────┘
        │                     │
┌───────▼────────┐   ┌────────▼────────┐
│  KNX TP Bus    │   │   IP Network    │
│  (2-wire bus)  │   │  224.0.23.12    │
└────────────────┘   └─────────────────┘
```

## 🔧 Struttura Directory

```
components/
├── knx_tp/                    # KNX Twisted Pair component
│   ├── __init__.py           # ESPHome integration
│   ├── const.py              # Constants
│   ├── knx_tp.h/cpp          # Main component (UART, Bau07B0)
│   ├── knx_stubs.h/cpp       # Thelsing library stubs
│   │
│   ├── dpt.h/cpp             # ✅ Shared: DPT encoders/decoders
│   ├── group_address.h/cpp   # ✅ Shared: Group address handling
│   │
│   ├── sensor.py/h/cpp       # Sensor platform
│   ├── binary_sensor.py/h/cpp # Binary sensor platform
│   ├── switch.py/h/cpp       # Switch platform
│   ├── light.py/h/cpp        # Light platform
│   ├── climate.py/h/cpp      # Climate platform
│   ├── cover.py/h/cpp        # Cover platform
│   ├── number.py/h/cpp       # Number platform
│   └── text_sensor.py/h/cpp  # Text sensor platform
│
└── knx_ip/                    # KNX/IP component
    ├── __init__.py           # ESPHome integration
    ├── const.py              # Constants
    ├── knx_ip.h/cpp          # Main component (Network, Bau57B0)
    ├── README.md             # Documentation
    │
    ├── sensor.py/h/cpp       # Sensor platform (example)
    └── ...                   # Other platforms (TODO)

examples/
├── knx-example-simple.yaml   # Simple TP example
├── knx-example-advanced.yaml # Advanced TP example
└── knx-ip-example.yaml       # IP example ✨ NEW
```

## 📊 Confronto Dettagliato

| Caratteristica | KNX-TP | KNX-IP |
|----------------|---------|---------|
| **Trasporto** | UART seriale | UDP/IP |
| **Hardware ESP32** | Qualsiasi | Qualsiasi con WiFi/ETH |
| **Hardware Extra** | Transceiver (NCN5120, etc.) | Nessuno |
| **Costo Hardware** | ~15-30€ transceiver | 0€ |
| **Velocità** | 9.6 kbit/s | 100+ Mbit/s |
| **Latenza** | 50-100ms | 5-20ms |
| **Range Fisico** | 1000m bus | Portata rete |
| **Topologia** | Bus lineare/ad albero | Rete IP |
| **Dipendenze ESPHome** | `uart` | `network` (WiFi/ETH) |
| **MASK_VERSION** | 0x07B0 | 0x57B0 |
| **BAU Class** | Bau07B0 | Bau57B0 |
| **Data Link Layer** | TpUartDataLinkLayer | IpDataLinkLayer |
| **Modalità Comunicazione** | Bus fisico | Routing/Tunneling |
| **Programmazione ETS** | Via TP | Via IP |
| **Multicast** | No | Sì (224.0.23.12) |
| **Gateway Richiesto** | No | Opzionale (per tunneling) |
| **Compatibilità KNX** | 100% | 100% |
| **Codice Condiviso** | DPT, GroupAddress, Entità | DPT, GroupAddress, Entità |

## 🚀 Scenari d'Uso

### Scenario 1: Solo TP (Controllo Locale)
```yaml
uart:
  id: knx_uart
  tx_pin: GPIO17
  rx_pin: GPIO16

knx_tp:
  physical_address: "1.1.100"
  uart_id: knx_uart
```

**Vantaggi:** Massima affidabilità, no dipendenze rete

### Scenario 2: Solo IP (Supervisione Remota)
```yaml
wifi:
  ssid: "MyNetwork"
  password: "password"

knx_ip:
  physical_address: "1.1.200"
  routing_mode: true
```

**Vantaggi:** Nessun hardware extra, facile da deployare

### Scenario 3: Gateway TP ↔ IP (Bridge)
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

knx_ip:
  physical_address: "1.1.200"
  routing_mode: true

# ESP32 inoltra automaticamente tra TP e IP!
```

**Vantaggi:** Integra reti separate, supervisione IP con controllo TP

## 🧩 Codice Condiviso

### DPT (Data Point Types)
File: `components/knx_tp/dpt.h/cpp`

**Usato da:** knx_tp ✅ | knx_ip ✅

Implementa encoding/decoding di tutti i DPT comuni:
- DPT 1 - Boolean
- DPT 5 - 8-bit unsigned
- DPT 9 - 2-byte float
- DPT 10 - Time of Day
- DPT 11 - Date
- DPT 14 - 4-byte float
- DPT 16 - String
- DPT 19 - DateTime
- DPT 20 - HVAC Mode

### GroupAddress
File: `components/knx_tp/group_address.h/cpp`

**Usato da:** knx_tp ✅ | knx_ip ✅

Gestisce indirizzi di gruppo KNX:
- Parsing formato "main/middle/sub"
- Conversione string ↔ uint16_t
- Ottimizzato per memoria (2 byte vs 31 byte)

### KNXEntity (interfaccia)
**Usato da:** knx_tp ✅ | knx_ip ✅

Classe base astratta per tutte le entità ESPHome:
```cpp
class KNXEntity {
 public:
  virtual void on_knx_telegram(const std::string &ga,
                                const std::vector<uint8_t> &data) = 0;
};
```

## 🔮 Roadmap Futura

### Fase 1: Completamento knx_ip ✅
- [x] Struttura base componente
- [x] Integrazione Bau57B0
- [x] Routing mode (multicast)
- [x] Sensor example
- [ ] Tutti i platform (switch, light, climate, etc.)
- [ ] Tunneling mode completo

### Fase 2: Refactoring codice condiviso
- [ ] Creare `components/knx_common/`
- [ ] Spostare DPT, GroupAddress in knx_common
- [ ] Aggiornare import in knx_tp e knx_ip

### Fase 3: Funzionalità avanzate
- [ ] KNX Secure (IP Secure)
- [ ] Gateway intelligente con routing rules
- [ ] Supporto KNX Data Secure
- [ ] Dashboard web per diagnostica

### Fase 4: Testing e stabilizzazione
- [ ] Test suite automatizzati
- [ ] Test di integrazione TP ↔ IP
- [ ] Performance benchmarks
- [ ] Certificazione KNX (opzionale)

## 📝 Note Implementative

### Differenze Chiave nel Codice

**Inizializzazione TP:**
```cpp
// knx_tp.cpp
this->platform_ = new Esp32IdfPlatform();
this->bau_ = new Bau07B0(*this->platform_);
// Setup UART pins
// Start TP data link layer
```

**Inizializzazione IP:**
```cpp
// knx_ip.cpp
this->platform_ = new Esp32IdfPlatform();
this->bau_ = new Bau57B0(*this->platform_);
// Network already up via ESPHome WiFi/ETH
// Setup multicast (routing) or gateway (tunneling)
// Start IP data link layer
```

### Stesso Codice Applicativo

Le entità (sensor, switch, etc.) usano la **stessa interfaccia**:

```cpp
// Funziona sia con knx_tp che knx_ip!
void send_group_write(const std::string &ga_id,
                      const std::vector<uint8_t> &data);

void on_knx_telegram(const std::string &ga,
                     const std::vector<uint8_t> &data);
```

## 🎓 Risorse

### Libreria Thelsing KNX
- Repository: https://github.com/thelsing/knx
- Documentazione: https://knx.readthedocs.io/
- Supporta: TP, IP, RF
- Piattaforme: ESP32, ESP8266, SAMD, RP2040, STM32, Linux

### Standard KNX
- KNX Association: https://www.knx.org/
- KNXnet/IP Spec: ISO 22510
- DPT Specifications: KNX System Specifications v2.1

### ESPHome
- ESPHome: https://esphome.io/
- Custom Components: https://esphome.io/custom/custom_component.html

## 🤝 Contributi

Contributi benvenuti! Vedi [CONTRIBUTING.md](CONTRIBUTING.md)

## 📄 Licenza

MIT License - vedi [LICENSE](LICENSE)

## ✍️ Autore

[@fdepalo](https://github.com/fdepalo)
