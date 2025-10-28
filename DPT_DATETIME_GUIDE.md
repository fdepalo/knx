# KNX Date/Time DPT Implementation Guide

## DPT Implementati

### ✅ DPT 10.001 - Time of Day (Ora del giorno)
**Formato**: 3 bytes
**Contenuto**:
- Giorno della settimana (0=nessun giorno, 1=Lunedì...7=Domenica)
- Ora (0-23)
- Minuto (0-59)
- Secondo (0-59)

**Struttura C++**:
```cpp
struct TimeOfDay {
  uint8_t day_of_week;  // 0-7
  uint8_t hour;         // 0-23
  uint8_t minute;       // 0-59
  uint8_t second;       // 0-59
};
```

**Encoding KNX**:
- Byte 0: bits 7-5 = day_of_week, bits 4-0 = hour
- Byte 1: bits 5-0 = minute
- Byte 2: bits 5-0 = second

---

### ✅ DPT 11.001 - Date (Data)
**Formato**: 3 bytes
**Contenuto**:
- Giorno (1-31)
- Mese (1-12)
- Anno (1990-2089)

**Struttura C++**:
```cpp
struct Date {
  uint8_t day;    // 1-31
  uint8_t month;  // 1-12
  uint16_t year;  // Anno completo (es. 2024)
};
```

**Encoding KNX**:
- Byte 0: day (1-31)
- Byte 1: month (1-12)
- Byte 2: year codificato
  - 0-89 = anni 2000-2089
  - 90-99 = anni 1990-1999

---

### ✅ DPT 19.001 - Date and Time (Data e Ora completa)
**Formato**: 8 bytes
**Contenuto**:
- Anno (1990-2089)
- Mese (1-12)
- Giorno (1-31)
- Giorno della settimana (0-7)
- Ora (0-23)
- Minuto (0-59)
- Secondo (0-59)
- Flag di qualità e fault

**Struttura C++**:
```cpp
struct DateTime {
  uint16_t year;        // 1990-2089
  uint8_t month;        // 1-12
  uint8_t day;          // 1-31
  uint8_t day_of_week;  // 0-7
  uint8_t hour;         // 0-23
  uint8_t minute;       // 0-59
  uint8_t second;       // 0-59
  bool fault;           // Flag errore
  bool working_day;     // Giorno lavorativo
  bool no_wd;           // No working day valid
  bool no_year;         // Anno non valido
  bool no_date;         // Data non valida
  bool no_dow;          // Giorno settimana non valido
  bool no_time;         // Ora non valida
  bool summer_time;     // Ora legale
  uint8_t quality;      // Qualità clock (0=ok, 1=fault)
};
```

**Encoding KNX**:
- Bytes 0-1: year (big-endian)
- Byte 2: month
- Byte 3: day
- Byte 4: bits 7-5 = day_of_week, bits 4-0 = hour
- Byte 5: minute
- Byte 6: second
- Byte 7: fault/quality flags

---

## Utilizzo nel Codice C++

### Decodifica (Ricezione da KNX)

```cpp
#include "dpt.h"

// DPT 10 - Time of Day
std::vector<uint8_t> knx_data = {0x45, 0x1E, 0x0F};  // Lun 5:30:15
DPT::TimeOfDay time = DPT::decode_dpt10(knx_data);
ESP_LOGI(TAG, "Time: %02d:%02d:%02d (DoW: %d)",
  time.hour, time.minute, time.second, time.day_of_week);

// DPT 11 - Date
std::vector<uint8_t> date_data = {20, 10, 24};  // 20/10/2024
DPT::Date date = DPT::decode_dpt11(date_data);
ESP_LOGI(TAG, "Date: %02d/%02d/%04d",
  date.day, date.month, date.year);

// DPT 19 - Date and Time
std::vector<uint8_t> datetime_data = {0x07, 0xE8, 10, 20, 0x45, 0x1E, 0x0F, 0x00};
DPT::DateTime dt = DPT::decode_dpt19(datetime_data);
ESP_LOGI(TAG, "DateTime: %04d-%02d-%02d %02d:%02d:%02d",
  dt.year, dt.month, dt.day, dt.hour, dt.minute, dt.second);
```

### Encoding (Invio su KNX)

```cpp
// DPT 10 - Invia ora corrente
DPT::TimeOfDay time = {1, 14, 30, 45};  // Lunedì 14:30:45
std::vector<uint8_t> time_telegram = DPT::encode_dpt10(time);
knx->send_group_write("time_ga", time_telegram);

// DPT 11 - Invia data
DPT::Date date = {20, 10, 2024};  // 20 ottobre 2024
std::vector<uint8_t> date_telegram = DPT::encode_dpt11(date);
knx->send_group_write("date_ga", date_telegram);

// DPT 19 - Invia data/ora completa
DPT::DateTime dt = {
  .year = 2024,
  .month = 10,
  .day = 20,
  .day_of_week = 1,  // Lunedì
  .hour = 14,
  .minute = 30,
  .second = 45,
  .fault = false,
  .working_day = true,
  .no_wd = false,
  .no_year = false,
  .no_date = false,
  .no_dow = false,
  .no_time = false,
  .summer_time = true,
  .quality = 0
};
std::vector<uint8_t> dt_telegram = DPT::encode_dpt19(dt);
knx->send_group_write("datetime_ga", dt_telegram);
```

