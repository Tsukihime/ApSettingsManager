# ApSettingsManager

**ApSettingsManager** is a robust library for managing Wi-Fi settings and custom parameters on **ESP8266** and **ESP32** microcontrollers. It provides a user-friendly web interface for configuring Wi-Fi credentials, custom settings, and displaying logs, making it ideal for projects requiring flexible network configuration.

## Features

- **Wi-Fi Connectivity**: Automatically connects to a saved Wi-Fi network or creates an access point (AP) if connection fails.
- **Web Interface**: Offers a responsive interface for selecting Wi-Fi networks, entering credentials, and managing custom parameters.
- **Network Scanning**: Displays available Wi-Fi networks with signal strength (RSSI).
- **Persistent Storage**: Uses the `Preferences` library to store Wi-Fi credentials and custom parameters in non-volatile memory.
- **Custom Parameters**: Supports user-defined parameters via JSON, with customizable input types (e.g., `text`, `password`, `number`) and default values.
- **Log Output**: Enables real-time log display through JSON data in the web interface.
- **Timeout and Reboot**: Reboots the device if no clients connect to the access point within a specified timeout.
- **Compressed Web Interface**: Embeds a gzip-compressed HTML interface to optimize storage.

## Dependencies

The library requires the following dependencies:

- **ArduinoJson** (^7.0.0): For JSON parsing and serialization.
- **ESPAsyncWebServer** (^3.6.0): For asynchronous web server functionality.
- **Preferences** (^2.0.0): For non-volatile storage of settings.

Install these dependencies via the Arduino IDE Library Manager or PlatformIO.

## Installation

1. **Download the Library**:
   - Clone the repository:
     ```bash
     git clone https://github.com/Tsukihime/ApSettingsManager.git
     ```
   - Alternatively, download the ZIP file from the [GitHub repository](https://github.com/Tsukihime/ApSettingsManager) and extract it.

2. **Add to Your Project**:
   - Copy the `ApSettingsManager` folder to the `libraries` directory of your Arduino IDE.
   - For PlatformIO, add the following to your `platformio.ini`:
     ```ini
     lib_deps = https://github.com/Tsukihime/ApSettingsManager.git
     ```

3. **Post-Installation**:
   - The `after_install.py` script compresses `index.html` into `index_html_gz.h`. Ensure Python and the `gzip` module are installed to run this script automatically.

## Usage

### Basic Example

```cpp
#include <ApSettingsManager.h>

ApSettingsManager apSettings;

void setup() {
  Serial.begin(115200);
  // Initialize with default AP SSID: ESP WiFI AP
  apSettings.begin();
}

void loop() {
  // Handle web server and DNS requests
  apSettings.handle();
}
```

### Adding Custom Parameters

Custom parameters are defined using a JSON structure, with support for input types (`type`) and default values (`value`). The `type` field specifies the HTML input type (e.g., `text`, `password`, `number`), and `value` sets the default value if no saved value exists in `Preferences`. Example:

```cpp
#include <ApSettingsManager.h>

ApSettingsManager apSettings;

void setup() {
  Serial.begin(115200);
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

  apSettings.begin("EnergyMonitor");
  apSettings.setCustomParameters(customParameters);
}

void loop() {
  apSettings.handle();
}
```

In this example:
- The `mqttServer` parameter uses a `text` input with a default value of `"192.168.0.1"`.
- The `mqttPort` parameter uses a `number` input with a default value of `"1883"`.
- The `mqttPassword` parameter uses a `password` input for secure entry.
- If no saved value exists in `Preferences`, the `value` field is used as the default and stored.

### Accessing and Modifying Parameters

```cpp
// Get custom parameter
String mqttServer = apSettings.getParameter("mqttServer", "192.168.0.1");

// Set custom parameter
apSettings.setParameter("mqttPort", "1883");
```

### Displaying Logs

To display logs in the web interface, provide JSON-formatted data:

```cpp
String logJson = R"({"Voltage": 220.5, "Current": 1.23, "Power": 270, "CosPhi": 0.95})";
apSettings.setLogJson(logJson);
```

### Advanced Example: Energy Monitor with MQTT

This [example](https://github.com/Tsukihime/ApSettingsManager/blob/main/examples/Energy_Monitor_with_MQTT/Energy_Monitor_with_MQTT.ino) demonstrates integrating `ApSettingsManager` with an energy monitoring application using MQTT. The full implementation can be found in the [EnergyMonitor](https://github.com/Tsukihime/EnergyMonitor) project.

## Web Interface

1. If the device fails to connect to a Wi-Fi network, it starts an access point (default SSID: `ESP WiFI AP`).
2. Connect to the access point using a device.
3. Open a browser and navigate to `http://192.168.4.1`.
4. The web interface allows:
   - Scanning available Wi-Fi networks (click "Refresh").
   - Entering Wi-Fi SSID and password.
   - Configuring custom parameters with specified input types.
   - Saving settings and rebooting (click "Save and Reboot").
   - Viewing logs updated via `setLogJson`.

## Compatibility

- **Platforms**: ESP8266, ESP32.
- **Frameworks**: Arduino.
- **Compilers**: Arduino IDE, PlatformIO.

## License

MIT License. Free to use, modify, and distribute.

## Contributors

- **Tsukihime** — Author.
