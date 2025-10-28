# ESP32 Board Comparison for KNX Projects

Quick reference guide to choose the right ESP32 board for your KNX project.

## Board Variants Comparison

| Model | Flash | PSRAM | Price | Best For | Config File |
|-------|-------|-------|-------|----------|-------------|
| **ESP32-WROOM-32** | 4 MB | 0 MB | $4 | Basic projects, single KNX stack | `knx-example-simple.yaml` |
| **ESP32-WROVER** | 4 MB | 4 MB | $6 | Advanced features, web UI | `knx-example-advanced.yaml` |
| **ESP32-S3-N8** | 8 MB | 0 MB | $7 | Dual KNX stack (TP+IP) | Custom config |
| **ESP32-S3-N16R8** | **16 MB** | **8 MB** | $12 | **Full featured gateway** | **`knx-esp32s3-full.yaml`** ✅ |
| **ESP32-C3** | 4 MB | 0 MB | $3 | Low-cost, single-core | Not recommended |
| **ESP32-C6** | 4 MB | 0 MB | $5 | WiFi 6, matter support | Future support |

## Feature Support Matrix

| Feature | WROOM-32 (4MB) | WROVER (4MB+4MB) | S3-N8 (8MB) | S3-N16R8 (16MB+8MB) |
|---------|----------------|------------------|-------------|---------------------|
| **KNX-TP** | ✅ | ✅ | ✅ | ✅ |
| **KNX-IP** | ✅ | ✅ | ✅ | ✅ |
| **Both TP+IP** | ❌ Low RAM | ⚠️ Tight | ✅ | ✅ **Best** |
| **Web Server** | ⚠️ Basic | ✅ v2 | ✅ v2 | ✅ **v3 Full** |
| **OTA Updates** | ✅ | ✅ | ✅ | ✅ |
| **Logging** | ⚠️ Minimal | ✅ Normal | ✅ Detailed | ✅ **Extensive** |
| **Data Logging** | ❌ | ⚠️ Limited | ✅ | ✅ **Weeks** |
| **Entities** | 5-10 | 10-20 | 20-30 | **50+** ✅ |
| **Native USB** | ❌ | ❌ | ✅ | ✅ |
| **Future Secure** | ❌ | ❌ | ⚠️ Maybe | ✅ **Yes** |

## Resource Usage by Configuration

### ESP32-WROOM-32 (4MB Flash, 320KB RAM)
```
Configuration: knx-example-simple.yaml
----------------------------------------
Flash:  1.8 MB / 4 MB   (45%) ✅
RAM:    60 KB / 320 KB  (18%) ✅

Features:
  - KNX-TP only
  - Basic entities (3-5 sensors/switches)
  - Minimal web UI
  - Basic logging
  - No data storage

Use Cases:
  ✅ Single room automation
  ✅ Sensor gateway (temperature, motion)
  ✅ Simple switch/relay control
  ❌ Complex logic
  ❌ Dual TP+IP stack
```

### ESP32-WROVER (4MB Flash + 4MB PSRAM, 320KB RAM)
```
Configuration: knx-example-advanced.yaml
----------------------------------------
Flash:  2.3 MB / 4 MB   (57%) ✅
RAM:    85 KB / 320 KB  (26%) ✅
PSRAM:  100 KB / 4 MB   (2%)  ✅

Features:
  - KNX-TP or KNX-IP (not both)
  - Advanced entities (10-15)
  - Web server v2
  - Normal logging
  - Some data storage

Use Cases:
  ✅ Multi-room automation
  ✅ Climate control
  ✅ Dimming, RGB lights
  ✅ Cover/blinds control
  ⚠️ Dual stack (tight fit)
```

### ESP32-S3-N8 (8MB Flash, 512KB RAM)
```
Configuration: Custom (modify knx-esp32s3-full.yaml)
----------------------------------------------------
Flash:  3.0 MB / 8 MB   (37%) ✅
RAM:    150 KB / 512 KB (29%) ✅

Features:
  - Both KNX-TP and KNX-IP
  - Many entities (20-25)
  - Web server v2
  - Good logging
  - Data storage (few days)

Use Cases:
  ✅ TP ↔ IP router/gateway
  ✅ Complex automation
  ✅ Multiple rooms
  ✅ Integration hub
  ⚠️ Long-term logging (limited)
```

