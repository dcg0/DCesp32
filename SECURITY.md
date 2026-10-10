# Política de seguridad

## Alcance

Esta política cubre el firmware DCESP32-OBDC2, su interfaz web local y los scripts de apoyo incluidos en este repositorio. El proyecto es experimental. No se afirma que el dispositivo, su enlace Bluetooth, su punto de acceso Wi-Fi o su servidor HTTP estén certificados, cifrados de extremo a extremo o preparados para una red no confiable.

## Reportar una vulnerabilidad

- **No publiques datos de explotación, credenciales ni detalles sensibles en un issue público.**
- Si GitHub Private Vulnerability Reporting está habilitado para este repositorio, usa **Security → Report a vulnerability** para enviar el reporte en privado.
- Si esa opción no está disponible, contacta al responsable del repositorio mediante un canal privado que este publique en su perfil o documentación y comparte solo los detalles necesarios para reproducir el problema.
- Incluye el componente y versión afectados, el impacto observado y pasos de reproducción seguros. No pruebes el firmware contra vehículos de terceros ni sistemas en producción sin permiso.

No se garantiza un plazo de respuesta. Las personas que reporten de buena fe deben evitar interrumpir servicios, acceder a datos ajenos o realizar cambios en vehículos.

## Uso seguro conocido

- Mantén el ESP32 en una red local controlada; no reenvíes el puerto ni expongas el servidor a Internet.
- Estaciona el vehículo antes de utilizar la interfaz.
- No borres códigos DTC ni cambies configuraciones sin comprender sus consecuencias.
- Las pruebas de software publicadas no sustituyen una revisión de seguridad, una validación eléctrica ni pruebas con un vehículo real.
