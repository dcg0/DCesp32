# Panel automotriz en tiempo real

La primera pantalla es un panel de instrumentos automotriz con tacómetro, velocímetro con aguja, refrigerante, voltaje, estado P/D, conexión BT/ELM327, reloj y silueta SVG.

## Enlace con datos

Cada ciclo de `tick()` consume `GET /data` cada 400 ms y llama `actualizaAuto(j)`. La función lee `j.d.rpm`, `j.d.vel`, `j.d.ect` y `j.d.volt`, actualiza los textos, el porcentaje del aro mediante `--p`, y la rotación de la aguja de velocidad. Si un valor llega como `null`, muestra `--` y deja su indicador sin carga.

No se agregaron rutas ni se modificó la lógica Bluetooth, ELM327, PID, DTC, VIN, alertas, configuración, gráficas, asistente o historial.

El tablero anterior queda como pantalla 2 con cuatro circulares y nueve tarjetas.
