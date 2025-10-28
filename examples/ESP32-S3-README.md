# ESP32-S3 KNX Gateway - Full Configuration Guide

## Hardware Requirements

### ESP32-S3 N16R8 Specifications
- **MCU**: ESP32-S3 (Xtensa dual-core LX7 @ 240MHz)
- **Flash**: 16MB (vs standard 4MB)
- **PSRAM**: 8MB PSRAM (Octal SPI)
- **WiFi**: 802.11 b/g/n (2.4GHz)
- **Bluetooth**: BLE 5.0
- **USB**: Native USB-OTG (no external USB-UART needed)

### Pin Configuration

```
GPIO Assignments (knx-esp32s3-full.yaml):
===============================================
KNX-TP UART:
  TX:  GPIO1  -> TPUART TX
  RX:  GPIO2  -> TPUART RX
  SAV: GPIO3  -> Save pin (optional)

Sensors:
  Motion:  GPIO4  (INPUT_PULLUP)
  Window:  GPIO5  (INPUT_PULLUP)
  Relay:   GPIO6  (OUTPUT)
  Dimmer:  GPIO7  (PWM/LEDC)

I2C (BME280):
  SDA: GPIO8
  SCL: GPIO9

SPI (future expansion):
  MOSI: GPIO11
  CLK:  GPIO12
  MISO: GPIO13

Status LED:
  RGB: GPIO48 (WS2812 built-in on many boards)
```

## What's Included in This Configuration

### ✅ Dual KNX Stack
- **KNX-TP** on UART (physical address 1.1.200)
- **KNX-IP** on Network (physical address 1.1.201)
- Both run **simultaneously** - acts as TP ↔ IP router

### ✅ Maximum Features Enabled
Thanks to 16MB flash, this config includes:

1. **Web Server v3** with full UI, logs, OTA
2. **Both KNX stacks** (TP + IP) - normally you'd use one
3. **Multiple entity types**:
   - Binary sensors (motion, window)
   - Sensors (temperature, humidity, pressure)
   - Switches (relays)
   - Lights (dimmable, RGB)
   - Climate control (thermostat)
   - Covers (blinds/shutters)
4. **Advanced logging** with UART debug output
5. **System monitoring** (heap, PSRAM, WiFi, uptime)
6. **Time broadcast** to KNX (from NTP)
7. **I2C sensors** (BME280 temperature/humidity)
8. **Debug mode** with detailed diagnostics

### 📊 Resource Usage Estimate

| Component | Flash Used | RAM Used | PSRAM Used |
|-----------|------------|----------|------------|
| ESPHome Core | ~1.5 MB | ~60 KB | - |
| KNX-TP Stack | ~200 KB | ~15 KB | - |
| KNX-IP Stack | ~200 KB | ~15 KB | - |
| Web Server v3 | ~800 KB | ~20 KB | ~100 KB |
| WiFi Stack | ~600 KB | ~50 KB | - |
| Entities | ~400 KB | ~30 KB | - |
| Logging | ~100 KB | ~10 KB | - |
| **TOTAL** | **~3.8 MB** | **~200 KB** | **~100 KB** |
| **Available** | **4 MB (app)** | **320 KB** | **8 MB** |
| **Utilization** | **95%** | **62%** | **1.2%** |

**Note**: With 16MB flash, you have TWO 4MB OTA partitions plus 7.5MB SPIFFS!

## Installation Steps

### 1. Hardware Setup

#### Option A: ESP32-S3 DevKit
```
Purchase: Any ESP32-S3-DevKitC-1-N16R8
Cost: ~$8-15 USD

Connections:
┌─────────────────┐
│   ESP32-S3      │
│  N16R8 Board    │
├─────────────────┤
│ GPIO1  (TX) ────┼──> TPUART TX
│ GPIO2  (RX) ────┼──< TPUART RX
│ GPIO3  (SAV)────┼──> TPUART SAVE (optional)
│ GND ────────────┼──> TPUART GND
│ 3.3V ───────────┼──> TPUART VCC
│                 │
│ GPIO8  (SDA)────┼──> BME280 SDA
│ GPIO9  (SCL)────┼──> BME280 SCL
│                 │
│ GPIO4  ─────────┼──> Motion Sensor
│ GPIO5  ─────────┼──> Window Sensor
│ GPIO6  ─────────┼──> Relay/Light
│ GPIO7  ─────────┼──> Dimmer (PWM)
└─────────────────┘
```

#### Option B: Custom PCB Design
For production, consider designing custom PCB with:
- ESP32-S3-WROOM-1-N16R8 module
- TPUART 2 interface (Siemens or compatible)
- Power supply (230V AC -> 3.3V DC)
- Relay outputs
- Sensor inputs
- Status LEDs

### 2. Software Setup

