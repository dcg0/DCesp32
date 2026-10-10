# Cambios · 0.7.0-dell

- Se integró la pantalla remasterizada en el HTML embebido del firmware.
- La base exacta es `trabajo-0.7.0/` (`FW_VERSION "0.7.0-dell"`); esta entrega agrega seis pantallas: Tablero, Gráficas, Asistente, Fallas y vehículo, Ajustes y Registro / Historial.
- Se mejoró contraste, tamaño de etiquetas y valores, botones, tarjetas y diseños para celular.
- RPM y velocidad ahora usan medidores circulares; los otros once sensores mantienen sus tarjetas numéricas.
- Se muestran en campos separados el estado Bluetooth, firmware, PID/s, tiempo activo, heap, VIN, estado de lectura, MIL y grupos de DTC.
- Se conserva la gráfica comparativa original y se añaden gráficas individuales para sensores seleccionados, con guardas de canvas/contexto.
- El Historial usa muestras de `/data` y respuestas existentes `/dtc` y `/vehiculo`; no añade endpoints ni estima kilometraje, distancia o consumo.
- La lógica de configuración, unidades, alertas, parsers y handlers C++ se copió intacta; fuera de `PAGINA`, el sketch y los headers son idénticos a la base funcional.
