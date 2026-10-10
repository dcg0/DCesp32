#pragma once
/*
 * ESP32-OBDC2 · Fase 6 · VIN
 * Basado en ESP32-OBD2-Gauge de VaAndCob (MIT No Commercial Use).
 * La consulta se encola en dc_obd.h y nunca bloquea los handlers web.
 */

static String dcVin = "";
static String dcVinEstado = "sin consultar";
static uint8_t dcVinPaso = 0;
static int8_t dcVinSolicitud = -1;
static bool dcVinActivo = false;

static bool dcVinValido(const String& vin) {
  if (vin.length() != 17) return false;
  for (uint8_t i = 0; i < 17; i++) {
    char c = vin[i];
    if (c == 'I' || c == 'O' || c == 'Q') return false;
    bool numero = c >= '0' && c <= '9';
    bool letra = c >= 'A' && c <= 'Z';
    if (!numero && !letra) return false;
  }
  return true;
}

static void dcVinAgregarHexAscii(const String& hex, String& salida) {
  for (int i = 0; i + 1 < (int)hex.length(); i += 2) {
    uint8_t byte = 0;
    if (!dcHex(hex, i, byte)) break;
    if (byte >= 32 && byte <= 126) salida += (char)byte;
  }
}

/*
 * Normaliza respuestas ISO-TP CAN y no-CAN.
 * CAN: 0: 10 14 49 02 01 ..., 1: 21 ..., 2: 22 ...
 * no-CAN: 49 02 01 ... y siguientes líneas 49 02 02 ...
 * Se eliminan también líneas de conteo como 014.
 */
static void dcVinParse(const String& respuesta) {
  bool iso;
  String lineas = dcLineasLimpias(respuesta, iso);
  String datosHex, l;
  for (unsigned i = 0; i < lineas.length(); i++) {
    if (lineas[i] != '|') { l += lineas[i]; continue; }
    if (l.length() >= 2 && l[1] == ':') l = l.substring(2);
    if (l.length() == 3 && dcEsHexTodo(l)) { l = ""; continue; }  // conteo, p. ej. 014
    int p = l.indexOf("4902");                                     // 49 02 NN (NN = 01..05)
    if (p >= 0) l = l.substring(p + 6);
    else if (l.startsWith("21") || l.startsWith("22") || l.startsWith("23")) l = l.substring(2);  // PCI CAN
    datosHex += l;
    l = "";
  }
  String vin;
  dcVinAgregarHexAscii(datosHex, vin);
  dcVin = dcVinValido(vin) ? vin : "";
}

static void dcVinIniciar() {
  if (dcVinActivo) return;
  dcVinPaso = 0;
  dcVinSolicitud = -1;
  dcVinActivo = true;
  dcVinEstado = "solicitando";
}

static void dcVinTick() {
  if (!dcVinActivo) return;

  if (dcVinSolicitud >= 0) {
    DcPqEstado estado = dcEstado(dcVinSolicitud);
    if (estado == PQ_LISTA) {
      if (dcVinPaso == 1) dcVinParse(dcRespuesta(dcVinSolicitud));
      dcLiberar(dcVinSolicitud);
      dcVinSolicitud = -1;
      dcVinPaso++;
    } else if (estado == PQ_TIMEOUT) {
      dcLiberar(dcVinSolicitud);
      dcVinSolicitud = -1;
      dcVinPaso++;
    }
  }

  if (dcVinPaso == 0 && dcVinSolicitud < 0) {
    dcVinSolicitud = dcPedir("0900", 1800);
    if (dcVinSolicitud < 0) {
      dcVinEstado = "cola llena";
      dcVinActivo = false;
    }
  } else if (dcVinPaso == 1 && dcVinSolicitud < 0) {
    dcVinSolicitud = dcPedir("0902", 3500);
    if (dcVinSolicitud < 0) {
      dcVinEstado = "cola llena";
      dcVinActivo = false;
    }
  } else if (dcVinPaso >= 2 && dcVinSolicitud < 0) {
    if (dcVin.length()) {
      dcPref.putString("vin", dcVin);
      dcVinEstado = "listo";
    } else {
      dcVinEstado = "VIN no disponible";
    }
    dcVinActivo = false;
  }
}

static String dcVehiculoJson() {
  String json = "{\"estado\":\"" + dcVinEstado + "\",\"vin\":\"";
  json += dcVin + "\",\"protocolo\":\"no consultado\",\"voltaje\":null}";
  return json;
}

static void dcVinBegin() {
  dcVin = dcPref.getString("vin", "");
  if (dcVin.length()) dcVinEstado = "en caché";
}
