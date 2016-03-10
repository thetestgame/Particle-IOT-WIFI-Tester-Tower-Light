# Particle-IOT-WIFI-Tester-Tower-Light
<b>V. 1.0.0</b><br/>
IOT Tower Light that displays your internet status based on the Particle Photon. Designed for the <a href="https://www.adafruit.com/products/2993">Tower Light - Red Yellow Green Alert Light with Buzzer</a> from <a href="adafruit.com">Adafruit</a>.

<hr/>
### Supported Services
* MQTT
* Particle Cloud (Restful and Subscription events)

<hr/>

### Dependencies
 * <a href="https://github.com/hirotakaster/MQTT">MQTT</a>
 * 
<hr/>

### Ping Configuration
<b>More documentation will be added later!<b/>
<hr/>

### MQTT
<b>All feeds respond to the values "ON" for enable and "OFF" for disable</b><br/>
Set the feed value to "" to disable the feed.
* mqttRedLightFeed - Controls the red light pin
* mqttYellowLightFeed - Controls the yellow light pin
* mqttGreenLightFeed - Controls the green light pin
* mqttBuzzerFeed - Controls the buzzer pin
* mqttPowerFeed - Controls the "power" of the device. Sending "OFF" to the feed will prevent the lights from turning on until "ON" is sent (Not currently implemented)

<hr/>
### Particle Cloud
<b>More documentation will be added later!<b/>
<hr/>

### License
<a href="https://github.com/thetestgame/Particle-IOT-WIFI-Tester-Tower-Light/blob/master/LICENSE">MIT</a>
