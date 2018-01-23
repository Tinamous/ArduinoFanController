#include <MQTTClient.h>
#include <system.h>

// MQTT Settings defined in wifiSecrets.h
WiFiClient networkClient; 
MQTTClient mqttClient; 

bool setupMqtt() {
  Serial.println("Connecting to Tinamous MQTT Server using an UNSECURE connection :-(");
  mqttClient.begin(MQTT_SERVER, 1883, networkClient);
  mqttClient.onMessage(messageReceived);

  connectToMqttServer();
}

bool connectToMqttServer() { 
  if (mqttClient.connected()) {
    return true;
  }

  Serial.println("checking wifi..."); 
  if (WiFi.status() != WL_CONNECTED) { 
    Serial.print("WiFi Not Connected. Status: "); 
    Serial.print(WiFi.status(), HEX); 
    Serial.println();
    
    //WiFi.begin(ssid, pass);
    //delay(1000); 
    return false;
  } 
 
  Serial.println("Connecting to MQTT Server..."); 
  //if (!mqttClient.connect(MQTT_CLIENT_ID, MQTT_USERNAME, MQTT_PASSWORD)) { 
  if (!mqttClient.connect("Fans", "OfficeFans.ddd", "OfficeFansPassw0rd")) { 
    Serial.println("Failed to connect to MQTT Server."); 
    Serial.print("Error: "); 
    Serial.print(mqttClient.lastError()); 
    Serial.print(", Return Code: "); 
    Serial.print(mqttClient.returnCode()); 
    Serial.println(); 

    if (mqttClient.lastError() == LWMQTT_CONNECTION_DENIED) {
      Serial.println("Access denied. Check your username and password. Username should be 'DeviceName.AccountName' e.g. MySensor.MyHome"); 
    }
    
    delay(1000); 
    return false;
  } 
 
  Serial.println("Connected!"); 
 
  mqttClient.subscribe("/Tinamous/V1/Status.To/" DEVICE_USERNAME); 
  mqttClient.subscribe("/Commands/Fans"); 
  mqttClient.subscribe("/Tinamous/V1/Commands/" DEVICE_USERNAME "/Fans"); 
  mqttClient.subscribe("/Tinamous/V1/Commands/" DEVICE_USERNAME "/#"); // Subscribe to them all...
  Serial.println("Subscribed."); 
  // client.unsubscribe("/hello"); 

  return true;
} 

void mqttLoop() {
  // Call anyway, does nothing if already connected.
  connectToMqttServer();
  
  mqttClient.loop(); 
}

void publishTinamousStatus(String message) {
  Serial.println("Status: " + message);
  mqttClient.publish("/Tinamous/V1/Status", message); 
}

void publishTinamousJsonMeasurements(String json) {
  Serial.println("Measurement: " + json);
  mqttClient.publish("/Tinamous/V1/Measurements/Json", json); 
}

void publishTinamousSenMLMeasurements(String senml) {
  Serial.println("SenML Measurement: " + senml);
  mqttClient.publish("/Tinamous/V1/Measurements/SenML", senml); 
}

void handleFansCommands(String &topic, String &payload) {
int value;

  if (topic == "/Fans/Power") {        
    value = payload.toInt();
    Serial.print("Handle fan power. Requested: ");
    Serial.print(value);
    Serial.println();
  } else if (topic == "/Fans/SetSpeed") {
    value = payload.toInt();
    Serial.print("Handle fan speed. Speed Requested: ");
    Serial.print(value); // 0..11
    Serial.println();
  } else {
    Serial.println("Unknown FAN command!");    
    publishTinamousStatus("Hello! Sorry I don't know that command. Please check your MQTT topic. Command: ");
  }
}

void handleLedsCommands(String &topic, String &payload) {
int value;
  
  if (topic == "/Leds/Power") {
    value = payload.toInt();
    Serial.println("Handle ledspower");
  } else if (topic == "/Leds/Brightness") {
    Serial.println("Handle leds brightness");
  } else if (topic == "/Leds/Fan1/DisplayType") {
    Serial.println("Handle fan 1 led display type");
  } else if (topic == "/Leds/Fan2/DisplayType") {
    Serial.println("Handle fan 2 led display type");
  } else if (topic == "/Leds/Fan3/DisplayType") {
    Serial.println("Handle fan 3 led display type");
  } else if (topic == "/Leds/Fan4/DisplayType") {
    Serial.println("Handle fan 4 led display type");
  } else if (topic == "/Leds/Strip/DisplayType") {
    Serial.println("Handle led strip display type");
  } else {
    Serial.println("Unknown LED command!");    
    publishTinamousStatus("Hello! Sorry I don't know that command. Please check your MQTT topic. Command: ");
  } 
}

void handleCommand(String &topic, String &payload) {
  // remove the common bit of the topic and handle just the specific command
  String baseCommand = "/Tinamous/V1/Commands/" DEVICE_USERNAME;
  // Caution! Replaces topic!
  topic.replace(baseCommand, "");
  Serial.print("Handle command: ");
  Serial.println(topic);

  if (topic.startsWith("/Fans/")) {
    handleFansCommands(topic, payload);
    return ;
  }

  if (topic.startsWith("/Leds/")) {
    handleLedsCommands(topic, payload);
    return;
  }

  Serial.print("Unknown command:");    
  Serial.println(topic);    
  publishTinamousStatus("Hello! Sorry I don't know that command. Please specify Leds or Fans in the topic.");
}

void messageReceived(String &topic, String &payload) { 
  Serial.println("Message from Tinamous on topic: " + topic + " - " + payload); 
  // If the payload starts with "/Tinamous/V1/Commands/" DEVICE_USERNAME 
  // then we should handle that...
  if (topic.startsWith("/Tinamous/V1/Commands/" DEVICE_USERNAME)) {
    handleCommand(topic, payload);
    return;
  }

  
  // Custom topic (old).
  if (topic == "/Commands/Fans") {
    if (payload == "On") {
      Serial.println("TURN ON THE FANS!!! (Old topic)");
    }
    
    if (payload == "OFF") {
      Serial.println("Fans off please. (Old topic)");
    }

    return;
  } 

  // Didn't get an expected command, so the message was to us.
  // Publish a help message.
  publishTinamousStatus("Hello! Please use the topic '/Tinamous/V1/Commands/" DEVICE_USERNAME "' to control the fans.");
} 