```bash
# 1. Clone repository (if not done)
cd ~/vscode/knx_tp

# 2. Copy secrets template
cp secrets.yaml.example secrets.yaml

# 3. Edit secrets.yaml with your credentials
nano secrets.yaml

# Required values:
# - wifi_ssid: "YourNetworkName"
# - wifi_password: "YourWiFiPassword"
# - api_encryption_key: (generate with: openssl rand -base64 32)
# - ota_password: "some-secure-password"
# - ap_password: "fallback-ap-password"

# 4. Compile (first time - will download toolchain)
esphome compile examples/knx-esp32s3-full.yaml

# 5. Flash via USB (ESP32-S3 has native USB)
esphome upload examples/knx-esp32s3-full.yaml

# 6. View logs
esphome logs examples/knx-esp32s3-full.yaml
```

### 3. Verify Installation

After flashing, you should see:
```
INFO KNX Gateway started - Flash: 16MB, PSRAM: 8MB
INFO Setting up KNX TP...
INFO   Physical Address: 1.1.200
INFO   UART: TX=GPIO1, RX=GPIO2, SAV=GPIO3
INFO   Group Addresses: 13
INFO KNX TP setup complete
INFO Setting up KNX IP...
INFO   Physical Address: 1.1.201
INFO   Mode: Routing (multicast)
INFO   Multicast: 224.0.23.12:3671
INFO   Group Addresses: 3
INFO KNX IP setup complete
INFO WiFi connected, IP: 192.168.1.xxx
INFO Free heap: 250 KB, Free PSRAM: 7900 KB
```

## Configuration Customization

### Disable KNX-IP (TP only)
If you only need KNX-TP, comment out the `knx_ip:` section:

```yaml
# knx_ip:
#   id: knx_ip_bus
#   physical_address: "1.1.201"
#   ...
```

This saves ~200KB flash and ~15KB RAM.

### Disable Web Server
Save ~800KB flash:

```yaml
# web_server:
#   port: 80
#   ...
```

### Reduce Logging
Save RAM and improve performance:

```yaml
logger:
  level: INFO  # Instead of DEBUG
  logs:
    knx_tp: INFO  # Instead of DEBUG
    knx_ip: INFO
```

### Use Ethernet Instead of WiFi
Replace `wifi:` section with:

```yaml
ethernet:
  type: W5500
  clk_pin: GPIO10
  mosi_pin: GPIO11
  miso_pin: GPIO13
  cs_pin: GPIO12
  interrupt_pin: GPIO14
  reset_pin: GPIO9
```

**Benefits**: More reliable, faster, no WiFi interference.

## Performance Optimization

### CPU Frequency
Already optimized at 240MHz:
```yaml
CONFIG_ESP_DEFAULT_CPU_FREQ_MHZ_240: y
```

### Compiler Optimization
Already set to maximum performance:
```yaml
CONFIG_COMPILER_OPTIMIZATION_PERF: y
```

### PSRAM Configuration
Already optimized for 80MHz Octal SPI:
```yaml
CONFIG_SPIRAM_MODE_OCT: y
CONFIG_SPIRAM_SPEED_80M: y
```

### Network Buffer Sizes
Already increased for high throughput:
```yaml
CONFIG_LWIP_TCP_SND_BUF_DEFAULT: "65535"
CONFIG_LWIP_TCP_WND_DEFAULT: "65535"
```

## Monitoring and Debugging

### Web Interface
Access at: `http://knx-gateway-s3.local` or `http://192.168.1.xxx`

**Available pages**:
- `/` - Main dashboard with all entities
- `/logs` - Real-time log viewer
- `/config` - Configuration viewer (read-only)
- `/metrics` - Prometheus metrics (if enabled)

### Serial Logs
```bash
# View live logs via USB
esphome logs examples/knx-esp32s3-full.yaml

# Or direct serial connection
screen /dev/cu.usbmodem* 115200
# or
minicom -D /dev/cu.usbmodem* -b 115200
```

### Memory Monitoring
Built-in memory sensors update every 10 seconds:
- **Free Heap**: Internal SRAM available (~250 KB typical)
- **Free PSRAM**: External RAM available (~7900 KB typical)

Low memory warnings trigger automatically if heap < 30KB.

### KNX Bus Monitoring
Enable UART debugging to see raw KNX telegrams:
```yaml
uart:
  debug:
    direction: BOTH
    after:
      timeout: 50ms
```

Output shows hex dump of all TPUART traffic:
```
[13:45:23][V][knx_uart:161]: Received: BC 11 01 E1 00 81 3A
[13:45:23][D][knx_tp:234]: Telegram: 1.1.1 -> 1/0/1: 01 (ON)
```

## Troubleshooting

### Issue: Device not booting
**Symptoms**: No serial output, LED not blinking
**Solutions**:
1. Check power supply (min 500mA @ 3.3V)
2. Try holding BOOT button while powering on
3. Reflash in safe mode: `esphome safe-mode examples/knx-esp32s3-full.yaml`

