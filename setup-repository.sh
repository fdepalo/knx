#!/bin/bash
# Script per preparare il repository ESPHome KNX TP per la pubblicazione
# Autore: @fdepalo
# Uso: bash setup-repository.sh

set -e  # Exit on error

echo "🚀 Setup Repository ESPHome KNX TP"
echo "===================================="
echo ""

# Colori per output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Step 1: Verifica directory corrente
echo "📍 Step 1: Verifica directory corrente..."
if [ ! -f "LICENSE" ] || [ ! -f "README.md" ]; then
    echo -e "${RED}❌ Errore: Esegui questo script dalla directory root del progetto${NC}"
    exit 1
fi
echo -e "${GREEN}✅ Directory corretta${NC}"
echo ""

# Step 2: Crea struttura directory
echo "📁 Step 2: Creazione struttura directory..."
mkdir -p components examples .github/ISSUE_TEMPLATE

# Sposta knx_tp in components/ se necessario
if [ -d "knx_tp" ] && [ ! -d "components/knx_tp" ]; then
    echo "   Spostamento knx_tp in components/..."
    mv knx_tp components/
    echo -e "${GREEN}✅ knx_tp spostato in components/${NC}"
elif [ -d "components/knx_tp" ]; then
    echo -e "${GREEN}✅ components/knx_tp già esistente${NC}"
else
    echo -e "${RED}❌ Errore: Directory knx_tp non trovata${NC}"
    exit 1
fi

# Sposta esempi in examples/
if [ -f "knx-example-simple.yaml" ]; then
    mv knx-example-simple.yaml examples/ 2>/dev/null || true
    echo -e "${GREEN}✅ knx-example-simple.yaml spostato${NC}"
fi
if [ -f "knx-example-advanced.yaml" ]; then
    mv knx-example-advanced.yaml examples/ 2>/dev/null || true
    echo -e "${GREEN}✅ knx-example-advanced.yaml spostato${NC}"
fi
echo ""

# Step 3: Pulisci file non necessari
echo "🧹 Step 3: Pulizia file temporanei..."
rm -rf .esphome/ 2>/dev/null || true
rm -f ciola.yaml example.yaml 2>/dev/null || true
echo -e "${GREEN}✅ File temporanei rimossi${NC}"
echo ""

# Step 4: Crea secrets.yaml.example
echo "🔐 Step 4: Creazione secrets.yaml.example..."
cat > secrets.yaml.example << 'EOF'
# Example secrets file for ESPHome KNX TP
# Copy this file to secrets.yaml and fill in your values

# WiFi credentials
wifi_ssid: "YourNetworkName"
wifi_password: "YourWiFiPassword"

# API encryption key (generate with: esphome config wizard)
api_encryption_key: "your-32-char-api-key-here=="

# OTA password
ota_password: "your-ota-password"

# Access Point fallback password
ap_password: "fallback-password"
EOF
echo -e "${GREEN}✅ secrets.yaml.example creato${NC}"
echo ""

# Step 5: Crea CHANGELOG.md
echo "📝 Step 5: Creazione CHANGELOG.md..."
cat > CHANGELOG.md << 'EOF'
# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.0.0] - 2025-10-21

### Added
- Initial release of ESPHome KNX TP component
- Support for 8 ESPHome platforms:
  - Binary Sensor (motion sensors, contacts, etc.)
  - Switch (lights, relays, etc.)
  - Sensor (temperature, humidity, brightness, etc.)
  - Climate (thermostats, HVAC control)
  - Cover (blinds, shutters)
  - Light (dimmable lights)
  - Text Sensor (status displays, time/date)
  - Number (setpoints, values)
- Complete DPT (Datapoint Type) support:
  - DPT 1.xxx (Boolean)
  - DPT 5.xxx (8-bit unsigned)
  - DPT 9.xxx (2-byte float)
  - DPT 10.001 (Time of Day)
  - DPT 11.001 (Date)
  - DPT 14.xxx (4-byte float)
  - DPT 16.001 (Character string)
  - DPT 19.001 (Date and Time)
  - DPT 20.102 (HVAC Mode)
- Time broadcast feature (DPT 19.001) for KNX time synchronization
- BCU connection detection via SAV pin monitoring
- Memory optimizations:
  - std::vector instead of std::map for group addresses (-3 KB)
  - uint16_t for addresses instead of std::string (-1.5 KB)
  - Optimized struct padding (-0.1 KB)
  - Static buffers for encoding/decoding (-0.3 KB)