---

## Utilizzo in ESPHome con Text Sensor

Per ricevere date/ora da KNX e visualizzarle in Home Assistant, puoi usare un componente custom o estendere text_sensor:

### Esempio: Ricezione Ora da KNX

```yaml
# YAML Configuration
text_sensor:
  - platform: knx_tp
    name: "Clock Time"
    state_ga: knx_clock_time
    # Il sensore riceverà i dati raw,
    # dovrai formattarli nel componente C++
```

### Esempio: Sincronizzazione Orologio KNX

```cpp
// In un componente ESPHome custom
#include "esphome/core/time.h"

void sync_knx_clock() {
  auto time = id(homeassistant_time).now();

  // Crea struttura DPT 19
  DPT::DateTime dt = {
    .year = (uint16_t)time.year,
    .month = (uint8_t)time.month,
    .day = (uint8_t)time.day_of_month,
    .day_of_week = (uint8_t)time.day_of_week,
    .hour = (uint8_t)time.hour,
    .minute = (uint8_t)time.minute,
    .second = (uint8_t)time.second,
    .fault = false,
    .working_day = time.day_of_week >= 1 && time.day_of_week <= 5,
    .no_wd = false,
    .no_year = false,
    .no_date = false,
    .no_dow = false,
    .no_time = false,
    .summer_time = time.is_dst,
    .quality = 0
  };

  // Invia su KNX
  auto telegram = DPT::encode_dpt19(dt);
  id(knx_component).send_group_write("clock_datetime", telegram);
}
```

---

## Esempi Pratici

### 1. Master Clock KNX
Invia l'ora corrente ogni minuto sul bus KNX:

```cpp
void loop() {
  static uint32_t last_sync = 0;
  if (millis() - last_sync > 60000) {  // Ogni minuto
    sync_knx_clock();
    last_sync = millis();
  }
}
```

### 2. Display Orologio KNX
Ricevi l'ora da un master clock KNX e visualizzala:

```cpp
void on_knx_telegram(const std::string &ga, const std::vector<uint8_t> &data) {
  if (ga == "1/0/0") {  // Clock group address
    auto dt = DPT::decode_dpt19(data);

    char buffer[32];
    sprintf(buffer, "%02d:%02d:%02d", dt.hour, dt.minute, dt.second);
    publish_state(buffer);

    ESP_LOGI(TAG, "Clock updated: %s", buffer);
  }
}
```

### 3. Programmazione Oraria
Attiva/disattiva dispositivi in base all'ora:

```cpp
void check_schedule(const DPT::TimeOfDay &time) {
  // Accendi luci alle 18:00
  if (time.hour == 18 && time.minute == 0) {
    id(evening_lights).turn_on();
  }

  // Spegni luci alle 23:00
  if (time.hour == 23 && time.minute == 0) {
    id(evening_lights).turn_off();
  }
}
```

---

## Validazione e Limiti

Tutti i metodi di encoding includono validazione automatica:

- **Time of Day**: Ore clamped 0-23, minuti/secondi 0-59
- **Date**: Giorno 1-31, mese 1-12, anno 1990-2089
- **Date and Time**: Tutti i campi validati, flags booleani gestiti correttamente

## Note Implementative

1. **Giorno della settimana**: 0=nessun giorno, 1=Lunedì, 7=Domenica
2. **Anno**: Rappresentazione completa a 4 cifre in C++, encoding compatto in KNX
3. **Flags**: DPT 19 supporta 8 flag di qualità/validità
4. **Endianness**: Anno in DPT 19 è big-endian (MSB first)

---

## Test Unitari

Per testare le funzioni:

```cpp
// Test encoding/decoding round-trip
DPT::TimeOfDay original_time = {1, 14, 30, 45};
auto encoded = DPT::encode_dpt10(original_time);
auto decoded = DPT::decode_dpt10(encoded);

assert(decoded.day_of_week == original_time.day_of_week);
assert(decoded.hour == original_time.hour);
assert(decoded.minute == original_time.minute);
assert(decoded.second == original_time.second);
```

---

## Documentazione KNX

Per maggiori dettagli sui DPT, consulta:
- KNX Standard: DPT 10.001, DPT 11.001, DPT 19.001
- KNX System Specifications Volume 3 Part 7 Section 2
