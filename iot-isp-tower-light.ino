/*
The MIT License (MIT)

Copyright (c) 2016 Jordan Maxwell

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "MQTT/MQTT.h"

/***********************************
 * general settings definitions
 * *********************************/ 
 
//DEBUG
#define SERIAL_DEBUG // Unncomment this to see general debug prints

// check IP
IPAddress remoteIP(0,0,0,0);
int numberOfReceivedPackage;
unsigned long lastTime = 0UL;

/***********************************
 * tower light settings definitions
 * ********************************/

int towerPowered = 1; //Do not touch. Used for checking if tower is "on"

// Tower Light pin defintions
// Comment out a defintion to disable that light
#define RED_LIGHT 0
#define YELLOW_LIGHT 1
#define GREEN_LIGHT 2
#define BUZZER 3

//Enable/Disable light catcher. Light catcher checks prevents two lights from 
// being on at the same time.
bool enableLightCatch = true;
//Enable/Disable to change light catcher functionality to switch lights instead of prevent
bool lightCatchSwitch = true; 

//Do not touch. Used for checking status of lights
int redActive = 0; 
int yellowActive = 0;
int greenActive = 0;
int buzzerActive = 0;

/***********************************
 * particle settings definitions
 * *********************************/ 

// PARTICLE RESTFUL API

//Enable/Disable Particle cloud breakout api
bool particleApi = true; 

//Enable/Disable control over if the tower light is "on" or "off" from the Particle Cloud
//Requires particleApi to be set to true
bool cloudPowerControl = true;

// PARTICLE FEEDS

 //Enable/Disable Particle cloud feed functionality
bool particleFeeds = false;

//Feeds to subscribe to for light change requests
//Change the feed to "" to disable it
//Particle feeds require particleFeeds to be set to true
String particleRedLightFeed = "";
String particleYellowLightFeed = "";
String particleGreenLightFeed = "";
String particleBuzzerFeed = "";
String particlePowerFeed = "";
 
/***********************************
 * MQTT settings definitions
 * *********************************/ 
 
//Enable/Disable MQTT functionality 
bool mqttEnabled = true;
char* mqttHost = "HOSTNAMEHERE";
int mqttPort = 1883;
String mqttUsername = "USERNAME";
String mqttPassword = "PASSWORD";

//MQTT feeds to subscribe to for light change requests
//Change the feed to "" to disable it
//MQTT feeds require mqttEnabled to be set to true
String mqttRedLightFeed = "";
String mqttYellowLightFeed = "";
String mqttGreenLightFeed = "";
String mqttBuzzerFeed = "";
String mqttPowerFeed = "";

//Do not change. MQTT client declaration
void MQTTcallback(char* topic, byte* payload, unsigned int length);
MQTT client(mqttHost, mqttPort, MQTTcallback);

/**************************************
 * startup and loop functions *
 * ************************************/

void setup() {
    Serial.begin(9600);
    initPins();
    initMQTT();
    initParticleVariables();
}

void loop() {
    if (client.isConnected()) {
        client.loop(); 
    } else {
        initMQTT();
    }

    if (particleFeeds || particleApi) {
        Particle.process();
    }  
    
    testInternet();   
}

void testInternet() {
    unsigned long now = millis();
    //Tower is not powered
    if (towerPowered == 0) {
        setLight(GREEN_LIGHT, LOW);
        setLight(YELLOW_LIGHT, LOW);
        setLight(RED_LIGHT, LOW);
            
        publishMQTT(mqttRedLightFeed, "OFF");
        publishMQTT(mqttYellowLightFeed, "OFF");
        publishMQTT(mqttGreenLightFeed, "OFF");        
        return;
    }
    //Every 15 seconds check if host is up or down
    if (now-lastTime>15000UL) {
        lastTime = now;
        // now is in milliseconds
        numberOfReceivedPackage = WiFi.ping(remoteIP);
        if (numberOfReceivedPackage > 0) {
            setLight(GREEN_LIGHT, HIGH);
            setLight(YELLOW_LIGHT, LOW);
            setLight(RED_LIGHT, LOW);
            
            publishMQTT(mqttRedLightFeed, "OFF");
            publishMQTT(mqttYellowLightFeed, "OFF");
            publishMQTT(mqttGreenLightFeed, "ON");
        } else {
            setLight(GREEN_LIGHT, LOW);
            setLight(YELLOW_LIGHT, LOW);
            setLight(RED_LIGHT, HIGH);
            
            publishMQTT(mqttRedLightFeed, "ON");
            publishMQTT(mqttYellowLightFeed, "OFF");
            publishMQTT(mqttGreenLightFeed, "OFF");           
        }
    }
}

void initPins() {
    pinMode(RED_LIGHT, OUTPUT);
    pinMode(YELLOW_LIGHT, OUTPUT);
    pinMode(GREEN_LIGHT, OUTPUT);
    pinMode(BUZZER, OUTPUT);
}


/***********************************
 * debug functions and definitions *
 * *********************************/
 
