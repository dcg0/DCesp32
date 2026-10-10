#!/bin/bash
# Uso: ./cargar.sh sim|real [puerto]   (puerto por defecto /dev/ttyUSB0)
export PATH="$HOME/.local/bin:$PATH"
cd "$(dirname "$0")/.."
MODO=${1:-sim}; PUERTO=${2:-/dev/ttyUSB0}
FQBN="esp32:esp32:esp32:PartitionScheme=huge_app"
./herramientas/compilar.sh "$MODO" || exit 1
arduino-cli upload -p "$PUERTO" --fqbn "$FQBN" ESP32-OBDC2
