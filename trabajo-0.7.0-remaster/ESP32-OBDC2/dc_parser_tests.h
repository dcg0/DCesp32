#pragma once
/*
 * ESP32-OBDC2 · pruebas de parsers (DTC y VIN) · crédito VaAndCob: MIT No Commercial Use.
 * Usan '\r' como separador de línea, igual que el ELM327 real.
 * Se ejecutan al arrancar con SIMULATOR (monitor serie 115200) y en PC con herramientas/test_parsers.sh.
 */

static String dcPruebaDtc(const char* resp, const char* cab) {
  DcDtcItem lista[8];
  uint8_t n = 0;
  dcDtcParse(resp, lista, n, cab);
  String s;
  for (uint8_t i = 0; i < n; i++) { s += lista[i].codigo; s += " "; }
  return s;
}

static String dcPruebaVin(const char* resp) {
  dcVin = "";
  dcVinParse(resp);
  return dcVin;
}

static uint8_t dcPruebasFallos = 0;
static void dcCaso(const char* nombre, const String& obtenido, const char* esperado) {
  bool ok = (obtenido == String(esperado));
  if (!ok) dcPruebasFallos++;
  Serial.print(ok ? "OK    " : "FALLO ");
  Serial.print(nombre);
  Serial.print(" -> [");
  Serial.print(obtenido.c_str());
  Serial.println("]");
}

static bool dcEjecutarPruebasParsers() {
  dcPruebasFallos = 0;
  dcCaso("DTC sin conteo 43 01 33 00 00 00 00", dcPruebaDtc("43 01 33 00 00 00 00", "43"), "P0133 ");
  dcCaso("DTC con conteo 43 01 01 33", dcPruebaDtc("43 01 01 33", "43"), "P0133 ");
  dcCaso("DTC 2 codigos 43 02 01 33 03 00", dcPruebaDtc("43 02 01 33 03 00", "43"), "P0133 P0300 ");
  dcCaso("DTC sin fallas 43 00", dcPruebaDtc("43 00", "43"), "");
  dcCaso("DTC 2 ECU", dcPruebaDtc("43 01 01 33\r43 01 02 45\r", "43"), "P0133 P0245 ");
  dcCaso("DTC C/B/U", dcPruebaDtc("43 03 C1 00 81 23 41 11", "43"), "U0100 B0123 C0111 ");
  dcCaso("DTC pendientes 47", dcPruebaDtc("47 01 03 00", "47"), "P0300 ");
  dcCaso("DTC ISO-TP multitrama", dcPruebaDtc("00A\r0: 43 04 01 33 01 34\r1: 01 35 01 36\r", "43"),
         "P0133 P0134 P0135 P0136 ");
  dcCaso("VIN CAN ELM real", dcPruebaVin("014\r0: 49 02 01 57 50 30\r1: 5A 5A 5A 39 39 5A\r2: 54 53 33 39 32 30 30 31\r"),
         "WP0ZZZ99ZTS392001");
  dcCaso("VIN CAN con PCI", dcPruebaVin("014\r0: 10 14 49 02 01 57 50 30 5A 5A 5A 39 39\r1: 21 5A 54 53 33 39 32 30 30 31\r"),
         "WP0ZZZ99ZTS392001");
  dcCaso("VIN no-CAN 5 lineas", dcPruebaVin("49 02 01 00 00 00 57\r49 02 02 50 30 5A 5A\r49 02 03 5A 39 39 5A\r49 02 04 54 53 33 39\r49 02 05 32 30 30 31\r"),
         "WP0ZZZ99ZTS392001");
  dcCaso("VIN basura", dcPruebaVin("NO DATA\r"), "");
  dcVin = "";
  Serial.print("PRUEBAS PARSERS: fallos = ");
  Serial.println((int)dcPruebasFallos);
  return dcPruebasFallos == 0;
}
