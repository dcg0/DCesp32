#!/bin/bash
# Monitor serie a 115200. Salir con Ctrl+C. Uso: ./monitor.sh [puerto]
export PATH="$HOME/.local/bin:$PATH"
arduino-cli monitor -p "${1:-/dev/ttyUSB0}" -c baudrate=115200
