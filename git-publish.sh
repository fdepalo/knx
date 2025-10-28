#!/bin/bash
# Script per pubblicare il repository ESPHome KNX TP su GitHub
# Autore: @fdepalo
# Uso: bash git-publish.sh

set -e  # Exit on error

echo "🚀 Pubblicazione Repository ESPHome KNX TP su GitHub"
echo "======================================================"
echo ""

# Colori per output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Step 1: Verifica prerequisiti
echo "📍 Step 1: Verifica prerequisiti..."
if [ ! -f "README.md" ] || [ ! -f "LICENSE" ]; then
    echo -e "${RED}❌ Errore: File README.md o LICENSE non trovati${NC}"
    exit 1
fi

if ! command -v git &> /dev/null; then
    echo -e "${RED}❌ Errore: Git non installato${NC}"
    exit 1
fi
echo -e "${GREEN}✅ Prerequisiti verificati${NC}"
echo ""

# Step 2: Configurazione Git
echo "⚙️  Step 2: Configurazione Git..."

# Verifica configurazione utente
if [ -z "$(git config user.name)" ]; then
    echo -e "${YELLOW}Configurazione utente Git non trovata${NC}"
    read -p "Inserisci il tuo nome Git: " git_name
    git config user.name "$git_name"
fi

if [ -z "$(git config user.email)" ]; then
    read -p "Inserisci la tua email Git: " git_email
    git config user.email "$git_email"
fi

echo "   Git User: $(git config user.name)"
echo "   Git Email: $(git config user.email)"
echo -e "${GREEN}✅ Configurazione Git completata${NC}"
echo ""

# Step 3: Inizializza repository Git (se necessario)
echo "📦 Step 3: Inizializzazione repository Git..."
if [ ! -d ".git" ]; then
    git init
    echo -e "${GREEN}✅ Repository Git inizializzato${NC}"
else
    echo -e "${GREEN}✅ Repository Git già esistente${NC}"
fi
echo ""

# Step 4: Aggiungi file al repository
echo "➕ Step 4: Aggiunta file al repository..."
git add .

# Mostra status
echo ""
echo -e "${BLUE}File da committare:${NC}"
git status --short
echo ""

read -p "Continuare con il commit? (y/n): " confirm
if [ "$confirm" != "y" ]; then
    echo -e "${YELLOW}Operazione annullata${NC}"
    exit 0
fi

# Step 5: Crea commit iniziale
echo ""
echo "💾 Step 5: Creazione commit iniziale..."

COMMIT_MSG="Initial commit: ESPHome KNX TP Component

✨ Features:
- Complete KNX TP integration with 8 platforms
- Memory optimized (~5KB RAM saved)
- Full DPT support (1.xxx, 5.xxx, 9.xxx, 10.xxx, 11.xxx, 14.xxx, 16.xxx, 19.xxx, 20.xxx)
- Time broadcast support (DPT 19.001)
- BCU connection detection
- Production ready with complete error handling

📦 Platforms:
- Binary Sensor (motion sensors, contacts)
- Switch (lights, relays)
- Sensor (temperature, humidity, brightness)
- Climate (thermostats, HVAC)
- Cover (blinds, shutters)
- Light (dimmable lights)
- Text Sensor (status, time/date)
- Number (setpoints, values)

🔧 Technical:
- Thelsing KNX library integration
- ESP32 ESP-IDF framework support
- TPUART, NCN5120, Siemens BCU compatible
- Complete validation and error handling

📚 Documentation:
- Comprehensive README
- Example configurations
- Troubleshooting guide
- Contributing guidelines"

git commit -m "$COMMIT_MSG"
echo -e "${GREEN}✅ Commit creato${NC}"
echo ""

# Step 6: Configurazione GitHub Remote
echo "🌐 Step 6: Configurazione GitHub Remote..."
echo ""
echo -e "${YELLOW}Prima di continuare, crea il repository su GitHub:${NC}"
echo "   1. Vai su: https://github.com/new"
echo "   2. Repository name: esphome-knx-tp"
echo "   3. Description: Complete KNX Twisted Pair integration for ESPHome"
echo "   4. Visibility: Public"
echo "   5. ❌ NON aggiungere README, .gitignore o LICENSE"
echo "   6. Clicca 'Create repository'"
echo ""

read -p "Hai già creato il repository su GitHub? (y/n): " github_ready
if [ "$github_ready" != "y" ]; then
    echo ""
    echo -e "${YELLOW}Crea prima il repository su GitHub e poi riesegui questo script${NC}"
    exit 0
fi

# Chiedi username GitHub
echo ""
read -p "Inserisci il tuo username GitHub (default: fdepalo): " github_user
github_user=${github_user:-fdepalo}

REMOTE_URL="https://github.com/${github_user}/esphome-knx-tp.git"

# Verifica se remote già esiste
if git remote | grep -q '^origin$'; then
    echo -e "${YELLOW}Remote 'origin' già esistente${NC}"
    current_url=$(git remote get-url origin)
    echo "   URL corrente: $current_url"
    read -p "Vuoi aggiornarlo a $REMOTE_URL? (y/n): " update_remote
    if [ "$update_remote" = "y" ]; then
        git remote set-url origin "$REMOTE_URL"
        echo -e "${GREEN}✅ Remote aggiornato${NC}"
    fi
else
    git remote add origin "$REMOTE_URL"
    echo -e "${GREEN}✅ Remote 'origin' aggiunto: $REMOTE_URL${NC}"
