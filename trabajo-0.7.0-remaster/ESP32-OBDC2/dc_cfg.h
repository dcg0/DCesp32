#pragma once
/*
 * ESP32-OBDC2 · dc_cfg.h · Fase 14 · Configuración guardada en el ESP32 (Preferences)
 * Basado en ESP32-OBD2-Gauge de VaAndCob (MIT No Commercial Use).
 * Los umbrales se guardan SIEMPRE en unidades base (km/h, °C, kPa). Las unidades (mph, °F, psi)
 * son solo de presentación: la web convierte al mostrar. No cambia ninguna fórmula interna.
 * Todo valor se valida aquí en el ESP32: si algo es inválido NO se guarda nada.
 */
#include <Preferences.h>
#include "dc_data.h"

static Preferences dcCfgPref;
static char dcCfgAdaptador[24] = "OBDII";
static char dcCfgSsid[33] = "DC-OBD";
static char dcCfgPass[33] = "12345678";
static uint8_t dcCfgUV = 0, dcCfgUT = 0, dcCfgUP = 0;  // 0 = km/h, °C, kPa · 1 = mph, °F, psi
static float dcCfgVoltBajo = 11.8f;                    // 0 = alerta de voltaje bajo desactivada
static float dcCfgWarnDefecto[NPIDS];

typedef String (*DcArg)(const String&);  // lector de argumentos del formulario

static bool dcCfgNumero(const String& s, float& v, float mn, float mx) {
  if (!s.length()) return false;
  char* fin = nullptr;
  v = strtof(s.c_str(), &fin);
  if (*fin) return false;
  return v >= mn && v <= mx;
}

