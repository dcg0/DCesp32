#pragma once
/*
 * ESP32-OBDC2 · Fase 5 · DTC
 * Basado en ESP32-OBD2-Gauge de VaAndCob (MIT No Commercial Use).
 * Este módulo solo encola solicitudes; dc_obd.h es el único hablante con el ELM327.
 */

struct DcDtcItem {
  String codigo;
  String descripcion;
};

static DcDtcItem dcDtcAlmacenados[8];
static DcDtcItem dcDtcPendientes[8];
static DcDtcItem dcDtcPermanentes[8];
static uint8_t dcDtcNAlmacenados = 0;
static uint8_t dcDtcNPendientes = 0;
static uint8_t dcDtcNPermanentes = 0;
static uint8_t dcDtcMil = 0;
static uint8_t dcDtcCuenta = 0;
static uint8_t dcDtcPaso = 0;
static int8_t dcDtcSolicitud = -1;
static bool dcDtcActivo = false;
static bool dcDtcBorrando = false;
static String dcDtcMensaje = "";

static const char* dcDtcDescripcion(const String& codigo) {
  if (codigo == "P0133") return "Respuesta lenta del sensor O2 (banco 1, sensor 1)";
  if (codigo == "P0300") return "Fallos de encendido aleatorios/multiples";
  if (codigo == "P0420") return "Eficiencia del catalizador por debajo del umbral";
  if (codigo == "P0171") return "Sistema demasiado pobre, banco 1";
  if (codigo == "P0172") return "Sistema demasiado rico, banco 1";
  return "sin descripción en la base";
}

static void dcDtcVaciar(DcDtcItem* lista, uint8_t& cantidad) {
  cantidad = 0;
  for (uint8_t i = 0; i < 8; i++) {
    lista[i].codigo = "";
    lista[i].descripcion = "";
  }
}

static bool dcDtcEsHex4(const String& texto, int posicion) {
  if (posicion < 0 || posicion + 4 > (int)texto.length()) return false;
  for (int i = 0; i < 4; i++) {
    char c = texto[posicion + i];
    bool hex = (c >= '0' && c <= '9') || (c >= 'A' && c <= 'F');
    if (!hex) return false;
  }
  return true;
}

/*
 * Decodifica una respuesta DTC CAN o no-CAN.
 * El encabezado completo es 43, 47 o 4A. Después del encabezado puede existir
 * un byte de conteo: se detecta únicamente cuando (resto % 4) == 2.
 */
static void dcDtcParseUna(const String& linea, DcDtcItem* lista, uint8_t& cantidad, const String& cab) {
  if (!linea.startsWith(cab.c_str())) return;
  String resto = linea.substring(cab.length());
  if ((resto.length() % 4) == 2) resto = resto.substring(2);  // byte de conteo
  for (int pos = 0; pos + 4 <= (int)resto.length() && cantidad < 8; pos += 4) {
    String par = resto.substring(pos, pos + 4);
    if (!dcEsHexTodo(par)) break;
    if (par == "0000") continue;
    uint8_t b = (uint8_t)strtol(par.substring(0, 2).c_str(), nullptr, 16);
    String codigo;
    codigo += "PCBU"[(b >> 6) & 3];
    codigo += String((b >> 4) & 3);
    codigo += par[1];                 // nibble bajo del primer byte (faltaba)
    codigo += par.substring(2, 4);
    lista[cantidad].codigo = codigo;
    lista[cantidad].descripcion = dcDtcDescripcion(codigo);
    cantidad++;
  }
}

static void dcDtcParse(const String& respuesta, DcDtcItem* lista, uint8_t& cantidad, const char* encabezado) {
  bool iso;
  String lineas = dcLineasLimpias(respuesta, iso);
  String unido = dcUneLineas(lineas, iso);
  String l;
  for (unsigned i = 0; i < unido.length(); i++) {
    if (unido[i] != '|') { l += unido[i]; continue; }
    dcDtcParseUna(l, lista, cantidad, encabezado);  // una ECU por línea
    l = "";
  }
  if (l.length()) dcDtcParseUna(l, lista, cantidad, encabezado);  // ISO-TP: una sola cadena
}

static String dcDtcListaJson(DcDtcItem* lista, uint8_t cantidad) {
  String json = "[";
  for (uint8_t i = 0; i < cantidad; i++) {
    if (i) json += ',';
    json += "{\"c\":\"" + lista[i].codigo + "\",\"d\":\"";
    json += lista[i].descripcion + "\"}";
  }
  return json + "]";
}

