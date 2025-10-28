# Security Analysis Report
**Project:** ESPHome KNX Component
**Date:** 2025-10-22
**Analyzed by:** Claude Code Security Audit

---

## Executive Summary

This report identifies security vulnerabilities found in the KNX-TP and KNX-IP components for ESPHome. The analysis covers:
- Buffer overflow vulnerabilities
- Input validation issues
- Memory safety problems
- Integer overflow risks
- Credential exposure
- Network security concerns

**Overall Risk Level:** 🟠 **MEDIUM-HIGH**

---

## 🔴 CRITICAL Vulnerabilities

### 1. **Hardcoded Credentials in Version Control**
**Severity:** CRITICAL
**CWE:** CWE-798 (Use of Hard-coded Credentials)
**Files Affected:**
- `ciola.yaml:23` - WiFi password: "MySecurePass123!"
- `ciola.yaml:28` - Fallback AP password: "Fallback2024"
- `examples/knx-example-simple.yaml:23` - WiFi password
- `examples/knx-example-simple.yaml:28` - AP password

**Impact:**
- Credentials exposed in public repository
- Anyone can access WiFi network
- Fallback AP can be exploited

**Remediation:**
```yaml
# Replace with:
wifi:
  ssid: !secret wifi_ssid
  password: !secret wifi_password
```

---

## 🟠 HIGH Severity Vulnerabilities

### 2. **Uncaught Exception in String Parsing**
**Severity:** HIGH
**CWE:** CWE-248 (Uncaught Exception)
**Files Affected:**
- `components/knx_tp/knx_tp.cpp:310-312`
- `components/knx_ip/knx_ip.cpp:257-259`
- `components/knx_ip/knx_ip.cpp:282-284`

**Vulnerable Code:**
```cpp
// knx_tp.cpp:310
int area = std::stoi(addr.substr(0, first_dot));
int line = std::stoi(addr.substr(first_dot + 1, second_dot - first_dot - 1));
int device = std::stoi(addr.substr(second_dot + 1));
```

**Impact:**
- `std::stoi()` throws `std::invalid_argument` and `std::out_of_range`
- Unhandled exceptions can crash the ESP32
- DoS attack vector if malicious input provided

**Remediation:**
```cpp
try {
  int area = std::stoi(addr.substr(0, first_dot));
  int line = std::stoi(addr.substr(first_dot + 1, second_dot - first_dot - 1));
  int device = std::stoi(addr.substr(second_dot + 1));
} catch (const std::invalid_argument& e) {
  ESP_LOGE(TAG, "Invalid address format: %s", address.c_str());
  return 0;
} catch (const std::out_of_range& e) {
  ESP_LOGE(TAG, "Address out of range: %s", address.c_str());
  return 0;
}
```

### 3. **Buffer Access Without Bounds Checking**
**Severity:** HIGH
**CWE:** CWE-125 (Out-of-bounds Read)
**Files Affected:**
- `components/knx_tp/dpt.cpp` (multiple locations)
- `components/knx_ip/dpt.cpp` (multiple locations)

**Vulnerable Code:**
```cpp
// dpt.cpp:56-59
float DPT::decode_dpt9(const std::vector<uint8_t> &data) {
  if (data.size() < 2) return 0.0f;

  uint16_t raw = (data[0] << 8) | data[1];  // ✅ Safe: checked size >= 2
}

// dpt.cpp:10-12
bool DPT::decode_dpt1(const std::vector<uint8_t> &data) {
  if (data.empty()) return false;
  return (data[0] & 0x01) != 0;  // ✅ Safe: checked not empty
}
```

**Analysis:**
Most accesses are safe because they check `data.size()` first. However, there are potential issues in complex functions:

```cpp
// dpt.cpp:260-293 - DPT 19 (DateTime)
if (data.size() < 8) return dt;

dt.year = (data[0] << 8) | data[1];      // Safe: checked size >= 8
dt.month = data[2];                      // Safe
dt.day = data[3];                        // Safe
dt.day_of_week = (data[4] >> 5) & 0x07;  // Safe
// ... continues accessing data[5], data[6], data[7]
```

