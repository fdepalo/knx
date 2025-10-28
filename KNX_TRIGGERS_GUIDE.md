# KNX Triggers Guide

Questa guida spiega come usare i trigger `on_telegram` e `on_group_address` nel componente KNX TP per ESPHome.

## Indice

- [Panoramica](#panoramica)
- [on_telegram - Trigger Generico](#on_telegram---trigger-generico)
- [on_group_address - Trigger Specifico](#on_group_address---trigger-specifico)
- [Confronto e Performance](#confronto-e-performance)
- [Disabilitare i Trigger](#disabilitare-i-trigger)
- [Helper DPT per Trigger](#helper-dpt-per-trigger-)
- [Esempi Pratici](#esempi-pratici)

## Panoramica

Il componente KNX TP supporta due tipi di trigger per eseguire azioni quando arrivano telegrammi KNX:

1. **`on_telegram`** - Trigger generico chiamato per TUTTI i telegrammi ricevuti
2. **`on_group_address`** - Trigger specifici chiamati solo per group address specifici

Entrambi i trigger possono essere disabilitati singolarmente tramite compile flags per ottimizzare memoria e performance.

## on_telegram - Trigger Generico

### Descrizione

Il trigger `on_telegram` viene chiamato per **ogni telegramma KNX** ricevuto sul bus, indipendentemente dal group address.

### Configurazione

```yaml
knx_tp:
  physical_address: "1.1.100"
  uart_id: knx_uart

  on_telegram:
    - logger.log:
        format: "Ricevuto GA: %s, Dati: %d bytes"
        args: ['group_address.c_str()', 'data.size()']

    - lambda: |-
        ESP_LOGI("knx", "GA=%s", group_address.c_str());

        // Filtra per group address specifico
        if (group_address == "0/0/1") {
          // Fai qualcosa
        }
```

### Variabili Disponibili

- `group_address` (string) - Group address del telegramma (es. "0/0/1")
- `data` (vector<uint8_t>) - Dati payload del telegramma

### Quando Usarlo

✅ **Usalo per:**
- Debug e monitoring di tutti i telegrammi KNX
- Logging centralizzato
- Packet sniffing KNX
- Statistiche sul traffico KNX

❌ **NON usarlo per:**
- Logica applicativa normale (usa `on_group_address` invece)
- Bus KNX molto trafficati (>100 telegrammi/sec)
- Dispositivi con poca RAM

### Impatto Performance

| Metrica | Valore |
|---------|--------|
| RAM aggiunta | ~80 bytes per trigger |
| CPU overhead | ~75 µs per telegramma |
| Scalabilità | Peggiora con traffico KNX |

**Esempio:** Con 200 telegrammi/sec → ~1.5% CPU utilizzato

## on_group_address - Trigger Specifico

### Descrizione

Il trigger `on_group_address` viene chiamato **solo** quando arriva un telegramma sul group address specificato. Usa una hash map per lookup O(1).

### Configurazione

```yaml
knx_tp:
  physical_address: "1.1.100"
  uart_id: knx_uart

  group_addresses:
    - id: luce_soggiorno
      address: "0/0/1"

  on_group_address:
    - address: "0/0/1"
      then:
        - logger.log: "Luce soggiorno modificata!"
        - switch.toggle: ventilatore

    - address: "1/0/1"
      then:
        - lambda: |-
            // Decodifica temperatura (DPT 9)
            if (data.size() >= 2) {
              uint16_t raw = (data[0] << 8) | data[1];
              int16_t m = (raw & 0x7FF);
              if (raw & 0x8000) m = -(~m & 0x7FF) - 1;
              int8_t e = (raw >> 11) & 0x0F;
              float temp = 0.01f * m * (1 << e);

              ESP_LOGI("temp", "Temperatura: %.1f°C", temp);

              if (temp > 25.0) {
                id(ventilatore).turn_on();
              }
            }
```

### Variabili Disponibili

- `data` (vector<uint8_t>) - Dati payload del telegramma

**Nota:** Il `group_address` NON è disponibile (già lo conosci dalla configurazione!)

### Quando Usarlo

✅ **Usalo per:**
- Logica applicativa normale
- Reazioni a specifici group address
- Automazioni KNX
- Controllo dispositivi
- Decodifica valori DPT

✅ **Vantaggi:**
- Molto efficiente (lookup O(1))
- Performance costanti anche con traffico alto
- Minimo overhead CPU
- Scalabile

### Impatto Performance

| Metrica | Valore |
|---------|--------|
| RAM aggiunta | ~100 bytes per trigger |
| CPU overhead | ~0.15 µs per telegramma (lookup hash map) |
| Scalabilità | Indipendente dal traffico KNX |

**Esempio:** Con 200 telegrammi/sec e 10 trigger → ~0.02% CPU utilizzato

## Confronto e Performance

### Confronto Diretto

| Feature | `on_telegram` | `on_group_address` |
|---------|---------------|-------------------|
| Chiamato per | TUTTI i telegrammi | Solo GA specifici |
| RAM per trigger | 60-80 bytes | 90-110 bytes |
| CPU overhead | Alto (~75 µs/msg) | Basso (~0.15 µs/msg) |
| Scalabilità | ❌ Peggiora con traffico | ✅ Costante |
| Uso consigliato | Debug/monitoring | Logica applicativa |
| Variabili | `group_address`, `data` | `data` |

### Scenario Reale

**Setup:**
- 5 trigger configurati
- 200 telegrammi/sec sul bus KNX
- Solo 10 telegrammi/sec ci interessano

#### on_telegram:
```
RAM: 5 × 80 = 400 bytes
CPU: 200 msg/s × 75 µs = 15 ms/sec = 1.5% CPU
Problema: Esegue lambda anche per 190 msg non interessanti!
```

#### on_group_address:
```
RAM: 5 × 100 = 500 bytes  (+100 bytes rispetto a on_telegram)
CPU: 200 msg/s × 5 trigger × 0.15 µs = 0.15 ms/sec = 0.015% CPU
Esecuzione: Solo 10 msg/s che ci interessano effettivamente eseguiti
```

### Raccomandazione

🎯 **Usa `on_group_address` per default** - È 50-100x più efficiente!

Solo usa `on_telegram` se hai bisogno di:
- Monitorare TUTTI i telegrammi (debugging)
- Implementare un packet sniffer KNX
- Statistiche sul traffico globale

## Disabilitare i Trigger

Entrambi i trigger possono essere disabilitati individualmente tramite compile flags per risparmiare memoria e performance.

### Disabilitare on_telegram

```yaml
esphome:
  name: my-device
  platformio_options:
    build_flags:
      - "-DUSE_KNX_ON_TELEGRAM=0"  # Disabilita on_telegram
```

**Risparmio:**
- RAM: ~80-100 bytes per trigger + 12 bytes vector overhead
- Flash: ~200-400 bytes per trigger
- CPU: Elimina completamente l'overhead

### Disabilitare on_group_address

```yaml
esphome:
  name: my-device
  platformio_options:
    build_flags:
      - "-DUSE_KNX_ON_GROUP_ADDRESS=0"  # Disabilita on_group_address
```

**Risparmio:**
- RAM: ~100-120 bytes per trigger + hash map overhead
- Flash: ~200-400 bytes per trigger
- CPU: Elimina il lookup hash map

### Disabilitare Entrambi

```yaml
esphome:
  name: my-device
  platformio_options:
    build_flags:
      - "-DUSE_KNX_ON_TELEGRAM=0"
      - "-DUSE_KNX_ON_GROUP_ADDRESS=0"
```

**Nota:** Se disabiliti un trigger, le relative configurazioni YAML saranno ignorate (non causeranno errori di compilazione).

## Helper DPT per Trigger ✨

Per semplificare la decodifica dei DPT nei trigger, il componente fornisce **funzioni helper** nel namespace `dpt_helpers`.

### Funzioni Disponibili

```cpp
using namespace esphome::knx_tp::dpt_helpers;

// DPT 1.xxx - Boolean
bool state = decode_dpt1(data);

// DPT 5.xxx - 8-bit
uint8_t value = decode_dpt5(data);
float percent = decode_dpt5_percentage(data);  // DPT 5.001
float angle = decode_dpt5_angle(data);  // DPT 5.003

// DPT 9.xxx - 2-byte float
float value = decode_dpt9(data);  // Temperatura, umidità, lux, ecc.

// DPT 14.xxx - 4-byte float
float value = decode_dpt14(data);

// DPT 16.001 - String
std::string text = decode_dpt16(data);

// DPT 10.001 - Time of Day
DPT::TimeOfDay time = decode_dpt10(data);

// DPT 11.001 - Date
DPT::Date date = decode_dpt11(data);

// DPT 19.001 - DateTime
DPT::DateTime dt = decode_dpt19(data);

// DPT 20.102 - HVAC Mode
DPT::HVACMode mode = decode_dpt20_102(data);
```

### Vantaggi

✅ **Codice più pulito** - Nessuna decodifica manuale
✅ **Meno errori** - Funzioni testate e validate
✅ **Più leggibile** - Codice auto-documentante
✅ **Generico** - `decode_dpt9()` funziona per temperatura, umidità, lux, ecc.

### Esempio Base

```yaml
knx_tp:
  on_group_address:
    # Temperatura (DPT 9.001)
    - address: "1/0/1"
      then:
        - lambda: |-
            using namespace esphome::knx_tp::dpt_helpers;
            float temp = decode_dpt9(data);
            ESP_LOGI("temp", "Temperatura: %.1f°C", temp);

    # Umidità (DPT 9.007) - stessa funzione!
    - address: "1/0/2"
      then:
        - lambda: |-
            using namespace esphome::knx_tp::dpt_helpers;
            float hum = decode_dpt9(data);
            ESP_LOGI("hum", "Umidità: %.1f%%", hum);

    # Switch (DPT 1.001)
    - address: "0/0/1"
      then:
        - lambda: |-
            using namespace esphome::knx_tp::dpt_helpers;
            bool state = decode_dpt1(data);
            ESP_LOGI("switch", "State: %s", state ? "ON" : "OFF");
```

**⚠️ Importante:**
- Le funzioni sono **generiche** per tipo DPT
- **L'utente decide** come interpretare il valore
- Non esistono funzioni come `decode_temperature()`, usa `decode_dpt9()` e formatta come vuoi

## Esempi Pratici

### Esempio 1: Monitoring e Debug

```yaml
knx_tp:
  physical_address: "1.1.100"
  uart_id: knx_uart

  # Monitora tutto il traffico (solo per debug!)
  on_telegram:
    - if:
        condition:
          lambda: 'return id(debug_mode).state;'
        then:
          - logger.log:
              format: "📨 GA=%s, Size=%d"
              args: ['group_address.c_str()', 'data.size()']

switch:
  - platform: template
    name: "KNX Debug Mode"
    id: debug_mode
    optimistic: true
```

### Esempio 2: Automazione Intelligente

```yaml
knx_tp:
  physical_address: "1.1.100"
  uart_id: knx_uart

  group_addresses:
    - id: motion_sensor
      address: "2/0/1"
    - id: light_control
      address: "0/0/1"
    - id: temperature
      address: "1/0/1"

  on_group_address:
    # Rileva movimento
    - address: "2/0/1"
      then:
        - logger.log: "🚶 Movimento rilevato!"
        - light.turn_on: led_indicator
        - switch.turn_on: auto_light
        - delay: 5min
        - switch.turn_off: auto_light

    # Temperatura alta → accendi ventilatore
    - address: "1/0/1"
      then:
        - lambda: |-
            if (data.size() >= 2) {
              // Decodifica DPT 9
              uint16_t raw = (data[0] << 8) | data[1];
              int16_t m = (raw & 0x7FF);
              if (raw & 0x8000) m = -(~m & 0x7FF) - 1;
              int8_t e = (raw >> 11) & 0x0F;
              float temp = 0.01f * m * (1 << e);

              if (temp > 26.0) {
                id(fan).turn_on();
              } else if (temp < 22.0) {
                id(fan).turn_off();
              }
            }

switch:
  - platform: gpio
    name: "Ventilatore"
    id: fan
    pin: GPIO4

  - platform: knx_tp
    name: "Luce Automatica"
    id: auto_light
    command_ga: light_control
```

### Esempio 3: Decodifica DPT con Helper ✨

```yaml
knx_tp:
  physical_address: "1.1.100"
  uart_id: knx_uart

  on_group_address:
    # DPT 1 - Boolean
    - address: "0/0/1"
      then:
        - lambda: |-
            using namespace esphome::knx_tp::dpt_helpers;
            bool state = decode_dpt1(data);
            ESP_LOGI("dpt1", "Boolean: %s", state ? "ON" : "OFF");

    # DPT 5.001 - Percentage (0-100%)
    - address: "0/1/1"
      then:
        - lambda: |-
            using namespace esphome::knx_tp::dpt_helpers;
            float percent = decode_dpt5_percentage(data);
            ESP_LOGI("dpt5", "Percentage: %.1f%%", percent);

    # DPT 9.001 - Temperature (°C)
    - address: "1/0/1"
      then:
        - lambda: |-
            using namespace esphome::knx_tp::dpt_helpers;
            float temp = decode_dpt9(data);
            ESP_LOGI("dpt9", "Temperature: %.2f°C", temp);

    # DPT 9.007 - Humidity (%) - stessa funzione!
    - address: "1/0/2"
      then:
        - lambda: |-
            using namespace esphome::knx_tp::dpt_helpers;
            float hum = decode_dpt9(data);
            ESP_LOGI("dpt9", "Humidity: %.1f%%", hum);

    # DPT 14 - 4-byte float
    - address: "1/1/1"
      then:
        - lambda: |-
            using namespace esphome::knx_tp::dpt_helpers;
            float value = decode_dpt14(data);
            ESP_LOGI("dpt14", "Float: %.3f", value);

    # DPT 10.001 - Time of Day
    - address: "2/0/1"
      then:
        - lambda: |-
            using namespace esphome::knx_tp::dpt_helpers;
            auto time = decode_dpt10(data);
            ESP_LOGI("time", "Time: %02d:%02d:%02d",
                     time.hour, time.minute, time.second);
```

**Nota:** Molto più semplice rispetto alla decodifica manuale! 🎉

### Esempio 4: Uso Combinato (Ottimale)

```yaml
knx_tp:
  physical_address: "1.1.100"
  uart_id: knx_uart

  # Debug generico (disabilitabile)
  on_telegram:
    - if:
        condition:
          - switch.is_on: enable_debug
        then:
          - logger.log:
              format: "Debug: %s"
              args: ['group_address.c_str()']

  # Logica applicativa (sempre attiva, efficiente)
  on_group_address:
    - address: "0/0/1"
      then:
        - switch.toggle: my_light

    - address: "1/0/1"
      then:
        - lambda: 'ESP_LOGI("temp", "Updated");'

switch:
  - platform: template
    name: "Enable KNX Debug"
    id: enable_debug
    optimistic: true

  - platform: gpio
    name: "My Light"
    id: my_light
    pin: GPIO5
```

## Best Practices

1. **Preferisci `on_group_address`** per la logica applicativa normale
2. **Usa `on_telegram` solo per debug** e disabilitalo in produzione
3. **Disabilita trigger non usati** con compile flags
4. **Evita operazioni pesanti** nei trigger (usali solo per trigger azioni rapide)
5. **Usa `lambda` per decodifica DPT** custom
6. **Filtra traffico** se usi `on_telegram` su bus molto trafficati
7. **Testa le performance** con il tuo carico KNX reale

## Troubleshooting

### Troppo overhead CPU

```yaml
# Soluzione 1: Usa on_group_address invece di on_telegram
on_group_address:  # ✅ Efficiente
  - address: "0/0/1"
    then: ...

# Soluzione 2: Disabilita on_telegram
platformio_options:
  build_flags:
    - "-DUSE_KNX_ON_TELEGRAM=0"
```

### RAM insufficiente

```yaml
# Disabilita trigger non usati
platformio_options:
  build_flags:
    - "-DUSE_KNX_ON_TELEGRAM=0"  # Se non usato
```

### Trigger non viene chiamato

1. Verifica che il group address sia corretto
2. Controlla i log: `logger: level: VERBOSE`
3. Verifica che il telegramma arrivi effettivamente sul bus
4. Controlla che il compile flag non abbia disabilitato il trigger

## Conclusione

I trigger KNX offrono grande flessibilità per reagire ai telegrammi sul bus:

- **`on_group_address`**: Efficiente, per logica applicativa ✅
- **`on_telegram`**: Per debug e monitoring (usa con cautela) ⚠️

Usa i compile flags per ottimizzare memoria e performance in base alle tue esigenze!
