/***************************************************
 ****************************************************/
#include <ESP8266WiFi.h>
#include <PubSubClient.h>


/************************* WiFi Access Point *********************************/
#define NW_WAGNER
#include "mynetwork.h"


/************ Global State (you don't need to change this!) ******************/


const char* ssid        = MY_WLAN_SSID;
const char* password    = MY_WLAN_PWD;

int pin_state = 0;


WiFiClient espClient;
PubSubClient client(espClient);


#define PIN_LED  2
#define LED_SUB     "/test/led"
#define UPTIME_PUB  "/test/uptime"



void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.print(ssid);
  Serial.println("...");

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}



void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '0') {
    pin_state = 0;
    Serial.println("  Pin -> LOW");
  } else if ((char)payload[0] == '1') {
    pin_state = 1;
    Serial.println("  Pin -> HIGH");
  } else if ((char)payload[0] == '2') {
    pin_state = 1 - pin_state;
    Serial.print("  Pin -> switched to ");
    Serial.println(pin_state);
  }
  // buildin LED: LOW= on, HIGH= off
  digitalWrite(PIN_LED, (pin_state==1) ? LOW : HIGH);
}


void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("MQTT server: ");
    Serial.print(MQTT_SERVER);
    Serial.print(":");
    Serial.println(MQTT_SERVERPORT);
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client", MQTT_USERNAME, MQTT_PASSWORD)) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish(UPTIME_PUB, "0");
      // ... and resubscribe
      client.subscribe(LED_SUB);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
    }
  }
}



void setup() {
  pinMode(PIN_LED, OUTPUT);
 
  Serial.begin(115200);
  setup_wifi();
  client.setServer(MQTT_SERVER, MQTT_SERVERPORT);
  client.setCallback(callback);
}


void loop() {
  char buf[50];
  
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  delay(1000);
  sprintf(buf, "%lu", millis()/1000); 
  client.publish(UPTIME_PUB, buf);
}

// -EOF-
