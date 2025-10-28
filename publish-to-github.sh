#!/bin/bash
# Script master per guidare la pubblicazione del repository su GitHub
# Autore: @fdepalo
# Uso: bash publish-to-github.sh

echo ""
echo "╔════════════════════════════════════════════════════════════╗"
echo "║   🚀 Pubblicazione ESPHome KNX TP Component su GitHub      ║"
echo "║                                                            ║"
echo "║   Questo script ti guiderà attraverso tutti i passi        ║"
echo "║   necessari per pubblicare il componente su GitHub         ║"
echo "╚════════════════════════════════════════════════════════════╝"
echo ""

# Colori
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
NC='\033[0m'

# Funzione per mostrare il progresso
show_step() {
    echo ""
    echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
    echo -e "${BLUE}$1${NC}"
    echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
    echo ""
}

# Benvenuto
echo -e "${GREEN}Benvenuto!${NC}"
echo ""
echo "Questo script eseguirà automaticamente:"
echo "  1️⃣  Setup del repository (struttura, file, documentazione)"
echo "  2️⃣  Verifica completezza (controllo file, compilazione)"
echo "  3️⃣  Pubblicazione su GitHub (git commit, push, tag)"
echo ""
read -p "Vuoi continuare? (y/n): " continue
if [ "$continue" != "y" ]; then
    echo "Operazione annullata."
    exit 0
fi

# STEP 1: Setup Repository
show_step "STEP 1/3: Setup Repository"
echo "Preparo la struttura del repository e creo tutti i file necessari..."
echo ""
read -p "Premere INVIO per continuare..."

if [ -f "setup-repository.sh" ]; then
    bash setup-repository.sh
    SETUP_RESULT=$?
    if [ $SETUP_RESULT -ne 0 ]; then
        echo ""
        echo -e "${YELLOW}⚠️  Setup completato con avvisi. Vuoi continuare? (y/n)${NC}"
        read -p "> " continue_after_setup
        if [ "$continue_after_setup" != "y" ]; then
            exit 1
        fi
    fi
else
    echo "❌ Script setup-repository.sh non trovato!"
    exit 1
fi

# STEP 2: Verifica Repository
show_step "STEP 2/3: Verifica Repository"
echo "Verifico che tutto sia pronto per la pubblicazione..."
echo ""
read -p "Premere INVIO per continuare..."

if [ -f "verify-repository.sh" ]; then
    bash verify-repository.sh
    VERIFY_RESULT=$?
    if [ $VERIFY_RESULT -ne 0 ]; then
        echo ""
        echo "❌ Verifica fallita. Risolvi gli errori e riesegui questo script."
        exit 1
    fi
else
    echo "❌ Script verify-repository.sh non trovato!"
    exit 1
fi

# STEP 3: Pubblicazione su GitHub
show_step "STEP 3/3: Pubblicazione su GitHub"
echo "Ora pubblicheremo il repository su GitHub."
echo ""
echo -e "${YELLOW}⚠️  IMPORTANTE:${NC}"
echo "   Prima di continuare, assicurati di avere:"
echo "   - Un account GitHub"
echo "   - Accesso per creare repository"
echo "   - (Opzionale) Personal Access Token per l'autenticazione"
echo ""
read -p "Sei pronto per pubblicare su GitHub? (y/n): " ready
if [ "$ready" != "y" ]; then
    echo ""
    echo "Nessun problema! Puoi farlo più tardi eseguendo:"
    echo "   bash git-publish.sh"
    exit 0
fi

if [ -f "git-publish.sh" ]; then
    bash git-publish.sh
    PUBLISH_RESULT=$?
    if [ $PUBLISH_RESULT -eq 0 ]; then
        show_step "✅ PUBBLICAZIONE COMPLETATA CON SUCCESSO! 🎉"
        echo ""
        echo "Il tuo componente ESPHome KNX TP è ora pubblico su GitHub!"
        echo ""
        echo -e "${GREEN}Prossimi passi:${NC}"
        echo ""
        echo "1. 🌐 Visita il tuo repository su GitHub"
        echo "2. 🏷️  Aggiungi i topics (esphome, knx, home-automation, etc.)"
        echo "3. 📦 Crea una Release v1.0.0 dal tag"
        echo "4. 💬 Abilita Discussions nelle impostazioni"
        echo "5. 📢 Condividi il componente:"
        echo "   - ESPHome Discord (#showcase)"
        echo "   - Home Assistant Forum"
        echo "   - Reddit r/homeassistant"
        echo ""
        echo -e "${BLUE}Gli utenti potranno ora usare il componente con:${NC}"
        echo ""
        echo "external_components:"
        echo "  - source:"
        echo "      type: git"
        echo "      url: https://github.com/YOUR_USERNAME/esphome-knx-tp"
        echo "      ref: v1.0.0"
        echo "    components: [knx_tp]"
        echo ""
        echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
        echo -e "${GREEN}Congratulazioni! 🎊 Il tuo componente è live! 🚀${NC}"
        echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
        echo ""
    else
        echo ""
        echo "❌ Pubblicazione fallita. Controlla i messaggi di errore sopra."
        echo ""
        echo "Puoi riprovare eseguendo:"
        echo "   bash git-publish.sh"
        exit 1
    fi
else
    echo "❌ Script git-publish.sh non trovato!"
    exit 1
fi
