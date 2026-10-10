#pragma once
/*
 * ESP32-OBDC2 · dc_alert.h · Fase 9 · Asistente de alertas (no hace consultas nuevas al ELM327)
 * Lee solo DCPids[], dcBt y dcDtcMil. Basado en ESP32-OBD2-Gauge de VaAndCob (MIT No Commercial Use).
 *
 * Reglas:
 *  - Cada PID con umbral (warn > 0): AVISO al llegar al umbral y CRITICO al 105 % del umbral.
 *    Debe mantenerse DC_AL_DURACION_MS para activarse y baja con histéresis del 3 % (evita parpadeo).
 *  - Voltaje bajo: motor en marcha (RPM > 500) y voltaje < dcCfgVoltBajo (configurable, 0 = desactivada).
 *  - Conexión: tras haber estado LISTO, si pasa 5 s sin estarlo (desconectado o sin respuesta del auto) -> CRITICO.
 *  - Luz MIL: solo si ya se leyeron las fallas y la MIL estaba encendida.
 */
#include <string.h>
#include "dc_data.h"
#include "dc_cfg.h"

#define DC_AL_DURACION_MS 2000
#define DC_AL_BT_DURACION_MS 5000
#define DC_AL_HIST 0.03f
#define DC_AL_CRIT 1.05f

struct DcAlerta { bool activa; uint8_t nivel; uint32_t t0; };
static const uint8_t DC_AL_N = NPIDS + 3;
static const uint8_t DC_AL_VBAJO = NPIDS, DC_AL_BT = NPIDS + 1, DC_AL_MIL = NPIDS + 2;
static DcAlerta dcAl[DC_AL_N];
static bool dcAlVistoListo = false;
static uint32_t dcAlT = 0;

static bool dcAlFresco(const DCPid& p) { return p.valido && millis() - p.ts < 20000; }

static DCPid* dcAlPid(const char* id) {
  for (uint8_t i = 0; i < NPIDS; i++) if (strcmp(DCPids[i].id, id) == 0) return &DCPids[i];
  return nullptr;
}

static void dcAlEval(DcAlerta& a, bool cond, bool libre, uint8_t nivel, uint32_t durMs, uint32_t now) {
  if (!a.activa) {
    if (cond) {
      if (!a.t0) a.t0 = now ? now : 1;
      if (now - a.t0 >= durMs) { a.activa = true; a.nivel = nivel; }
    } else {
      a.t0 = 0;
    }
  } else {
    if (libre) { a.activa = false; a.t0 = 0; }
    else if (cond) a.nivel = nivel;
  }
}

void dcAlertTick() {
  uint32_t now = millis();
  if (now - dcAlT < 200) return;
  dcAlT = now;

  for (uint8_t i = 0; i < NPIDS; i++) {
    const DCPid& p = DCPids[i];
    if (p.warn <= 0) { dcAl[i].activa = false; dcAl[i].t0 = 0; continue; }
    bool fr = dcAlFresco(p);
    bool cond = fr && p.valor >= p.warn;
    bool libre = !fr || p.valor < p.warn - p.warn * DC_AL_HIST;
    dcAlEval(dcAl[i], cond, libre, (fr && p.valor >= p.warn * DC_AL_CRIT) ? 2 : 1, DC_AL_DURACION_MS, now);
  }

  DCPid* rpm = dcAlPid("rpm");
  DCPid* volt = dcAlPid("volt");
  bool run = rpm && volt && dcAlFresco(*rpm) && dcAlFresco(*volt) && rpm->valor > 500;
  bool vcond = run && dcCfgVoltBajo > 0 && volt->valor < dcCfgVoltBajo;
  bool vlibre = !run || dcCfgVoltBajo <= 0 || volt->valor >= dcCfgVoltBajo + 0.3f;
  dcAlEval(dcAl[DC_AL_VBAJO], vcond, vlibre, 1, DC_AL_DURACION_MS, now);

  if (dcBt == BT_LISTO) dcAlVistoListo = true;
  dcAlEval(dcAl[DC_AL_BT], dcAlVistoListo && dcBt != BT_LISTO, dcBt == BT_LISTO, 2, DC_AL_BT_DURACION_MS, now);

  bool mil = (dcDtcMil == 1);
  dcAlEval(dcAl[DC_AL_MIL], mil, !mil, 1, 0, now);
}

// GET /alertas -> {"a":[{"id":"ect","n":"Refrigerante","t":"alto","nv":2,"p":"ect","v":101.2}],"n":1}
String dcAlertJson() {
  String s;
  s.reserve(300);
  s += "{\"a\":[";
  uint8_t n = 0;
  for (uint8_t i = 0; i < DC_AL_N; i++) {
    if (!dcAl[i].activa) continue;
    if (n++) s += ',';
    s += "{\"id\":\"";
    if (i < NPIDS) {
      s += DCPids[i].id; s += "\",\"n\":\""; s += DCPids[i].nombre; s += "\",\"t\":\"alto\",\"p\":\""; s += DCPids[i].id;
      s += "\",\"v\":"; s += String(DCPids[i].valor, 1);
    } else if (i == DC_AL_VBAJO) {
      DCPid* v = dcAlPid("volt");
      s += "volt_bajo\",\"n\":\"Voltaje\",\"t\":\"bajo\",\"p\":\"volt\",\"v\":"; s += String(v ? v->valor : 0, 1);
    } else if (i == DC_AL_BT) {
      s += "bt\",\"n\":\"Conexion\",\"t\":\""; s += (dcBt == BT_ERROR ? "sin respuesta del auto" : "Bluetooth desconectado");
      s += "\",\"p\":null,\"v\":null";
    } else {
      s += "mil\",\"n\":\"Luz MIL\",\"t\":\"encendida (hay fallas)\",\"p\":null,\"v\":null";
    }
    s += ",\"nv\":"; s += (int)dcAl[i].nivel; s += "}";
  }
  s += "],\"n\":"; s += (int)n; s += "}";
  return s;
}
