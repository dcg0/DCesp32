# Ajuste visual: cuatro medidores circulares

Se modificó únicamente la página HTML/CSS/JavaScript embebida en `ESP32-OBDC2.ino`.

Medidores circulares superiores:
- RPM
- Velocidad
- Refrigerante
- Voltaje

Los otros nueve PIDs permanecen como tarjetas numéricas debajo. La lógica OBD2, Bluetooth, rutas HTTP, alertas, DTC, VIN, configuración, gráficas, asistente e historial no se modificó.

Validación:
- Auditoría API: PASS, 13 rutas/métodos.
- Parsers DTC/VIN: fallos = 0.
- JavaScript: `node --check` correcto.
- Simulador: 1,630,969 bytes, 51% flash; 59,644 bytes RAM global, 18%.
- Bluetooth real: 1,639,877 bytes, 52% flash; 60,476 bytes RAM global, 18%.
