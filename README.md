# luxtronik_v1 for Home Assistant
Luxtronik_v1 ESPHome Custom Sensor

# English version
This is a Custom ESPHome Sensor, that is able to read values from a Alpha Innotec Luxtronic_v1 Heatpump via Serial Connection (RS232)
and send the values to Homeassistant via Homeassistant-API or MQTT


HowTo:
1. Place luxtronik_v1_sensor.h in your ESPHome-ConfigRoot
2. Edit luxtronik_v1.yaml according to your needs
3. Compile and Upload to your Sensor

ToDo:
- more Documentation
- Write-Acess to some of the values


# German Version
Mit einem ESP wird eine serielle Verbindung zur Luxtronik v1 Wärmepumpensteuerung hergestellt, es werden Werte ausgelesen und über die ESPHOME API für Home Assistant bereitgestellt. Alternativ werden die Werte per MQTT an einen MQTT-broker geliefert.

## Voraussetzungen

### Software
- Python [Anleitung](https://esphome.io/guides/installing_esphome)
- ESPHome Installation --> [download](https://esphome.io/)

### Hardware
- ESP (empfohlen ESP32)
- MAX3232-Modul
- serielles Kabel
- Sub-D9 Stecker männlich

## Serielle Verbindung
Die größte Hürde in diesem Projekt stellt die serielle Verbindung von der Luxtronik v1 zum ESP dar. Dieser Schritt wird die meiste Zeit in der Umsetzung erfordern.
Die Luxtronik verwendet 5V Pegel, während der ESP 3,3V Pegel verwendet, daher braucht es das MAX3232-Modul. Von allen möglichen Belegungen bei einem seriellen Kabel funktioniert letztlich nur eine, hier RX, TX und Ground/GND/Erde.

### ESP an MAX3232
Eine direkte Verbindung des ESP mit der Luxtronik v1 ist wegen der unterschiedlichen Spannungen nicht erfolgreich. Daher werden die PINs aus der ESP-Konfiguration mit dem MAX3232 verbunden. 

![luxtronik-esphome_Steckplatine](https://github.com/optimismus/luxtronik_v1/assets/16720046/2aa45ad3-bc5a-492d-bca8-580a3bfd1155)
#### D1 Mini
- ESP PIN D5 an MAX3232 RX
- ESP PIN D6 an MAX3232 TX
- ESP 3,3V an MAX3232 VCC
- ESP G an MAX3232 GND

#### ESP32
- ESP 3,3V an MAX3232 VCC
- ESP GND an MAX3232 GND
- ESP PIN D2 an MAX3232 RX
- ESP PIN D4 an MAX3232 TX

### Serielles Kabel
Das serielle Kabel muss an der Luxtronik Platine angeschlossen werden. Dort gibt es einen männlichen Steckeranschluss, entsprechend muss das Kabel hier Buchsen haben. Das MAX3232-Modul hat einen weiblichen Sub-D9 Buchsenanschluss, entsprechend braucht das Kabel hier einen männlichen Stecker. Kurz das Kabel braucht an einem Ende einen männlichen und am anderen Ende einen weiblichen Sub-D9 Stecker. Solche Kabel mit richtiger Belegung sind schwer zu finden, daher basteln wir uns ein solches Kabel selbst.

Ausgangspunkt ist ein Cisco Konsolkabel Sub-D9 auf RJ45, wie es für die Konfiguration von Netzwerkkomponenten des Herstellers zum Einsatz kommt - das ist kein MUSS, es funktioniert auch mit jedem beliebigen anderen Kabel.
Vom Sub-D9-Stecker werden von nur die PINs ??5 (RX), 6 (TX) und 4 (GND) benötigt. Diese haben folgende Kabelfarben:
- SubD9-PIN2, TX = rot
- SubD9-PIN3, RX = grün
- SubD9-PIN4, GND = gelb (auch PIN5 ist GND = orange)

siehe auch: [hier](https://www.cable-tester.com/rj45-rs232-console-cable-pin-out/)
Schneide den RJ45-Stecker ab und löte den separten, männlichen Sub-D9-Stecker wie folgt an:
- grünes Kabel an SubD9-PIN 2 (TX)
- rotes Kabel an SubD9-PIN 3 (RX)
- gelbes Kabel an SubD9-PIN 5 (GND)


Info: Belegung von [Sub-D9](https://www.cable-tester.com/rs232-pin-out/).
### Luxtronik v1 Wartungszugang freischalten
An deiner Wärmepumpe gehe unter Einstellungen, Datenzugang die PIN 9445 für den Kundendienstzugang eingeben und bestätigen. Es passiert erstmal nix, es sind jetzt aber weitere Menüs erreichbar. Jetzt unter Einstellungen, Diagnose Standard auswählen. Damit ist die serielle Schnittstelle freigegeben. 

### secrets.yaml anlegen
Du brauchst eine Datei secrets.yaml in der du insbesondere die Werte für das WLAN einträgst:
```
# Your Wi-Fi SSID and password
wifi_ssid: "<SSID>"
wifi_password: "<WPA-Key>"
#only if you use MQTT instead of ESPHOME API
#mqtt_broker: "<IP/Hostname>"
#mqtt_user: "<mqtt user name>"
#mqtt_password: "<mqtt password>"

```
Wenn du die Home Assistant für die Erstellung von ESPHOME verwendest, dann hast du wahrscheinlich schon eine passende secrets.yaml.

Alle anderen Werte solltest du nur ändern, wenn du weißt, was du tust. MQTT brauchst du nur, wenn du nicht Home Assistant verwenden möchtest, sondern einen MQTT Broker. Die am besten integrierte Einbindung in Home Assistant erfolgt über die ESPHOME API, weil ESPHOME vom selben Hersteller ist.

### ESPHome

#### Home Assistant Add-on 
Sofern noch nicht geschehen installierst du das Add-on "ESPHOME" (Einstellungen, Add-ons, Add-on-Store).

Mit dem (sicherlich) vorhandenen "File editor" gehst du nun in das Verzeichnis `config/esphome`. In dieses Verzeichnis lädst du die yaml-Datei(en) und `luxtronik_v1_sensor.h`.

Wechsel in die "Benutzeroberfläche" von ESPHOME (z.B. über Einstellungen, Add-ons, ESPHOME, Benutzeroberfläche öffnen). Dort erscheint nun der "luxtronik-v1-esp32". Bei der erstmaligen Installation muss du ihn per Micro-USB an den PC anschließen und einen geeigneten Browser verwenden, um den Scetch auf den ESP zu laden. Weitere Updates können via WLAN erfolgen.

#### Windows
ESPHome ist eine Konsolenanwendung (unter Linux wie unter Windows).
Du kopierst alle Dateien des Projektes und deine secrets.yaml in das Hauptverzeichnis von esphome unter Windows standardmäßig c:\Users\\deinName\\.

Hinweis: Weitere Informationen findest du unter [esphome.io](https://esphome.io/guides/getting_started_command_line)

Stelle eine serielle Verbindung des ESP zum PC her. Das geht am einfachsten mit einem NodeMCU-Board, in diesem Fall schließe den ESP per Micro-USB-Kabel an deinen PC an. Es wird ein serieller Port am PC erzeugt (COM-Port, z.B. COM3).

Öffne eine Konsole, unter Windows öffne "command" und gib `esphome run luxtronik_v1-esp32.yaml` ein. Jetzt wird der Scetch compiliert, beim ersten mal dauert das ein paar Minuten.
Am Ende musst du auswählen, wie die Binärdatei auf den ESP kommt. Beim ersten Durchlauf musst du die serielle Verbindung nutzen, weitere Änderungen kannst du "OTA" (over the air) über das WLAN auf den ESP bringen, wenn dieser online ist.

#### WebInterface
Du kannst das WebInterface des ESP aufrufen und wirst die Werte der Luxtronik sehen können. 
Die IP-Adresse findest in deinem Router/DHCP-Server oder direkt im LOG des ESPHome-Devices.

## weitere Quellen
- https://wiki.fhem.de/wiki/Luxtronik_1_in_FHEM
- https://homematic-forum.de/forum/viewtopic.php?t=38814 
- https://knx-user-forum.de/forum/%C3%B6ffentlicher-bereich/knx-eib-forum/38901-%E2%88%9A-alpha-innotec-luxtronic-serielle-verbindung 
- http://www.hoerndlein.de/cms/index.php/openhab/8-einbindung-der-luxtronik-1-in-openhab
- https://bitbucket.org/tobylorenz/ait_luxtronik/src/master/
- https://www.haustechnikdialog.de/Forum/t/82258/Alpha-InnoTec-Anschluss-am-PC-klappt-nicht?page=23 