### Issue: WiFi not connecting
**Symptoms**: "WiFi connection timeout"
**Solutions**:
1. Verify `secrets.yaml` has correct SSID/password
2. Check 2.4GHz WiFi is enabled (ESP32 doesn't support 5GHz)
3. Move closer to router
4. Check router logs for MAC address blocking
5. Try fallback AP: Connect to "KNX-Gateway-S3-AP"

### Issue: KNX-TP not working
**Symptoms**: No telegrams received/sent
**Solutions**:
1. Verify TPUART connections (TX/RX swapped?)
2. Check KNX bus voltage (should be ~30V DC)
3. Verify physical address doesn't conflict
4. Check ETS programming (download button pressed?)
5. Test with KNX monitor tool (Calimero, ETS)

### Issue: Out of memory errors
**Symptoms**: "heap_caps_alloc failed", random reboots
**Solutions**:
1. Check memory sensors (should have >30KB free heap)
2. Reduce log level to INFO
3. Disable web server if not needed
4. Disable unused entity types
5. Increase `CONFIG_LWIP_MAX_SOCKETS` if many connections

### Issue: OTA update fails
**Symptoms**: "Upload failed", "MD5 mismatch"
**Solutions**:
1. Ensure stable WiFi connection during update
2. Try wired upload first time: `esphome upload --device 192.168.1.xxx`
3. Check OTA password matches
4. Reboot device before OTA
5. Use safe mode: Hold BOOT button, trigger restart

## Advanced Features

### KNX Router Mode (TP ↔ IP)
This configuration already acts as a router between TP and IP:

```yaml
# Route telegrams from TP bus to IP network
knx_tp:
  route_to_ip: true  # Forward TP telegrams to IP

knx_ip:
  route_to_tp: true  # Forward IP telegrams to TP
```

**Use case**: Connect KNX TP installation to KNX/IP network.

### Data Logging to SPIFFS
Store KNX telegram history:

```yaml
# Store last 1000 telegrams
logger:
  level: DEBUG
  logs:
    knx_tp: DEBUG
  baud_rate: 0  # Disable serial, only file
  file: /spiffs/knx.log
  max_size: 1048576  # 1MB

# Access via: http://knx-gateway-s3.local/logs
```

### Home Assistant Integration
Auto-discovered via ESPHome API:

```yaml
# Home Assistant configuration.yaml (auto-added)
esphome:
  - host: knx-gateway-s3.local
    encryption_key: !secret knx_gateway_api_key
```

All entities appear in Home Assistant automatically.

### KNX Telegram Scripting
React to specific KNX telegrams:

```yaml
# Example: Turn on LED when motion detected
esphome:
  on_boot:
    then:
      - lambda: |-
          id(knx_tp_bus).register_telegram_callback(
            [](std::string ga, std::vector<uint8_t> data) {
              if (ga == "3/0/1" && data.size() > 0 && data[0] == 1) {
                id(rgb_light).turn_on();
              }
            }
          );
```

## Comparison: ESP32 vs ESP32-S3

| Feature | ESP32 | ESP32-S3 N16R8 |
|---------|-------|----------------|
| CPU | Dual-core Xtensa LX6 @ 240MHz | Dual-core Xtensa LX7 @ 240MHz |
| SRAM | 520 KB | 512 KB |
| Flash (typical) | 4 MB | **16 MB** ✅ |
| PSRAM (typical) | 0-4 MB | **8 MB** ✅ |
| USB | External chip | **Native USB-OTG** ✅ |
| WiFi | 802.11 b/g/n | 802.11 b/g/n |
| Bluetooth | BT Classic + BLE | **BLE 5.0** ✅ |
| Security | RSA, AES | **RSA, AES, SHA, HMAC** ✅ |
| GPIO | 34 | **45** ✅ |
| AI Acceleration | No | **Vector extensions** ✅ |
| Price | $4-8 | $8-15 |

**Verdict**: ESP32-S3 N16R8 is ideal for KNX gateway with web UI and extensive logging.

## Future Enhancements

With 16MB flash and 8MB PSRAM, you have room for:

- ✅ **KNX Secure** implementation (when library supports it)
- ✅ **Telegram logging** (store weeks of history in SPIFFS)
- ✅ **Web-based ETS import** (upload .knxproj files)
- ✅ **Grafana dashboard** (metrics via Prometheus)
- ✅ **MQTT bridge** (KNX ↔ MQTT gateway)
- ✅ **BACnet gateway** (KNX ↔ BACnet translator)
- ✅ **Modbus gateway** (KNX ↔ Modbus RTU/TCP)
- ✅ **Lua scripting** (custom telegram logic without recompiling)

## Support and Resources

- **ESPHome Docs**: https://esphome.io
- **KNX Specification**: https://www.knx.org
- **Thelsing KNX Library**: https://github.com/thelsing/knx
- **ESP32-S3 Datasheet**: https://www.espressif.com/en/products/socs/esp32-s3
- **Issues/PRs**: https://github.com/yourusername/esphome-knx

## License

MIT License - See LICENSE file for details.

---

**Configuration**: `knx-esp32s3-full.yaml`
**Target**: ESP32-S3-N16R8 (16MB Flash, 8MB PSRAM)
**ESPHome**: 2024.10.0+
**Last Updated**: 2025-10-23
