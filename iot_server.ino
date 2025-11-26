#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>

// --- CONFIGURACIÓN DE PARÁMETROS ---
const char* ap_ssid = "Configurar_ESP";
const char* ap_password = "password_config";
const int EEPROM_SIZE = 96;
const long tokenInterval = 300000; // 5 minutos en milisegundos

// --- OBJETOS Y VARIABLES GLOBALES ---
DNSServer dnsServer;
ESP8266WebServer webServer(80);

char wifi_ssid[32];
char wifi_pass[64];
IPAddress localIP;
unsigned long lastTokenTime = 0; 
String currentToken = ""; 

// --- FUNCIONES DE ALMACENAMIENTO EEPROM ---

void saveCredentials() {
  EEPROM.begin(EEPROM_SIZE);
  for (int i = 0; i < 32; ++i) {
    EEPROM.write(i, wifi_ssid[i]);
  }
  for (int i = 0; i < 64; ++i) {
    EEPROM.write(32 + i, wifi_pass[i]);
  }
  EEPROM.commit();
  EEPROM.end();
}

void loadCredentials() {
  EEPROM.begin(EEPROM_SIZE);
  for (int i = 0; i < 32; ++i) {
    wifi_ssid[i] = EEPROM.read(i);
  }
  for (int i = 0; i < 64; ++i) {
    wifi_pass[i] = EEPROM.read(32 + i);
  }
  EEPROM.end();
  wifi_ssid[31] = '\0';
  wifi_pass[63] = '\0';
}

// --- FUNCIÓN DE GENERACIÓN DE TOKEN ---

String generateToken() {
  if (lastTokenTime == 0) {
    randomSeed(analogRead(0));
  }
  String token = String(millis()) + String(random(10000, 99999));
  Serial.print("TOKEN GENERADO: ");
  Serial.println(token);
  return token;
}

// --- MANEJO DE LA WEB (FORMULARIO AP) ---

void handleRoot() {
  String html = "<html><body>";
  html += "<h2>Configuración WiFi ESP8266</h2>";
  html += "<form method='get' action='guardar'>";
  html += "SSID de la red: <input name='ssid' length=32><br><br>";
  html += "Contraseña: <input name='pass' type='password' length=64><br><br>";
  html += "<input type='submit' value='Guardar y Conectar'>";
  html += "</form></body></html>";
  webServer.send(200, "text/html", html);
}

void handleSave() {
  String ssid_str = webServer.arg("ssid");
  String pass_str = webServer.arg("pass");

  ssid_str.toCharArray(wifi_ssid, 32);
  pass_str.toCharArray(wifi_pass, 64);
  
  saveCredentials();

  String html = "<html><body><h1>Credenciales Guardadas!</h1>";
  html += "<p>Intentando conectar a " + ssid_str + ". Reiniciando...</p>";
  webServer.send(200, "text/html", html);
  
  delay(2000);
  ESP.restart();
}

void startConfigPortal() {
  Serial.println("Entrando en Modo de Configuración...");
  
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ap_ssid, ap_password);
  
  IPAddress apIP(192, 168, 4, 1);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  
  dnsServer.start(53, "*", apIP);
  
  webServer.on("/", handleRoot);
  webServer.on("/guardar", handleSave);
  webServer.onNotFound(handleRoot);
  webServer.begin();

  Serial.print("AP Creado: ");
  Serial.print(ap_ssid);
  Serial.print(" / IP: ");
  Serial.println(apIP);

  while (true) {
    dnsServer.processNextRequest();
    webServer.handleClient();
    if (Serial.available()) break; 
  }
}

// --- FUNCIÓN DE RESET DE FÁBRICA POR SERIAL ---

void handleSerialInput() {
  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    command.trim();

    Serial.print("Comando recibido: ");
    Serial.println(command);

    if (command == "RESET_WIFI") {
      Serial.println("\n*** COMANDO DE BORRADO RECIBIDO ***");
      EEPROM.begin(EEPROM_SIZE);
      for (int i = 0; i < EEPROM_SIZE; i++) {
        EEPROM.write(i, 0xFF);
      }
      EEPROM.commit();
      
      Serial.println("EEPROM borrada. Reiniciando en modo de configuracion...");
      delay(100); 
      ESP.restart();
    } else {
      Serial.println("Comando Serial no reconocido.");
    }
  }
}

// --- SERVIDOR WEB DE TRABAJO (TOKEN y IP visible) ---

void handleWorkServer() {
  // 1. Verificación y Renovación del Token
  unsigned long currentTime = millis();
  
  if (currentTime - lastTokenTime >= tokenInterval) {
    currentToken = generateToken();
    lastTokenTime = currentTime;
  }
  
  // 2. Respuesta HTTP (Modo STA - IP y Token visibles en el navegador)
  webServer.sendHeader("Connection", "close");
  webServer.send(200, "text/html", 
    "<html><body>"
    "<h1>SERVICIO ACTIVO</h1>"
    "<h2>IP de Trabajo: " + localIP.toString() + "</h2>"
    "<p>El token se actualiza cada 5 minutos.</p>"
    "<h3>Token de Seguridad Actual:</h3>"
    "<h2>" + currentToken + "</h2>"
    "</body></html>"
  );
}

// ------------------------------------
//          SETUP & LOOP
// ------------------------------------

void setup() {
  Serial.begin(115200); 
  
  loadCredentials();
  
  if (wifi_ssid[0] == 0xFF || wifi_ssid[0] == '\0') {
    Serial.println("No hay credenciales guardadas.");
    startConfigPortal();
    return;
  }
  
  // ==========================================
  //  NUEVO: REPORTE DE CREDENCIALES POR SERIAL
  // ==========================================
  Serial.println("\n*** CREDENCIALES CARGADAS DESDE EEPROM ***");
  Serial.print("SSID Guardado: ");
  Serial.println(wifi_ssid);
  Serial.print("Contraseña Guardada: ");
  Serial.println(wifi_pass);
  Serial.println("******************************************");
  // ==========================================

  // Intentar conexión STA
  Serial.print("Intentando conectar a: ");
  Serial.println(wifi_ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(wifi_ssid, wifi_pass);
  
  int i = 0;
  while (WiFi.status() != WL_CONNECTED && i < 20) {
    delay(500);
    Serial.print(".");
    i++;
    handleSerialInput(); 
  }

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("\nConexión fallida. Iniciando Portal.");
    startConfigPortal();
  } else {
    // Conexión exitosa
    localIP = WiFi.localIP(); 
    currentToken = generateToken(); 
    lastTokenTime = millis();
    
    Serial.println("\nConexión OK.");
    Serial.print("IP de trabajo: ");
    Serial.println(localIP);
    
    webServer.on("/", handleWorkServer);
    webServer.begin();
    Serial.println("Servidor web de trabajo iniciado.");
  }
}

void loop() {
  if (WiFi.getMode() == WIFI_AP || WiFi.getMode() == WIFI_STA) {
    webServer.handleClient();
  }
  handleSerialInput();
}