- Complete error handling and logging
- Production-ready implementation with validation
- Integration with Thelsing KNX library
- Support for TPUART, NCN5120, Siemens BCU hardware
- ESP32 ESP-IDF framework support

### Documentation
- Comprehensive README.md with installation instructions
- Example configurations (simple and advanced)
- Troubleshooting guide
- DPT reference table
- Hardware requirements documentation

### Examples
- knx-example-simple.yaml - Basic configuration
- knx-example-advanced.yaml - Full-featured gateway

[1.0.0]: https://github.com/fdepalo/esphome-knx-tp/releases/tag/v1.0.0
EOF
echo -e "${GREEN}✅ CHANGELOG.md creato${NC}"
echo ""

# Step 6: Crea Issue Templates
echo "📋 Step 6: Creazione Issue Templates..."

# Bug Report Template
cat > .github/ISSUE_TEMPLATE/bug_report.md << 'EOF'
---
name: Bug Report
about: Report a bug in the KNX TP component
title: '[BUG] '
labels: bug
assignees: ''
---

## Bug Description
A clear and concise description of what the bug is.

## Environment
- **ESPHome Version**: [e.g., 2025.10.0]
- **ESP32 Board**: [e.g., esp32dev]
- **KNX Hardware**: [e.g., TPUART, NCN5120, Siemens BCU]
- **Component Version**: [e.g., v1.0.0 or main branch]

## Configuration
```yaml
# Paste your ESPHome YAML configuration here (remove sensitive data)
knx_tp:
  physical_address: "1.1.100"
  # ... rest of your config
```

## Steps to Reproduce
1. Configure ESPHome with '...'
2. Flash to ESP32 '...'
3. Send KNX telegram '...'
4. See error

## Expected Behavior
A clear description of what you expected to happen.

## Actual Behavior
What actually happened.

## Logs
```
# Paste relevant ESPHome logs here
[timestamp][level][component] log message
```

## Additional Context
Add any other context about the problem here (screenshots, KNX ETS configuration, etc.).
EOF

# Feature Request Template
cat > .github/ISSUE_TEMPLATE/feature_request.md << 'EOF'
---
name: Feature Request
about: Suggest an idea for the KNX TP component
title: '[FEATURE] '
labels: enhancement
assignees: ''
---

## Feature Description
A clear and concise description of the feature you'd like to see.

## Use Case
Describe the problem you're trying to solve or the use case for this feature.

## Proposed Solution
Describe how you'd like this feature to work.

## Alternatives Considered
Describe any alternative solutions or features you've considered.

## Additional Context
Add any other context, screenshots, or examples about the feature request here.

## KNX Specification Reference
If applicable, reference the relevant KNX specification or DPT.
EOF

# Question Template
cat > .github/ISSUE_TEMPLATE/question.md << 'EOF'
---
name: Question
about: Ask a question about the KNX TP component
title: '[QUESTION] '
labels: question
assignees: ''
---

## Question
What would you like to know?

## Context
Provide any relevant context that might help answer your question.

## What I've Tried
Describe what you've already tried or researched.

## Configuration (if applicable)
```yaml
# Paste relevant configuration here
```
EOF

echo -e "${GREEN}✅ Issue templates creati${NC}"
echo ""

# Step 7: Crea CONTRIBUTING.md
echo "🤝 Step 7: Creazione CONTRIBUTING.md..."
cat > CONTRIBUTING.md << 'EOF'
# Contributing to ESPHome KNX TP Component

Thank you for your interest in contributing to the ESPHome KNX TP component! 🎉

## How to Contribute

### Reporting Bugs

1. **Search existing issues** to avoid duplicates
2. **Use the bug report template** when creating a new issue
3. **Include all requested information**:
   - ESPHome version
   - ESP32 board type
   - KNX hardware (TPUART, NCN5120, etc.)
   - Complete configuration
   - Relevant logs
   - Steps to reproduce

### Suggesting Features

1. **Check existing feature requests** first
2. **Use the feature request template**
3. **Clearly describe the use case**
4. **Reference KNX specifications** if applicable

### Pull Requests

