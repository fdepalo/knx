# KNX TP Component Architecture

## Overview

This component integrates the [Thelsing KNX stack](https://github.com/thelsing/knx) into ESPHome, providing a bridge between KNX Twisted Pair networks and Home Assistant.

## Component Structure

```
knx_tp/
├── components/              # ESPHome component files
│   ├── __init__.py         # Python config/codegen
│   ├── const.py            # Constants
│   ├── knx_tp.h            # Main component header
│   ├── knx_tp.cpp          # Main component implementation
│   ├── knx_platform.h      # Platform abstraction header
│   ├── knx_platform.cpp    # Platform abstraction implementation
│   ├── group_address.h/cpp # Group address management
│   ├── dpt.h/cpp           # DPT encoding/decoding
│   └── [entity types]/     # Binary sensor, switch, sensor, etc.
├── README.md
├── example.yaml
└── library.json
```

## Key Components

### 1. KNXTPComponent (knx_tp.h/cpp)

Main component class that:
- Manages the KNX stack lifecycle
- Handles UART communication via `KNXPlatform`
- Registers and manages group addresses
- Routes telegrams to registered entities
- Integrates with Thelsing's `Bau07B0` (Bus Access Unit)

**Key Methods:**
- `setup()`: Initializes the KNX stack and configures the BAU
- `loop()`: Processes incoming/outgoing KNX telegrams
- `send_group_write()`: Sends data to a group address
- `send_group_read()`: Requests data from a group address

### 2. KNXPlatform (knx_platform.h/cpp)

Platform abstraction layer that bridges ESPHome with the Thelsing KNX stack:

**Implements:**
- UART communication (read/write operations)
- Time functions (millis, delay)
- Memory management (EEPROM simulation)
- Platform-specific operations

**Purpose:** The Thelsing stack is platform-agnostic and requires a platform implementation. This class provides that implementation for ESP32/ESP8266 running ESPHome.

### 3. Group Address Management

**GroupAddress class:**
- Represents a KNX group address
- Maps string IDs to KNX addresses
- Used by entities to subscribe to specific addresses

### 4. DPT (Data Point Types)

**DPT class:**
- Encodes/decodes KNX data according to DPT specifications
- Supports common types (boolean, percentage, temperature, etc.)
- Provides type-safe conversion between ESPHome and KNX formats

### 5. Entity Types

Each entity type (binary_sensor, switch, sensor, etc.) implements:
- `KNXEntity` interface
- `on_knx_telegram()`: Callback for incoming telegrams
- Platform-specific logic (e.g., `BinarySensor`, `Switch`)

## Data Flow

### Incoming Telegrams (KNX → ESPHome)

```
KNX Bus
  ↓
UART (Hardware)
  ↓
KNXPlatform::readUart()
  ↓
Thelsing Stack (telegram parsing)
  ↓
Bau07B0::loop()
  ↓
KNXTPComponent::group_object_callback_()
  ↓
KNXTPComponent::notify_entities_()
  ↓
Entity::on_knx_telegram()
  ↓
ESPHome State Update
  ↓
Home Assistant
```

### Outgoing Telegrams (ESPHome → KNX)

```
Home Assistant
  ↓
ESPHome Action/Service
  ↓
Entity::turn_on() / set_value()
  ↓
KNXTPComponent::send_group_write()
  ↓
Thelsing Stack (telegram creation)
  ↓
Bau07B0 processing
  ↓
KNXPlatform::writeUart()
  ↓
UART (Hardware)
  ↓
KNX Bus
```

## Integration with Thelsing KNX Stack

### Why Thelsing?

- **Mature and tested**: Used in production KNX devices
- **Full protocol support**: Complete KNX TP1 implementation
- **Standards compliant**: Implements KNX specifications correctly
- **Active development**: Regular updates and bug fixes

### BAU (Bus Access Unit)

The component uses `Bau07B0`, which is designed for:
- KNX TP1 (Twisted Pair)
- BCU1 compatibility
- ESP32/ESP8266 platforms

### Group Object Table

The Thelsing stack uses a Group Object Table (GOT) to manage:
- Mappings between group addresses and internal objects
- Communication flags (read, write, update, transmit)
- DPT configurations

**Current implementation**: Simplified GOT management. Future versions will support full ETS integration.

## Configuration Flow

1. **Python (`__init__.py`)**: Validates YAML config
2. **Code Generation**: Creates C++ initialization code
3. **Component Setup**:
   - Creates `KNXPlatform` instance
   - Initializes `Bau07B0` with platform
   - Configures physical address
   - Registers group addresses
   - Enables the BAU

## Memory Considerations

- **Stack size**: The Thelsing stack requires ~10-20KB of RAM
- **Group objects**: Each group object adds ~50-100 bytes
- **Entities**: Each entity adds ~100-200 bytes
- **UART buffers**: Managed by ESPHome (typically 256-512 bytes)

**Recommendation**: ESP32 is preferred over ESP8266 for larger installations.

## Future Enhancements

### Planned Features

1. **Full ETS support**: Import/export ETS project files
2. **Advanced DPT support**: Additional data types (datetime, RGB, etc.)
3. **Scenes and programs**: KNX scene management
4. **Diagnostics**: Bus monitoring and statistics
5. **Security**: KNX Data Secure and IP Secure support
6. **Programming mode**: OTA KNX programming via ETS

### Contributing

See component source files for TODOs and enhancement opportunities.

## Debugging

### Enable Verbose Logging

```yaml
logger:
  level: VERBOSE
  logs:
    knx_tp: VERBOSE
    knx_platform: VERBOSE
```

### Common Debug Points

- `KNXPlatform::readUart()`: Check incoming bytes
- `KNXTPComponent::group_object_callback_()`: Verify telegram reception
- `KNXTPComponent::send_group_write()`: Verify telegram transmission
- `Entity::on_knx_telegram()`: Check entity updates

## References

- [Thelsing KNX Stack Documentation](https://github.com/thelsing/knx)
- [KNX Specifications](https://www.knx.org/knx-en/for-professionals/index.php)
- [ESPHome Custom Components](https://esphome.io/custom/custom_component.html)