static void dcDtcIniciar() {
  if (dcDtcActivo) return;
  dcDtcVaciar(dcDtcAlmacenados, dcDtcNAlmacenados);
  dcDtcVaciar(dcDtcPendientes, dcDtcNPendientes);
  dcDtcVaciar(dcDtcPermanentes, dcDtcNPermanentes);
  dcDtcPaso = 0;
  dcDtcSolicitud = -1;
  dcDtcActivo = true;
  dcDtcMensaje = "solicitando";
}

static void dcDtcTick() {
  if (!dcDtcActivo) return;

  if (dcDtcSolicitud >= 0) {
    DcPqEstado estado = dcEstado(dcDtcSolicitud);
    if (estado == PQ_LISTA) {
      String respuesta = dcRespuesta(dcDtcSolicitud);
      if (dcDtcPaso == 0) {
        String limpia = dcLimpia(respuesta);
        int posicion = limpia.indexOf("4101");
        if (posicion >= 0 && posicion + 6 <= (int)limpia.length()) {
          uint8_t estadoMil = (uint8_t)strtol(
              limpia.substring(posicion + 4, posicion + 6).c_str(), nullptr, 16);
          dcDtcMil = (estadoMil & 0x80) ? 1 : 0;
          dcDtcCuenta = estadoMil & 0x7F;
        }
      } else if (dcDtcPaso == 1) {
        dcDtcParse(respuesta, dcDtcAlmacenados, dcDtcNAlmacenados, "43");
      } else if (dcDtcPaso == 2) {
        dcDtcParse(respuesta, dcDtcPendientes, dcDtcNPendientes, "47");
      } else {
        dcDtcParse(respuesta, dcDtcPermanentes, dcDtcNPermanentes, "4A");
      }
      dcLiberar(dcDtcSolicitud);
      dcDtcSolicitud = -1;
      dcDtcPaso++;
    } else if (estado == PQ_TIMEOUT) {
      dcLiberar(dcDtcSolicitud);
      dcDtcSolicitud = -1;
      dcDtcPaso++;
    }
  }

  if (dcDtcPaso < 4 && dcDtcSolicitud < 0) {
    const char* comandos[] = {"0101", "03", "07", "0A"};
    dcDtcSolicitud = dcPedir(comandos[dcDtcPaso], 2000);
    if (dcDtcSolicitud < 0) {
      dcDtcMensaje = "cola llena";
      dcDtcActivo = false;
    }
  } else if (dcDtcPaso >= 4 && dcDtcSolicitud < 0) {
    dcDtcMensaje = "listo";
    dcDtcActivo = false;
  }
}

static String dcDtcJson() {
  String json = "{\"estado\":\"" + dcDtcMensaje + "\",\"mil\":";
  json += String(dcDtcMil) + ",\"cuenta\":" + String(dcDtcCuenta);
  json += ",\"almacenados\":" + dcDtcListaJson(dcDtcAlmacenados, dcDtcNAlmacenados);
  json += ",\"pendientes\":" + dcDtcListaJson(dcDtcPendientes, dcDtcNPendientes);
  json += ",\"permanentes\":" + dcDtcListaJson(dcDtcPermanentes, dcDtcNPermanentes) + "}";
  return json;
}

static bool dcDtcBorrar(const String& cuerpo) {
  if (cuerpo != "confirmar=BORRAR") return false;
  int8_t solicitud = dcPedir("04", 2500);
  if (solicitud < 0) return false;
  dcDtcBorrando = true;
  dcDtcMensaje = "borrando";
  return true;
}

static void dcDtcTickBorrado() {
  if (!dcDtcBorrando) return;
  for (uint8_t i = 0; i < 4; i++) {
    if (dcQ[i].cmd != "04") continue;
    DcPqEstado estado = dcQ[i].estado;
    if (estado != PQ_LISTA && estado != PQ_TIMEOUT) continue;

    bool confirmado = false;
    if (estado == PQ_LISTA) {
      String respuesta = dcLimpia(dcQ[i].rx);
      confirmado = respuesta.indexOf("44") >= 0;
    }
    dcDtcMensaje = confirmado ? "borrado confirmado por ELM327" : "fallo al borrar: respuesta sin 44";
    dcLiberar(i);
    dcDtcBorrando = false;
    if (confirmado) dcDtcIniciar();
    break;
  }
}
