#!/bin/bash
# Prueba los parsers DTC/VIN en la Dell/Termux, sin ESP32. Necesita g++ (apt install g++ / pkg install clang).
cd "$(dirname "$0")"
g++ -std=c++17 -w test_pc.cpp -o /tmp/test_parsers && /tmp/test_parsers
