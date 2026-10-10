# DCESP32-OBDC2 · interfaz remasterizada sobre 0.7.0-dell

Esta carpeta se reconstruyó copiando el firmware funcional **0.7.0-dell** y sustituyendo únicamente el contenido de su página web embebida (`PAGINA`). El `FW_VERSION` sigue indicando `0.7.0-dell`. Los encabezados `.h` y todo el C++ situado fuera de la página son idénticos a la base funcional.

## Interfaz

El tablero muestra las 13 lecturas que entrega `/data`. RPM y velocidad se presentan como medidores circulares; los otros 11 sensores conservan sus tarjetas. Hay seis pantallas: Tablero, Gráficas, Asistente, Fallas y vehículo, Ajustes y Registro / Historial. La pantalla de gráficas conserva la comparativa original y agrega una gráfica individual para cada sensor seleccionado, con límite de cuatro. Los canvases se comprueban antes de obtener su contexto y antes de dibujar.

El Registro guarda en el navegador muestras recibidas por `/data`, promedios disponibles y eventos de lectura DTC. No requiere rutas `/historial` ni `/api/*`, y no inventa kilometraje, distancia, consumo ni duración conectada. Usa `portada_web.webp` como fondo de página y banner; un filtro oscuro conserva el contraste de los datos.

## Rutas reales

La página usa las rutas del servidor 0.7.0: `/data`, `/cfg`, `/status`, `/dtc`, `/dtc/leer`, `/dtc/borrar`, `/vehiculo`, `/vehiculo/leer`, `/config`, `/config/reset`, `/reiniciar` y `/alertas`, además de `/`, `/logo.webp` y `/portada.webp`. El verificador `python3 herramientas/verificar_api.py` compara método y ruta para cada llamada del JavaScript contra los `server.on` del sketch y valida el contrato `/data -> d[id]` y `/cfg -> id,n,u,w`.

## Compilar y probar

Con Arduino ESP32 core **2.0.17**, desde esta carpeta:

```sh
bash herramientas/compilar.sh sim
bash herramientas/test_parsers.sh
python3 herramientas/verificar_api.py
```

También se validan la sintaxis del JavaScript embebido (`node --check`) y la regresión existente de gráficas/asistente desde la raíz del paquete. Revisa `docs/PRUEBAS.md` para los resultados y sus límites.

> La compilación, la auditoría de rutas y la vista previa con respuestas de API simuladas pasaron. No se ha cargado el sketch ni conectado un ESP32/ELM327/coche reales en esta prueba.
