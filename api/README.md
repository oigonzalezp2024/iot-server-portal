¡Entendido! El informe será limpio y profesional, centrado únicamente en el desarrollo y la validación final.

Aquí tienes el informe conciso y libre de errores sobre el desarrollo de la API y el script de prueba.

---

## 📄 Informe de Desarrollo: API REST y Script de Prueba

Este informe resume la funcionalidad implementada y la validación exitosa de la API de registro de IPs (`api.php`) y su script de prueba cliente (`api_test.php`).

---

## 1. ⚙️ Desarrollo y Funcionalidad de la API (`api.php`)

El endpoint (`api.php`) fue diseñado para ser el punto de comunicación para dispositivos IOT (ESP8266) que envían su ID y dirección IP.

### **Características Implementadas:**

| Área | Implementación | Propósito |
| :--- | :--- | :--- |
| **Persistencia de Datos** | **UPSERT** (Merge/Actualización Condicional) | Utiliza una única consulta SQL (`INSERT ... ON DUPLICATE KEY UPDATE`) para **INSERTAR** un nuevo `device_id` o **ACTUALIZAR** la IP de uno existente. Esto asegura la unicidad y optimiza el rendimiento. |
| **Seguridad** | **Autenticación API Key** | Valida la cabecera `X-Device-Auth` contra una clave secreta fija. Rechaza el acceso con código **HTTP 401** si la clave es inválida. |
| **Integridad de Datos** | **Validación de Campos** | Verifica la presencia de los campos requeridos (`device_id`, `ip_address`). Rechaza la solicitud con código **HTTP 400** si faltan datos. |
| **Acceso a Base de Datos** | **PDO y Prepared Statements** | Utiliza la extensión PDO para gestionar la conexión y sentencias preparadas, previniendo ataques de inyección SQL. |
| **Flujo de Servidor** | **Función `sendResponse` con `exit;`** | La API finaliza correctamente la ejecución del script mediante `exit;` después de enviar la respuesta JSON y el código de estado HTTP (comportamiento estándar de un servidor web). |
| **Auditoría** | **`logMessage`** | Registra los eventos críticos (peticiones, autenticación, conexión a BD y resultados de UPSERT) en el archivo `ip_log_quemados.txt`. |

---

## 2. 🔬 Validación con Script Cliente (`api_test.php`)

El script de prueba fue diseñado para simular ser un cliente externo, respetando completamente la lógica y el flujo de la API. Se utilizó la **función cURL** de PHP para simular llamadas HTTP reales, garantizando que el `exit;` en la API no interrumpiera el ciclo de prueba.

### **Resultados de la Validación:**

El `api_test.php` ejecutó 4 escenarios críticos con un **100% de éxito**. Los logs del servidor (`ip_log_quemados.txt`) confirmaron el flujo de trabajo correcto para cada caso.

| Test ID | Escenario de Prueba | Objetivo de la Prueba | Código HTTP Resultado | Estado |
| :--- | :--- | :--- | :--- | :--- |
| **1** | Inserción Inicial | Verificar el **INSERT** del UPSERT. | **200** | ✅ PASSED |
| **2** | Actualización | Verificar el **UPDATE** del UPSERT. | **200** | ✅ PASSED |
| **3** | Fallo de Autenticación | Probar el rechazo de clave inválida. | **401** | ✅ PASSED |
| **4** | Fallo de Validación | Probar el rechazo de campos faltantes. | **400** | ✅ PASSED |

---

## 3. ✨ Conclusión

La API de registro de IPs es **funcional, segura y ha sido completamente validada** contra los casos de uso esperados. El proceso de prueba demostró que la autenticación, la validación de datos y la lógica de persistencia (UPSERT) operan según las especificaciones. La API está lista para el despliegue en producción.
