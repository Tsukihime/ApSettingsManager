#include "ApSettingsManager.h"

ApSettingsManager::ApSettingsManager(): server(80) {}

void ApSettingsManager::begin(const String& ap_ssid, const String& ap_password, unsigned long ap_timeout) {
    apSsid = ap_ssid;
    apPassword = ap_password;
    apTimeout = ap_timeout;
    customValues = "{}";

    preferences.begin("__ap_settings");

    wifiSsid = preferences.getString("wifiSsid", "");
    wifiPassword = preferences.getString("wifiPassword", "");

    parameters["header"] = apSsid;
    parameters["wifiSsid"] = wifiSsid;
    parameters["wifiPassword"] = wifiPassword;
    parameters["sections"].to<JsonArray>();

    WiFi.hostname(apSsid);

    if (!connectToWiFi()) {
        startAccessPoint();
    }

    if (!LittleFS.begin()) {
        Serial.println("LittleFS Mount Error");
        return;
    }

    server.on("/", HTTP_GET, handleRoot);
    server.onNotFound(handleNotFound);
    server.on("/scan", HTTP_GET, handleScan);
    server.on("/parameters", HTTP_GET, [this](AsyncWebServerRequest *request) { this->handleParameters(request); });
    server.on("/settings", HTTP_GET, [this](AsyncWebServerRequest *request) { this->handleSettings(request); });
    server.on("/save", HTTP_POST, [this](AsyncWebServerRequest *request) { this->handleSave(request); });
    server.serveStatic("/", LittleFS, "/");
    server.begin();
}

bool ApSettingsManager::connectToWiFi() {
    Serial.println("Connecting to Wi-Fi: " + wifiSsid);
    WiFi.disconnect(true);
    WiFi.mode(WIFI_STA);
    WiFi.begin(wifiSsid.c_str(), wifiPassword.c_str());
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 30) {
        delay(500);
        Serial.print(".");
        attempts++;
    }
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("\nFailed to connect to Wi-Fi");
        return false;
    }
    Serial.println("\nConnected! IP: " + WiFi.localIP().toString());
    return true;
}

void ApSettingsManager::startAccessPoint() {
    Serial.println("Starting access point");
    WiFi.disconnect();
    WiFi.softAP(apSsid.c_str(), apPassword.c_str());
    Serial.println("Access point: " + apSsid + ", IP: " + WiFi.softAPIP().toString());
    dnsServer.start(53, "*", WiFi.softAPIP());
    apStartTime = millis();
}

void ApSettingsManager::handle() {
     static unsigned long lastWiFiCheck = 0;
    if (WiFi.getMode() == WIFI_AP || WiFi.getMode() == WIFI_AP_STA) {
        dnsServer.processNextRequest();
        if (apStartTime > 0 && WiFi.softAPgetStationNum() == 0 && millis() - apStartTime > apTimeout) {
            Serial.println("Nobody connected to the access point for the timeout period, rebooting...");
            ESP.restart();
        }
    } else if (WiFi.getMode() == WIFI_STA && millis() - lastWiFiCheck > WIFI_CHECK_INTERVAL) {
        if (WiFi.status() != WL_CONNECTED) {
            Serial.println("Wi-Fi connection lost, trying to reconnect...");
            connectToWiFi();
        }
        lastWiFiCheck = millis();
    }
}

void ApSettingsManager::handleRoot(AsyncWebServerRequest *request) {
    AsyncWebServerResponse *response = request->beginResponse(LittleFS, "/index.html.gz", "text/html");
    response->addHeader("Content-Encoding", "gzip");
    request->send(response);
}

void ApSettingsManager::handleNotFound(AsyncWebServerRequest *request) {
    Serial.println("Intercepting request: " + request->url() + ", redirecting to the main page");
    request->redirect("http://" + WiFi.softAPIP().toString() + "/");
}

void ApSettingsManager::handleScan(AsyncWebServerRequest *request) {
    int n = WiFi.scanComplete();
    if (n >= 0) {
        Serial.printf("Scan complete, found %d networks\n", n);
        String json = "[";
        for (int i = 0; i < n; ++i) {
            json += "{\"ssid\":\"" + WiFi.SSID(i) + "\",\"rssi\":" + String(WiFi.RSSI(i)) + "}";
            if (i < n - 1) json += ",";
        }
        json += "]";
        request->send(200, "application/json", json);
        WiFi.scanDelete();
        return;
    }
    if (n == -1) {
        Serial.println("Scan in progress, returning wait state");
    } else {
        Serial.println("No scan in progress, starting new scan");
        WiFi.scanNetworks(true, true);
    }
    String json = R"({"state": "wait"})";
    request->send(200, "application/json", json);
}

void ApSettingsManager::handleParameters(AsyncWebServerRequest *request) {
    request->send(200, "application/json", customValues);
}

void ApSettingsManager::handleSettings(AsyncWebServerRequest *request) {
    String json;
    serializeJson(parameters, json);
    request->send(200, "application/json", json);
}

void ApSettingsManager::handleSave(AsyncWebServerRequest *request) {
    if (request->hasParam("wifiSsid", true)) {
        preferences.putString("wifiSsid", request->getParam("wifiSsid", true)->value());
    }
    if (request->hasParam("wifiPassword", true)) {
        preferences.putString("wifiPassword", request->getParam("wifiPassword", true)->value());
    }

    for (JsonObject section : parameters["sections"].as<JsonArray>()) {
        for (JsonObject row : section["rows"].as<JsonArray>()) {
            String paramName = row["name"].as<String>();
            if (request->hasParam(paramName.c_str(), true)) {
                String paramValue = request->getParam(paramName.c_str(), true)->value();
                preferences.putString(paramName.c_str(), paramValue);
            }
        }
    }

    request->send(200, "text/plain", "Settings saved! Rebooting...");
    request->onDisconnect([]() {
        Serial.println("Settings saved! Rebooting...");
        ESP.restart();
    });
}

void ApSettingsManager::setLogJson(String& values_json) {
    customValues = values_json;
}

void ApSettingsManager::setCustomParameters(String& params_json) {
    JsonDocument inputDoc;
    DeserializationError error = deserializeJson(inputDoc, params_json);

    if (error) {
        Serial.print("json deserialization error: ");
        Serial.println(error.c_str());
        return;
    }

    parameters["sections"] = inputDoc.as<JsonArray>();

    for (JsonObject section : parameters["sections"].as<JsonArray>()) {
        for (JsonObject row : section["rows"].as<JsonArray>()) {
            String paramName = row["name"].as<String>();
            if (preferences.isKey(paramName.c_str())) {
                row["value"] = preferences.getString(paramName.c_str(), "");
            } else {
                preferences.putString(paramName.c_str(), row["value"].as<String>());
            }
        }
    }
}

String ApSettingsManager::getWifiSsid() {
    return wifiSsid;
}

void ApSettingsManager::setWifiSsid(const String& ssid) {
    wifiSsid = ssid;
    preferences.putString("wifiSsid", ssid);
}

String ApSettingsManager::getWifiPassword() {
    return wifiPassword;
}

void ApSettingsManager::setWifiPassword(const String& pwd) {
    wifiPassword = pwd;
    preferences.putString("wifiPassword", pwd);
}

String ApSettingsManager::getParameter(const char* key, const String defaultValue) {
    return preferences.getString(key, defaultValue);
}

size_t ApSettingsManager::setParameter(const char* key, const char* value) {
    return preferences.putString(key, value);
}