void debug(String message) {
#ifdef SERIAL_DEBUG
    Serial.print(message);
#endif
}

void debugln(String message) {
#ifdef SERIAL_DEBUG
    Serial.println(message);
#endif
}

/***********************************
 * tower functions and definitions *
 * *********************************/

bool setLight(int pin, int value) {
    digitalWrite(pin, value);
    return true;
}

/**************************************
 * MQTT API functions and definitions *
 * ************************************/

void initMQTT() {
    if (mqttEnabled) {
        client.connect(mqttHost, mqttUsername, mqttPassword);   
        if (client.isConnected()) {
            MQTTSubscribe(mqttRedLightFeed);
            MQTTSubscribe(mqttYellowLightFeed);
            MQTTSubscribe(mqttGreenLightFeed);
            MQTTSubscribe(mqttBuzzerFeed);
            MQTTSubscribe(mqttPowerFeed);
        } else {
            debugln("Failed to initalize MQTT services!");
        }
    }
}

void MQTTSubscribe(String feed) {
    if (feed != "") {
        client.subscribe(feed);
    }
}

void MQTTcallback(char* topic, byte* payload, unsigned int length) {
    char p[length + 1];
    memcpy(p, payload, length);
    p[length] = NULL;
    String message(p);
    debugln("Received MQTT data (" + message + ") from " + topic);
    String feed(topic);
    if (feed.equals(mqttRedLightFeed)) {
        if (message.equals("ON"))
            setLight(RED_LIGHT, HIGH);
        else
            setLight(RED_LIGHT, LOW);
    }

    if (feed.equals(mqttYellowLightFeed)) {
        if (message.equals("ON"))
            setLight(YELLOW_LIGHT, HIGH);
        else
            setLight(YELLOW_LIGHT, LOW);
    }
    
    if (feed.equals(mqttGreenLightFeed)) {
        if (message.equals("ON"))
            setLight(GREEN_LIGHT, HIGH);
        else
            setLight(GREEN_LIGHT, LOW);
    }
    
    if (feed.equals(mqttBuzzerFeed)) {
        if (message.equals("ON"))
            setLight(BUZZER, HIGH);
        else
            setLight(BUZZER, LOW);
    }
    
    if (feed.equals(mqttPowerFeed)) {
        if (message.equals("ON")) {
            towerPowered = 1;
        } else {
            towerPowered = 0;
        }
    }
}

bool publishMQTT(String feed, String payload) {
    if (feed == "")
        return false;
    if (!mqttEnabled) 
        return false;
    if (!client.isConnected()) {
        debugln("Failed to publish to mqtt feed (" + feed + "). No MQTT connection.");
        return false;
    }
    client.publish(feed, payload);
    return true;
}

/******************************************
 * Particle API functions and definitions *
 * ***************************************/
 
//initializes the shared variables and functions that are accessible through the particle API
void initParticleVariables() {
    if (particleApi) {
        debugln("Initializing Particle cloud callbacks...");
        
        //Particle Cloud API variable defintions
        Particle.variable("redLightState", &redActive, INT);
        Particle.variable("yellowLightState", &yellowActive, INT);
        Particle.variable("greenLightState", &greenActive, INT);
        Particle.variable("buzzerState", &buzzerActive, INT);
        Particle.variable("powerState", &towerPowered, INT);
        
        //Particle Cloud API function callback defintions
        Particle.function("activeLight", (int (*)(String)) enableLight);
        Particle.function("deactiveLight", (int (*)(String)) disableLight);
        
        if (cloudPowerControl)
            Particle.function("setPowerState", (int (*)(String)) setPowerState);
    }
    
    if (particleFeeds) {
        debugln("Initializing Particle cloud feed subscriptions...");
        Particle.subscribe(particleRedLightFeed, ParticleHandler);
        Particle.subscribe(particleYellowLightFeed, ParticleHandler);
        Particle.subscribe(particleGreenLightFeed, ParticleHandler);
        Particle.subscribe(particleBuzzerFeed, ParticleHandler);
    }
    
    if (particleFeeds || particleApi)
        debugln("Particle cloud ready!");
    Particle.connect();
}

int enableLight(String incoming) {
    int pin = incoming.toInt();
    bool success = setLight(pin, HIGH);
    if (success && towerPowered)
        return 1;
    else 
        return 0;    
}

int disableLight(String incoming) {
    int pin = incoming.toInt();
    bool success = setLight(pin, LOW);
    if (success && towerPowered)
        return 1;
    else 
        return 0;
}

int setPowerState(String incoming) {
    int state = incoming.toInt();
    towerPowered = state;
    return state;
}

void ParticleHandler(const char *event, const char *data) {

}

bool publishParticle(String feed, String payload) {
    if (feed == "")
        return false;
    if (!particleFeeds) 
        return false;
    if (!Particle.connected()) {
        debugln("Failed to publish to Particle feed (" + feed + "). No Particle Cloud connection.");
        return false;
    }
    Particle.publish(feed, payload);
    return true;
}
