# luxtronik_v1
Luxtronik_v1 ESPHome Custom Sensor

HowTo:
1. Place luxtronik_v1_sensor.h in your ESPHome-ConfigRoot
2. Edit luxtronik_v1.yaml according to your needs
3. Compile and Upload to your Sensor

ToDo:
- Show Measures in Webinterface

# German Version
Mit einem ESP wird eine serielle Verbindung zur Luxtronik Wärmepumpensteuerung hergestellt, Werte gelesen und diese per MQTT z.B. für die Einbindung in Home Assistant bereitgestellt.

## Anleitung

### Voraussetzungen
- Python [Anleitung](https://esphome.io/guides/installing_esphome)
- ESPHome Installation --> [download](https://esphome.io/)
- ESP mittels USB verbunden (alternativ eine andere serielle Verbindung)

### .yaml anpassen
Zunächst musst du entscheiden, welche Hardware du einsetzt. Verwende die entsprechende Muster-yaml-Datei.

- luxtronik_v1_d1mini für den D1-Mini
- luxtronik_v1_nodemcu für einen ESP32 auf einem NodeMCU
- 
In der .yaml müssen die Zeilen mit spitzen Klammern angepasst werden, so z.B. die Werte für die WLAN-SSID und der WPA-Key (das WLAN-Kennwort).
Alle anderen Werte solltest du nur ändern, wenn du weißt, was du tust.

### ESPHome
ESPHome ist eine Konsolenanwendung (unter Linux wie unter Windows).
Du kopierst die angepasste yaml-Datei und luxtronik_v1.h in das Hauptverzeichnis von esphome unter Windows standardmäßig c:\Users\<du>\.

Hinweis: Weitere Informationen findest du unter [esphome.io](https://esphome.io/guides/getting_started_command_line)

Öffne eine Konsole, unter Windows öffne "command" und gib `esphome run luxtronik_v1.yaml` ein.

### WebInterface
Du kannst das WebInterface des ESP aufrufen und wirst die Werte der Luxtronik sehen können.