**Status:** ✅ **Generally Safe** - All observed accesses have proper bounds checking.

---

## 🟡 MEDIUM Severity Vulnerabilities

### 4. **Memory Leak - Unmatched new/delete**
**Severity:** MEDIUM
**CWE:** CWE-401 (Missing Release of Memory after Effective Lifetime)
**Files Affected:**
- `components/knx_tp/knx_tp.cpp:45-48`
- `components/knx_ip/knx_ip.cpp:25-28`

**Vulnerable Code:**
```cpp
// knx_tp.cpp:45
this->platform_ = new Esp32IdfPlatform(UART_NUM_1);

// knx_tp.cpp:48
this->bau_ = new Bau07B0(*this->platform_);
```

**Impact:**
- Memory allocated with `new` but never freed with `delete`
- On ESP32 reboot cycles, memory can accumulate
- ESPHome components typically live for device lifetime, so impact is minimal

**Remediation:**
```cpp
class KNXTPComponent : public Component {
 public:
  ~KNXTPComponent() {
    if (bau_) delete bau_;
    if (platform_) delete platform_;
  }

 private:
  Bau07B0 *bau_{nullptr};
  Esp32IdfPlatform *platform_{nullptr};
};
```

**OR** use smart pointers:
```cpp
std::unique_ptr<Esp32IdfPlatform> platform_;
std::unique_ptr<Bau07B0> bau_;
```

### 5. **Integer Overflow in Shift Operations**
**Severity:** MEDIUM
**CWE:** CWE-190 (Integer Overflow)
**Files Affected:**
- `components/knx_tp/knx_tp.cpp:317`
- `components/knx_ip/knx_ip.cpp:262`

**Vulnerable Code:**
```cpp
// knx_tp.cpp:317
uint16_t encoded = ((area & 0x1F) << 11) | ((line & 0x07) << 8) | (device & 0xFF);
```

**Analysis:**
Code uses masking (`& 0x1F`, `& 0x07`, `& 0xFF`) before shifting, which prevents overflow.

**Status:** ✅ **Safe** - Proper masking prevents integer overflow.

### 6. **No Input Validation on Group Address Values**
**Severity:** MEDIUM
**CWE:** CWE-20 (Improper Input Validation)
**Files Affected:**
- `components/knx_tp/__init__.py:35-50`
- `components/knx_ip/__init__.py:33-50`

**Vulnerable Code:**
```python
def validate_knx_address(value):
    value = cv.string(value)
    value = value.replace("/", ".")
    parts = value.split(".")
    if len(parts) != 3:
        raise cv.Invalid("KNX address must have 3 parts (e.g., 1.2.3)")
    try:
        main = int(parts[0])
        middle = int(parts[1])
        sub = int(parts[2])
        if not (0 <= main <= 31 and 0 <= middle <= 7 and 0 <= sub <= 255):
            raise ValueError
    except ValueError:
        raise cv.Invalid("Invalid KNX address format")
    return f"{main}.{middle}.{sub}"
```

**Status:** ✅ **Safe** - Python validation properly checks ranges.

### 7. **No Network Encryption for KNX/IP**
**Severity:** MEDIUM
**CWE:** CWE-311 (Missing Encryption of Sensitive Data)
**Files Affected:**
- `components/knx_ip/knx_ip.cpp`

**Impact:**
- KNX/IP traffic sent unencrypted over network
- Multicast address 224.0.23.12:3671 is standard but unencrypted
- Anyone on same network can sniff KNX commands

**Note:** This is a limitation of the KNX/IP protocol itself, not a coding error.

**Mitigation:**
- Use VLANs to segregate KNX traffic
- Enable KNX Secure (if supported by devices)
- Use tunneling mode over VPN

---

## 🟢 LOW Severity Issues

### 8. **Format String Safety**
**Severity:** LOW (Informational)
**Status:** ✅ **Safe**

