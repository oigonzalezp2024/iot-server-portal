#include <ESP8266WiFi.h>
#include <WiFiManager.h>
#include <EEPROM.h>
#include <NAPTAdapter.h>

#define EEPROM_SIZE 128
#define CONFIG_START_ADDR 0
#define SERIAL_BAUD 115200

NAPTAdapter naptAdapter;
WiFiManager wifiManager;
ConfigData currentConfig;

ExecutionMode currentMode;

/**
 * @brief Carga la configuración de la EEPROM. Si está vacía (0xFF),
 * inicializa las credenciales a cadenas vacías y el modo por defecto (PROD), 
 * pero NO las guarda, forzando la configuración inicial.
 */
void loadConfiguration() {
    EEPROM.begin(EEPROM_SIZE);
    EEPROM.get(CONFIG_START_ADDR, currentConfig);
    EEPROM.end();

    // Verificación de credenciales: si la EEPROM está limpia, inicializamos a vacío.
    if (currentConfig.sta_ssid[0] == 0xFF || currentConfig.sta_ssid[0] == 0) {
        Serial.println("EEPROM vacía o reseteada. Inicializando variables a vacío.");
        
        // **IMPORTANTE: Inicializar a cadenas vacías, no a valores quemados.**
        // Esto asegura que autoConnect() no tenga ninguna credencial que usar.
        strcpy(currentConfig.sta_ssid, "");
        strcpy(currentConfig.sta_password, "");
        
        currentConfig.mode = MODE_PRODUCTION;
        
        // NO llamamos a saveConfiguration() aquí. Los datos en EEPROM deben permanecer 0xFF 
        // hasta que el usuario guarde la configuración a través del portal web.
    }
    
    currentMode = currentConfig.mode;
    Serial.printf("Sistema inicializado en modo: %s\n", (currentMode == MODE_PRODUCTION ? "PRODUCCION" : "SOPORTE"));
}

/**
 * @brief Guarda la configuración actual en la EEPROM.
 */
void saveConfiguration() {
    EEPROM.begin(EEPROM_SIZE);
    EEPROM.put(CONFIG_START_ADDR, currentConfig);
    EEPROM.commit();
    EEPROM.end();
}

/**
 * @brief Ejecuta un reinicio de fábrica, borrando las credenciales de EEPROM y WiFiManager.
 */
void performFactoryReset() {
    Serial.println(">>> BORRANDO EEPROM (Custom Config)");
    // 1. Borrar la configuración personalizada de la EEPROM
    EEPROM.begin(EEPROM_SIZE);
    for (int i = 0; i < EEPROM_SIZE; i++) {
        EEPROM.write(i, 0xFF);
    }
    EEPROM.commit();
    EEPROM.end();
    
    // 2. CRÍTICO: Borrar las credenciales de Wi-Fi guardadas por WiFiManager en la flash.
    // Esto asegura que autoConnect() fallará y el portal de configuración se iniciará.
    Serial.println(">>> BORRANDO SETTINGS DE WiFiManager (Credenciales STA)");
    wifiManager.resetSettings(); 

    naptAdapter.factoryReset();
    
    Serial.println(">>> REINICIANDO...");
    delay(1000);
    ESP.restart();
}

/**
 * @brief Maneja comandos seriales del usuario (RESET, STATUS).
 */
void handleSerialCommands() {
    if (Serial.available()) {
        // Leemos todo hasta un salto de línea.
        String command = Serial.readStringUntil('\n'); 
        
        // Limpiamos cualquier carácter restante.
        while (Serial.available()) {
            Serial.read(); 
        }
        
        command.trim();
        command.toUpperCase();

        if (command.length() == 0) {
            return; 
        }

        // --- Prioridad al RESET ---
        if (command.equals("RESET")) {
            Serial.println(">>> COMANDO RESET DETECTADO. INICIANDO REINICIO DE FÁBRICA.");
            performFactoryReset();
            return; 
        } 
        
        // --- Comandos Secundarios ---
        if (command.equals("STATUS")) {
            if (currentMode == MODE_SUPPORT) {
                Serial.println(naptAdapter.getStatusInfo());
            } else {
                Serial.println("ERROR: Comando STATUS no disponible en modo PRODUCCION.");
            }
        } else {
            Serial.printf("Comando no reconocido: '%s'\n", command.c_str());
        }
    }
}

/**
 * @brief Inicia la lógica de conexión WiFiManager.
 */
void startWebServer() {
    // Parámetro para cambiar el modo de ejecución
    WiFiManagerParameter custom_mode("mode", "Modo (0=PROD, 1=SUPPORT)", String(currentConfig.mode).c_str(), 1);
    
    wifiManager.addParameter(&custom_mode);
    
    // Callback que se ejecuta cuando el usuario guarda la configuración en el portal web
    wifiManager.setSaveConfigCallback([&]() {
        // Guardar las credenciales que WiFiManager usó para conectarse
        strlcpy(currentConfig.sta_ssid, WiFi.SSID().c_str(), sizeof(currentConfig.sta_ssid));
        strlcpy(currentConfig.sta_password, WiFi.psk().c_str(), sizeof(currentConfig.sta_password));
        currentConfig.mode = (ExecutionMode)String(custom_mode.getValue()).toInt();
        
        saveConfiguration();
    });

    // Intenta autoConnect. Como las credenciales en flash están borradas, 
    // fallará y levantará el portal de configuración ("NAPT_Config").
    if (!wifiManager.autoConnect("NAPT_Config", "12345678")) {
        Serial.println("Fallo en la conexión/configuración. Reiniciando...");
        delay(3000);
        ESP.restart();
    } else {
        // Si se conecta exitosamente o si se configura por primera vez (a través del portal), 
        // actualizamos la configuración con el SSID/PASS real
        if (WiFi.SSID().length() > 0) {
             strlcpy(currentConfig.sta_ssid, WiFi.SSID().c_str(), sizeof(currentConfig.sta_ssid));
             strlcpy(currentConfig.sta_password, WiFi.psk().c_str(), sizeof(currentConfig.sta_password));
             currentConfig.mode = (ExecutionMode)String(custom_mode.getValue()).toInt(); // Mantiene el modo actual
             saveConfiguration();
        }
    }
    
    currentMode = currentConfig.mode;
}

void setup() {
    Serial.begin(SERIAL_BAUD);
    
    loadConfiguration();

    startWebServer();

    // Las credenciales en currentConfig son ahora las reales, o vacías si no hay configuración.
    if (!naptAdapter.begin(currentConfig)) {
        Serial.println("ERROR al iniciar el adaptador NAPT.");
    }
}

void loop() {
    handleSerialCommands();
}