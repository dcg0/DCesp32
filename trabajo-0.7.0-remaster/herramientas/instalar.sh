#!/bin/bash
# Instala arduino-cli y el core ESP32 2.0.17 (necesita internet). Ejecutar UNA vez.
set -euo pipefail
mkdir -p "$HOME/.local/bin"
export PATH="$HOME/.local/bin:$PATH"
if ! command -v arduino-cli >/dev/null; then
  installer=$(mktemp)
  trap 'rm -f "$installer"' EXIT
  curl -fsSL --proto '=https' --tlsv1.2 \
    -o "$installer" https://raw.githubusercontent.com/arduino/arduino-cli/v1.5.1/install.sh
  printf '%s  %s\n' \
    dfa8aa41202a3752f1a716d56584c24e7e93087b94c4ae8c50df50012ea6095d \
    "$installer" | sha256sum --check --status
  BINDIR="$HOME/.local/bin" sh "$installer"
fi
arduino-cli config init 2>/dev/null || true
arduino-cli config add board_manager.additional_urls https://espressif.github.io/arduino-esp32/package_esp32_index.json
arduino-cli core update-index
arduino-cli core install esp32:esp32@2.0.17
sudo usermod -aG dialout "$USER" || true
grep -q '.local/bin' "$HOME/.bashrc" || echo 'export PATH="$HOME/.local/bin:$PATH"' >> "$HOME/.bashrc"
echo "LISTO. Cierra la sesión y vuelve a entrar (para el permiso dialout)."
