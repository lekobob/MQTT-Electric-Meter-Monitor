
#include <ArduinoJson.h>
#include <PubSubClient.h>

#include "Meter_Reader_mqtt_esp8266.h"

#ifdef OTA
#include <ESP8266mDNS.h>
#include <ArduinoOTA.h>
#endif



WiFiClient espClient;
PubSubClient mqtt_client(espClient);

unsigned long SEND_FREQUENCY =
    10000; // Minimum time between send (in milliseconds). We don't wnat to spam the gateway.
double ppwh = ((double)PULSE_FACTOR)/1000; // Pulses per watt hour
volatile unsigned long pulseCount = 0;
volatile unsigned long lastBlink = 0;
volatile unsigned long watt = 0;
unsigned long oldPulseCount = 0;
unsigned long oldWatt = 0;
double oldKwh;
unsigned long lastSend;
bool _debugMode = true;



void setup()
{
  Serial.begin(9600);
  pinMode(blueLedPin, OUTPUT);
  digitalWrite(blueLedPin, HIGH);
  //wifiManager.autoConnect(client_id);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    // wait 500ms, flashing the blue LED to indicate WiFi connecting...
    digitalWrite(blueLedPin, LOW);
    delay(250);
    digitalWrite(blueLedPin, HIGH);
    delay(250);
  }

  // startup mqtt connection
  mqtt_client.setServer(mqtt_server, mqtt_port);
  mqtt_client.setCallback(mqttCallback);
  mqttConnect();


#ifdef OTA
  ArduinoOTA.onStart([]() {
    Serial.println("Start");
  });

  ArduinoOTA.setHostname(client_id);
  ArduinoOTA.begin();
  Serial.println("OTA Start");
#endif

  // Use the internal pullup to be able to hook up this sketch directly to an energy meter with S0 output
  // If no pullup is used, the reported usage will be too high because of the floating pin
  pinMode(DIGITAL_INPUT_SENSOR,INPUT_PULLUP); 
  attachInterrupt(digitalPinToInterrupt(DIGITAL_INPUT_SENSOR), onPulse, RISING);
  lastSend=millis();
}

void onPulse()
{;
    unsigned long newBlink = micros();
    unsigned long interval = newBlink-lastBlink;
 
 //   Serial.print("pulse") ;
 //   Serial.println(interval);
    if (interval<10000L) { // Sometimes we get interrupt on RISING
      return;
    }
    watt = (3600000000.0 /interval) / ppwh;
    lastBlink = newBlink;
  
  pulseCount++;
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  // Copy payload into message buffer
  char message[length + 1];
  for (int i = 0; i < length; i++) {
    message[i] = (char)payload[i];
  }
  message[length] = '\0';
  if (strcmp(topic, metereader_debug_set_topic) == 0) { //if the incoming message is on the heatpump_debug_set_topic topic...
    if (strcmp(message, "on") == 0) {
      _debugMode = true;
      mqtt_client.publish(metereader_debug_topic, "debug mode enabled");
    } else if (strcmp(message, "off") == 0) {
      _debugMode = false;
      mqtt_client.publish(metereader_debug_topic, "debug mode disabled");
    }
  } else {
    mqtt_client.publish(metereader_debug_topic, strcat("heatpump: wrong mqtt topic: ", topic));
  }
}

void mqttConnect() {
  // Loop until we're reconnected
  while (!mqtt_client.connected()) {
    // Attempt to connect
    if (mqtt_client.connect(client_id, mqtt_username, mqtt_password)) {
      mqtt_client.subscribe(metereader_debug_set_topic);
    } else {
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void loop()
{
 if (!mqtt_client.connected()) {
    mqttConnect();
  }
#ifdef OTA
  ArduinoOTA.handle();
#endif

  unsigned long now = millis();
  // Only send values at a maximum frequency or woken up from sleep
  bool sendTime = now - lastSend > SEND_FREQUENCY;
  if ( sendTime) {
   
      Serial.print("watt=");
      Serial.print(watt);
      Serial.print(",PulseCount=");
      Serial.println(pulseCount);
      oldWatt = watt;

      const size_t bufferSizeInfo = JSON_OBJECT_SIZE(2);
      DynamicJsonBuffer jsonBufferInfo(bufferSizeInfo);
      JsonObject& rootInfo = jsonBufferInfo.createObject();
      rootInfo["pulsecount"] = pulseCount; //int(myTempRA.getAverage());
      rootInfo["watt"]       = watt;//int(myHumRA.getAverage());

      char bufferInfo[512];
      rootInfo.printTo(bufferInfo, sizeof(bufferInfo));

      if (!mqtt_client.publish(metereader_status_topic, bufferInfo, false)) {
        mqtt_client.publish(metereader_debug_topic, "failed to publish to status topic");
    }


    // Pulse cout has changed
    if (pulseCount != oldPulseCount) {
      double kwh = ((double)pulseCount/((double)PULSE_FACTOR));
      oldPulseCount = pulseCount;
      if (kwh != oldKwh) {
        oldKwh = kwh;
      }
    }
    lastSend = now;
    

  }
  mqtt_client.loop();

}




