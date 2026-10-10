// Prueba de parsers en PC/Termux/Dell (sin ESP32). Usa un String de mentira (shim_arduino_pc.h).
#include "shim_arduino_pc.h"
#include "../ESP32-OBDC2/dc_parsers.h"
#include "../ESP32-OBDC2/dc_dtc.h"
#include "../ESP32-OBDC2/dc_vin.h"
#include "../ESP32-OBDC2/dc_parser_tests.h"
int main() { return dcEjecutarPruebasParsers() ? 0 : 1; }
