# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.0.0] - 2025-10-21

### Added
- Initial release of ESPHome KNX TP component
- Support for 8 ESPHome platforms:
  - Binary Sensor (motion sensors, contacts, etc.)
  - Switch (lights, relays, etc.)
  - Sensor (temperature, humidity, brightness, etc.)
  - Climate (thermostats, HVAC control)
  - Cover (blinds, shutters)
  - Light (dimmable lights)
  - Text Sensor (status displays, time/date)
  - Number (setpoints, values)
- Complete DPT (Datapoint Type) support:
  - DPT 1.xxx (Boolean)
  - DPT 5.xxx (8-bit unsigned)
  - DPT 9.xxx (2-byte float)
  - DPT 10.001 (Time of Day)
  - DPT 11.001 (Date)
  - DPT 14.xxx (4-byte float)
  - DPT 16.001 (Character string)
  - DPT 19.001 (Date and Time)
  - DPT 20.102 (HVAC Mode)
- Time broadcast feature (DPT 19.001) for KNX time synchronization
- BCU connection detection via SAV pin monitoring
- Memory optimizations:
  - std::vector instead of std::map for group addresses (-3 KB)
  - uint16_t for addresses instead of std::string (-1.5 KB)
  - Optimized struct padding (-0.1 KB)
  - Static buffers for encoding/decoding (-0.3 KB)
- Complete error handling and logging
- Production-ready implementation with validation
- Integration with Thelsing KNX library
- Support for TPUART, NCN5120, Siemens BCU hardware
- ESP32 ESP-IDF framework support

### Documentation
- Comprehensive README.md with installation instructions
- Example configurations (simple and advanced)
- Troubleshooting guide
- DPT reference table
- Hardware requirements documentation

### Examples
- knx-example-simple.yaml - Basic configuration
- knx-example-advanced.yaml - Full-featured gateway

[1.0.0]: https://github.com/fdepalo/esphome-knx-tp/releases/tag/v1.0.0
