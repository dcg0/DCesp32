#!/bin/bash
# Uso: ./compilar.sh sim    -> modo simulador (sin ELM327)
#      ./compilar.sh real   -> modo Bluetooth real
set -euo pipefail
export PATH="$HOME/.local/bin:$PATH"
cd "$(dirname "$0")/.."
MODO=${1:-sim}
FQBN="esp32:esp32:esp32:PartitionScheme=huge_app"
case "$MODO" in
  sim)
    arduino-cli compile --fqbn "$FQBN" --build-property "compiler.cpp.extra_flags=-DSIMULATOR" ESP32-OBDC2
    ;;
  real)
    arduino-cli compile --fqbn "$FQBN" ESP32-OBDC2
    ;;
  *)
    printf 'Uso: %s [sim|real]\n' "$0" >&2
    exit 2
    ;;
esac
