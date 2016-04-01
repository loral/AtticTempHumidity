/* ESP8266 + MQTT Attic Temperature and Humidity Node
   Can also receive commands; adjust messageReceived() function
   Modified from a sketch from MakeUseOf.com
   Written by: Loral Godfrey, March 31, 2016
*/

#include <MQTTClient.h>
#include <ESP8266WiFi.h>
#include <DHT.h>

// MQTT
const char* ssid = "ssid";
const char* password = "password";

String subscribeTopic = "openhab/attic/esp8266"; // subscribe to this topic; anything sent here will be passed into the messageReceived function
String tempTopic = "openhab/attic/temperature"; //topic to publish temperatures readings to
String humidityTopic = "openhab/attic/humidity"; //topic to publish humidity readings to
String heatIndexTopic = "openhab/attic/heat_index"; //topic to publish heat index readings to
const char* server = "192.168.1.22"; // server or URL of MQTT broker
String clientName = "ESP8266_attic"; // just a name used to talk to MQTT broker

WiFiClient wifiClient;
MQTTClient client;

String hPayload;
String tPayload;
String hiPayload;

// Temperatrue and Humidity
#define DHTTYPE DHT22 // DHT11 or DHT22
#define DHTPIN  5

DHT dht(DHTPIN, DHTTYPE, 11);

float t, h, i;
unsigned long previousMillis = 0;
const long getTempInterval = 60000; // in milliseconds
unsigned long currentMillis = millis();

// Create custom name for MQTT using mac address
String macToStr(const uint8_t* mac)
{
  String result;
  for (int i = 0; i < 6; ++i) {
    result += String(mac[i], 16);
    if (i < 5)
      result += ':';
  }
  return result;
}

void setup() {
  Serial.begin(115200);
  dht.begin();
  client.begin(server, wifiClient);
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // Generate client name based on MAC address and last 8 bits of microsecond counter
  uint8_t mac[6];
  WiFi.macAddress(mac);
  clientName += macToStr(mac);
  clientName += "-";
  clientName += String(micros() & 0xff, 16);

  Serial.print("Connecting to ");
  Serial.print(server);
  Serial.print(" as ");
  Serial.println(clientName);

  if (client.connect((char*) clientName.c_str())) {
    Serial.println("Connected to MQTT broker");
    Serial.print("Subscribed to: ");
    Serial.println(subscribeTopic);
    client.subscribe(subscribeTopic);
  }
  else {
    Serial.println("MQTT connect failed");
    Serial.println("Will reset and try again...");
    abort();
  }
}

void loop() {
  currentMillis = millis();
  if (currentMillis - previousMillis >= getTempInterval)
  {
    previousMillis = currentMillis;
    
    h = dht.readHumidity();
    t = dht.readTemperature(true);
    i = dht.computeHeatIndex(t, h);
    
    if (isnan(t) || isnan(h) || !client.connected()) 
    {
      Serial.println("Something wasn't right, waiting to try again.");
    }
    else {
      tPayload = String(t,2);
      Serial.println(tPayload);
      hPayload = String(h,2);
      Serial.println(hPayload);
      hiPayload = String(i,2);
      Serial.println(hiPayload);
      client.publish(tempTopic, tPayload);
      Serial.println("published temperatrue");
      client.publish(humidityTopic, hPayload);
      Serial.println("published humidity");
      client.publish(heatIndexTopic, hiPayload);
      Serial.println("published heat index");
    }
  }
}

void messageReceived(String topic, String payload, char * bytes, unsigned int length) {
  Serial.print("incoming: ");
  Serial.print(topic);
  Serial.print(" - ");
  Serial.print(payload);
  Serial.println();
}
