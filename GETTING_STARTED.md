# Getting Started with KNX TP Component

## Quick Start

### 1. Hardware Setup

You need a KNX bus coupler to connect your ESP32/ESP8266 to the KNX TP bus. Common options:

- **Siemens BCU (recommended)**: NCN5120, NCN5121
- **TPUART modules**: Various third-party modules
- **Weinzierl KNX BAOS**: More advanced option

#### Wiring Example (NCN5120)

```
ESP32           NCN5120
GPIO17  ------>  TX
GPIO16  <------  RX
GND     ------>  GND
3.3V    ------>  VCC
```

**Important**: The KNX bus coupler should be connected to the KNX bus according to its specifications.

### 2. ESPHome Configuration

Create a new ESPHome configuration file or add to existing:

```yaml
esphome:
  name: knx-gateway
  platform: ESP32
  board: esp32dev

# Configure UART for KNX
uart:
  id: knx_uart
  tx_pin: GPIO17
  rx_pin: GPIO16
  baud_rate: 19200
  parity: EVEN
  stop_bits: 1

# Add the external component
external_components:
  - source:
      type: local
      path: /path/to/knx_tp  # Or use GitHub URL
    components: [ knx_tp ]

# Configure KNX
knx_tp:
  physical_address: "1.1.100"
  uart_id: knx_uart
  group_addresses:
    - id: light_1
      address: "0/0/1"

# Example: Control a light via KNX
switch:
  - platform: knx_tp
    name: "Living Room Light"
    group_address: light_1
```

### 3. Upload to Device

```bash
esphome run your-config.yaml
```

## Understanding KNX Addresses

### Physical Address
- Format: `Area.Line.Device` (e.g., `1.1.100`)
- Each device on the KNX bus needs a unique physical address
- Typically: Area 1-15, Line 0-15, Device 0-255

### Group Address
- Format: `Main/Middle/Sub` or `Main.Middle.Sub` (e.g., `0/0/1` or `0.0.1`)
- Used for communication between devices
- Main: 0-31, Middle: 0-7, Sub: 0-255

## Common Use Cases

### Binary Sensor (Motion Detector)

```yaml
binary_sensor:
  - platform: knx_tp
    name: "Motion Sensor"
    group_address: motion_ga
    device_class: motion
```

### Temperature Sensor

```yaml
sensor:
  - platform: knx_tp
    name: "Room Temperature"
    group_address: temp_ga
    unit_of_measurement: "°C"
    accuracy_decimals: 1
```

### Dimmable Light

```yaml
light:
  - platform: knx_tp
    name: "Dimmable Light"
    group_address: light_switch_ga
    brightness_address: light_dim_ga
```

## Troubleshooting

### No Communication

1. **Check UART configuration**: Ensure baud rate is 19200, EVEN parity
2. **Verify wiring**: TX/RX pins might need to be swapped
3. **Check physical address**: Must be unique on the bus
4. **Bus power**: Ensure the KNX bus coupler is properly powered

### Compilation Errors

1. **Missing library**: The Thelsing KNX library should auto-install
2. **Platform compatibility**: Ensure you're using ESP32 or ESP8266
3. **ESPHome version**: Requires ESPHome 2023.11.0 or newer

### Check Logs

Enable verbose logging to debug:

```yaml
logger:
  level: VERBOSE
  logs:
    knx_tp: VERBOSE
    knx_platform: VERBOSE
```

## Next Steps

- Read the [README.md](README.md) for full documentation
- Check the [example.yaml](example.yaml) for complete configuration examples
- Explore different entity types (sensors, switches, climate, etc.)

## Resources

- [KNX Association](https://www.knx.org/)
- [Thelsing KNX Stack](https://github.com/thelsing/knx)
- [ESPHome Documentation](https://esphome.io/)