// Nombre/SSID: 1..max caracteres entre letras, números, espacio, '_', '-', '.'
static bool dcCfgTextoOk(const String& s, unsigned max) {
  if (!s.length() || s.length() > max) return false;
  for (unsigned i = 0; i < s.length(); i++) {
    char c = s[i];
    bool ok = (c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == ' ' || c == '_' || c == '-' || c == '.';
    if (!ok) return false;
  }
  return true;
}

// Clave WiFi: 8..32 caracteres ASCII imprimibles, sin comillas ni barra invertida
static bool dcCfgClaveOk(const String& s) {
  if (s.length() < 8 || s.length() > 32) return false;
  for (unsigned i = 0; i < s.length(); i++) {
    char c = s[i];
    if (c < 32 || c > 126 || c == '"' || c == '\\') return false;
  }
  return true;
}

static void dcCfgCargaDefectos() {
  for (uint8_t i = 0; i < NPIDS; i++) DCPids[i].warn = dcCfgWarnDefecto[i];
  dcCfgUV = dcCfgUT = dcCfgUP = 0;
  dcCfgVoltBajo = 11.8f;
  strcpy(dcCfgAdaptador, "OBDII");
  strcpy(dcCfgSsid, "DC-OBD");
  strcpy(dcCfgPass, "12345678");
}

void dcCfgBegin() {
  static bool defectosCapturados = false;  // los valores de fábrica se capturan una sola vez
  if (!defectosCapturados) {
    for (uint8_t i = 0; i < NPIDS; i++) dcCfgWarnDefecto[i] = DCPids[i].warn;
    defectosCapturados = true;
  }
  dcCfgPref.begin("dccfg", false);
  for (uint8_t i = 0; i < NPIDS; i++) {
    String k = String("w_") + DCPids[i].id;
    if (dcCfgPref.isKey(k.c_str())) DCPids[i].warn = dcCfgPref.getFloat(k.c_str(), DCPids[i].warn);
  }
  dcCfgUV = dcCfgPref.getUChar("uv", 0) ? 1 : 0;
  dcCfgUT = dcCfgPref.getUChar("ut", 0) ? 1 : 0;
  dcCfgUP = dcCfgPref.getUChar("up", 0) ? 1 : 0;
  dcCfgVoltBajo = dcCfgPref.getFloat("vb", 11.8f);
  String ad = dcCfgPref.getString("ad", "OBDII");
  String ss = dcCfgPref.getString("ss", "DC-OBD");
  String pw = dcCfgPref.getString("pw", "12345678");
  if (dcCfgTextoOk(ad, 23)) strcpy(dcCfgAdaptador, ad.c_str());
  if (dcCfgTextoOk(ss, 32)) strcpy(dcCfgSsid, ss.c_str());
  if (dcCfgClaveOk(pw)) strcpy(dcCfgPass, pw.c_str());
}

// GET /config  (la clave WiFi NUNCA se devuelve)
String dcCfgEstadoJson() {
  String s;
  s.reserve(420);
  s += "{\"warn\":{";
  for (uint8_t i = 0; i < NPIDS; i++) {
    if (i) s += ',';
    s += '"'; s += DCPids[i].id; s += "\":"; s += String(DCPids[i].warn, 1);
  }
  s += "},\"vb\":"; s += String(dcCfgVoltBajo, 1);
  s += ",\"uv\":"; s += (int)dcCfgUV;
  s += ",\"ut\":"; s += (int)dcCfgUT;
  s += ",\"up\":"; s += (int)dcCfgUP;
  s += ",\"ad\":\""; s += dcCfgAdaptador;
  s += "\",\"ssid\":\""; s += dcCfgSsid;
  s += "\"}";
  return s;
}

// POST /config: valida TODO primero; si algo falla devuelve false y no guarda nada.
bool dcCfgGuardar(DcArg arg, String& error) {
  float warn[NPIDS];
  bool hayWarn[NPIDS];
  for (uint8_t i = 0; i < NPIDS; i++) {
    String v = arg(String("w_") + DCPids[i].id);
    hayWarn[i] = v.length() > 0;
    if (hayWarn[i] && !dcCfgNumero(v, warn[i], 0, 10000)) { error = String("umbral invalido: ") + DCPids[i].nombre; return false; }
  }
  float vb = dcCfgVoltBajo;
  String sv = arg("vb");
  bool hayVb = sv.length() > 0;
  if (hayVb && !dcCfgNumero(sv, vb, 0, 16)) { error = "voltaje bajo invalido (0 a 16)"; return false; }
  uint8_t uv = dcCfgUV, ut = dcCfgUT, up = dcCfgUP;
  String x;
  if ((x = arg("uv")).length()) { if (x != "0" && x != "1") { error = "unidad de velocidad invalida"; return false; } uv = x.toInt(); }
  if ((x = arg("ut")).length()) { if (x != "0" && x != "1") { error = "unidad de temperatura invalida"; return false; } ut = x.toInt(); }
  if ((x = arg("up")).length()) { if (x != "0" && x != "1") { error = "unidad de presion invalida"; return false; } up = x.toInt(); }
  String ad = arg("ad"), ss = arg("ss"), pw = arg("pw");
  if (ad.length() && !dcCfgTextoOk(ad, 23)) { error = "nombre de adaptador invalido (1-23: letras, numeros, espacio, _ - .)"; return false; }
  if (ss.length() && !dcCfgTextoOk(ss, 32)) { error = "SSID invalido (1-32: letras, numeros, espacio, _ - .)"; return false; }
  if (pw.length() && !dcCfgClaveOk(pw)) { error = "clave WiFi invalida (8-32 caracteres, sin comillas ni barra invertida)"; return false; }

  // todo validado: aplicar y guardar
  for (uint8_t i = 0; i < NPIDS; i++) {
    if (!hayWarn[i]) continue;
    DCPids[i].warn = warn[i];
    dcCfgPref.putFloat((String("w_") + DCPids[i].id).c_str(), warn[i]);
  }
  if (hayVb) { dcCfgVoltBajo = vb; dcCfgPref.putFloat("vb", vb); }
  dcCfgUV = uv; dcCfgUT = ut; dcCfgUP = up;
  dcCfgPref.putUChar("uv", uv); dcCfgPref.putUChar("ut", ut); dcCfgPref.putUChar("up", up);
  if (ad.length() && ad != dcCfgAdaptador) {
    strcpy(dcCfgAdaptador, ad.c_str());
    dcCfgPref.putString("ad", ad);
    Preferences p;  // otro adaptador = otra MAC: olvidar la guardada
    p.begin("dcobd", false); p.remove("recent_client"); p.end();
  }
  if (ss.length()) { strcpy(dcCfgSsid, ss.c_str()); dcCfgPref.putString("ss", ss); }
  if (pw.length()) { strcpy(dcCfgPass, pw.c_str()); dcCfgPref.putString("pw", pw); }
  return true;
}

void dcCfgReset() {
  dcCfgPref.clear();
  dcCfgCargaDefectos();
}
