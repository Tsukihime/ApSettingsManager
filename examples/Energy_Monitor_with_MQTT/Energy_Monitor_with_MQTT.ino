#include <Arduino.h>
#include <ApSettingsManager.h>
#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <Ticker.h>

ApSettingsManager apSettings;
WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);
Ticker ticker;

unsigned long lastMqttReconnectAttempt = 0;
const unsigned long MQTT_RECONNECT_INTERVAL = 5000; // 5 seconds

void reconnectMqtt() {
    String mqttServer = apSettings.getParameter("mqttServer", "192.168.0.1");
    int mqttPort = apSettings.getParameter("mqttPort", "1883").toInt();
    String mqttLogin = apSettings.getParameter("mqttLogin", "");
    String mqttPassword = apSettings.getParameter("mqttPassword", "");

    if (!mqttClient.connected()) {
        Serial.print("Attempting MQTT connection to ");
        Serial.print(mqttServer);
        Serial.print(":");
        Serial.println(mqttPort);

        String clientId = "ESPClient-" + String(random(0xffff), HEX);
        if (mqttClient.connect(clientId.c_str(), mqttLogin.c_str(), mqttPassword.c_str())) {
            Serial.println("Connected to MQTT broker");
        } else {
            Serial.print("Failed, rc=");
            Serial.print(mqttClient.state());
            Serial.println(" Will retry later...");
        }
    }
}

void updateState() {
    float vrms = 220.5;
    float irms = 1.23;
    float power = 270.0;
    float cosPhi = 0.95;

    // Publish to MQTT
    String stateTopic = "home/EnergyMonitorExample/state";
    String payload = "{\"v_rms\":" + String(vrms, 1) +
                     ",\"i_rms\":" + String(irms, 2) +
                     ",\"power\":" + String(power, 0) +
                     ",\"cos_phi\":" + String(cosPhi, 3) + "}";
    mqttClient.publish(stateTopic.c_str(), payload.c_str());

    // Update logs in ApSettingsManager
    String logJson = "{\"Voltage\":" + String(vrms, 1) +
                     ",\"Current\":" + String(irms, 2) +
                     ",\"Power\":" + String(power, 0) +
                     ",\"Cos φ\":" + String(cosPhi, 3) + "}";
    apSettings.setLogJson(logJson);
}

String mqttServer;

void setup() {
    Serial.begin(115200);
    Serial.println("Starting Energy Monitor Example...");

    // Define custom parameters for MQTT settings
    String customParameters = R"([
        {
            "title": "MQTT Settings",
            "rows": [
                {"name": "mqttServer", "type": "text", "title": "Server", "value": "192.168.0.1"},
                {"name": "mqttPort", "type": "number", "title": "Port", "value": "1883"},
                {"name": "mqttLogin", "type": "text", "title": "Login", "value": ""},
                {"name": "mqttPassword", "type": "password", "title": "Password", "value": ""}
            ]
        }
    ])";

    // Initialize ApSettingsManager
    apSettings.begin("EnergyMonitorExample");
    apSettings.setCustomParameters(customParameters);

    // Initialize MQTT
    mqttServer = apSettings.getParameter("mqttServer", "192.168.0.1");
    int mqttPort = apSettings.getParameter("mqttPort", "1883").toInt();
    mqttClient.setServer(mqttServer.c_str(), mqttPort);

    // Start periodic updates
    ticker.attach(5, updateState);
}

void loop() {
    apSettings.handle();
    // Check MQTT connection periodically without blocking to ensure the ApSettingsManager remains responsive
    unsigned long currentMillis = millis();
    if (!mqttClient.connected() && (currentMillis - lastMqttReconnectAttempt >= MQTT_RECONNECT_INTERVAL)) {
        reconnectMqtt();
        lastMqttReconnectAttempt = currentMillis;
    }
    mqttClient.loop();
}