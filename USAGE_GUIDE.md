# KNX TP Component for ESPHome - Usage Guide

## Quick Start

### 1. Hardware Requirements

- **ESP32 board** (tested with ESP32-DevKit)
- **KNX TP transceiver** (one of the following):
  - Siemens BCU (Bus Coupling Unit)
  - TPUART module (e.g., NCN5120, NCN5121)
  - Any KNX TP UART interface

### 2. Wiring

Connect your KNX TP transceiver to the ESP32:

```
ESP32 GPIO17 (TX) → KNX UART RX
ESP32 GPIO16 (RX) → KNX UART TX
ESP32 GND        → KNX UART GND
ESP32 3.3V       → KNX UART VCC (if needed)
```

**Note:** Some TPUART modules are powered directly from the KNX bus and don't need separate VCC.

### 3. Configuration

#### Simple Configuration

Use `knx-example-simple.yaml` for a minimal setup:

```bash
# Copy the simple example
cp knx-example-simple.yaml my-knx-device.yaml

# Edit the configuration
nano my-knx-device.yaml
```

Update these values:
- `wifi_ssid` and `wifi_password` with your WiFi credentials
- `physical_address` with your KNX device address (e.g., "1.1.100")
- Group addresses to match your KNX installation

#### Advanced Configuration

Use `knx-example-advanced.yaml` for a full-featured gateway:

```bash
# Copy secrets example
cp secrets.yaml.example secrets.yaml

# Edit secrets
nano secrets.yaml

# Copy advanced example
cp knx-example-advanced.yaml my-knx-gateway.yaml

# Edit configuration
nano my-knx-gateway.yaml
```

### 4. Compile and Upload

```bash
# Validate configuration
esphome config my-knx-device.yaml

# Compile
esphome compile my-knx-device.yaml

# Upload (first time via USB)
esphome upload my-knx-device.yaml

# Or compile and upload in one step
esphome run my-knx-device.yaml
```

## Component Configuration

### Basic KNX TP Component

```yaml
external_components:
  - source:
      type: local
      path: /path/to/knx_tp/
    components: [ knx_tp ]

uart:
  id: knx_uart
  tx_pin: GPIO17
  rx_pin: GPIO16
  baud_rate: 19200
  parity: EVEN
  stop_bits: 1

knx_tp:
  physical_address: "1.1.100"  # Your KNX device address
  uart_id: knx_uart
  group_addresses:
    - id: my_light
      address: "0/0/1"
```

### KNX Address Format

KNX supports two address formats (both are equivalent):
- **3-level:** `area/line/device` (e.g., "1/1/100")
- **Dot notation:** `area.line.device` (e.g., "1.1.100")

Valid ranges:
- **Area:** 0-31
- **Line:** 0-7
- **Device/Group:** 0-255

## Supported Components

### Binary Sensor

Receives KNX telegrams and updates state:

```yaml
binary_sensor:
  - platform: knx_tp
    name: "Motion Sensor"
    state_ga: motion_ga_id
    device_class: motion
```

### Switch

Sends ON/OFF commands to KNX:

```yaml
switch:
  - platform: knx_tp
    name: "Light Switch"
    command_ga: light_ga_id
    state_ga: light_state_ga_id  # Optional feedback address
    icon: "mdi:lightbulb"
```

### Light

Controls lights with optional dimming:

```yaml
light:
  - platform: knx_tp
    name: "Dimmable Light"
    switch_ga: light_switch_ga
    brightness_ga: light_dimmer_ga  # Optional
    state_ga: light_state_ga  # Optional feedback address
```

### Sensor

Receives numeric values (temperature, humidity, power, etc.):

```yaml
sensor:
  - platform: knx_tp
    name: "Temperature"
    state_ga: temp_ga_id
    type: temperature  # Optional, defaults to generic_2byte
    unit_of_measurement: "°C"
    accuracy_decimals: 1
    device_class: temperature
    state_class: measurement
```

### Climate

Thermostat/HVAC control:

