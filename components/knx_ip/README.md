# KNX IP Component for ESPHome

ESPHome component for **KNX/IP (KNXnet/IP)** communication over WiFi/Ethernet networks.

This is the **IP version** of the KNX component - for physical TP bus connection, use [knx_tp](../knx_tp/).

## Features

- ✅ **KNX/IP Routing** (multicast 224.0.23.12:3671)
- ✅ **KNX/IP Tunneling** (point-to-point gateway connection)
- ✅ **No hardware transceiver needed** - pure network communication
- ✅ **Shares DPT encoders** with knx_tp for consistency
- ✅ **Master clock broadcast** (DPT 19.001)
- ✅ **Full ESPHome integration** (sensors, switches, climate, etc.)
- ✅ **Based on Thelsing KNX library** (proven, ETS-compatible)

## Hardware Requirements

- ESP32 with WiFi or Ethernet connectivity
- **No KNX transceiver needed** (unlike knx_tp)
- Network infrastructure with multicast support (for routing mode)

## KNX-TP vs KNX-IP Comparison

| Feature | KNX-TP | KNX-IP |
|---------|---------|---------|
| **Transport** | UART + TP transceiver | WiFi/Ethernet |
| **Hardware** | NCN5120, TPUART, BCU | None (network only) |
| **Speed** | 9.6 kbit/s | 100+ Mbit/s |
| **Latency** | 50-100ms | 5-20ms |
| **Range** | 1000m bus | Network reach |
| **Topology** | Physical bus | IP network |
| **BAU Class** | Bau07B0 | Bau57B0 |
| **Use Case** | Direct device control | Remote control, supervision |

## Installation

Add this component to your ESPHome configuration:

```yaml
external_components:
  - source:
      type: git
      url: https://github.com/fdepalo/esphome-knx-tp
      ref: main
    components: [ knx_ip ]
```

Or for local development:

```yaml
external_components:
  - source:
      type: local
      path: components
    components: [ knx_ip ]
```

## Configuration

### Routing Mode (Multicast - Recommended)

```yaml
knx_ip:
  physical_address: "1.1.200"
  routing_mode: true              # Enable routing
  multicast_address: "224.0.23.12"  # Standard KNX multicast
  gateway_port: 3671

  group_addresses:
    - id: my_light
      address: "0/0/1"
```

**Advantages:**
- ✅ All devices receive messages (broadcast)
- ✅ No single point of failure
- ✅ Simpler configuration

**Requirements:**
- Network must support multicast
- Devices must be on same subnet

### Tunneling Mode (Gateway Connection)

```yaml
knx_ip:
  physical_address: "1.1.200"
  routing_mode: false            # Disable routing
  gateway_ip: "192.168.1.50"    # Your KNX/IP gateway
  gateway_port: 3671

  group_addresses:
    - id: my_light
      address: "0/0/1"
```

**Advantages:**
- ✅ Works across subnets
- ✅ More secure (point-to-point)
- ✅ Gateway filters traffic

**Requirements:**
- KNX/IP gateway with tunneling support
- Gateway must be reachable on network

## Configuration Options

### Component Configuration

| Option | Type | Required | Default | Description |
|--------|------|----------|---------|-------------|
| `physical_address` | string | **Yes** | - | Physical KNX address (e.g., "1.1.200") |
| `routing_mode` | boolean | No | `true` | `true` = routing, `false` = tunneling |
| `gateway_ip` | string | No* | - | Gateway IP (required if tunneling) |
| `gateway_port` | int | No | `3671` | KNX/IP port |
| `multicast_address` | string | No | `224.0.23.12` | Multicast address (routing mode) |
| `time_id` | time_id | No | - | Time source for clock broadcast |
| `time_broadcast_ga` | string | No | - | Group address for time broadcast |
| `time_broadcast_interval` | duration | No | `60s` | Time broadcast interval |

*Required when `routing_mode: false`

### Group Address Configuration

