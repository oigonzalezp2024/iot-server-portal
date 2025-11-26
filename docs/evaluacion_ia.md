 ## Evaluación Actual del Software - <a href="../README.md">volver</a> 

### Fortalezas Técnicas (Logros)

  * **Persistencia de la Configuración (EEPROM):** La configuración Wi-Fi se guarda de forma segura y sobrevive a los ciclos de energía, cumpliendo el requisito básico de un dispositivo IoT autónomo.
  * **Diseño Profesional del *Firmware*:** La lógica del `setup()` es condicional: intenta la conexión **STA (Cliente)** y, si falla, cae en el modo **AP (Punto de Acceso/Portal Cautivo)**. Esto es el estándar de la industria.
  * **Mantenimiento Integrado:** La función de **`RESET_WIFI`** por Serial es una característica esencial para el mantenimiento y depuración, lo que elimina la dependencia de *sketches* de borrado separados.
  * **Servicio de Seguridad Implementado:** El **Token Temporal** con un intervalo de 5 minutos (`tokenInterval = 300000`) proporciona una capa de seguridad básica para el control de acceso.
  * **UX (Experiencia de Usuario) Mejorada:** La **IP de trabajo** se muestra visiblemente en la página web servida por la placa, resolviendo la dependencia del Monitor Serial para la operación remota.

### Debilidades y Limitaciones Actuales

  * **Seguridad del Token:** El token es una simple concatenación de `millis()` y un valor `random()`. Esto no es un token criptográficamente seguro (como un **JWT** o una clave **HMAC**), y es susceptible a la predicción si un atacante conoce el algoritmo.
  * **Ausencia de *Deep Sleep*:** El dispositivo está activo en todo momento. Para aplicaciones alimentadas por batería, el consumo de energía sería demasiado alto, ya que el ESP8266 necesita entrar en modo **Deep Sleep** y despertar solo para renovar el token o enviar datos.
  * **Interfaz HTML Simple:** La interfaz es HTML puro. No tiene estilos CSS, lo que dificulta la lectura en diferentes dispositivos móviles.
  * **Bloqueo en Bucle AP:** El `while(true)` dentro de `startConfigPortal()` bloquea la ejecución. Si el usuario se conecta y luego se desconecta sin guardar, el *loop* se detiene hasta que alguien se conecta.

-----

## Recomendaciones Futuras

Para llevar este proyecto al siguiente nivel de seguridad y eficiencia energética, se sugieren las siguientes mejoras:

### 1\. Seguridad Avanzada (Criptografía)

  * **Implementar HMAC (Hash-based Message Authentication Code):** Utilizar una librería de *hash* (como SHA-256) y una clave secreta (guardada en la EEPROM) para generar el token. El token debería ser una combinación de (Timestamp + Clave Secreta) *hashed*. Esto lo haría mucho más robusto.
  * **Uso de SSL/TLS (HTTPS):** Aunque es intensivo en recursos, la comunicación de tokens debe ser cifrada. Se puede investigar la librería **WiFiClientSecure** para implementar HTTPS.

### 2\. Eficiencia Energética

  * **Integración de Deep Sleep:** Modificar la lógica para que, una vez conectado el dispositivo, la placa entre en modo **Deep Sleep** por 4 minutos y 50 segundos. Al despertar, renovaría el token y volvería a dormir.
      * **Función:** Reemplazar el `loop()` por una rutina que gestione el `ESP.deepSleep(tokenInterval - tiempo_de_trabajo)`.

### 3\. Mejoras en la Interfaz (UX)

  * **Estilos CSS:** Añadir estilos CSS en línea o minimalistas al código HTML del Portal Cautivo para mejorar la legibilidad y la experiencia en el móvil.
  * **Retroalimentación Visual:** Añadir un mensaje visual de "Conectando..." en la página web servida para que el usuario sepa que la placa está trabajando antes de que se reinicie.

### 4\. Mantenimiento y Control Remoto

  * **Ruta de Estado del Token:** Agregar una ruta web (ej., `/status`) que muestre el **tiempo restante** exacto antes de que el token expire.
