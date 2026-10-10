#pragma once
// ESP32-OBDC2 · dc_data.h · modelo de datos único (todo lo demás lee de aquí)
// Basado en ESP32-OBD2-Gauge de VaAndCob (MIT No Commercial Use)
#include <Arduino.h>

enum BtEstado : uint8_t { BT_DESCONECTADO, BT_CONECTANDO, BT_INICIALIZANDO, BT_LISTO, BT_ERROR };
enum Formula : uint8_t { F_PCT, F_TEMP, F_RPM, F_VOLT, F_MAF, F_TRIM, F_RAW };

struct DCPid {
  const char* id;      // clave corta en el JSON
  const char* nombre;
  const char* unidad;
  const char* cmd;     // modo 01 + PID
  Formula formula;
  uint8_t bytes;       // bytes de dato que usa la fórmula (1 = solo A, 2 = A y B)
  uint8_t periodo;     // se consulta 1 de cada N ciclos (1 = rápido, 10 = lento)
  float warn;          // 0 = sin alerta
  float valor;
  bool valido;
  bool soportado;      // lo llena la detección 0100/0120/0140
  uint32_t ts;
};

// warn: valores de Va&Cob (80, 99, 35 psi ≈ 240 kPa, 4000, 15, 99) donde aplican
DCPid DCPids[] = {
  { "rpm",  "RPM",             "rpm",  "010C", F_RPM,  2, 1,  4000, 0, false, false, 0 },
  { "vel",  "Velocidad",       "km/h", "010D", F_RAW,  1, 1,  0,    0, false, false, 0 },
  { "acel", "Acelerador",      "%",    "0111", F_PCT,  1, 1,  0,    0, false, false, 0 },
  { "carga","Carga motor",     "%",    "0104", F_PCT,  1, 2,  80,   0, false, false, 0 },
  { "ect",  "Refrigerante",    "°C",   "0105", F_TEMP, 1, 5,  99,   0, false, false, 0 },
  { "map",  "MAP",             "kPa",  "010B", F_RAW,  1, 2,  240,  0, false, false, 0 },
  { "maf",  "MAF",             "g/s",  "0110", F_MAF,  2, 2,  0,    0, false, false, 0 },
  { "iat",  "Aire admisión",   "°C",   "010F", F_TEMP, 1, 5,  0,    0, false, false, 0 },
  { "volt", "Voltaje",         "V",    "0142", F_VOLT, 2, 5,  15,   0, false, false, 0 },
  { "stft", "Trim corto B1",   "%",    "0106", F_TRIM, 1, 3,  0,    0, false, false, 0 },
  { "ltft", "Trim largo B1",   "%",    "0107", F_TRIM, 1, 3,  0,    0, false, false, 0 },
  { "comb", "Combustible",     "%",    "012F", F_PCT,  1, 10, 0,    0, false, false, 0 },
  { "oil",  "Aceite",          "°C",   "015C", F_TEMP, 1, 10, 99,   0, false, false, 0 },
};
const uint8_t NPIDS = sizeof(DCPids) / sizeof(DCPids[0]);

BtEstado dcBt = BT_DESCONECTADO;
uint8_t dcPps = 0;  // PIDs leídos por segundo

float dcDecodificar(Formula f, uint8_t A, uint8_t B) {
  switch (f) {
    case F_PCT:  return A * 100.0f / 255.0f;
    case F_TEMP: return (float)A - 40.0f;
    case F_RPM:  return (256 * A + B) / 4.0f;
    case F_VOLT: return (256 * A + B) / 1000.0f;
    case F_MAF:  return (256 * A + B) / 100.0f;
    case F_TRIM: return A * 100.0f / 128.0f - 100.0f;
    default:     return (float)A;
  }
}

void dcInvalidarTodo() {
  for (uint8_t i = 0; i < NPIDS; i++) DCPids[i].valido = false;
}

// /data -> {"bt":3,"pps":12,"up":340,"d":{"rpm":850.0,"vel":null,...}}
String dcJson() {
  String s;
  s.reserve(480);
  s += "{\"fw\":\""; s += FW_VERSION; s += "\",\"bt\":"; s += (int)dcBt;
  s += ",\"pps\":"; s += (int)dcPps;
  s += ",\"up\":"; s += (unsigned long)(millis() / 1000);
  s += ",\"d\":{";
  for (uint8_t i = 0; i < NPIDS; i++) {
    const DCPid& p = DCPids[i];
    if (i) s += ',';
    s += '"'; s += p.id; s += "\":";
    if (p.valido && millis() - p.ts < 20000) s += String(p.valor, 1);
    else s += "null";
  }
  s += "}}";
  return s;
}

// /cfg -> nombres, unidades y alertas (una sola fuente de verdad para la web)
String dcCfgJson() {
  String s = "[";
  for (uint8_t i = 0; i < NPIDS; i++) {
    const DCPid& p = DCPids[i];
    if (i) s += ',';
    s += "{\"id\":\""; s += p.id;
    s += "\",\"n\":\""; s += p.nombre;
    s += "\",\"u\":\""; s += p.unidad;
    s += "\",\"w\":"; s += String(p.warn, 0);
    s += "}";
  }
  s += "]";
  return s;
}

// /status -> diagnóstico mínimo sin tocar el planificador ELM
String dcStatusJson() {
  String s;
  s.reserve(220);
  s += "{\"fw\":\""; s += FW_VERSION;
  s += "\",\"bt\":"; s += (int)dcBt;
  s += ",\"pps\":"; s += (int)dcPps;
  s += ",\"up\":"; s += (unsigned long)(millis() / 1000);
  s += ",\"heap\":"; s += (unsigned long)ESP.getFreeHeap();
  s += "}";
  return s;
}
