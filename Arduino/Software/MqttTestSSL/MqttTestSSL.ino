#include <MQTTClient.h>
#include <system.h>

#include <WiFi101.h>

#include "wifiSecrets.h" 

unsigned long lastMessageSent = 0;

void setup() {
  // Setup debug LED.
  pinMode(6, OUTPUT);
  
  //Initialize serial and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  // Give us a small delay to open the serial monitor...
  delay(5000);
  
  setupWiFi();

  //delay(5000);
  setupMqtt();
}

void loop() {
  digitalWrite(6, LOW);
  
  // check the network connection once every 10 seconds:
  reconnectWiFi();
  //printCurrentNet();
  //printWiFiData();
  //doPing();
  delay(500);

  // MQTT connection needs to be kept alive and 
  // checked for incomming data.
  mqttLoop(); 

  publishFakeMeasurements();

  digitalWrite(6, HIGH);
  delay(500);
} 

void publishFakeMeasurements() {

  // Every 5 minutes.
  if (millis() - lastMessageSent > (5 * 60 *1000)) { 
  //if (millis() - lastMessageSent > (10 *1000)) { 
    Serial.println("------------------------"); 
    Serial.println("MQTT Publish!"); 
    lastMessageSent = millis(); 

    publishTinamousStatus("Hello World!");

    String json = "{'Temperature':";
    json = json + "23.5";
    json = json + ",'Humidity':";
    json = json + "'60'";
    json = json +  "}";
    publishTinamousJsonMeasurements(json);

    // And do one as senml...
    String senml = "{'e':[";
    // First sensor
    senml = senml + "{'n':'Light',";
    senml = senml + "'v':100,";
    senml = senml + "'u':'lux'},";
    // Second sensor
    senml = senml + "{'n':'VOC',";
    senml = senml + "'v':'1001',";
    senml = senml + "'u':'ppm'}";
    senml = senml +  "]}";
    publishTinamousSenMLMeasurements(senml);
    Serial.println("------------------------"); 
    Serial.println(); 
  }   
}