fi
echo ""

# Step 7: Push su GitHub
echo "⬆️  Step 7: Push del codice su GitHub..."
echo ""
read -p "Procedere con il push su GitHub? (y/n): " push_confirm
if [ "$push_confirm" != "y" ]; then
    echo -e "${YELLOW}Push annullato${NC}"
    echo ""
    echo "Puoi farlo manualmente con:"
    echo "   git branch -M main"
    echo "   git push -u origin main"
    exit 0
fi

# Rinomina branch in main e push
git branch -M main
echo "Pushing to GitHub..."
if git push -u origin main; then
    echo -e "${GREEN}✅ Codice pubblicato su GitHub!${NC}"
else
    echo -e "${RED}❌ Errore durante il push${NC}"
    echo ""
    echo "Se richiesta autenticazione, potrebbero essere necessari i GitHub Personal Access Tokens:"
    echo "   1. Vai su: https://github.com/settings/tokens"
    echo "   2. Generate new token (classic)"
    echo "   3. Scopes: repo (tutti i permessi)"
    echo "   4. Usa il token come password quando richiesto"
    exit 1
fi
echo ""

# Step 8: Creazione Tag e Release
echo "🏷️  Step 8: Creazione tag versione..."
echo ""
read -p "Vuoi creare un tag v1.0.0 per la prima release? (y/n): " create_tag
if [ "$create_tag" = "y" ]; then
    TAG_MSG="Release v1.0.0 - Initial Stable Release

🎉 First stable release of ESPHome KNX TP Component!

✨ Features:
- 8 ESPHome platforms (Binary Sensor, Switch, Sensor, Climate, Cover, Light, Text Sensor, Number)
- Complete DPT support (1.xxx, 5.xxx, 9.xxx, 10.xxx, 11.xxx, 14.xxx, 16.xxx, 19.xxx, 20.xxx)
- Memory optimizations (~5KB RAM saved)
- Time broadcast (DPT 19.001)
- BCU connection detection
- Production ready with full error handling

📦 Tested with:
- ESPHome 2025.10.0
- ESP32 (ESP-IDF framework)
- TPUART, NCN5120, Siemens BCU

📚 Documentation:
- Complete README with examples
- Troubleshooting guide
- Contributing guidelines

🔗 Installation:
\`\`\`yaml
external_components:
  - source:
      type: git
      url: https://github.com/${github_user}/esphome-knx-tp
      ref: v1.0.0
    components: [knx_tp]
\`\`\`

See CHANGELOG.md for full details."

    git tag -a v1.0.0 -m "$TAG_MSG"
    git push origin v1.0.0
    echo -e "${GREEN}✅ Tag v1.0.0 creato e pubblicato${NC}"
    echo ""
    echo -e "${YELLOW}Ora crea la Release su GitHub:${NC}"
    echo "   1. Vai su: https://github.com/${github_user}/esphome-knx-tp/releases/new"
    echo "   2. Choose tag: v1.0.0"
    echo "   3. Release title: v1.0.0 - Initial Stable Release"
    echo "   4. Description: (copia dal tag o usa il changelog)"
    echo "   5. Clicca 'Publish release'"
fi
echo ""

# Step 9: Configurazione Topics GitHub
echo "🏷️  Step 9: Configurazione GitHub Topics..."
echo ""
echo -e "${YELLOW}Aggiungi i topics su GitHub per migliore discoverability:${NC}"
echo "   1. Vai su: https://github.com/${github_user}/esphome-knx-tp"
echo "   2. Clicca su ⚙️ Settings (accanto ad About)"
echo "   3. Aggiungi topics:"
echo "      - esphome"
echo "      - knx"
echo "      - home-automation"
echo "      - esp32"
echo "      - knx-tp"
echo "      - iot"
echo "      - smart-home"
echo "      - home-assistant"
echo ""

# Step 10: Riepilogo finale
echo ""
echo "═══════════════════════════════════════════════════════"
echo -e "${GREEN}✅ Pubblicazione completata con successo! 🎉${NC}"
echo "═══════════════════════════════════════════════════════"
echo ""
echo -e "${BLUE}📊 Repository pubblicato:${NC}"
echo "   🔗 URL: https://github.com/${github_user}/esphome-knx-tp"
echo ""
echo -e "${BLUE}📋 Prossimi passi consigliati:${NC}"
echo "   1. ✅ Vai su GitHub e verifica che tutto sia corretto"
echo "   2. 🏷️  Aggiungi i topics al repository"
echo "   3. 📦 Crea la Release v1.0.0 (se hai creato il tag)"
echo "   4. 💬 Abilita Discussions: Settings → Features → Discussions"
echo "   5. 📖 Considera di aggiungere il repository a:"
echo "      - ESPHome Discord (#showcase)"
echo "      - Home Assistant Community Forum"
echo "      - Reddit r/homeassistant"
echo ""
echo -e "${BLUE}🧪 Test installazione:${NC}"
echo "   Crea un file test.yaml e aggiungi:"
echo ""
echo "   external_components:"
echo "     - source:"
echo "         type: git"
echo "         url: https://github.com/${github_user}/esphome-knx-tp"
echo "         ref: main"
echo "       components: [knx_tp]"
echo ""
echo -e "${GREEN}Fatto! Il tuo componente è ora pubblico e pronto all'uso! 🚀${NC}"
echo ""
