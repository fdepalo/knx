#!/bin/bash
# Script per cambiare l'username GitHub nel repository
# Uso: bash change-username.sh YOUR_NEW_USERNAME

if [ -z "$1" ]; then
    echo "Uso: bash change-username.sh YOUR_NEW_USERNAME"
    echo ""
    echo "Esempio: bash change-username.sh john-smith"
    exit 1
fi

OLD_USERNAME="fdepalo"
NEW_USERNAME="$1"

echo "🔄 Cambio username da '$OLD_USERNAME' a '$NEW_USERNAME'"
echo ""

# Trova e mostra tutti i file che contengono il vecchio username
echo "📁 File che verranno modificati:"
grep -r "$OLD_USERNAME" . --exclude-dir=.git --exclude-dir=.esphome --exclude="*.pyc" -l 2>/dev/null

echo ""
read -p "Continuare con la sostituzione? (y/n): " confirm
if [ "$confirm" != "y" ]; then
    echo "Operazione annullata"
    exit 0
fi

# Sostituisci in tutti i file
echo ""
echo "🔧 Sostituzione in corso..."

# macOS usa sed -i '' mentre Linux usa sed -i
if [[ "$OSTYPE" == "darwin"* ]]; then
    # macOS
    find . -type f \( -name "*.py" -o -name "*.md" -o -name "*.sh" \) \
        -not -path "./.git/*" \
        -not -path "./.esphome/*" \
        -exec sed -i '' "s/$OLD_USERNAME/$NEW_USERNAME/g" {} \;
else
    # Linux
    find . -type f \( -name "*.py" -o -name "*.md" -o -name "*.sh" \) \
        -not -path "./.git/*" \
        -not -path "./.esphome/*" \
        -exec sed -i "s/$OLD_USERNAME/$NEW_USERNAME/g" {} \;
fi

echo "✅ Username cambiato con successo!"
echo ""
echo "📋 Verifica le modifiche:"
grep -r "$NEW_USERNAME" . --exclude-dir=.git --exclude-dir=.esphome --exclude="*.pyc" -l 2>/dev/null

echo ""
echo "✅ Fatto! Ora il repository usa l'username: $NEW_USERNAME"
