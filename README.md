# DCESP32-OBDC2 · DC Automotriz

Panel de diagnóstico OBD-II local basado en ESP32 y adaptador ELM327 Bluetooth. Esta entrega remasteriza la interfaz web de la versión funcional 0.7.0-dell; conserva la lógica C++, parsers, handlers HTTP y configuración descritos en la documentación del firmware.

[![Página del proyecto](https://img.shields.io/badge/WEB-DCESP32-00a9e8?style=for-the-badge)](https://dcg0.github.io/DC-laboratory/dcesp32.html)
[![Validación](https://github.com/dcg0/DCesp32/actions/workflows/validation.yml/badge.svg)](https://github.com/dcg0/DCesp32/actions/workflows/validation.yml)
[![CodeQL](https://github.com/dcg0/DCesp32/actions/workflows/codeql.yml/badge.svg)](https://github.com/dcg0/DCesp32/actions/workflows/codeql.yml)
[![OpenSSF Scorecard](https://api.scorecard.dev/projects/github.com/dcg0/DCesp32/badge)](https://scorecard.dev/viewer/?uri=github.com/dcg0/DCesp32)
[![Política de seguridad](https://img.shields.io/badge/Seguridad-SECURITY.md-157a53?logo=github)](SECURITY.md)
[![Dependabot](https://img.shields.io/badge/Dependabot-actualizaciones%20configuradas-2678c7?logo=dependabot)](.github/dependabot.yml)

> Las insignias apuntan a workflows y políticas incluidos en el repositorio; no son un certificado de seguridad ni una auditoría del hardware.

## Página del proyecto

La [demo pública interactiva de DCESP32](https://dcg0.github.io/DC-laboratory/dcesp32.html) está alojada en el sitio de DC Laboratory; la tarjeta del proyecto en ese sitio enlaza directamente a ella y usa la portada original incluida en el ZIP. La fuente de esa página está en [`index.html`](index.html).

**La demo es únicamente visual:** sus lecturas, alertas, códigos DTC y estados del VIN son datos de ejemplo. No se conecta a un ESP32, adaptador ELM327 ni automóvil; no envía comandos OBD-II, no lee un VIN ni borra fallas. Para usar el firmware real se necesita el hardware y los archivos de este repositorio.

## Qué incluye

- Panel automotriz con medidores circulares de RPM y velocidad, junto con las demás lecturas expuestas por `/data`.
- Siete pantallas en el firmware y representadas en la demo: **Auto, Tablero, Gráficas, Asistente, Fallas y vehículo, Ajustes, Registro / Historial**.
- Los 13 PIDs definidos por el firmware, selección y gráficas individuales de hasta cuatro sensores.
- Consulta de VIN, estados de MIL y grupos DTC almacenados, pendientes y permanentes; en la web demostrativa se muestran solo ejemplos ficticios.
- Preferencias y umbrales del firmware; la demo web no los guarda en el ESP32 ni persiste cambios.
- Pruebas de parsers DTC/VIN, auditoría de rutas HTTP, verificación del JavaScript y análisis CodeQL del tooling Python.
- Ilustraciones genéricas de ESP32 y adaptador ELM327, preparadas para esta página; el modelo real puede variar.

## Requisitos

Para ejecutar el firmware real: placa ESP32 compatible, Arduino CLI con Arduino ESP32 core **2.0.17** y partición **Huge APP**, adaptador ELM327 Bluetooth compatible, `g++` para las pruebas de parsers, Python 3 para la auditoría de rutas y Node.js para comprobar el JavaScript embebido.

## Compilar y probar

Desde el directorio del firmware:

```sh
cd trabajo-0.7.0-remaster
bash herramientas/compilar.sh sim
bash herramientas/test_parsers.sh
python3 herramientas/verificar_api.py
```

El modo `sim` compila el firmware como simulador; no prueba un automóvil real. Consulta [`LEEME-DELL.md`](trabajo-0.7.0-remaster/LEEME-DELL.md), [`docs/README.md`](trabajo-0.7.0-remaster/docs/README.md), [`docs/PRUEBAS.md`](trabajo-0.7.0-remaster/docs/PRUEBAS.md), [`docs/PANEL-AUTO.md`](trabajo-0.7.0-remaster/docs/PANEL-AUTO.md) y [`docs/CHANGELOG.md`](trabajo-0.7.0-remaster/docs/CHANGELOG.md) para el procedimiento, historial y límites.

## Conexión al panel real

Tras cargar el sketch en el ESP32 y enlazar el ELM327, conecta el teléfono a la red Wi-Fi local **`DC-OBD`** y visita **`http://192.168.4.1`**. Al cambiar a esa red el teléfono puede perder temporalmente su conexión a Internet. Usa la interfaz únicamente con el vehículo estacionado.

## Seguridad y licencia

El panel se sirve en la red local del ESP32; este proyecto no promete cifrado TLS, endurecimiento para exposición pública ni compatibilidad universal con vehículos. **No publiques el dispositivo en Internet, no uses las opciones de diagnóstico al conducir y no borres DTC sin evaluar el impacto.** Las pruebas del paquete no incluyen un ESP32, ELM327 ni automóvil conectados físicamente. Lee [`SECURITY.md`](SECURITY.md) para reportar vulnerabilidades.

El ZIP original señala que el proyecto base `ESP32-OBD2-Gauge` de VaAndCob tiene una condición “MIT No Commercial Use”, pero no incluye el texto íntegro de su licencia. Este repositorio no añade una licencia nueva a la obra derivada; verifica los avisos y permisos aplicables antes de redistribuirla o utilizarla comercialmente.

## Estructura

```text
.
├── index.html                         # Página demostrativa del proyecto
├── assets/                            # Ilustraciones web optimizadas
├── SECURITY.md                        # Política de seguridad
├── .github/workflows/                 # Validación, CodeQL y Scorecard
└── trabajo-0.7.0-remaster/
    ├── ESP32-OBDC2/                   # Sketch y headers
    ├── herramientas/                  # Compilación, carga y pruebas
    ├── imagenes/                      # Identidad visual del proyecto
    └── docs/                          # Guías, pruebas y notas de versión
```
