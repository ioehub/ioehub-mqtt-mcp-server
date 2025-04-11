#include <WiFi.h>
#include <PubSubClient.h>

#include <ArduinoJson.h>
#include <DHT.h>
//#include <esp32DHT.h>

#include <ESP32Servo.h>


// WiFi
const char *ssid = "IoEHubSSID"; // Enter your WiFi name
const char *password = "password";  // Enter WiFi password

// MQTT Broker
const char *mqtt_broker = "172.30.1.100";
const char *topic = "ioehub/mcp/command";
const char *mqtt_username = "ioehub";
const char *mqtt_password = "public";
//const int mqtt_port = 9883;
const int mqtt_port = 1883;

WiFiClient espClient;
PubSubClient client(espClient);
void callback(char* topic, byte* payload, unsigned int length);


//DHT22 sensor;
 
void setup() 
{
    // Set software serial baud to 115200;
    Serial.begin(9600);
    Serial.println("start...");
    delay(1000);
    // connecting to a WiFi network
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) 
    {
        delay(500);
        Serial.println("Connecting to WiFi..");
    }
    Serial.println("Connected to the WiFi network");
    //connecting to a mqtt broker
    client.setServer(mqtt_broker, mqtt_port);
    client.setCallback(callback);
    while (!client.connected()) {
        String client_id = "ioehub-hello-esp32-client-";
        client_id += String(WiFi.macAddress());
        Serial.printf("The client %s connects to the public mqtt broker\n", client_id.c_str());
        if (client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
            Serial.println("Public emqx mqtt broker connected");
        } else {
            Serial.print("failed with state ");
            Serial.print(client.state());
            delay(2000);
        }
    }
    // publish and subscribe
   // client.publish(topic, "Hi EMQ X I'm ESP32 ^^");
    client.subscribe(topic);
/*
     sensor.setup(13);  // optionally use another RMT channel: sensor.setup(23, RMT_CHANNEL_2);
     sensor.onData([](float humid, float temp) {
      Serial.printf("Temp: %.1f°C\nHumid: %.1f%%\n", temp, humid);
      Serial.println("");
    
    });
    sensor.onError([](uint8_t error) {
      Serial.printf("Error: %d-%s\n", error, sensor.getError());
      Serial.println("");
    });
   */  
}

/*
void callback(char *topic, byte *payload, unsigned int length) 
{
   Serial.print("Message arrived in topic: ");
   Serial.println(topic);
   Serial.print("Message:");
   for (int i = 0; i < length; i++) {
       Serial.print((char) payload[i]);
   }
   Serial.println();
   Serial.println("-----------------------");
}

*/

#include "procMcp.h"
#include "func.h"

int cnt=0;
void loop() 
{
    char buf[256];
    cnt++;
    // MQTT 연결 확인
    if (!client.connected()) {
        reconnect();
    }    
      client.loop();
    //sprintf(buf,"YEJIS [%d]",cnt);
    //client.publish(topic, buf);
    //client.subscribe(topic);

}