### ESP32-S3-N16R8 (16MB Flash + 8MB PSRAM, 512KB RAM) ⭐ RECOMMENDED
```
Configuration: knx-esp32s3-full.yaml
----------------------------------------------------
Flash:  3.8 MB / 16 MB  (23%) ✅✅✅ Plenty of room!
RAM:    200 KB / 512 KB (39%) ✅
PSRAM:  100 KB / 8 MB   (1%)  ✅✅✅ Huge headroom!

Features:
  - Both KNX-TP and KNX-IP simultaneously
  - Unlimited entities (50+)
  - Web server v3 (full featured)
  - Extensive logging
  - Data storage (weeks/months)
  - Room for future features

Use Cases:
  ✅ Professional KNX gateway
  ✅ TP ↔ IP router with logging
  ✅ Building automation hub
  ✅ Integration with Home Assistant
  ✅ Data analytics and monitoring
  ✅ Future KNX Secure support
  ✅ Development and testing
```

## Choosing the Right Board

### Decision Tree

```
START: What's your project?

┌─────────────────────────────────────────┐
│ Do you need both KNX-TP and KNX-IP?     │
└────────┬─────────────────────────┬──────┘
         NO                        YES
         │                          │
         v                          v
    ┌────────┐              ┌──────────────┐
    │ Budget │              │ ESP32-S3-N16 │ ⭐
    │  < $5? │              │   (16MB)     │
    └─┬────┬─┘              └──────────────┘
      NO  YES
      │    │
      v    v
   ┌────┐ ┌─────┐
   │WROVER│ │WROOM│
   │ 4MB │ │ 4MB │
   └────┘ └─────┘
```

### By Use Case

**1. Simple Sensor Gateway** ($4)
- **Board**: ESP32-WROOM-32 (4MB)
- **Config**: `knx-example-simple.yaml`
- **Entities**: 3-5 sensors
- **Example**: Temperature + humidity → KNX

**2. Room Controller** ($6)
- **Board**: ESP32-WROVER (4MB + 4MB PSRAM)
- **Config**: `knx-example-advanced.yaml`
- **Entities**: 10-15 (lights, switches, sensors, climate)
- **Example**: Complete room automation

**3. Multi-Room Gateway** ($8)
- **Board**: ESP32-S3-N8 (8MB)
- **Config**: Modified `knx-esp32s3-full.yaml`
- **Entities**: 20-30
- **Example**: Floor controller with multiple rooms

**4. Professional Gateway** ($12) ⭐ BEST VALUE
- **Board**: ESP32-S3-N16R8 (16MB + 8MB PSRAM)
- **Config**: `knx-esp32s3-full.yaml`
- **Entities**: Unlimited
- **Example**: Building automation hub, TP↔IP router, data logger

**5. Development/Testing** ($12)
- **Board**: ESP32-S3-N16R8
- **Config**: `knx-esp32s3-full.yaml`
- **Why**: Room for extensive debugging, all features enabled

## Recommended Boards to Buy

### Budget Option: ESP32-DevKitC (WROOM-32)
```
Price: $4-6
Flash: 4MB
PSRAM: None
USB: CH340 (external chip)

Where to buy:
- AliExpress: ~$4 (search "ESP32 DevKitC")
- Amazon: ~$6
- Mouser: ~$8 (official)

Model Numbers:
- ESP32-DevKitC-32E (newer, recommended)
- ESP32-DevKitC-V4 (older, works fine)
```

### Mid-Range: ESP32-WROVER-KIT
```
Price: $6-8
Flash: 4MB
PSRAM: 4MB (or 8MB variant)
USB: CH340 or CP2102

Where to buy:
- AliExpress: ~$6 (search "ESP32 WROVER")
- Amazon: ~$8

Model Numbers:
- ESP32-WROVER-B (4MB PSRAM)
- ESP32-WROVER-E (8MB PSRAM) - Preferred
```

### Professional: ESP32-S3-DevKitC-1-N16R8 ⭐ RECOMMENDED
```
Price: $10-15
Flash: 16MB
PSRAM: 8MB (Octal SPI)
USB: Native USB-OTG (no external chip)

Where to buy:
- AliExpress: ~$10 (search "ESP32-S3 N16R8")
- Amazon: ~$12
- Mouser: ~$15 (official Espressif)
- DigiKey: ~$15

Model Numbers:
- ESP32-S3-DevKitC-1-N16R8 (official)
- ESP32-S3-WROOM-1-N16R8 (module only)

Look for: "16MB Flash" or "N16R8" in product name
```

## Hardware Differences Summary