```yaml
climate:
  - platform: knx_tp
    name: "Room Thermostat"
    temperature_ga: current_temp_ga
    setpoint_ga: setpoint_ga
    mode_ga: hvac_mode_ga  # Optional
```

### Cover

Blinds, shutters, and roller shutters:

```yaml
cover:
  - platform: knx_tp
    name: "Bedroom Blinds"
    move_ga: blinds_move_ga
    position_ga: blinds_position_ga  # Optional
    stop_ga: blinds_stop_ga  # Optional
    device_class: blind
```

### Number

Numeric input (setpoints, percentages):

```yaml
number:
  - platform: knx_tp
    name: "Temperature Setpoint"
    command_ga: setpoint_ga
    state_ga: setpoint_state_ga  # Optional feedback address
    min_value: 15
    max_value: 30
    step: 0.5
    unit_of_measurement: "°C"
```

## Troubleshooting

### Enable Debug Logging

```yaml
logger:
  level: DEBUG
  logs:
    knx_tp: VERBOSE
    uart: DEBUG
```

### UART Issues

If you see UART errors, try:
- Checking wiring (TX/RX might be swapped)
- Verifying baud rate (should be 19200 for KNX TP)
- Checking parity (should be EVEN)
- Ensuring proper ground connection

### No KNX Communication

1. **Check physical address:** Must be unique on your KNX bus
2. **Verify group addresses:** Must match your ETS configuration
3. **Check KNX bus power:** TPUART needs bus power to operate
4. **Test with ETS:** Use ETS diagnostic tools to verify bus communication

### Compilation Errors

If you get compilation errors:
- Ensure ESP-IDF framework is selected (not Arduino)
- Check that all Python dependencies are installed
- Clean build: `rm -rf .esphome/build/`

## Integration with Home Assistant

Once configured, your ESPHome device will automatically appear in Home Assistant if you have the ESPHome integration enabled.

Entities will be named based on the `name` field in your YAML configuration.

## KNX DPT (Datapoint Types)

The component automatically handles these DPT types:

- **DPT 1:** Boolean (ON/OFF, True/False)
- **DPT 5:** 8-bit unsigned (0-255, percentages, angles)
- **DPT 9:** 2-byte float (temperature, humidity, etc.)
- **DPT 14:** 4-byte float (high precision values)
- **DPT 16:** ASCII string (text)

## Examples

### Simple Light Control

```yaml
knx_tp:
  physical_address: "1.1.100"
  uart_id: knx_uart
  group_addresses:
    - id: kitchen_light
      address: "0/0/5"

switch:
  - platform: knx_tp
    name: "Kitchen Light"
    command_ga: kitchen_light
```

### Temperature Monitoring

```yaml
knx_tp:
  physical_address: "1.1.100"
  uart_id: knx_uart
  group_addresses:
    - id: living_temp
      address: "1/0/1"

sensor:
  - platform: knx_tp
    name: "Living Room Temperature"
    state_ga: living_temp
    unit_of_measurement: "°C"
    device_class: temperature
```

### Complete Room Control

```yaml
knx_tp:
  physical_address: "1.1.100"
  uart_id: knx_uart
  group_addresses:
    - id: room_light
      address: "0/0/1"
    - id: room_temp
      address: "1/0/1"
    - id: room_setpoint
      address: "3/0/1"
    - id: room_blinds
      address: "4/0/1"

light:
  - platform: knx_tp
    name: "Room Light"
    switch_ga: room_light

sensor:
  - platform: knx_tp
    name: "Room Temperature"
    state_ga: room_temp
    unit_of_measurement: "°C"
    device_class: temperature

number:
  - platform: knx_tp
    name: "Temperature Setpoint"
    command_ga: room_setpoint
    min_value: 15
    max_value: 30
    step: 0.5

cover:
  - platform: knx_tp
    name: "Room Blinds"
    move_ga: room_blinds
    device_class: blind
```

## Support

For issues, questions, or contributions, please refer to the main repository.

## License

This component uses the Thelsing KNX library, which is licensed under GPL v3.
