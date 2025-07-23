#ifndef AP_SETTINGS_MANAGER_H
#define AP_SETTINGS_MANAGER_H

#if defined(ESP8266)
    #include <ESP8266WiFi.h>
#elif defined(ESP32)
    #include <WiFi.h>
#else
    #error "Unsupported platform! Please use ESP8266 or ESP32."
#endif

#include <ArduinoJson.h>
#include <Preferences.h>
#include <ESPAsyncWebServer.h>
#include <DNSServer.h>

class ApSettingsManager {
public:
    AsyncWebServer server;

    ApSettingsManager();

    void begin(
        const String& ap_ssid = "ESP WiFI AP",
        const String& ap_password = "",
        unsigned long ap_timeout = 180000
    );

    bool connectToWiFi();
    void startAccessPoint();
    void handle();

    void setLogJson(String& values_json);
    void setCustomParameters(String& params_json);

    String getWifiSsid();
    void setWifiSsid(const String& ssid);
    String getWifiPassword();
    void setWifiPassword(const String& pwd);

    String getParameter(const char* key, const String defaultValue = "");
    size_t setParameter(const char* key, const char* value);    

private:
    JsonDocument parameters;
    Preferences preferences;
    DNSServer dnsServer;    

    String wifiSsid;
    String wifiPassword;
    String apSsid;
    String apPassword;
    unsigned long apStartTime;
    unsigned long apTimeout;
    static constexpr unsigned long WIFI_CHECK_INTERVAL = 30000;

    String customValues;

    static void handleRoot(AsyncWebServerRequest *request);
    static void handleNotFound(AsyncWebServerRequest *request);
    static void handleScan(AsyncWebServerRequest *request);
    void handleParameters(AsyncWebServerRequest *request);
    void handleSettings(AsyncWebServerRequest *request);
    void handleSave(AsyncWebServerRequest *request);
};

#endif