### WROOM vs WROVER
```
WROOM (Budget):
  - No PSRAM
  - Lower price
  - Sufficient for simple projects

WROVER (Better):
  - 4-8MB PSRAM
  - Better for web UI and logging
  - More expensive
  - Worth the extra $2
```

### ESP32 vs ESP32-S3
```
ESP32 (Classic):
  - Older architecture (LX6)
  - More mature, stable
  - Cheaper
  - No native USB

ESP32-S3 (Modern):
  - Newer architecture (LX7)
  - Better performance
  - Native USB (no CH340 chip needed)
  - More GPIO pins (45 vs 34)
  - Better security features
  - Recommended for new projects
```

### Flash Size Impact
```
4MB Flash:
  - One KNX stack (TP or IP)
  - Basic web UI
  - Limited logging
  - Standard OTA (2x2MB partitions)

8MB Flash:
  - Both KNX stacks (TP + IP)
  - Better web UI
  - Normal logging
  - Larger OTA partitions

16MB Flash:
  - Both KNX stacks
  - Full web UI
  - Extensive logging
  - Data storage (SPIFFS)
  - Future expansion
  - BEST CHOICE for serious projects
```

## Price-to-Performance Ratio

| Board | Price | Performance Score | Value Rating |
|-------|-------|-------------------|--------------|
| ESP32-WROOM | $4 | 5/10 | ⭐⭐⭐ Good for basic |
| ESP32-WROVER | $6 | 6/10 | ⭐⭐⭐ Decent value |
| ESP32-S3-N8 | $8 | 7/10 | ⭐⭐⭐⭐ Good balance |
| **ESP32-S3-N16R8** | **$12** | **10/10** | **⭐⭐⭐⭐⭐ Best value!** |

**Recommendation**: Spend the extra $6-8 for ESP32-S3-N16R8 if:
- You want future-proof solution
- Need both TP and IP
- Want data logging
- Plan to add features later
- Building a professional product

## FAQ

### Q: Can I use ESP32-C3 or ESP32-C6?
**A**: Not recommended yet. These are single-core RISC-V chips with less RAM. While cheaper ($3-5), they struggle with dual KNX stack. Wait for official support.

### Q: Is 16MB overkill for my project?
**A**: If you're only controlling a few lights, yes. But:
- Price difference is only $6-8 vs basic ESP32
- Future-proofs your project
- Allows easy expansion
- Better resale value if selling product
- Development is easier with more space

**Our take**: Buy the 16MB version unless cost is critical.

### Q: Do I need external PSRAM?
**A**: Depends on features:
- No PSRAM: 3-5 entities, basic logging
- 4MB PSRAM: 10-15 entities, web UI
- 8MB PSRAM: 50+ entities, extensive features

**Our take**: Get PSRAM if using web server or many entities.

### Q: USB-UART chip doesn't matter, right?
**A**: For production, native USB (ESP32-S3) is better:
- One less component to fail
- Faster data transfer
- Lower power consumption
- Cheaper BOM cost
- USB-OTG host mode possible

For hobby projects, CH340/CP2102 work fine.

### Q: Where can I get the cheapest boards?
**A**: AliExpress ($4-10) but:
- 2-4 week shipping
- Sometimes clones/fakes
- Less reliable support

Amazon ($6-15):
- 1-2 day shipping
- Easy returns
- Slightly higher price
- More reliable

Official distributors (Mouser, DigiKey) $8-18:
- Genuine Espressif chips
- Best quality
- Technical support
- Fastest shipping (if in stock)

**Our take**: For prototypes, AliExpress is fine. For production, use official distributors.

---

## Quick Start Table

| Your Situation | Buy This | Use This Config | Expected Cost |
|----------------|----------|-----------------|---------------|
| Learning KNX | ESP32-WROOM | `knx-example-simple.yaml` | $4-6 |
| Home project | ESP32-WROVER | `knx-example-advanced.yaml` | $6-8 |
| Professional | ESP32-S3-N16R8 | `knx-esp32s3-full.yaml` | $10-15 |
| Development | ESP32-S3-N16R8 | `knx-esp32s3-full.yaml` | $10-15 |
| Production | ESP32-S3-N16R8 | Custom config | $8-12 (bulk) |

---

**Last Updated**: 2025-10-23
**Recommended Board**: ESP32-S3-DevKitC-1-N16R8 (16MB + 8MB PSRAM)
**Best Value**: ESP32-S3-N16R8 @ $10-12
