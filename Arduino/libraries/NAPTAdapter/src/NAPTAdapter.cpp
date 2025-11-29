#include "NAPTAdapter.h"
#include <ESP8266WiFi.h>
#include <lwip/napt.h>
#include <lwip/dns.h>

#define NAPT_ENTRIES 1000
#define NAPT_PORT_MAP 10
#define HAVE_NETDUMP 0 

#if HAVE_NETDUMP
#include <NetDump.h>
void dump(int netif_idx, const char* data, size_t len, int out, int success) {
  (void)success;
  Serial.print(out ? F("out ") : F(" in "));
  Serial.printf("%d ", netif_idx);
  {
    netDump(Serial, data, len);
  }
}
#endif

bool NAPTAdapter::begin(const ConfigData& config) {
    
    Serial.printf("\n\nNAPT Range extender\n");
    Serial.printf("Heap on start: %d\n", ESP.getFreeHeap());

#if HAVE_NETDUMP
    phy_capture = dump;
#endif

    auto& server = WiFi.softAPDhcpServer();
    server.setDns(WiFi.dnsIP(0));

    String ap_ssid = String(config.sta_ssid) + "extender";
    String ap_pass = String(config.sta_password);

    WiFi.softAPConfig(
      IPAddress(172, 217, 28, 254), IPAddress(172, 217, 28, 254), IPAddress(255, 255, 255, 0));
    WiFi.softAP(ap_ssid.c_str(), ap_pass.c_str());
    
    Serial.printf("AP Extender: %s\n", WiFi.softAPIP().toString().c_str());

    Serial.printf("Heap before NAPT init: %d\n", ESP.getFreeHeap());
    err_t ret = ip_napt_init(NAPT_ENTRIES, NAPT_PORT_MAP);
    Serial.printf("ip_napt_init(%d,%d): ret=%d (OK=%d)\n", NAPT_ENTRIES, NAPT_PORT_MAP, (int)ret, (int)ERR_OK);
    if (ret == ERR_OK) {
        ret = ip_napt_enable_no(SOFTAP_IF, 1);
        Serial.printf("ip_napt_enable_no(SOFTAP_IF): ret=%d (OK=%d)\n", (int)ret, (int)ERR_OK);
        if (ret == ERR_OK) { Serial.printf("WiFi Network '%s' is now NATed behind '%s'\n", ap_ssid.c_str(), config.sta_ssid); }
    }
    Serial.printf("Heap after napt init: %d\n", ESP.getFreeHeap());
    
    return ret == ERR_OK;
}

void NAPTAdapter::factoryReset() {
}

String NAPTAdapter::getStatusInfo() {
    String status = "--- Trazabilidad del Sistema ---\n";
    status += "Modo de Ejecución: SUPPORT\n";
    status += "Tiempo activo (s): " + String(millis() / 1000) + "\n";
    status += "Heap Libre: " + String(ESP.getFreeHeap()) + " bytes\n";
    status += "IP STA: " + WiFi.localIP().toString() + "\n";
    status += "MAC STA: " + WiFi.macAddress() + "\n";
    status += "Datos de Usuario: [LOG DE ACCESO, DATOS DE SESIONES NAT]\n";
    status += "--------------------------------\n";
    return status;
}
