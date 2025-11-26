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

String handleRootView() {
  String html = "";
  html += "<!DOCTYPE html>";
  html += "<html lang='es'>";
  html += "<head>";
  html += "<meta charset='UTF-8'>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
  html += "<title>Configuración WiFi ESP8266</title>";

  html += "<style>";
  html += "body{font-family:'Segoe UI',Tahoma,Geneva,Verdana,sans-serif;background:linear-gradient(135deg,#d1e4ff 0%,#eef6fd 100%);display:flex;justify-content:center;align-items:center;min-height:100vh;margin:0;padding:20px;}";
  html += ".card{background:#ffffffcc;backdrop-filter:blur(8px);-webkit-backdrop-filter:blur(8px);border-radius:20px;padding:0;width:100%;max-width:380px;box-shadow:12px 12px 25px rgba(0,0,0,0.08),-12px -12px 25px rgba(255,255,255,0.7);overflow:hidden;}";
  html += ".header{background:linear-gradient(135deg,#0066ff 0%,#00b4ff 100%);color:#fff;padding:18px 22px;font-size:1.15em;font-weight:600;display:flex;justify-content:space-between;align-items:center;}";
  html += ".form-content{padding:28px 32px 34px 32px;}";
  html += "label{display:block;margin-bottom:9px;font-weight:600;font-size:0.9em;color:#444;}";
  html += ".form-group{margin-bottom:28px;}";
  html += "input[type=text],input[type=password]{width:100%;padding:14px 12px;border-radius:10px;background:#f1f5fa;border:none;box-shadow:inset 4px 4px 8px rgba(0,0,0,0.08),inset -4px -4px 8px rgba(255,255,255,0.9);font-size:0.95em;transition:all .25s ease;}";
  html += "input[type=text]:focus,input[type=password]:focus{background:#fff;box-shadow:0 0 0 3px rgba(0,132,255,0.35);outline:none;}";
  html += "input[type=submit]{width:100%;padding:15px;border:none;border-radius:12px;cursor:pointer;font-size:1.05em;font-weight:700;color:#fff;background:linear-gradient(135deg,#28a745 0%,#20bf55 100%);transition:0.2s;box-shadow:6px 6px 14px rgba(0,0,0,0.15),-3px -3px 7px rgba(255,255,255,0.7);}";
  html += "input[type=submit]:hover{filter:brightness(1.08);transform:translateY(-1px);}";
  html += "input[type=submit]:active{transform:translateY(2px);box-shadow:inset 4px 4px 10px rgba(0,0,0,0.2);}";
  html += "::placeholder{color:#9ca3b2;font-style:italic;}";
  html += "</style>";

  html += "</head><body>";

  html += "<div class='card'>";
  html += "<div class='header'><span>📶 Configuración WiFi ESP8266</span><span>⚙️</span></div>";
  html += "<div class='form-content'>";
  html += "<form method='get' action='guardar'>";
  html += "<div class='form-group'>";
  html += "<label for='ssid'>SSID de la red:</label>";
  html += "<input name='ssid' id='ssid' type='text' maxlength='32' placeholder='Ej: MiRed_Oficina'>";
  html += "</div>";
  html += "<div class='form-group'>";
  html += "<label for='pass'>Contraseña:</label>";
  html += "<input name='pass' id='pass' type='password' maxlength='64' placeholder='Ingresa la contraseña'>";
  html += "</div>";
  html += "<input type='submit' value='Guardar y Conectar'>";
  html += "</form></div></div>";

  html += "</body></html>";
  return html;
}


void handleRoot() {
  String html = handleRootView();
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
