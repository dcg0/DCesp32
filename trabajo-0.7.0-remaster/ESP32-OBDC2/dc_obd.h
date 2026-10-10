/*
 * ESP32-OBDC2 · Motor ELM327 no bloqueante
 * Basado en ESP32-OBD2-Gauge de VaAndCob (MIT No Commercial Use).
 * Mantiene el nombre OBDII, la MAC en Preferences y la secuencia AT original.
 * El planificador es el único código que transmite al ELM327.
 */
#pragma once
#include "dc_data.h"
#include <BluetoothSerial.h>
#include <Preferences.h>
#define DC_BT_ROLE ESP_SPP_ROLE_MASTER
static BluetoothSerial BTSerial;
static Preferences dcPref;
static const char *DC_ADAPTADOR = dcCfgAdaptador;  // configurable desde la web (Fase 14)
static const uint8_t DC_INIT_N = 8;
static const char *dcInit[DC_INIT_N] = {"ATZ",  "ATBRD23", "ATSP0", "ATAT2",
                                        "ATL0", "ATH0",    "ATE0",  "0100"};
enum DcFase : uint8_t { FASE_CONECTAR, FASE_INIT, FASE_SOPORTE, FASE_CICLO };
enum DcPqEstado : uint8_t { PQ_LIBRE, PQ_PENDIENTE, PQ_ENVIADA, PQ_LISTA, PQ_TIMEOUT };
struct DcPuntual {
  String cmd;
  uint16_t timeout;
  String rx;
  DcPqEstado estado;
};
static DcPuntual dcQ[4];
static DcFase dcFase = FASE_CONECTAR;
static uint8_t dcPaso = 0, dcIdx = 0, dcFallos = 0, dcPpsCont = 0, dcPuntualActiva = 255;
static uint32_t dcCiclo = 0, dcTEnvio = 0, dcTIntento = 0, dcTError = 0, dcPpsT = 0;
static uint16_t dcTimeout = 0;
static bool dcEsperando = false;
static String dcRx;
static uint8_t dcAddr[6] = {0};
static bool dcTieneAddr = false;
static int8_t dcPedir(const char *cmd, uint16_t timeout = 1800) {
  for (uint8_t i = 0; i < 4; i++)
    if (dcQ[i].estado != PQ_LIBRE && dcQ[i].cmd == cmd)
      return i;
  for (uint8_t i = 0; i < 4; i++)
    if (dcQ[i].estado == PQ_LIBRE) {
      dcQ[i].cmd = cmd;
      dcQ[i].timeout = timeout;
      dcQ[i].rx = "";
      dcQ[i].estado = PQ_PENDIENTE;
      return i;
    }
  return -1;
}
static DcPqEstado dcEstado(uint8_t i) { return i < 4 ? dcQ[i].estado : PQ_LIBRE; }
static String dcRespuesta(uint8_t i) { return i < 4 ? dcQ[i].rx : String(); }
static void dcLiberar(uint8_t i) {
  if (i < 4)
    dcQ[i].estado = PQ_LIBRE;
}
static void dcEnviar(const String &c, uint16_t to) {
  while (BTSerial.available())
    BTSerial.read();
  dcRx = "";
  BTSerial.print(c + "\r");
  dcTEnvio = millis();
  dcTimeout = to;
  dcEsperando = true;
}
static String dcLimpia(const String &r) {
  String o;
  o.reserve(r.length());
  for (uint16_t i = 0; i < r.length(); i++) {
    char c = r[i];
    if (c != ' ' && c != '\r' && c != '\n' && c != '\t')
      o += c;
  }
  o.toUpperCase();
  return o;
}
static bool dcHex(const String &s, int i, uint8_t &v) {
  if (i < 0 || i + 2 > (int)s.length())
    return false;
  char b[3] = {s[i], s[i + 1], 0};
  char *e = nullptr;
  long n = strtol(b, &e, 16);
  if (!e || *e)
    return false;
  v = (uint8_t)n;
  return true;
}
static bool dcExtraer(const String &r, const char *cmd, uint8_t bytes, uint8_t &A, uint8_t &B) {
  String c = dcLimpia(r), esp = "41";
  esp += (cmd + 2);
  int i = c.indexOf(esp);
  if (i < 0)
    return false;
  i += 4;
  if (!dcHex(c, i, A))
    return false;
  B = 0;
  return bytes < 2 || dcHex(c, i + 2, B);
}
static bool dcIntentaConectar(uint8_t *addr) {
  BTSerial.connect(addr, 0, ESP_SPP_SEC_NONE, DC_BT_ROLE);
  if (BTSerial.connected(3000))
    return true;
  BTSerial.disconnect();
  return false;
}
static bool dcConectar() {
  dcBt = BT_CONECTANDO;
  if (dcTieneAddr && dcIntentaConectar(dcAddr))
    return true;
  BTScanResults *r = BTSerial.discover(6000);
  if (!r)
    return false;
  for (int i = 0; i < r->getCount(); i++) {
    BTAdvertisedDevice *d = r->getDevice(i);
    if (String(d->getName().c_str()) == DC_ADAPTADOR) {
      memcpy(dcAddr, d->getAddress().getNative(), 6);
      if (dcIntentaConectar(dcAddr)) {
        dcPref.putBytes("recent_client", dcAddr, 6);
        dcTieneAddr = true;
        return true;
      }
    }
  }
  return false;
}
static void dcError() {
  dcBt = BT_ERROR;
  dcTError = millis();
  dcInvalidarTodo();
  dcEsperando = false;
}
static void dcProcesa(bool timeout) {
  if (dcFase == FASE_INIT) {
    dcPaso++;
    if (dcPaso >= DC_INIT_N) {
      dcFase = FASE_SOPORTE;
      dcPaso = 0;
    }
    return;
  }
  if (dcFase == FASE_SOPORTE) {
    static const char *rango[3] = {"00", "20", "40"};
    String c = dcLimpia(dcRx), esp = "41" + String(rango[dcPaso]);
    int i = c.indexOf(esp);
    bool ok = !timeout && i >= 0 && (int)c.length() >= i + 12;
    if (!ok) {
      if (dcPaso == 0) {
        dcError();
        return;
      }
    } else {
      uint32_t mask = strtoul(c.substring(i + 4, i + 12).c_str(), nullptr, 16);
      uint8_t base = dcPaso * 0x20;
      for (uint8_t k = 0; k < NPIDS; k++) {
        uint8_t n = strtol(DCPids[k].cmd + 2, nullptr, 16);
        if (n > base && n <= base + 0x20 && ((mask >> (32 - (n - base))) & 1))
          DCPids[k].soportado = true;
      }
      if ((mask & 1) && dcPaso < 2) {
        dcPaso++;
        return;
      }
    }
    dcBt = BT_LISTO;
    dcFase = FASE_CICLO;
    dcIdx = 0;
    dcCiclo = 0;
    dcFallos = 0;
    return;
  }
  if (dcFase == FASE_CICLO) {
    DCPid &p = DCPids[dcIdx];
    uint8_t A, B;
    if (!timeout && dcExtraer(dcRx, p.cmd, p.bytes, A, B)) {
      p.valor = dcDecodificar(p.formula, A, B);
      p.valido = true;
      p.ts = millis();
      dcPpsCont++;
      dcFallos = 0;
    } else {
      p.valido = false;
      if (++dcFallos >= 10) {
        dcError();
        return;
      }
    }
    dcIdx++;
  }
}
static void dcFinalPuntual(bool timeout) {
  if (dcPuntualActiva >= 4)
    return;
  DcPuntual &q = dcQ[dcPuntualActiva];
  q.rx = dcRx;
  q.estado = timeout ? PQ_TIMEOUT : PQ_LISTA;
  dcPuntualActiva = 255;
  dcEsperando = false;
}
static bool dcSiguientePuntual() {
  for (uint8_t i = 0; i < 4; i++)
    if (dcQ[i].estado == PQ_PENDIENTE) {
      dcPuntualActiva = i;
      dcQ[i].estado = PQ_ENVIADA;
      dcEnviar(dcQ[i].cmd, dcQ[i].timeout);
      return true;
    }
  return false;
}
static void dcSiguiente() {
  if (dcSiguientePuntual())
    return;
  if (dcFase == FASE_INIT)
    dcEnviar(dcInit[dcPaso], dcPaso == 0 ? 2500 : (dcPaso == 7 ? 10000 : 800));
  else if (dcFase == FASE_SOPORTE) {
    static const char *cmd[3] = {"0100", "0120", "0140"};
    dcEnviar(cmd[dcPaso], 4000);
  } else if (dcFase == FASE_CICLO) {
    for (uint8_t k = 0; k < NPIDS; k++) {
      if (dcIdx >= NPIDS) {
        dcIdx = 0;
        dcCiclo++;
      }
      DCPid &p = DCPids[dcIdx];
      if (p.soportado && (dcCiclo % p.periodo) == 0) {
        dcEnviar(p.cmd, 600);
        return;
      }
      dcIdx++;
    }
  }
}
void dcObdBegin() {
  dcPref.begin("dcobd", false);
  if (dcPref.getBytesLength("recent_client") == 6) {
    dcPref.getBytes("recent_client", dcAddr, 6);
    dcTieneAddr = true;
  }
  for (uint8_t i = 0; i < 4; i++)
    dcQ[i].estado = PQ_LIBRE;
  BTSerial.begin("ESP32-OBDC2", true);
  dcBt = BT_DESCONECTADO;
}
void dcObdLoop() {
  uint32_t now = millis();
  if (now - dcPpsT >= 1000) {
    dcPps = dcPpsCont;
    dcPpsCont = 0;
    dcPpsT = now;
  }
  if (!BTSerial.connected()) {
    if (dcBt != BT_DESCONECTADO && dcBt != BT_CONECTANDO)
      dcInvalidarTodo();
    dcBt = BT_DESCONECTADO;
    dcFase = FASE_CONECTAR;
    dcEsperando = false;
    if (now - dcTIntento >= 5000) {
      if (dcConectar()) {
        dcBt = BT_INICIALIZANDO;
        dcFase = FASE_INIT;
        dcPaso = 0;
      }
      dcTIntento = millis();
    }
    return;
  }
  if (dcBt == BT_ERROR) {
    if (now - dcTError > 3000) {
      dcBt = BT_INICIALIZANDO;
      dcFase = FASE_INIT;
      dcPaso = 0;
      for (uint8_t i = 0; i < NPIDS; i++)
        DCPids[i].soportado = false;
    }
    return;
  }
  while (BTSerial.available()) {
    char ch = BTSerial.read();
    if (ch == '>') {
      if (dcEsperando) {
        if (dcPuntualActiva < 4)
          dcFinalPuntual(false);
        else {
          dcEsperando = false;
          dcProcesa(false);
        }
      }
    } else if (dcRx.length() < 240)
      dcRx += ch;
  }
  if (dcEsperando && millis() - dcTEnvio > dcTimeout) {
    if (dcPuntualActiva < 4)
      dcFinalPuntual(true);
    else {
      dcEsperando = false;
      dcProcesa(true);
    }
  }
  if (!dcEsperando)
    dcSiguiente();
}
static void dcObdSimPuntuales() {
  for (uint8_t i = 0; i < 4; i++)
    if (dcQ[i].estado == PQ_ENVIADA) {
      if (dcQ[i].cmd == "0101")
        dcQ[i].rx = "41 01 81 02";
      else if (dcQ[i].cmd == "03")
        dcQ[i].rx = "43 01 33 00 00 00 00";
      else if (dcQ[i].cmd == "07")
        dcQ[i].rx = "47 01 07 00 00";
      else if (dcQ[i].cmd == "0A")
        dcQ[i].rx = "4A 00 00 00 00";
      else if (dcQ[i].cmd == "04")
        dcQ[i].rx = "44";
      else if (dcQ[i].cmd == "0900")
        dcQ[i].rx = "49 00 01 00 00";
      else if (dcQ[i].cmd == "0902")
        dcQ[i].rx =
            "014\n0: 10 14 49 02 01 57 50 30 5A 5A 5A 39 39\n1: 21 5A 54 53 33 39 32 30 30 31";
      dcQ[i].estado = PQ_LISTA;
    }
}