1. **Fork the repository**
2. **Create a feature branch**: `git checkout -b feature/amazing-feature`
3. **Follow the coding style**:
   - C++ code follows ESPHome conventions
   - Python code follows PEP 8
   - Use meaningful variable names
   - Add comments for complex logic
4. **Test your changes**:
   - Ensure compilation works
   - Test with real KNX hardware if possible
   - Verify memory usage hasn't increased significantly
5. **Update documentation** if needed
6. **Commit with clear messages**:
   ```
   Add feature: Support for DPT 6.xxx

   - Implement encoding/decoding for DPT 6.xxx
   - Add tests for new DPT type
   - Update README with DPT 6 examples
   ```
7. **Push and create a Pull Request**

### Code Style Guidelines

#### C++ Code
- Use `snake_case` for variables and functions
- Use `PascalCase` for class names
- Add comments for public API methods
- Keep functions focused and small
- Follow ESPHome memory optimization practices

#### Python Code
- Follow PEP 8 style guide
- Use type hints where appropriate
- Add docstrings for public functions
- Use ESPHome validation helpers

#### YAML Examples
- Use consistent indentation (2 spaces)
- Add comments explaining configuration options
- Include realistic use cases

## Development Setup

### Prerequisites
- ESPHome 2025.10.0 or newer
- ESP32 development board
- KNX TP transceiver (TPUART, NCN5120, or Siemens BCU)
- KNX bus access for testing

### Local Testing
```bash
# Clone your fork
git clone https://github.com/YOUR_USERNAME/esphome-knx-tp.git
cd esphome-knx-tp

# Test with ESPHome
esphome compile examples/knx-example-simple.yaml
```

### Memory Testing
After making changes, verify memory usage:
```bash
esphome compile examples/knx-example-advanced.yaml

# Check output for:
# RAM:   [X] used XXXXX bytes
# Flash: [X] used XXXXXX bytes
```

## Questions?

- 💬 Open a [Discussion](https://github.com/fdepalo/esphome-knx-tp/discussions)
- 🐛 Check [Issues](https://github.com/fdepalo/esphome-knx-tp/issues)
- 📖 Read the [README](README.md)

Thank you for contributing! 🙏
EOF
echo -e "${GREEN}✅ CONTRIBUTING.md creato${NC}"
echo ""

# Step 8: Verifica .gitignore
echo "🔍 Step 8: Verifica .gitignore..."
if [ ! -f ".gitignore" ]; then
    cat > .gitignore << 'EOF'
# ESPHome
.esphome/
secrets.yaml
*.pyc
__pycache__/

# Python
*.py[cod]
*$py.class
*.so
.Python
build/
develop-eggs/
dist/
downloads/
eggs/
.eggs/
lib/
lib64/
parts/
sdist/
var/
wheels/
*.egg-info/
.installed.cfg
*.egg

# IDE
.vscode/
.idea/
*.swp
*.swo
*~
.DS_Store

# OS
Thumbs.db
ehthumbs.db
Desktop.ini
EOF
    echo -e "${GREEN}✅ .gitignore creato${NC}"
else
    echo -e "${GREEN}✅ .gitignore già esistente${NC}"
fi
echo ""

# Step 9: Mostra struttura finale
echo "📊 Step 9: Struttura finale del repository..."
echo ""
if command -v tree &> /dev/null; then
    tree -L 2 -I '.git|.esphome|__pycache__|*.pyc' .
else
    find . -maxdepth 2 -type f -not -path '*/\.*' | sort
fi
echo ""

# Step 10: Riepilogo
echo "✅ Setup completato con successo!"
echo ""
echo -e "${YELLOW}📋 File creati/aggiornati:${NC}"
echo "   ✅ Struttura directory (components/, examples/, .github/)"
echo "   ✅ secrets.yaml.example"
echo "   ✅ CHANGELOG.md"
echo "   ✅ CONTRIBUTING.md"
echo "   ✅ Issue templates (bug_report, feature_request, question)"
echo "   ✅ .gitignore"
echo ""
echo -e "${YELLOW}🎯 Prossimi passi:${NC}"
echo "   1. Verifica che tutto sia corretto"
echo "   2. Esegui: bash git-publish.sh"
echo "   3. Segui le istruzioni per pubblicare su GitHub"
echo ""
echo -e "${GREEN}Repository pronto per la pubblicazione! 🚀${NC}"
