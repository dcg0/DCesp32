#pragma once
/*
 * ESP32-OBDC2 · dc_parsers.h · utilidades comunes para parsers DTC y VIN (fase 05-06c)
 * Basado en ESP32-OBD2-Gauge de VaAndCob (MIT No Commercial Use).
 * El ELM327 separa líneas con '\r' (no '\n'); aquí se parten ambos.
 */

static bool dcEsHexTodo(const String& s) {
  if (!s.length()) return false;
  for (unsigned i = 0; i < s.length(); i++) {
    char c = s[i];
    if (!((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F'))) return false;
  }
  return true;
}

// Divide la respuesta en líneas limpias (sin espacios), separando por '\r' o '\n'.
// Devuelve las líneas unidas con '|' y marca si hay formato ISO-TP (prefijos "0:", "1:"...).
static String dcLineasLimpias(const String& r, bool& iso) {
  String salida, linea;
  iso = false;
  for (unsigned i = 0; i <= r.length(); i++) {
    char c = (i < r.length()) ? r[i] : '\n';
    if (c == '\r' || c == '\n') {
      String l = dcLimpia(linea);
      linea = "";
      if (!l.length()) continue;
      if (l.length() >= 2 && l[1] == ':') iso = true;
      salida += l;
      salida += '|';
    } else {
      linea += c;
    }
  }
  return salida;
}

// Quita prefijos ISO-TP y líneas de conteo; si es ISO-TP concatena todo en una sola cadena.
static String dcUneLineas(const String& lineas, bool iso) {
  String unido, l;
  for (unsigned i = 0; i < lineas.length(); i++) {
    if (lineas[i] != '|') { l += lineas[i]; continue; }
    if (l.length() >= 2 && l[1] == ':') l = l.substring(2);
    else if (iso && l.length() == 3 && dcEsHexTodo(l)) { l = ""; continue; }  // línea de conteo, p. ej. 014
    unido += l;
    if (!iso) unido += '|';
    l = "";
  }
  return unido;
}
