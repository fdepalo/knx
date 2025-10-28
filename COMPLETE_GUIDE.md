# ESPHome KNX TP - Complete Guide

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![ESPHome](https://img.shields.io/badge/ESPHome-2025.10+-blue.svg)](https://esphome.io)
[![Platform](https://img.shields.io/badge/Platform-ESP32-green.svg)](https://www.espressif.com/en/products/socs/esp32)

**Complete guide for using the KNX Twisted Pair component for ESPHome**

---

## 📑 Table of Contents

1. [Introduction](#1-introduction)
2. [Quick Start (5 minutes)](#2-quick-start-5-minutes)
3. [Hardware and Wiring](#3-hardware-and-wiring)
4. [Basic Configuration](#4-basic-configuration)
5. [Supported Platforms](#5-supported-platforms)
6. [Triggers and Automations](#6-triggers-and-automations)
7. [DPT (Datapoint Types)](#7-dpt-datapoint-types)
8. [Advanced Features](#8-advanced-features)
9. [Optimization and Performance](#9-optimization-and-performance)
10. [Troubleshooting](#10-troubleshooting)
11. [References](#11-references)

---

## 1. Introduction

### 🎯 What is this Component?

A complete, production-ready integration for communicating with KNX devices via Twisted Pair (TP) bus using ESPHome and ESP32. Based on the [Thelsing KNX library](https://github.com/thelsing/knx), it offers full support for 8 ESPHome platforms.

### ✨ Main Features

- **8 ESPHome Platforms**: Binary Sensor, Switch, Sensor, Climate, Cover, Light, Text Sensor, Number
- **Complete DPT**: Support for DPT 1.xxx, 5.xxx, 9.xxx, 10.xxx, 11.xxx, 14.xxx, 16.xxx, 19.xxx, 20.xxx
- **Hardware**: Compatible with TPUART, NCN5120, NCN5121, Siemens BCU
- **Optimized**: Reduced RAM usage (~5KB saved compared to standard implementations)
- **Time Broadcast**: Optional KNX date/time synchronization (DPT 19.001)
- **BCU Detection**: SAV pin monitoring for bus connection status
- **Advanced Triggers**: `on_telegram` and `on_group_address` for custom automations
- **Production Ready**: Complete error handling, detailed logging, validation

### 📊 Memory and Performance

**RAM Usage** (ESP32):
- Basic configuration (2-3 platforms): ~34 KB RAM, ~820 KB Flash
- Advanced configuration (all platforms): ~35 KB RAM, ~1.06 MB Flash

**Implemented Optimizations:**
- ✅ std::vector instead of std::map (-3 KB)
- ✅ uint16_t for addresses instead of std::string (-1.5 KB)
- ✅ Optimized struct padding (-0.1 KB)
- ✅ Static buffers for encoding/decoding (-0.3 KB)
- ✅ O(1) hash map for group address triggers

---

## 2. Quick Start (5 minutes)

### Step 1: Hardware

Connect the KNX transceiver to your ESP32:

```
ESP32         KNX Transceiver (e.g. NCN5120)
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
GPIO17   →   TX
GPIO16   ←   RX
GND      →   GND
3.3V     →   VCC (if needed)
```

**Note:** Some TPUART modules are powered directly from the KNX bus.

### Step 2: Minimal Configuration

Create `my-knx-device.yaml`:

```yaml
esphome:
  name: my-knx-device

esp32:
  board: esp32dev
  framework:
    type: esp-idf  # IMPORTANT: use ESP-IDF, not Arduino

wifi:
  ssid: "YourWiFi"
  password: "YourPassword"

logger:
  level: DEBUG

api:
  encryption:
    key: "your-api-key"

ota:
  password: "your-password"

# UART for KNX
uart:
  id: knx_uart
  tx_pin: GPIO17
  rx_pin: GPIO16
  baud_rate: 19200
  parity: EVEN
  stop_bits: 1

# External component
external_components:
  - source:
      type: git
      url: https://github.com/fdepalo/esphome-knx-tp
      ref: main
    components: [knx_tp]

# KNX Configuration
knx_tp:
  physical_address: "1.1.100"  # Unique physical address
  uart_id: knx_uart

  group_addresses:
    - id: living_room_light
      address: "0/0/1"

# Example: Switch to control a KNX light
switch:
  - platform: knx_tp
    name: "Living Room Light"
    command_ga: living_room_light
```

### Step 3: Compile and Upload

```bash
# Validate configuration
esphome config my-knx-device.yaml

# Compile and upload
esphome run my-knx-device.yaml
```

**Done!** Your ESP32 device can now communicate with the KNX bus.

---

## 3. Hardware and Wiring

### 3.1 Required Hardware

**ESP32:**
- ESP32-DevKit (recommended)
- ESP32-WROOM-32
- Any ESP32 board with ESP-IDF support

**KNX TP Transceiver:**
| Model | Type | Power | Notes |
|-------|------|-------|-------|
| **NCN5120** | TPUART | From KNX bus | Economical, reliable |
| **NCN5121** | TPUART | From KNX bus | Improved version |
| **Siemens BCU** | Bus Coupling Unit | From KNX bus | More complete |
| **TPUART generic** | TPUART | Varies | Various compatible models |

### 3.2 Wiring

#### Basic Schema

```
┌──────────────┐
│    ESP32     │
│              │
│  GPIO17 (TX) ├────────► RX
│  GPIO16 (RX) ◄────────┤ TX    ┌────────────┐
│         GND  ├────────┤ GND   │ Transceiver│
│        3.3V  ├────────┤ VCC   │   KNX TP   │
│              │        │       │            │
└──────────────┘        │   +   ├─────┬──────┤ KNX Bus
                        │   -   ├─────┘      │  (2 wires)
                        └───────┴────────────┘
```

#### Alternative Pins

You can use different GPIO pins by modifying the UART configuration:

```yaml
uart:
  id: knx_uart
  tx_pin: GPIO1   # Use GPIO1 instead of GPIO17
  rx_pin: GPIO3   # Use GPIO3 instead of GPIO16
  baud_rate: 19200
  parity: EVEN
  stop_bits: 1
```

**⚠️ Warning:**
- Some pins (GPIO0, GPIO2, GPIO15) are strapping pins and may cause issues
- Check your ESP32 board documentation

### 3.3 SAV Pin (Optional)

To detect KNX bus connection via BCU's SAV pin:

```yaml
knx_tp:
  physical_address: "1.1.100"
  uart_id: knx_uart
  sav_pin: GPIO5  # Connect to BCU SAV pin
```

**Behavior:**
- SAV HIGH = BCU connected to bus
- SAV LOW = BCU disconnected

---

## 4. Basic Configuration

### 4.1 KNX Addresses

#### Physical Address

Format: `Area.Line.Device` or `Area/Line/Device`

```yaml
knx_tp:
  physical_address: "1.1.100"  # Or "1/1/100"
```

**Valid ranges:**
- Area: 0-31
- Line: 0-7
- Device: 0-255

**⚠️ Important:** Physical address must be **unique** on the KNX bus!

#### Group Address

Format: `Main/Middle/Sub` or `Main.Middle.Sub`

```yaml
knx_tp:
  group_addresses:
    - id: kitchen_light
      address: "0/0/1"    # Or "0.0.1"

    - id: living_temp
      address: "1/2/10"   # Or "1.2.10"
```

**Valid ranges:**
- Main: 0-31
- Middle: 0-7
- Sub: 0-255

### 4.2 UART Configuration

```yaml
uart:
  id: knx_uart
  tx_pin: GPIO17
  rx_pin: GPIO16
  baud_rate: 19200   # KNX TP standard
  parity: EVEN       # IMPORTANT: EVEN parity
  stop_bits: 1
```

**⚠️ Do not modify:**
- `baud_rate` must be 19200
- `parity` must be EVEN
- `stop_bits` must be 1

### 4.3 Complete Configuration Example

```yaml
esphome:
  name: knx-home-gateway
  friendly_name: "KNX Home Gateway"

esp32:
  board: esp32dev
  framework:
    type: esp-idf

logger:
  level: DEBUG
  logs:
    knx_tp: VERBOSE  # Detailed KNX logs

wifi:
  ssid: !secret wifi_ssid
  password: !secret wifi_password
  ap:
    ssid: "knx-gateway-fallback"
    password: !secret ap_password

api:
  encryption:
    key: !secret api_key

ota:
  - platform: esphome
    password: !secret ota_password

uart:
  id: knx_uart
  tx_pin: GPIO17
  rx_pin: GPIO16
  baud_rate: 19200
  parity: EVEN
  stop_bits: 1

external_components:
  - source:
      type: local
      path: /path/to/knx_tp
    components: [knx_tp]

knx_tp:
  physical_address: "1.1.100"
  uart_id: knx_uart
  sav_pin: GPIO5  # Optional

  group_addresses:
    - id: entrance_light
      address: "0/0/1"
    - id: living_light
      address: "0/0/2"
    - id: living_temp
      address: "1/0/1"
    - id: entrance_motion
      address: "2/0/1"
```

---

## 5. Supported Platforms

### 5.1 Binary Sensor

Reads boolean KNX telegrams (DPT 1.xxx).

**Examples:** Motion detectors, door/window contacts, buttons

```yaml
knx_tp:
  group_addresses:
    - id: living_motion
      address: "2/0/1"
    - id: entrance_door
      address: "2/1/1"

binary_sensor:
  - platform: knx_tp
    name: "Living Room Motion"
    state_ga: living_motion
    device_class: motion

  - platform: knx_tp
    name: "Entrance Door"
    state_ga: entrance_door
    device_class: door
```

**Options:**
- `state_ga` (required): Group address to read state from
- `device_class`: motion, door, window, etc.
- `invert`: Invert the state (default: false)

### 5.2 Switch

Sends ON/OFF commands (DPT 1.001).

**Examples:** Lights, relays, pumps

```yaml
knx_tp:
  group_addresses:
    - id: kitchen_light
      address: "0/0/3"

switch:
  - platform: knx_tp
    name: "Kitchen Light"
    command_ga: kitchen_light        # Sends commands
    state_ga: kitchen_light          # Receives state (optional)
    icon: "mdi:lightbulb"
```

**Common configurations:**

```yaml
# Command only (no feedback)
switch:
  - platform: knx_tp
    name: "Light"
    command_ga: light_id

# With feedback
switch:
  - platform: knx_tp
    name: "Light"
    command_ga: light_cmd_id
    state_ga: light_state_id

# Same address for command and state
switch:
  - platform: knx_tp
    name: "Light"
    command_ga: light_id
    state_ga: light_id
```

### 5.3 Sensor

Reads numeric values (DPT 5.xxx, 9.xxx, 14.xxx).

**Examples:** Temperature, humidity, brightness, power

```yaml
knx_tp:
  group_addresses:
    - id: living_temp
      address: "1/0/1"
    - id: living_humidity
      address: "1/0/2"
    - id: outdoor_lux
      address: "1/1/1"

sensor:
  # Temperature (DPT 9.001)
  - platform: knx_tp
    name: "Living Room Temperature"
    state_ga: living_temp
    unit_of_measurement: "°C"
    accuracy_decimals: 1
    device_class: temperature
    state_class: measurement

  # Humidity (DPT 9.007)
  - platform: knx_tp
    name: "Living Room Humidity"
    state_ga: living_humidity
    unit_of_measurement: "%"
    accuracy_decimals: 0
    device_class: humidity
    state_class: measurement

  # Brightness (DPT 9.004)
  - platform: knx_tp
    name: "Outdoor Brightness"
    state_ga: outdoor_lux
    unit_of_measurement: "lx"
    accuracy_decimals: 0
    device_class: illuminance
    state_class: measurement
```

### 5.4 Light

Light control with optional dimming (DPT 1.001 + 5.001).

```yaml
knx_tp:
  group_addresses:
    - id: light_switch
      address: "0/1/1"
    - id: light_brightness
      address: "0/1/2"
    - id: light_state
      address: "0/1/10"

light:
  # Simple ON/OFF light
  - platform: knx_tp
    name: "Bedroom Light"
    switch_ga: bedroom_light

  # Dimmable light
  - platform: knx_tp
    name: "Living Room Light"
    switch_ga: light_switch
    brightness_ga: light_brightness
    state_ga: light_state
```

### 5.5 Climate

Thermostat/HVAC control (DPT 9.001 + 20.102).

```yaml
knx_tp:
  group_addresses:
    - id: current_temp
      address: "3/0/1"
    - id: temp_setpoint
      address: "3/0/2"
    - id: hvac_mode
      address: "3/0/3"
    - id: hvac_action
      address: "3/0/4"

climate:
  - platform: knx_tp
    name: "Living Room Thermostat"
    temperature_ga: current_temp
    setpoint_ga: temp_setpoint
    mode_ga: hvac_mode           # Optional
    action_ga: hvac_action       # Optional
```

### 5.6 Cover

Blinds, shutters, awnings control (DPT 5.001).

```yaml
knx_tp:
  group_addresses:
    - id: bedroom_blinds
      address: "4/0/1"
    - id: blinds_position
      address: "4/0/2"

cover:
  - platform: knx_tp
    name: "Bedroom Blinds"
    move_ga: bedroom_blinds
    position_ga: blinds_position  # Optional
    device_class: blind
```

### 5.7 Text Sensor

Display text, date, time (DPT 10.001, 11.001, 19.001, 16.001).

```yaml
knx_tp:
  group_addresses:
    - id: knx_datetime
      address: "6/0/1"
    - id: knx_time
      address: "6/0/2"

text_sensor:
  # Date and time (DPT 19.001)
  - platform: knx_tp
    name: "KNX DateTime"
    state_ga: knx_datetime
    dpt_type: "19"
    icon: "mdi:clock-digital"

  # Time only (DPT 10.001)
  - platform: knx_tp
    name: "KNX Time"
    state_ga: knx_time
    dpt_type: "10"
    icon: "mdi:clock-outline"
```

### 5.8 Number

Configurable numeric values (DPT 9.xxx, 14.xxx).

```yaml
knx_tp:
  group_addresses:
    - id: bedroom_setpoint
      address: "3/1/1"

number:
  - platform: knx_tp
    name: "Bedroom Temperature Setpoint"
    command_ga: bedroom_setpoint
    state_ga: bedroom_setpoint
    min_value: 15
    max_value: 30
    step: 0.5
    unit_of_measurement: "°C"
```

---

## 6. Triggers and Automations

The component supports two types of triggers to execute actions when KNX telegrams arrive:

### 6.1 on_telegram - Generic Trigger

Called for **ALL** telegrams received on the bus.

```yaml
knx_tp:
  physical_address: "1.1.100"
  uart_id: knx_uart

  # Generic trigger
  on_telegram:
    - logger.log:
        format: "📨 KNX: GA=%s, Size=%d bytes"
        args: ['group_address.c_str()', 'data.size()']

    - lambda: |-
        ESP_LOGI("knx", "Received on %s", group_address.c_str());

        // Filter by specific GA
        if (group_address == "0/0/1") {
          ESP_LOGI("knx", "Light changed!");
        }
```

**Available variables:**
- `group_address` (string): Telegram's group address
- `data` (vector<uint8_t>): Payload data

**Overhead:**
- RAM: ~80 bytes per trigger
- CPU: ~75 µs per telegram

**Use when:**
- Global debug and monitoring
- KNX packet sniffing
- Traffic statistics

### 6.2 on_group_address - Specific Triggers

Called **only** for specific group addresses (much more efficient!).

```yaml
knx_tp:
  physical_address: "1.1.100"
  uart_id: knx_uart

  group_addresses:
    - id: living_light
      address: "0/0/1"

  # Specific triggers
  on_group_address:
    # Living room light
    - address: "0/0/1"
      then:
        - logger.log: "🔆 Living room light changed!"
        - switch.toggle: fan

    # Temperature - With DPT Helper ✨
    - address: "1/0/1"
      then:
        - lambda: |-
            using namespace esphome::knx_tp::dpt_helpers;

            float temp = decode_dpt9(data);  // DPT 9.001 = temperature °C
            ESP_LOGI("temp", "Temperature: %.1f°C", temp);

            if (temp > 25.0) {
              id(fan).turn_on();
            }

    # Humidity - Same decoder, different meaning!
    - address: "1/0/2"
      then:
        - lambda: |-
            using namespace esphome::knx_tp::dpt_helpers;

            float hum = decode_dpt9(data);  // DPT 9.007 = humidity %
            ESP_LOGI("hum", "Humidity: %.1f%%", hum);
```

**Available variables:**
- `data` (vector<uint8_t>): Payload data

**Overhead:**
- RAM: ~100 bytes per trigger
- CPU: ~0.15 µs per lookup (O(1) with hash map!)

**Use when:**
- Normal application logic
- KNX automations
- Reactions to specific GAs

### 6.3 Performance Comparison

| Feature | `on_telegram` | `on_group_address` |
|---------|---------------|-------------------|
| **Called for** | ALL telegrams | Specific GAs only |
| **RAM/trigger** | 80 bytes | 100 bytes |
| **CPU overhead** | ~75 µs/msg | ~0.15 µs/lookup |
| **Scalability** | ❌ Degrades | ✅ O(1) constant |
| **Recommended use** | Debug | Production |

**Scenario:** 200 telegrams/sec, 5 configured triggers

- `on_telegram`: 15 ms/sec = **1.5% CPU**
- `on_group_address`: 0.15 ms/sec = **0.015% CPU**

**🎯 Recommendation:** Use `on_group_address` for normal logic!

### 6.4 Disabling Triggers

You can disable triggers individually with compile flags:

```yaml
esphome:
  name: my-device
  platformio_options:
    build_flags:
      # Disable on_telegram (saves RAM/CPU)
      - "-DUSE_KNX_ON_TELEGRAM=0"

      # Disable on_group_address
      - "-DUSE_KNX_ON_GROUP_ADDRESS=0"
```

### 6.5 DPT Helpers for Triggers ✨

To simplify DPT decoding in triggers, the component provides **helper functions** in the `dpt_helpers` namespace.

#### Available Functions

```cpp
using namespace esphome::knx_tp::dpt_helpers;

// DPT 1.xxx - Boolean (1 bit)
bool state = decode_dpt1(data);  // DPT 1.001 = switch, 1.002 = bool, etc.

// DPT 5.xxx - 8-bit (0-255)
uint8_t value = decode_dpt5(data);  // DPT 5.010 = counter, etc.
float percent = decode_dpt5_percentage(data);  // DPT 5.001 = 0-100%
float angle = decode_dpt5_angle(data);  // DPT 5.003 = 0-360°

// DPT 9.xxx - 2-byte float (-671088.64 to 670760.96)
float value = decode_dpt9(data);  // DPT 9.001 = temp, 9.004 = lux, 9.007 = humidity, etc.

// DPT 14.xxx - 4-byte float (IEEE 754)
float value = decode_dpt14(data);  // DPT 14.xxx various

// DPT 16.001 - String (ASCII, 14 bytes max)
std::string text = decode_dpt16(data);

// DPT 10.001 - Time of Day
DPT::TimeOfDay time = decode_dpt10(data);
ESP_LOGI("time", "%02d:%02d:%02d", time.hour, time.minute, time.second);

// DPT 11.001 - Date
DPT::Date date = decode_dpt11(data);
ESP_LOGI("date", "%04d-%02d-%02d", date.year, date.month, date.day);

// DPT 19.001 - Date and Time
DPT::DateTime dt = decode_dpt19(data);

// DPT 20.102 - HVAC Mode
DPT::HVACMode mode = decode_dpt20_102(data);
```

#### Complete Example

```yaml
knx_tp:
  on_group_address:
    # Temperature (DPT 9.001)
    - address: "1/0/1"
      then:
        - lambda: |-
            using namespace esphome::knx_tp::dpt_helpers;
            float temp = decode_dpt9(data);
            ESP_LOGI("temp", "Temperature: %.1f°C", temp);

    # Humidity (DPT 9.007) - same function!
    - address: "1/0/2"
      then:
        - lambda: |-
            using namespace esphome::knx_tp::dpt_helpers;
            float hum = decode_dpt9(data);
            ESP_LOGI("hum", "Humidity: %.1f%%", hum);

    # Switch (DPT 1.001)
    - address: "0/0/1"
      then:
        - lambda: |-
            using namespace esphome::knx_tp::dpt_helpers;
            bool state = decode_dpt1(data);
            ESP_LOGI("switch", "State: %s", state ? "ON" : "OFF");

    # Percentage (DPT 5.001)
    - address: "4/0/1"
      then:
        - lambda: |-
            using namespace esphome::knx_tp::dpt_helpers;
            float pos = decode_dpt5_percentage(data);
            ESP_LOGI("blind", "Position: %.0f%%", pos);

    # Time (DPT 10.001)
    - address: "2/0/1"
      then:
        - lambda: |-
            using namespace esphome::knx_tp::dpt_helpers;
            auto time = decode_dpt10(data);
            ESP_LOGI("time", "Time: %02d:%02d:%02d",
                     time.hour, time.minute, time.second);
```

**⚠️ Important:**
- DPT functions are **generic**: `decode_dpt9()` decodes all DPT 9.xxx types (temperature, humidity, lux, pressure, etc.)
- **The user decides** how to interpret and format the value
- Function names reflect the **DPT type**, not semantic meaning

**Example:**
❌ **NO** `decode_temperature()` function exists
✅ **Use** `decode_dpt9(data)` and format as temperature

---

## 7. DPT (Datapoint Types)

### 7.1 Supported DPTs

| DPT | Description | Size | Platforms |
|-----|-------------|------|-----------|
| **1.xxx** | Boolean (ON/OFF) | 1 bit | Binary Sensor, Switch |
| **5.001** | Percentage (0-100%) | 1 byte | Sensor, Cover, Light |
| **5.003** | Angle (0-360°) | 1 byte | Sensor |
| **5.xxx** | Unsigned 8-bit (0-255) | 1 byte | Sensor, Number |
| **9.001** | Temperature (°C) | 2 bytes | Sensor, Climate |
| **9.004** | Brightness (lux) | 2 bytes | Sensor |
| **9.007** | Humidity (%) | 2 bytes | Sensor |
| **9.xxx** | 2-byte float | 2 bytes | Sensor, Number |
| **10.001** | Time of Day | 3 bytes | Text Sensor |
| **11.001** | Date | 3 bytes | Text Sensor |
| **14.xxx** | 4-byte float | 4 bytes | Sensor, Number |
| **16.001** | ASCII String | 14 bytes | Text Sensor |
| **19.001** | Date and Time | 8 bytes | Text Sensor, Time Broadcast |
| **20.102** | HVAC Mode | 1 byte | Climate |

### 7.2 DPT 1 - Boolean

```cpp
// DPT 1.001 - Switch (ON/OFF)
bool state = data[0] & 0x01;

// Encoding
std::vector<uint8_t> telegram = {state ? 0x01 : 0x00};
```

**YAML usage:**
```yaml
binary_sensor:
  - platform: knx_tp
    name: "Motion"
    state_ga: motion_ga

switch:
  - platform: knx_tp
    name: "Light"
    command_ga: light_ga
```

### 7.3 DPT 5 - Unsigned 8-bit

```cpp
// DPT 5.001 - Percentage (0-100%)
uint8_t percent = data[0];  // 0-255 → 0-100%
float percentage = (percent * 100.0f) / 255.0f;

// DPT 5.003 - Angle (0-360°)
uint8_t angle_raw = data[0];  // 0-255 → 0-360°
float angle = (angle_raw * 360.0f) / 255.0f;
```

**YAML usage:**
```yaml
sensor:
  - platform: knx_tp
    name: "Blind Position"
    state_ga: position_ga
    unit_of_measurement: "%"

cover:
  - platform: knx_tp
    name: "Blind"
    move_ga: blind_ga
```

### 7.4 DPT 9 - 2-byte Float

Most common format for temperature, humidity, brightness.

```cpp
// Decode DPT 9
uint16_t raw = (data[0] << 8) | data[1];

// Extract mantissa and exponent
int16_t mantissa = (raw & 0x7FF);  // 11 bits
if (raw & 0x8000) {  // Sign bit
  mantissa = -(~mantissa & 0x7FF) - 1;
}
int8_t exponent = (raw >> 11) & 0x0F;  // 4 bits

// Calculate value
float value = 0.01f * mantissa * (1 << exponent);
```

**DPT 9 Encoding:**
```cpp
float temp = 21.5;  // Temperature in °C

// Find best exponent
int8_t exponent = 0;
int32_t mantissa = (int32_t)(temp * 100.0f);

while (mantissa > 2047 || mantissa < -2048) {
  exponent++;
  mantissa = (int32_t)(temp * 100.0f / (1 << exponent));
}

// Create raw value
uint16_t raw = ((exponent & 0x0F) << 11) | (mantissa & 0x7FF);
if (mantissa < 0) raw |= 0x8000;

std::vector<uint8_t> telegram = {
  (uint8_t)(raw >> 8),
  (uint8_t)(raw & 0xFF)
};
```

**YAML usage:**
```yaml
sensor:
  - platform: knx_tp
    name: "Temperature"
    state_ga: temp_ga
    unit_of_measurement: "°C"
    device_class: temperature
```

### 7.5 DPT 10 - Time of Day

```cpp
// Decode DPT 10.001
uint8_t day_of_week = (data[0] >> 5) & 0x07;  // 0-7
uint8_t hour = data[0] & 0x1F;                 // 0-23
uint8_t minute = data[1] & 0x3F;               // 0-59
uint8_t second = data[2] & 0x3F;               // 0-59

ESP_LOGI("time", "%d:%02d:%02d (DoW: %d)",
         hour, minute, second, day_of_week);
```

**DPT 10 Encoding:**
```cpp
uint8_t hour = 14, minute = 30, second = 45;
uint8_t day_of_week = 1;  // Monday

std::vector<uint8_t> telegram = {
  (uint8_t)((day_of_week << 5) | hour),
  (uint8_t)minute,
  (uint8_t)second
};
```

### 7.6 DPT 11 - Date

```cpp
// Decode DPT 11.001
uint8_t day = data[0];    // 1-31
uint8_t month = data[1];  // 1-12
uint8_t year_coded = data[2];

// Convert year
uint16_t year;
if (year_coded >= 90) {
  year = 1900 + year_coded;  // 1990-1999
} else {
  year = 2000 + year_coded;  // 2000-2089
}

ESP_LOGI("date", "%02d/%02d/%04d", day, month, year);
```

### 7.7 DPT 19 - Date and Time

Complete format with quality flags (8 bytes).

```cpp
// Decode DPT 19.001
uint16_t year = (data[0] << 8) | data[1];
uint8_t month = data[2];
uint8_t day = data[3];
uint8_t day_of_week = (data[4] >> 5) & 0x07;
uint8_t hour = data[4] & 0x1F;
uint8_t minute = data[5];
uint8_t second = data[6];

// Flags byte
bool fault = data[7] & 0x80;
bool working_day = data[7] & 0x40;
bool no_wd = data[7] & 0x20;
bool no_year = data[7] & 0x10;
bool no_date = data[7] & 0x08;
bool no_dow = data[7] & 0x04;
bool no_time = data[7] & 0x02;
bool summer_time = data[7] & 0x01;
```

---

## 8. Advanced Features

### 8.1 Time Broadcast

Synchronize KNX clock from NTP or Home Assistant.

```yaml
# Time source (Home Assistant)
time:
  - platform: homeassistant
    id: ha_time
    timezone: Europe/Rome

# KNX with time broadcast
knx_tp:
  physical_address: "1.1.100"
  uart_id: knx_uart

  # Time broadcast configuration
  time_id: ha_time
  time_broadcast_ga: clock_sync
  time_broadcast_interval: 60s  # Every 60 seconds

  group_addresses:
    - id: clock_sync
      address: "6/0/1"  # DPT 19.001
```

**What it does:**
- Reads current time from time source
- Encodes in DPT 19.001 (Date and Time)
- Sends on `time_broadcast_ga` every `time_broadcast_interval`
- Includes DST, working day, quality

**Example log output:**
```
[14:30:00] [I] [knx_tp:401] Broadcasted time: 2024-10-26 14:30:00 (DST)
```

### 8.2 BCU Connection Detection

Monitor KNX bus connection status via SAV pin.

```yaml
knx_tp:
  physical_address: "1.1.100"
  uart_id: knx_uart
  sav_pin: GPIO5  # Connect to BCU SAV pin
```

**Behavior:**
- SAV HIGH → BCU connected, communication active
- SAV LOW → BCU disconnected, communication suspended

**Automatic logs:**
```
[I] [knx_tp:86] BCU connected to KNX bus (SAV pin HIGH)
[W] [knx_tp:88] BCU disconnected from KNX bus (SAV pin LOW)
```

**Use in automations:**
```yaml
binary_sensor:
  - platform: template
    name: "KNX Bus Connected"
    lambda: |-
      return id(knx_tp_component).is_bcu_connected();
```

### 8.3 Advanced Logging

```yaml
logger:
  level: VERBOSE
  logs:
    # Detailed KNX component logs
    knx_tp: VERBOSE

    # Specific logs for debugging
    uart: DEBUG            # UART communication
    component: DEBUG       # Component lifecycle
```

**Log levels:**
- `ERROR`: Critical errors only
- `WARN`: Warnings and errors
- `INFO`: Normal operational information
- `DEBUG`: Detailed debugging
- `VERBOSE`: Everything including raw telegrams

### 8.4 Web Server for Debugging

```yaml
web_server:
  port: 80
  auth:
    username: admin
    password: !secret web_password
```

Access `http://knx-device.local` to:
- See real-time entity states
- Control switches/lights
- View logs
- Interactive debugging

---

## 9. Optimization and Performance

### 9.1 RAM Memory

**Implemented optimizations:**

```cpp
// ❌ Before (naive implementation)
std::map<std::string, GroupAddress*> addresses;  // ~8KB
std::string ga_cache[100];                       // ~2KB

// ✅ After (optimized)
std::vector<GroupAddress*> addresses;            // ~400 bytes
uint16_t ga_cache[100];                          // ~200 bytes

// Savings: ~10KB RAM!
```

**Tips to reduce RAM:**

1. **Limit number of group addresses:**
```yaml
# ❌ Too many unnecessary GAs
group_addresses:
  - id: ga_1
  - id: ga_2
  # ... 50+ addresses

# ✅ Only actually used GAs
group_addresses:
  - id: main_lights
  - id: main_temp
```

2. **Disable unused triggers:**
```yaml
platformio_options:
  build_flags:
    - "-DUSE_KNX_ON_TELEGRAM=0"  # -80 bytes per trigger
```

3. **Reduce logging:**
```yaml
logger:
  level: INFO  # Instead of VERBOSE
  logs:
    knx_tp: INFO  # Instead of VERBOSE
```

### 9.2 CPU Performance

**O(1) Triggers with Hash Map:**

```cpp
// O(1) group address trigger lookup
std::unordered_map<uint16_t, CallbackManager> ga_callbacks_;

// Instead of O(N) with vector:
for (auto &trigger : triggers) {  // O(N) - slow!
  if (trigger.address == ga) { ... }
}
```

**Result:**
- 200 msg/sec with 10 triggers = 0.02% CPU
- Scalable to thousands of triggers without degradation

### 9.3 Compilation Optimization

```yaml
esphome:
  platformio_options:
    build_flags:
      # Optimize for size
      - "-Os"

      # Disable C++ exceptions (saves ~10KB Flash)
      - "-fno-exceptions"

      # Disable RTTI (saves ~5KB Flash)
      - "-fno-rtti"

      # Link-time optimizations
      - "-flto"
```

---

## 10. Troubleshooting

### 10.1 No Communication

**Symptom:** No telegrams received/sent

**Solutions:**

1. **Verify wiring:**
```
ESP32 TX → Transceiver RX  ✅
ESP32 RX → Transceiver TX  ✅
Common GND                 ✅
```

2. **Check UART:**
```yaml
logger:
  logs:
    uart: DEBUG

# Check in logs:
# [D][uart:xxx] TX: ...  ← Data sent
# [D][uart:xxx] RX: ...  ← Data received
```

3. **Test TX/RX swap:**
```yaml
uart:
  tx_pin: GPIO16  # Swap
  rx_pin: GPIO17  # these
```

4. **Verify transceiver power:**
- Check LED (if present)
- Measure voltage on VCC (should be 3.3V or powered from bus)

5. **Check physical address:**
```yaml
knx_tp:
  physical_address: "1.1.100"  # MUST be unique!
```

### 10.2 Compilation Errors

**Error: `knx library not found`**

```yaml
# Solution: Add explicitly
platformio_options:
  lib_deps:
    - https://github.com/thelsing/knx.git#master
```

**Error: `esp-idf required`**

```yaml
# Solution: Use ESP-IDF framework
esp32:
  framework:
    type: esp-idf  # NOT arduino!
```

**Error: `MASK_VERSION redefined`**

Ignore this warning - it's normal and harmless.

### 10.3 Telegrams Not Received

**Symptom:** Switch works, sensors don't

**Debug:**

```yaml
logger:
  logs:
    knx_tp: VERBOSE

# In logs look for:
# [V][knx_tp:xxx] Received telegram on GA 0/0/1
# If you see this, component is working!
```

**Common causes:**

1. **Wrong group address:**
```yaml
# Verify in ETS that GA is correct
sensor:
  - platform: knx_tp
    state_ga: temp_ga  # Does this GA really exist?
```

2. **KNX device not transmitting:**
- Verify with ETS that device actually sends
- Use Group Monitor in ETS to see traffic

3. **Incompatible DPT:**
```yaml
# Sensor expects DPT 9 (2-byte float)
# But receives DPT 5 (1-byte) → silent error

# Solution: use correct type
sensor:
  - platform: knx_tp
    # Add type if needed
```

### 10.4 Insufficient RAM

**Symptom:** Random crashes, continuous reboots

**Solutions:**

1. **Reduce group addresses:**
```yaml
# Each GA costs ~40 bytes
# Remove unused ones
```

2. **Disable features:**
```yaml
# Disable time broadcast if not needed
knx_tp:
  # time_id: ...  ← Comment out
```

3. **Optimize logging:**
```yaml
logger:
  level: INFO  # Instead of VERBOSE
  baud_rate: 0  # Disable UART logger
```

4. **Use optimized partition scheme:**
```yaml
esp32:
  board: esp32dev
  framework:
    type: esp-idf
    platform_version: 5.0.0
  flash_size: 4MB
```

### 10.5 High Latency

**Symptom:** Delay between command and action

**Causes:**

1. **Busy KNX bus:** Normal, max 9.6kbit/s
2. **Slow WiFi:** Check connection
3. **Too many logs:** Disable VERBOSE

**Optimizations:**

```yaml
# Reduce logs
logger:
  level: INFO

# Optimize WiFi
wifi:
  power_save_mode: NONE  # Disable power save

# Fast scan
wifi:
  fast_connect: true
```

### 10.6 Debug Logging

```yaml
# Complete debug configuration
logger:
  level: VERBOSE
  logs:
    knx_tp: VERBOSE
    uart: DEBUG
    component: DEBUG

# Enable raw UART debug
uart:
  id: knx_uart
  debug:
    direction: BOTH
    dummy_receiver: true
```

**Typical output:**
```
[V][knx_tp:248] Notifying 5 entities about telegram for GA 0/0/1
[D][knx_tp:253] Group write to light_ga (0/0/1)
[V][uart:123] TX: BC 11 01 00 00 E1 00 81
[V][uart:145] RX: BC 11 01 00 00 E1 00 81
```

---

## 11. References

### 11.1 Useful Links

**Documentation:**
- [ESPHome Docs](https://esphome.io)
- [KNX Association](https://www.knx.org)
- [Thelsing KNX Library](https://github.com/thelsing/knx)
- [ESP-IDF Docs](https://docs.espressif.com/projects/esp-idf/)

**Community:**
- [ESPHome Discord](https://discord.gg/KhAMKrd)
- [Home Assistant Forum](https://community.home-assistant.io)
- [KNX User Forum](https://knx-user-forum.de/)

**Repository:**
- [GitHub Project](https://github.com/fdepalo/esphome-knx-tp)
- [Issues](https://github.com/fdepalo/esphome-knx-tp/issues)
- [Discussions](https://github.com/fdepalo/esphome-knx-tp/discussions)

### 11.2 Example Files

In the repository you'll find:

- `examples/knx-example-simple.yaml` - Basic configuration
- `examples/knx-example-advanced.yaml` - All features
- `examples/knx-triggers-example.yaml` - Triggers and automations
- `examples/knx-ip-example.yaml` - KNX/IP instead of TP

### 11.3 DPT Reference

| DPT | Name | Range | Bytes |
|-----|------|-------|-------|
| 1.001 | Switch | ON/OFF | 1 bit |
| 5.001 | Percentage | 0-100% | 1 byte |
| 5.003 | Angle | 0-360° | 1 byte |
| 9.001 | Temperature | -273...+670760°C | 2 bytes |
| 9.004 | Illuminance | 0...670760 lux | 2 bytes |
| 9.007 | Humidity | 0...670760% | 2 bytes |
| 10.001 | Time of Day | HH:MM:SS | 3 bytes |
| 11.001 | Date | DD.MM.YYYY | 3 bytes |
| 14.xxx | Float | 32-bit IEEE float | 4 bytes |
| 16.001 | String | ASCII text | 14 bytes |
| 19.001 | DateTime | Full date/time | 8 bytes |
| 20.102 | HVAC Mode | Heat/Cool/Auto | 1 byte |

### 11.4 Compile Flags

```yaml
platformio_options:
  build_flags:
    # Disable triggers
    - "-DUSE_KNX_ON_TELEGRAM=0"
    - "-DUSE_KNX_ON_GROUP_ADDRESS=0"

    # Optimizations
    - "-Os"                    # Size optimization
    - "-fno-exceptions"        # Disable exceptions
    - "-fno-rtti"             # Disable RTTI
    - "-flto"                 # Link-time optimization

    # Debug
    - "-DCORE_DEBUG_LEVEL=5"  # ESP32 debug level
```

### 11.5 Credits

**Developed by:** [@fdepalo](https://github.com/fdepalo)

**Based on:**
- [Thelsing KNX Library](https://github.com/thelsing/knx) - Complete KNX stack
- [ESPHome](https://esphome.io) - IoT Framework

**License:** MIT License

**Contributions:** PRs and Issues welcome!

---

## 🎯 Quick Reference Card

### 3-Step Setup

```yaml
# 1. ESP32 + ESP-IDF
esp32:
  board: esp32dev
  framework: { type: esp-idf }

# 2. UART
uart:
  id: knx_uart
  tx_pin: GPIO17
  rx_pin: GPIO16
  baud_rate: 19200
  parity: EVEN

# 3. KNX
knx_tp:
  physical_address: "1.1.100"
  uart_id: knx_uart
  group_addresses:
    - { id: my_ga, address: "0/0/1" }
```

### Platforms Quick Reference

```yaml
# Switch ON/OFF
switch:
  - platform: knx_tp
    name: "Light"
    command_ga: light_id

# Sensor (temp/humidity)
sensor:
  - platform: knx_tp
    name: "Temperature"
    state_ga: temp_id
    unit_of_measurement: "°C"

# Binary Sensor (motion/door)
binary_sensor:
  - platform: knx_tp
    name: "Motion"
    state_ga: motion_id

# Climate (thermostat)
climate:
  - platform: knx_tp
    name: "Thermostat"
    temperature_ga: temp_id
    setpoint_ga: setpoint_id

# Cover (blinds)
cover:
  - platform: knx_tp
    name: "Blinds"
    move_ga: blinds_id

# Light (dimmable)
light:
  - platform: knx_tp
    name: "Dimmable"
    switch_ga: light_switch_id
    brightness_ga: light_dim_id
```

### Troubleshooting Quick Fixes

```yaml
# Problem: No communication
uart:
  tx_pin: GPIO16  # ← Try swapping
  rx_pin: GPIO17  # ← TX with RX

# Problem: Crash/reboot
logger:
  level: INFO  # ← Reduce logs
platformio_options:
  build_flags:
    - "-DUSE_KNX_ON_TELEGRAM=0"  # ← Disable trigger

# Problem: Latency
wifi:
  power_save_mode: NONE  # ← Disable power save
```

---

**End of Complete Guide**

For questions, problems or suggestions, open an [Issue on GitHub](https://github.com/fdepalo/esphome-knx-tp/issues)!

**Made with ❤️ for the ESPHome and KNX communities**
