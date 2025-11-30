# Repetidor Wi-Fi Configurable

## 1\. Descripción General del Sistema

El Extensor de Rango NAPT (Network Address Translation) es un dispositivo basado en ESP8266 diseñado para tomar la conexión de su red Wi-Fi principal (Modo Estación/STA) y crear una red Wi-Fi secundaria aislada (Modo Access Point/SoftAP).

La función **NAT** aísla los dispositivos conectados al extensor de su red principal, utilizando una única dirección IP para la salida a Internet.

| Componente | Dirección IP por Defecto |
| :--- | :--- |
| **Red STA (Principal)** | Obtiene IP por DHCP de su *router*. |
| **Red SoftAP (Extensor)** | **172.217.28.254** (IP del *gateway*). |

## 2\. Configuración Inicial y Conexión

### 2.1 Primer Inicio o Reinicio de Fábrica

Al encender el dispositivo por primera vez o después de ejecutar el comando **`RESET`** (Reinicio de Fábrica), el sistema entrará automáticamente en el **Modo de Configuración** (Config Portal).

El dispositivo creará un punto de acceso temporal con las siguientes credenciales:

  * **Nombre de la Red (SSID):** `NAPT_Config`
  * **Contraseña:** `12345678`

### 2.2 Pasos para la Configuración

1.  **Conexión:** Conecte su dispositivo (teléfono, PC) a la red Wi-Fi temporal **`NAPT_Config`**.
2.  **Portal Web:** Acceda al portal de configuración abriendo su navegador y navegando a la dirección **`192.168.4.1`**.
3.  **Ajustes:**
      * Seleccione su **Red Wi-Fi Principal** (STA SSID) e ingrese su contraseña.
      * (Opcional) Seleccione el **Modo de Ejecución** (ver sección 4).
4.  **Guardar:** Haga clic en **Guardar**. El dispositivo se reiniciará.
5.  **Operación:** Al reiniciar, el dispositivo se conectará a su red principal y creará la red de extensión.

### 2.3 Red del Extensor (NAPT SoftAP)

Una vez configurado, el extensor creará una nueva red Wi-Fi.

  * **SSID del Extensor:** Estará basada en el nombre de su red principal (ejemplo: si la principal es `MiCasa`, el extensor será `MiCasaextender`).
  * **Contraseña del Extensor:** Será la misma que la de su red principal.

-----

## 3\. Comandos de Consola Serial para Soporte

Puede interactuar con el *firmware* para diagnóstico y soporte utilizando la consola serial (ej. el Monitor Serial del IDE de Arduino).

1.  **Velocidad (Baud Rate):** **115200**.
2.  **Terminación de Línea:** Se recomienda **Both NL & CR**.

### 3.1 Comando `RESET` (Reinicio de Fábrica)

Este comando es **prioritario** y borra toda la configuración.

| Comando | Acción |
| :--- | :--- |
| **`RESET`** | **Borra TODAS las credenciales** (de la memoria de la aplicación y la memoria *flash* del sistema Wi-Fi). El dispositivo se reinicia inmediatamente y entra en **Modo de Configuración** (`NAPT_Config`). |

**Output de Consola:**

```
>>> COMANDO RESET DETECTADO. INICIANDO REINICIO DE FÁBRICA.
>>> BORRANDO EEPROM (Custom Config)
>>> BORRANDO SETTINGS DE WiFiManager (Credenciales STA)
>>> REINICIANDO...
```

### 3.2 Comando `STATUS` (Diagnóstico)

Este comando se utiliza para obtener información de diagnóstico y **solo está disponible en Modo SOPORTE**.

| Comando | Acción |
| :--- | :--- |
| **`STATUS`** | Muestra el tiempo activo, la memoria libre (**Heap Libre**), las direcciones IP y las MAC del dispositivo. |

**Output de Consola (Ejemplo en Modo SOPORTE):**

```
--- Trazabilidad del Sistema ---
Modo de Ejecución: SUPPORT
Tiempo activo (s): 125
Heap Libre: 21000 bytes
IP STA: 192.168.1.10
MAC STA: 00:00:00:00:00:00
Datos de Usuario: [LOG DE ACCESO, DATOS DE SESIONES NAT]
--------------------------------
```

**Si está en Modo PRODUCCION:**

```
ERROR: Comando STATUS no disponible en modo PRODUCCION.
```

-----

## 4\. Modos de Ejecución

El sistema puede operar en dos modos, seleccionables durante la configuración web:

| Modo | Valor | Propósito |
| :--- | :--- | :--- |
| **PRODUCCION** | `0` | Modo de operación estándar. Prioriza el rendimiento de la red NAT. **Comandos de diagnóstico deshabilitados.** |
| **SOPORTE** | `1` | Modo de diagnóstico avanzado. Permite el uso del comando **`STATUS`** en la consola serial para *debugging* y monitoreo. |