All `snprintf` calls use proper bounds:
```cpp
char data_hex[97];
offset += snprintf(data_hex + offset, sizeof(data_hex) - offset, "%02X ", data[i]);
```

### 9. **Null Pointer Checks**
**Severity:** LOW (Informational)
**Status:** ✅ **Good**

Code consistently checks for `nullptr` before dereferencing:
```cpp
if (this->knx_ == nullptr) {
  ESP_LOGE(TAG, "KNX component is nullptr!");
  return;
}
```

---

## 📊 Summary Table

| Vulnerability | Severity | CWE | Status | Files Affected |
|---------------|----------|-----|--------|----------------|
| Hardcoded Credentials | 🔴 CRITICAL | CWE-798 | ❌ UNFIXED | ciola.yaml, knx-example-simple.yaml |
| Uncaught std::stoi Exception | 🟠 HIGH | CWE-248 | ❌ UNFIXED | knx_tp.cpp:310, knx_ip.cpp:257 |
| Memory Leak (new/delete) | 🟡 MEDIUM | CWE-401 | ❌ UNFIXED | knx_tp.cpp:45-48, knx_ip.cpp:25-28 |
| Buffer Access | 🟡 MEDIUM | CWE-125 | ✅ SAFE | dpt.cpp (multiple) |
| Integer Overflow | 🟡 MEDIUM | CWE-190 | ✅ SAFE | knx_tp.cpp:317 |
| No KNX/IP Encryption | 🟡 MEDIUM | CWE-311 | ⚠️ PROTOCOL | knx_ip.cpp |
| Format Strings | 🟢 LOW | - | ✅ SAFE | All snprintf calls |
| Null Checks | 🟢 LOW | - | ✅ GOOD | All components |

---

## 🛡️ Recommendations

### Immediate Actions (Priority 1)
1. ✅ **Remove hardcoded credentials from all YAML files**
2. ✅ **Add try-catch blocks around all std::stoi() calls**
3. ✅ **Add destructors to free allocated memory**

### Short-term (Priority 2)
4. ⚠️ Consider using smart pointers instead of raw pointers
5. ⚠️ Add fuzzing tests for DPT parsing functions
6. ⚠️ Document network security limitations

### Long-term (Priority 3)
7. 📋 Investigate KNX Secure protocol support
8. 📋 Add rate limiting for incoming telegrams
9. 📋 Implement authentication for configuration changes

---

## ✅ Good Security Practices Found

1. **Input Validation:** Python validators check address ranges
2. **Bounds Checking:** Most buffer accesses check size first
3. **Safe String Formatting:** Uses `snprintf` with size limits
4. **Null Pointer Checks:** Consistent nullptr validation
5. **Secrets Management:** Uses ESPHome secrets system
6. **Git Ignore:** secrets.yaml properly excluded from git

---

## 📝 Testing Recommendations

### Unit Tests Needed:
```cpp
// Test malformed addresses
TEST(KNXSecurity, MalformedAddress) {
  EXPECT_NO_THROW(address_to_int_("invalid"));
  EXPECT_NO_THROW(address_to_int_("999999.999.999"));
}

// Test buffer overflow
TEST(DPTSecurity, SmallBuffer) {
  std::vector<uint8_t> data = {0x01};
  EXPECT_NO_THROW(decode_dpt9(data));  // Should return 0.0f
}
```

### Fuzzing:
```bash
# Use AFL or libFuzzer to fuzz DPT parsing
afl-fuzz -i input_corpus -o findings ./fuzz_dpt
```

---

## 🔗 References

- [OWASP IoT Security](https://owasp.org/www-project-internet-of-things/)
- [CWE Top 25](https://cwe.mitre.org/top25/)
- [KNX Security](https://www.knx.org/knx-en/for-professionals/What-is-KNX/KNX-Secure/)

---

**Report Generated:** 2025-10-22
**Tool:** Claude Code Security Audit v1.0
