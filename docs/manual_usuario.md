## Manual de Usuario: Dispositivo IoT - Servidor de Tokens - <a href="../README.md">volver</a> 

### I. Propósito del Dispositivo

Este dispositivo es un **Servidor de Acceso Remoto** que se conecta a su red Wi-Fi. Una vez conectado, genera una **Clave de Seguridad (Token)** que se actualiza automáticamente cada **5 minutos** para permitir el acceso temporal y seguro a un sistema o servicio.

---

### II. Configuración Inicial (Primer Uso o Cambio de Red)

Si el dispositivo no está conectado a una red Wi-Fi existente o si las credenciales son incorrectas, automáticamente creará su propia red de configuración (Modo AP).

#### 1. Acceso al Portal de Configuración

1.  **Conexión Móvil:** En su teléfono o PC, busque y conéctese a la red Wi-Fi llamada **`Configurar_ESP`**.
    * **Contraseña de Red:** `password_config`
2.  **Apertura del Formulario:** Abra el navegador web (Chrome, Safari, etc.) e ingrese la siguiente dirección en la barra:
    > **`http://192.168.4.1`**
    Aparecerá el formulario de **Configuración WiFi ESP8266**.
3.  **Credenciales:** Ingrese el **Nombre (SSID)** y la **Contraseña** de su red Wi-Fi doméstica (la red a la que desea conectar el dispositivo).

#### 2. Guardar y Conectar

1.  **Confirmar:** Presione **"Guardar y Conectar"**.
2.  El dispositivo se reiniciará automáticamente y **se conectará a su red Wi-Fi** usando las credenciales guardadas.

---

### III. Operación Estándar (Servidor Activo)

Una vez que el dispositivo se conecta a su red, está listo para ser usado.

#### 1. Encontrar la Dirección del Servidor

Ya que la placa obtuvo una nueva dirección IP de su *router*, debe encontrarla para acceder al servidor.

1.  Asegúrese de que su teléfono o PC esté conectado a la **misma red Wi-Fi** que el dispositivo.
2.  Utilice una aplicación de **Escaneo de Red** (como **Fing** o **Net Analyzer**) para escanear su red y encontrar la **Dirección IP** del nuevo dispositivo conectado (ejemplo: **`192.168.1.105`**).

#### 2. Acceder al Token de Seguridad

1.  Abra su navegador y escriba la **Dirección IP** encontrada (ejemplo: `http://192.168.101.22`).
2.  La página mostrará:
    * **IP de Trabajo:** La dirección actual del dispositivo.
    * **Token de Seguridad Actual:** Una clave única de acceso.

> **Importante:** El Token se actualiza **cada 5 minutos**. Para acceder a un servicio seguro, debe usar el token más reciente.

---

### IV. Restablecimiento a Configuración de Fábrica

Si necesita cambiar la red Wi-Fi a la que está conectado el dispositivo o si la contraseña ingresada previamente es incorrecta, debe restablecerlo para que entre de nuevo en el Modo de Configuración.

#### 1. Método de Reseteo (Avanzado)

Este método requiere conectar la placa a una computadora temporalmente.

1.  Conecte la placa al PC y abra el **Monitor Serial** del IDE de Arduino (configurado a **115200 baudios**).
2.  En la barra de entrada del Monitor Serial, escriba exactamente el siguiente comando:
    > **`RESET_WIFI`**
3.  Presione Enter. El dispositivo borrará las credenciales, se reiniciará y entrará automáticamente en el **Modo de Configuración** (consulte la **Sección II**) para que pueda ingresar nuevas credenciales.
