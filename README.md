### This is Sonoff switch with thermometer sensor DS18B20

**Features**
* Sonoff relay is controlled by MQTT (setting ON, OFF)
* Pressing Sonoff button change the relay from OFF to ON or from ON to OFF
* Relay state is published in every change to MQTT Broker. It can be consumed by other services eg home automation software like openHAB
* DB18B20 sensor value (temperature) is published to MQTT Broker. It can be consumed by other services eg home automation software like openHAB
* It publishes MQTT messages only if temperature has been changed
* It's possible to correct temperature value returned by sensor by value specified in sketch configuration part or over the MQTT broker
* You can set how often Sonoff reads temperature from the DS18b29 sensor in sketch configuration part or over the MQTT Broker
* It's possible to reset switch by sending MQTT Message'

### Hardware
* Sonoff switch
* DS18B20 sensor
* Data line from DS18B20 sensor  should be connected to GPIO14, this is 5th pin counted from the top (1st one pin is the one closest to the sonoff switch button)


**MQTT Topic** 

| Topic  | Inbound / Outbound | Message | Description |
|---|---|---|---|---| 
| /sonoff/switch/*ID*/cmd | Inbound | turnON | Sets relay to ON  | 
| /sonoff/switch/*ID*/cmd | Inbound | turnOFF | Sets relay to OFF | 
| /sonoff/switch/*ID*/cmd | Inbound | reportStatus | Requests state of a relay. Sonoff repays with /sonoff/switch/*ID*/state topic | 
| /sonoff/switch/*ID*/cmd | Inbound | reset |  Resets switch | 
| /sonoff/switch/*ID*/cmd | Inbound | tempInterval:N |  Sets how often Sonoff should read value of DS18B20 sensor. N - number of seconds. Example: tempInterval:60 - means every 1 minute. Default: 10min  | 
| /sonoff/switch/*ID*/cmd | Inbound | tempCorrection:N |  Sets a value of temperature correction. Example tempCorrection:-1.2 - means temperature is correcte by -1.2. Default: 0 | 
| /sonoff/switch/*ID*/state | Outbound | ON | Sonoff  publishes this if relay is set to ON by MQTT or manually by pressing Sonoff button |
| /sonoff/switch/*ID*/state | Outbound | OFF | Sonoff  publishes this if relay is set to OFF by MQTT or manually by pressing Sonoff button |
| /sonoff/switch/*ID*/get | Outbound | defaultState | Sonoff switch sends this message to the broker while booting in order to get default value of the relay. If it's not implemented in the MQTT broker then default relay state is set: off | 
| /sonoff/switch/*ID*/temperature | Outbound | Number | Sonoff switch sends temperature from DS18B20 sensor if it was changed between subsequent measures | 


where 
*  _ID_ is a value of particular switch ChipID - it could be set manually to whatever value in the configuration part of a sketch


### Configuration
Configuration part of the sketch

| Parameter  | Description |
|---|---|
| ID | Device ID. It's used in the MQTT Topic to distinguish other similar sensors in your MQTT Broker. You can change it. Use either numbers or chars. Don't use space or special characters, default is ESP8266 ChipID |
| WIFI_SSID  | WiFi network name |
| WIFI_PASSWORD   | WiFi network password |
| MQTT_HOST  | MQTT Broker IP or host name |
| MQTT_PORT  | MQTT Port, default 1883 |
| MQTT_USER  | MQTT User name, leave blank if there is no user authentication in your MQTT broker |
| MQTT_PASSWORD  | MQTT User name, leave blank if there is no user authentication in your MQTT broker |
| MQTT_TOPIC  | MQTT Topic used by switch. Default: /sonoff/switch/ |
| TEMP_INTERVAL  | How often Sonoff should get temperature value from the sensor, default: 600sec. (10min) |
| TEMP_CORRECTION   | Temperature will be corrected by the value set in this parameter, default 0 |

**Installation**

Following files have to be compiled on ESP8266
* sonoff-sb18b29.mqtt.ino should be compiled in Ardruino environemnt

Following libraries are required: **Credits to their authors**
* DallasTemperature
* OneWire
* Ticker
* PubSubClient
* ESP8266WiFi


More info at my [blog](http://smart-house.adrian.czabanowski.com/sonoff-mqtt-ds18b20-oprogramowanie/) (in Polish only)