```yaml
knx_ip:
  group_addresses:
    - id: my_address_id
      address: "0/0/1"  # Format: main/middle/sub
```

## Supported Platforms

Same as knx_tp:
- ✅ Sensors
- ✅ Binary Sensors
- ✅ Switches
- ✅ Lights (with brightness)
- ✅ Climate (thermostats/HVAC)
- ✅ Covers (blinds/shutters)
- ✅ Numbers (setpoints)
- ✅ Text Sensors (date/time display)

### Example: Sensor

```yaml
sensor:
  - platform: knx_ip
    name: "Room Temperature"
    state_ga: room_temp
    unit_of_measurement: "°C"
    device_class: temperature
```

## Time Broadcast (Master Clock)

Make your ESPHome device a KNX master clock:

```yaml
time:
  - platform: homeassistant
    id: ha_time

knx_ip:
  time_id: ha_time
  time_broadcast_ga: clock_ga
  time_broadcast_interval: 60s

  group_addresses:
    - id: clock_ga
      address: "6/0/1"  # DPT 19.001
```

## Dual Mode: TP + IP Gateway

You can run **both** knx_tp and knx_ip on the same ESP32 to create a gateway:

```yaml
# Physical TP bus connection
uart:
  id: knx_uart
  tx_pin: GPIO17
  rx_pin: GPIO16
  baud_rate: 19200

knx_tp:
  physical_address: "1.1.100"
  uart_id: knx_uart

# Network IP connection
knx_ip:
  physical_address: "1.1.200"
  routing_mode: true

# Now ESP32 bridges between TP bus and IP network!
```

## Technical Details

### Architecture

```
┌─────────────────────────────────────┐
│      ESPHome Application            │
│  (sensors, switches, climate, ...)  │
└────────────────┬────────────────────┘
                 │
┌────────────────▼────────────────────┐
│      KNXIPComponent                 │
│  (Group addresses, entities)        │
└────────────────┬────────────────────┘
                 │
┌────────────────▼────────────────────┐
│     Thelsing KNX Stack              │
│        Bau57B0 (IP BAU)             │
│    Esp32IdfPlatform (WiFi/ETH)      │
└────────────────┬────────────────────┘
                 │
┌────────────────▼────────────────────┐
│         WiFi/Ethernet               │
│      UDP Multicast/Unicast          │
│       224.0.23.12:3671              │
└─────────────────────────────────────┘
```

### Library Details

- **Thelsing KNX Library**: https://github.com/thelsing/knx
- **MASK_VERSION**: 0x57B0 (IP device)
- **BAU Class**: Bau57B0 (vs Bau07B0 for TP)
- **Protocol**: KNXnet/IP (ISO 22510)
- **Standard**: KNX System Specifications v2.1+

### Memory Usage

Similar to knx_tp with optimizations:
- DPT encoders: ~2KB code
- Group addresses: 2 bytes + ID string each
- BAU stack: ~8-10KB

## Troubleshooting

### Device Not Discovered by ETS

- ✅ Check network connectivity
- ✅ Verify firewall allows UDP port 3671
- ✅ Ensure multicast routing enabled on switches/routers
- ✅ Device and ETS PC must be on same subnet (for routing mode)

### Multicast Not Working

- ✅ Check router supports IGMP (multicast management)
- ✅ Some WiFi routers block multicast by default
- ✅ Try tunneling mode as alternative

### Compilation Errors

**Error: `'Bau57B0' does not name a type`**
- ✅ Ensure MASK_VERSION=0x57B0 is set (should be automatic)
- ✅ Check KNX library version is recent

**Error: `network component required`**
- ✅ Add WiFi or Ethernet configuration to your YAML

## Examples

See [examples/knx-ip-example.yaml](../../examples/knx-ip-example.yaml) for a complete configuration.

## License

MIT License - see [LICENSE](../../LICENSE)

## Author

Created by [@fdepalo](https://github.com/fdepalo)

Based on [Thelsing KNX Library](https://github.com/thelsing/knx)
