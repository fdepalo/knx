# Quick Start - KNX TP Component

## 5-Minute Setup

### Step 1: Hardware Connection

```
ESP32         KNX Bus Coupler (NCN5120)
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
GPIO17   →   TX
GPIO16   ←   RX
GND      →   GND
3.3V     →   VCC
```

### Step 2: Minimal Configuration

Create `my-knx-device.yaml`:

```yaml
esphome:
  name: my-knx-device
  platform: ESP32
  board: esp32dev

wifi:
  ssid: "YourWiFi"
  password: "YourPassword"

logger:
  level: DEBUG

api:
  encryption:
    key: "your-api-key"

ota:
  password: "your-ota-password"

# UART for KNX
uart:
  id: knx_uart
  tx_pin: GPIO17
  rx_pin: GPIO16
  baud_rate: 19200
  parity: EVEN
  stop_bits: 1

# KNX Component
external_components:
  - source:
      type: local
      path: /Users/fdp/vscode/knx_tp
    components: [ knx_tp ]

knx_tp:
  physical_address: "1.1.100"
  uart_id: knx_uart
  group_addresses:
    - id: my_light
      address: "0/0/1"

# Example: Control a KNX light
switch:
  - platform: knx_tp
    name: "Living Room Light"
    group_address: my_light
```

### Step 3: Compile and Upload

```bash
esphome run my-knx-device.yaml
```

## Common Configurations

### Motion Sensor

```yaml
knx_tp:
  group_addresses:
    - id: motion_sensor
      address: "1/0/1"

binary_sensor:
  - platform: knx_tp
    name: "Motion Detector"
    group_address: motion_sensor
    device_class: motion
```

### Temperature Sensor

```yaml
knx_tp:
  group_addresses:
    - id: temp_sensor
      address: "2/0/1"

sensor:
  - platform: knx_tp
    name: "Room Temperature"
    group_address: temp_sensor
    unit_of_measurement: "°C"
    accuracy_decimals: 1
    device_class: temperature
```

### Dimmable Light

```yaml
knx_tp:
  group_addresses:
    - id: light_switch
      address: "0/1/1"
    - id: light_dim
      address: "0/1/2"

light:
  - platform: knx_tp
    name: "Bedroom Light"
    group_address: light_switch
    brightness_address: light_dim
```

### Thermostat

```yaml
knx_tp:
  group_addresses:
    - id: current_temp
      address: "3/0/1"
    - id: target_temp
      address: "3/0/2"
    - id: hvac_mode
      address: "3/0/3"

climate:
  - platform: knx_tp
    name: "Living Room Thermostat"
    temperature_address: current_temp
    target_temperature_address: target_temp
    mode_address: hvac_mode
```

### Blinds/Cover

```yaml
knx_tp:
  group_addresses:
    - id: blinds
      address: "4/0/1"

cover:
  - platform: knx_tp
    name: "Bedroom Blinds"
    group_address: blinds
    device_class: blind
```

## Troubleshooting

### Problem: No communication

**Solution:**
1. Check UART pins (might need TX/RX swap)
2. Verify baud rate: 19200, EVEN parity
3. Check KNX bus power

### Problem: Compilation fails

**Solution:**
```yaml
# Add to platformio_options if needed
platformio_options:
  lib_deps:
    - knx@^0.9.9
```

### Problem: Device not responding

**Solution:**
1. Check physical address is unique
2. Enable verbose logging:
```yaml
logger:
  level: VERBOSE
  logs:
    knx_tp: VERBOSE
```

## Next Steps

1. ✅ Test basic communication
2. ✅ Add your KNX devices
3. ✅ Integrate with Home Assistant
4. Read [GETTING_STARTED.md](GETTING_STARTED.md) for details
5. Check [example.yaml](example.yaml) for more examples

## Support

- Issues: Check logs with verbose mode
- Documentation: See README.md and ARCHITECTURE.md
- Stack: https://github.com/thelsing/knx

---

**Happy KNX hacking! 🏠⚡**
