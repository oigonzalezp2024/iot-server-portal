#ifndef NAPTAdapter_h
#define NAPTAdapter_h

#include <Arduino.h>

enum ExecutionMode {
    MODE_PRODUCTION = 0,
    MODE_SUPPORT = 1
};

struct ConfigData {
    char sta_ssid[32];
    char sta_password[64];
    ExecutionMode mode;
};

class NAPTAdapter {
public:
    bool begin(const ConfigData& config);

    void factoryReset();
    
    String getStatusInfo();
};

#endif
