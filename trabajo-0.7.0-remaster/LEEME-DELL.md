# ESP32-OBDC2 · interfaz remasterizada sobre 0.7.0-dell

Esta reconstrucción parte de la versión **0.7.0-dell**, que indicaste como funcional. Mantiene `FW_VERSION "0.7.0-dell"`, handlers HTTP, configuración, parsers y lógica C++; la interfaz web es lo único sustituido dentro de `PAGINA`.

## Compilar y probar

Desde esta carpeta, con Arduino ESP32 core **2.0.17**:

1. Si hace falta instalar el entorno: `bash herramientas/instalar.sh`.
2. Compilar en modo simulador: `bash herramientas/compilar.sh sim`.
3. Probar parsers DTC/VIN: `bash herramientas/test_parsers.sh` (debe acabar con `fallos = 0`).
4. Verificar que cada ruta/método cliente existe en el sketch: `python3 herramientas/verificar_api.py`.
5. Solo si quieres cargarlo, conecta el ESP32 por USB y ejecuta `bash herramientas/cargar.sh sim`; para el modo real usa `bash herramientas/cargar.sh real`.
6. En el celular conectado al WiFi `DC-OBD`, abre `http://192.168.4.1`.

La pantalla **Registro / Historial** usa datos de `/data`, `/dtc` y `/vehiculo`; no añade endpoints. RPM y velocidad se dibujan en círculos y los demás sensores conservan sus tarjetas. La interfaz conserva la comparativa original y dibuja gráficos individuales para sensores marcados. Consulta `docs/PRUEBAS.md` para resultados; la conexión física con el ELM327 y el auto requiere hardware real.

## Notas
- Partición: Huge APP (la usan los scripts). Sin OTA por ahora.
- Rol Bluetooth: `DC_BT_ROLE` en `dc_obd.h` (MASTER por defecto; si no conecta, prueba `ESP_SPP_ROLE_SLAVE`).
- Al cambiar el celular al WiFi `DC-OBD` se corta el SSH/internet del celular: es normal.
- Basado en ESP32-OBD2-Gauge de VaAndCob (MIT No Commercial Use).
