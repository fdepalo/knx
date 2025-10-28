# ESPHome KNX TP Component

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![ESPHome](https://img.shields.io/badge/ESPHome-2025.10+-blue.svg)](https://esphome.io)
[![Platform](https://img.shields.io/badge/Platform-ESP32-green.svg)](https://www.espressif.com/en/products/socs/esp32)

A complete, production-ready KNX Twisted Pair (TP) integration for ESPHome using the [Thelsing KNX library](https://github.com/thelsing/knx).

## ✨ Features

- 🎯 **8 ESPHome Platforms**: Binary Sensor, Switch, Sensor, Climate, Cover, Light, Text Sensor, Number
- 📊 **Complete DPT Support**: DPT 1.xxx, 5.xxx, 9.xxx, 10.xxx, 11.xxx, 14.xxx, 16.xxx, 19.xxx, 20.xxx
- 🔌 **Hardware Support**: Compatible with TPUART, NCN5120, Siemens BCU modules
- ⚡ **Memory Optimized**: Reduced RAM usage (~5KB saved on typical configurations)
- 🕐 **Time Broadcast**: Optional KNX time/date synchronization (DPT 19.001)
- 🔍 **BCU Detection**: SAV pin monitoring for bus connection status
- 📝 **Production Ready**: Complete error handling, logging, and validation

## 🚀 Quick Start

### Hardware Requirements

- **ESP32** (ESP-IDF framework required)
- **KNX TP Transceiver**: TPUART module, NCN5120, or Siemens BCU
- **Connection**: ESP32 GPIO ↔ Transceiver UART (typically 19200 baud, EVEN parity)

### Installation

Add to your ESPHome YAML configuration:

```yaml
external_components:
  - source:
      type: git
      url: https://github.com/fdepalo/esphome-knx-tp
      ref: main
    components: [knx_tp]
```

Or for local development:

```yaml
external_components:
  - source:
      type: local
      path: /path/to/esphome-knx-tp
    components: [knx_tp]
```

### Basic Configuration

```yaml
# ESP32 configuration (ESP-IDF required)
esp32:
  board: esp32dev
  framework:
    type: esp-idf

# UART for KNX transceiver
uart:
  id: knx_uart
  tx_pin: GPIO17
  rx_pin: GPIO16
  baud_rate: 19200
  parity: EVEN
  stop_bits: 1

# KNX TP Component
knx_tp:
  physical_address: "1.1.100"
  uart_id: knx_uart

  # Define group addresses
  group_addresses:
    - id: living_room_light
      address: "0/0/1"
    - id: temperature_sensor
      address: "1/2/3"

# Example: Simple switch
switch:
  - platform: knx_tp
    name: "Living Room Light"
    command_ga: living_room_light

# Example: Temperature sensor
sensor:
  - platform: knx_tp
    name: "Room Temperature"
    state_ga: temperature_sensor
    unit_of_measurement: "°C"
    device_class: temperature
```

## 📚 Documentation

### Supported Platforms

| Platform | Description | Example Use Cases |
|----------|-------------|-------------------|
| **Binary Sensor** | Read boolean KNX telegrams | Motion sensors, door contacts, window sensors |
| **Switch** | Control ON/OFF devices | Lights, relays, pumps |
| **Sensor** | Read analog values | Temperature, humidity, brightness, power consumption |
| **Climate** | HVAC control | Thermostats, heating/cooling systems |
| **Cover** | Control blinds/shutters | Window blinds, roller shutters, awnings |
| **Light** | Lighting with dimming | Dimmable lights, LED strips |
| **Text Sensor** | Display text/time/date | Status displays, KNX time broadcasts |
| **Number** | Numeric input/output | Setpoints, percentages, values |

### Advanced Features

#### Time Broadcast

Synchronize KNX bus time/date from NTP or other time sources:

```yaml
# Time source (e.g., Home Assistant)
time:
  - platform: homeassistant
    id: ha_time

# KNX with time broadcast
knx_tp:
  physical_address: "1.1.100"
  uart_id: knx_uart
  time_id: ha_time
  time_broadcast_ga: "time_sync"
  time_broadcast_interval: 60s  # Broadcast every 60 seconds

  group_addresses:
    - id: time_sync
      address: "0/0/100"
```

#### BCU Connection Detection

Monitor KNX bus connection status via SAV pin:

```yaml
knx_tp:
  physical_address: "1.1.100"
  uart_id: knx_uart
  sav_pin: GPIO5  # Connect to BCU SAV output
```

### Complete Examples

See the example YAML files in the repository:

- `knx-example-simple.yaml` - Basic configuration with switch and sensor
- `knx-example-advanced.yaml` - Full-featured gateway with all platforms

## 🛠️ DPT (Datapoint Types) Support

| DPT | Description | Platforms |
|-----|-------------|-----------|
| 1.xxx | Boolean (on/off) | Binary Sensor, Switch |
| 5.xxx | 8-bit unsigned (0-255) | Sensor, Number |
| 5.001 | Percentage (0-100%) | Sensor, Cover, Light |
| 5.003 | Angle (0-360°) | Sensor |
| 9.xxx | 2-byte float | Sensor, Climate, Number |
| 9.001 | Temperature (°C) | Sensor, Climate |
| 9.004 | Brightness (lux) | Sensor |
| 9.007 | Humidity (%) | Sensor |
| 10.001 | Time of Day | Text Sensor |
| 11.001 | Date | Text Sensor |
| 14.xxx | 4-byte float | Sensor, Number |
| 16.001 | Character string | Text Sensor |
| 19.001 | Date and Time | Text Sensor, Time Broadcast |
| 20.102 | HVAC Mode | Climate |

## 🔧 Troubleshooting

### Common Issues

**"KNX component not found"**
- Ensure `external_components` is correctly configured
- Verify the repository URL and `ref` branch

**"UART communication errors"**
- Check wiring: ESP32 TX → Transceiver RX, ESP32 RX → Transceiver TX
- Verify baud rate (usually 19200) and parity (EVEN)
- Ensure proper power supply for transceiver

**"No telegrams received"**
- Check KNX bus connection (use SAV pin monitoring)
- Verify physical address doesn't conflict with existing devices
- Ensure group addresses match your KNX installation

**"Compilation errors with time component"**
- The time component is optional - only needed for time broadcast
- Remove `time_id` if you don't need time synchronization

## 📊 Memory Usage

Typical RAM usage on ESP32 (with optimizations):

- **Basic config** (2-3 platforms): ~34 KB RAM, ~820 KB Flash
- **Advanced config** (all platforms): ~35 KB RAM, ~1.06 MB Flash

Memory optimizations implemented:
- ✅ std::vector instead of std::map for group addresses (-3 KB)
- ✅ uint16_t for addresses instead of std::string (-1.5 KB)
- ✅ Optimized struct padding (-0.1 KB)
- ✅ Static buffers for encoding/decoding (-0.3 KB)

## 🤝 Contributing

Contributions are welcome! Please:

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 🙏 Credits

- **Thelsing KNX Library**: https://github.com/thelsing/knx
- **ESPHome**: https://esphome.io
- **KNX Association**: https://www.knx.org
- **Developed by**: @fdepalo

## 📞 Support

- 🐛 **Issues**: [GitHub Issues](https://github.com/fdepalo/esphome-knx-tp/issues)
- 💬 **Discussions**: [GitHub Discussions](https://github.com/fdepalo/esphome-knx-tp/discussions)
- 📖 **ESPHome Community**: [ESPHome Discord](https://discord.gg/KhAMKrd)

---

**Made with ❤️ for the ESPHome and KNX communities**
