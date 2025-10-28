# 🚀 Guida Pubblicazione Repository

Questa guida ti aiuterà a pubblicare il componente ESPHome KNX TP su GitHub in 3 semplici passi.

## 📋 Prerequisiti

- Git installato sul tuo sistema
- Account GitHub
- ESPHome installato (per test compilazione)

## 🎯 Pubblicazione in 3 Passi

### Passo 1️⃣: Setup Repository

Prepara la struttura del repository e crea tutti i file necessari:

```bash
bash setup-repository.sh
```

**Cosa fa questo script:**
- ✅ Crea struttura directory (components/, examples/, .github/)
- ✅ Sposta i file nelle posizioni corrette
- ✅ Crea CHANGELOG.md
- ✅ Crea CONTRIBUTING.md
- ✅ Crea Issue Templates (bug report, feature request, question)
- ✅ Crea secrets.yaml.example
- ✅ Pulisce file temporanei
- ✅ Verifica .gitignore

**Output atteso:**
```
🚀 Setup Repository ESPHome KNX TP
====================================
✅ Directory corretta
✅ components/knx_tp spostato
✅ Esempi spostati
✅ File temporanei rimossi
✅ CHANGELOG.md creato
✅ CONTRIBUTING.md creato
✅ Issue templates creati
✅ secrets.yaml.example creato

Repository pronto per la pubblicazione! 🚀
```

### Passo 2️⃣: Verifica Repository

Controlla che tutto sia corretto prima di pubblicare:

```bash
bash verify-repository.sh
```

**Cosa controlla questo script:**
- ✅ Struttura directory corretta
- ✅ Tutti i file essenziali presenti
- ✅ Componente KNX completo
- ✅ Tutte le 8 piattaforme ESPHome
- ✅ File di esempio presenti
- ✅ Issue templates presenti
- ✅ Contenuto README.md
- ✅ Nessun path hardcoded
- ✅ Nessun secret in chiaro
- ✅ Test compilazione esempi
- ✅ Dimensione repository

**Output atteso:**
```
🔍 Verifica Repository ESPHome KNX TP
======================================
✅ TUTTO OK! Repository pronto per la pubblicazione! 🎉

📋 Prossimi passi:
   1. Se tutto è OK, esegui: bash git-publish.sh
   2. Segui le istruzioni per pubblicare su GitHub
```

### Passo 3️⃣: Pubblica su GitHub

Pubblica il repository su GitHub:

```bash
bash git-publish.sh
```

**Cosa fa questo script:**
- ✅ Verifica prerequisiti (git, files)
- ✅ Configura utente Git
- ✅ Inizializza repository Git (se necessario)
- ✅ Crea commit iniziale con messaggio dettagliato
- ✅ Configura remote GitHub
- ✅ Push del codice su GitHub
- ✅ Crea tag v1.0.0 (opzionale)
- ✅ Fornisce istruzioni per release e topics

**Durante l'esecuzione:**

1. Lo script ti chiederà di creare il repository su GitHub:
   - Vai su: https://github.com/new
   - Nome: `esphome-knx-tp`
   - Public ✅
   - Non aggiungere README/LICENSE/.gitignore

2. Inserisci il tuo username GitHub (default: fdepalo)

3. Conferma il push su GitHub

4. Opzionalmente crea tag v1.0.0

**Output finale:**
```
✅ Pubblicazione completata con successo! 🎉

📊 Repository pubblicato:
   🔗 URL: https://github.com/fdepalo/esphome-knx-tp

📋 Prossimi passi consigliati:
   1. ✅ Verifica su GitHub
   2. 🏷️  Aggiungi topics
   3. 📦 Crea Release v1.0.0
   4. 💬 Abilita Discussions
```

## 🔧 Comandi Git Utili

Se vuoi fare operazioni manuali:

```bash
# Verifica status
git status

# Vedi commit history
git log --oneline

# Vedi remote configurati
git remote -v

# Aggiorna remote
git remote set-url origin https://github.com/USERNAME/esphome-knx-tp.git

# Push manuale
git push -u origin main

# Crea tag
git tag -a v1.0.0 -m "Release v1.0.0"
git push origin v1.0.0
```

## ❓ Troubleshooting

### Errore: "Git non configurato"
```bash
git config user.name "Your Name"
git config user.email "your.email@example.com"
```

### Errore: "Authentication failed"
Usa GitHub Personal Access Token:
1. Vai su: https://github.com/settings/tokens
2. Generate new token (classic)
3. Scope: `repo` (tutti i permessi)
4. Usa il token come password

### Errore: "Remote già esistente"
```bash
git remote remove origin
git remote add origin https://github.com/USERNAME/esphome-knx-tp.git
```

### Errore di compilazione negli esempi
Modifica i file in `examples/` per correggere gli errori, poi:
```bash
bash verify-repository.sh
```

## 📝 Post-Pubblicazione

### 1. Aggiungi Topics su GitHub

1. Vai su: https://github.com/USERNAME/esphome-knx-tp
2. Clicca su ⚙️ (Settings) accanto ad "About"
3. Aggiungi topics:
   - `esphome`
   - `knx`
   - `home-automation`
   - `esp32`
   - `knx-tp`
   - `iot`
   - `smart-home`
   - `home-assistant`

### 2. Crea Release su GitHub

1. Vai su: https://github.com/USERNAME/esphome-knx-tp/releases/new
2. Choose tag: `v1.0.0`
3. Release title: `v1.0.0 - Initial Stable Release`
4. Description: Copia dal CHANGELOG.md
5. Clicca "Publish release"

### 3. Abilita GitHub Discussions

1. Settings → Features
2. ✅ Discussions

### 4. Condividi il Componente

- 💬 ESPHome Discord (#showcase channel)
- 🏠 Home Assistant Community Forum
- 📱 Reddit r/homeassistant
- 🐦 Twitter/X con #ESPHome #KNX #HomeAutomation

## 🎉 Fatto!

Il tuo componente è ora pubblico e pronto all'uso!

Gli utenti potranno installarlo con:

```yaml
external_components:
  - source:
      type: git
      url: https://github.com/USERNAME/esphome-knx-tp
      ref: v1.0.0
    components: [knx_tp]
```

## 📞 Supporto

Se hai problemi con questi script:
- Leggi i messaggi di errore attentamente
- Verifica i prerequisiti
- Controlla la documentazione Git: https://git-scm.com/doc
- Chiedi aiuto su GitHub Discussions

---

**Made with ❤️ for the ESPHome and KNX communities**
