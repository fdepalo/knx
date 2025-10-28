# KNX TP Component for ESPHome

Complete KNX Twisted Pair integration for ESPHome 2025.10+

## Features

- **8 Components**: Binary Sensor, Switch, Sensor, Climate, Cover, Light, Text Sensor, Number
- **Complete DPT Library**: 1.xxx, 5.xxx, 9.xxx, 14.xxx, 16.xxx, 20.xxx
- **34 Files**: Full implementation with no abbreviations
- **Production Ready**: Complete error handling and logging

## Installation

```yaml
external_components:
  - source:
      type: local
      path: components
    components: [knx_tp]
```

## Configuration

```yaml
knx_tp:
  physical_address: "1.1.1"
  group_addresses:
    - id: light_living
      address: "1/2/3"

binary_sensor:
  - platform: knx_tp
    name: "Motion"
    knx_id: knx_tp
    state_ga: light_living
    auto_reset_time: 5s

switch:
  - platform: knx_tp
    name: "Light"
    knx_id: knx_tp
    command_ga: light_living

sensor:
  - platform: knx_tp
    name: "Temperature"
    knx_id: knx_tp
    state_ga: temp_sensor
    type: temperature

climate:
  - platform: knx_tp
    name: "Thermostat"
    knx_id: knx_tp
    temperature_ga: temp
    setpoint_ga: setpoint

cover:
  - platform: knx_tp
    name: "Blind"
    knx_id: knx_tp
    move_ga: blind_move
    position_ga: blind_pos

light:
  - platform: knx_tp
    name: "Kitchen"
    knx_id: knx_tp
    switch_ga: kitchen_light
    brightness_ga: kitchen_dim

text_sensor:
  - platform: knx_tp
    name: "Status"
    knx_id: knx_tp
    state_ga: status_text

number:
  - platform: knx_tp
    name: "Setpoint"
    knx_id: knx_tp
    command_ga: setpoint
    min_value: 0
    max_value: 100
```

## License

MIT License

## Credits

Developed for ESPHome community
