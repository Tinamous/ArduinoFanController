#include <MQTTClient.h>
#include <system.h>
#include "secrets.h"

// Be sure to use WiFiSSLClient for an SSL connection.
// for a non ssl (port 1883) use regular WiFiClient.
//WiFiClient networkClient; 
WiFiSSLClient networkClient; 

// MQTT Settings defined in wifiSecrets.h
// Set buffer size as the default is WAY to small!.
MQTTClient mqttClient(4096); 

// If we have been connected since powered up 
bool was_connected = false;
String senml = "";
unsigned long nextSendMeasurementsAt = 0;

bool setupMqtt() {
  senml.reserve(4096);
  Serial.print("Connecting to Tinamous MQTT Server on port:");
  Serial.println(MQTT_SERVERPORT);
  mqttClient.begin(MQTT_SERVER, MQTT_SERVERPORT, networkClient);

  // Handle received messages.
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
  if (!mqttClient.connect(MQTT_CLIENT_ID, MQTT_USERNAME, MQTT_PASSWORD)) { 
  //if (!mqttClient.connect("Fans", "OfficeFans.ddd", "OfficeFansPassw0rd")) { 
    Serial.println("Failed to connect to MQTT Server."); 
    Serial.print("Error: "); 
    Serial.print(mqttClient.lastError()); 
    Serial.print(", Return Code: "); 
    Serial.print(mqttClient.returnCode()); 
    Serial.println(); 

    if (mqttClient.lastError() == LWMQTT_CONNECTION_DENIED) {
      Serial.println("Access denied. Check your username and password. Username should be 'DeviceName.AccountName' e.g. MySensor.MyHome"); 
    }
    
    delay(10000); 
    return false;
  } 
 
  Serial.println("Connected!"); 
 
  mqttClient.subscribe("/Tinamous/V1/Status.To/" DEVICE_USERNAME); 

  // Subscribe to external data feeds
  for (int i=0; i<4; i++) {
    if (mqttFeedsTopic[i] != "") {
      Serial.println("Subscribing to " + mqttFeedsTopic[i]);
      mqttClient.subscribe(mqttFeedsTopic[i]);
    }
  }
  
  //mqttClient.subscribe("/Commands/Fans"); 
  mqttClient.subscribe("/Tinamous/V1/Commands/" DEVICE_USERNAME "/Fans"); 
  //mqttClient.subscribe("/Tinamous/V1/Commands/" DEVICE_USERNAME "/#"); // Subscribe to them all...
  Serial.println("Subscribed."); 
  // client.unsubscribe("/hello"); 

  // Say Hi.
  if (was_connected) {
    publishTinamousStatus("Ouch. Appears we got disconnected, well I'm back now.");
  } else {
    publishTinamousStatus("Hello! I'm your biggest (and brightest) fan ;-) #SeeWhatIDidThere. Use '@" DEVICE_USERNAME " Help' for help.");
  }

  was_connected = true;
  return true;
} 

void mqttLoop() {
  // Call anyway, does nothing if already connected.
  connectToMqttServer();
  
  mqttClient.loop(); 

  sendMeasurements();
}

void sendMeasurements() {
  if (!mqttClient.connected()) {
    Serial.println("Not connected, not sending sensor measurements");
    delay(1000);
    return;
  }
  
  if (nextSendMeasurementsAt < millis()) {
    // Send the measurements
    Serial.println("Sending sensor measurements to Tinamous");

    senml = "{'e':[";
    // Temperature
    senml = senml + "{'n':'Temperature',";
    senml = senml + "'v':";
    senml = senml + temperature;
    senml = senml + ",";
    senml = senml + "'u':'°C'},";
    // Humidity
    senml = senml + "{'n':'humidity',";
    senml = senml + "'v':";
    senml = senml + humidity;
    senml = senml + ",";
    senml = senml + "'u':'%'},";
    // pressure
    senml = senml + "{'n':'Pressure',";
    senml = senml + "'v':";
    senml = senml + pressure;
    senml = senml + "},";
    // TVOC
    senml = senml + "{'n':'TVOC',";
    senml = senml + "'v':";
    senml = senml + tVOC;
    senml = senml + "},";   
    // eCO2
    senml = senml + "{'n':'eCO2',";
    senml = senml + "'v':";
    senml = senml + eCO2;
    senml = senml + "},";
        // eCO2
    senml = senml + "{'n':'light',";
    senml = senml + "'v':";
    senml = senml + light;
    senml = senml + "},";
    // RSSI
    senml = senml + "{'n':'rssi',";
    senml = senml + "'v':";
    senml = senml + rssi;
    senml = senml + "},";
    // Speed selected
    senml = senml + "{'n':'setSpeed',";
    senml = senml + "'v':";
    senml = senml + fanInfos[0].speedSet;
    senml = senml + "},";
    // Master power state
    int power = isMasterPowerEnabled() ? 1 : 0;
    senml = senml + "{'n':'masterPower',";
    senml = senml + "'v':";
    senml = senml + power;
    senml = senml + "},";
    // Fan speed (RPM)
    for (int fanId=0; fanId<4; fanId++) {
      senml = senml + "{'n':'FanRpm-" + fanId;
      senml = senml + "','v':";
      senml = senml + fanInfos[fanId].computedRpm;
      senml = senml + "},";
    }
    // Display mode for the fans
    for (int fanId2=0; fanId2<4; fanId2++) {
      senml = senml + "{'n':'Display-"+fanId2;
      senml = senml + "','v':";
      senml = senml + fanDisplayModes[fanId2];
      senml = senml + "},";
    }
    // ledBrightness
    senml = senml + "{'n':'ledBrightness',";
    senml = senml + "'v':";
    senml = senml + ledBrightness;
    senml = senml + "},";
    // ledsEnabled
    senml = senml + "{'n':'ledsEnabled',";
    senml = senml + "'v':";
    senml = senml + ledsEnabled;
    senml = senml + "},";
    // ledBrightness (Last)
    senml = senml + "{'n':'ledBrightness',";
    senml = senml + "'v':";
    senml = senml + ledBrightness;
    senml = senml + "}";
    senml = senml +  "]}";
    
    publishTinamousSenMLMeasurements(senml);

    nextSendMeasurementsAt = millis() + (30 * 1000);
  }
}

void publishTinamousStatus(String message) {
  Serial.println("Status: " + message);
  mqttClient.publish("/Tinamous/V1/Status", message); 

  if (mqttClient.lastError() != 0) {
    Serial.print("MQTT Error: "); 
    Serial.print(mqttClient.lastError()); 
    Serial.println(); 
  }

  if (!mqttClient.connected()) {
    Serial.print("Not connected after publishing status. What happened?");
  } else {
    Serial.print("Message sent.");
  }
}

void publishTinamousJsonMeasurements(String json) {
  Serial.println("Measurement: " + json);
  mqttClient.publish("/Tinamous/V1/Measurements/Json", json); 
}

void publishTinamousSenMLMeasurements(String senml) {
  if (senml.length()> 4096) {
    Serial.println("senml longer than buffer. Ignoring!!!");
    return;
  }
  
  Serial.println("SenML Measurement: " + senml);
  mqttClient.publish("/Tinamous/V1/Measurements/SenML", senml); 
}

// /Fans/Power + value in payload (1 = on, 0 = off).
void handleFansCommands(String &topic, String &payload) {
int value;

  if (topic == "/Fans/Power") {        
    value = payload.toInt();
    Serial.print("Handle fan power. Requested: ");
    Serial.print(value);
    Serial.println();
    setPower(value);
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

// Commands:
// /Leds/Power + power in payload (1 on, 0 off)
// /Leds/Brightness + brigthness in payload (0..255)
// /Leds/Fanx/DisplayMode x = fan (1-4) + display mode in payload.
// /Leds/Strip/DisplayMode + display mode in payload.
void handleLedsCommands(String &topic, String &payload) {
int value;
  
  if (topic == "/Leds/Power") {
    value = payload.toInt();
    // turn on/off the LEDs.
    ledsEnabled =  (DisplayMode)value;
    Serial.println("Setting ledsEnabled: " + ledsEnabled);
  } else if (topic == "/Leds/Brightness") {
    // Set the brightness
    ledBrightness =  (DisplayMode)value;
    Serial.println("Setting leds brightness: " + ledBrightness);
  } else if (topic == "/Leds/Fan1/DisplayMode") {
    // Set display type for LED1
    Serial.println("Handle fan 1 led DisplayMode: " + value);
    fanDisplayModes[0] =  (DisplayMode)value;
  } else if (topic == "/Leds/Fan2/DisplayModey") {
    Serial.println("Handle fan 2 led DisplayMode: " + value);
    fanDisplayModes[1] =  (DisplayMode)value;
  } else if (topic == "/Leds/Fan3/DisplayMode") {
    Serial.println("Handle fan 3 led DisplayMode: " + value);
    fanDisplayModes[2] =  (DisplayMode)value;
  } else if (topic == "/Leds/Fan4/DisplayType") {
    //Serial.println("Handle fan 4 led DisplayMode: " + value);
    fanDisplayModes[3] = (DisplayMode)value;
  } else if (topic == "/Leds/Strip/DisplayMode") {
    Serial.println("Handle led strip DisplayMode");
    // TODO: When we know how to handle this...
  } else {
    Serial.println("Unknown LED command!");    
    publishTinamousStatus("Hello! Sorry I don't know that command. Please check your MQTT topic. Command: ");
  } 
}

// Expect this to be a simple value containing payload
void handleCustomMqttFeed(int index, String &topic, String &payload) {
int value = payload.toInt();
  Serial.println("Setting custom mqtt feed value to " + value);
  mqttFeedsValue[index] = payload.toInt();
}

// Commands:
// /Sleep/Now - sleeps NOW Fans and LEDs full off.
// /Sleep/At + value in payload
void handleSleepCommand(String &topic, String &payload) {
  int value;
  
  if (topic == "/Sleep/At") {
    value = payload.toInt();
    // Setup sleep mode at this tine
    Serial.println("Sleep at: " + value); 
  } else if (topic == "/Sleep/Now") {
    Serial.println("Sleep!"); 
  }
}

// Commands:
// /Sleep/Now - sleeps NOW Fans and LEDs full off.
// /Sleep/At + value in payload
void handleWakeCommand(String &topic, String &payload) {
  int value;
  
  if (topic == "/Wake/At") {
    value = payload.toInt();
    // Setup sleep mode at this tine
    Serial.println("Wake at: " + value); 
  } else if (topic == "/Wake/Now") {
    Serial.println("Wake!"); 
    // Power on,
    // fans to x
    // master power on
  }
}


// Handle a command that's come in via the MQTT subscription
void handleCommand(String &topic, String &payload) {

  for (int i=0; i<4; i++) {
    if (mqttFeedsTopic[i] == topic) {
      handleCustomMqttFeed(i, topic, payload);
      return;
    }
  }
 
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

  // what are we doing with this? Use display instead?
  if (topic.startsWith("/Leds/")) {
    handleLedsCommands(topic, payload);
    return;
  }

  // /Sleep/Now | /Sleep/At/
  if (topic.startsWith("/Sleep/")) {
    handleSleepCommand(topic, payload);
    return;
  }

  // /Wake/Now | /Wake/At/
  if (topic.startsWith("/Wake/")) {
    handleWakeCommand(topic, payload);
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
  
  // A status post to me?
  if (payload.startsWith("@")) {
    payload.toLowerCase();
    if (handleStatusMessage(payload)) {
      return;
    }
  } 

  publishTinamousStatus("Unknown message. use help.");

  // Didn't get an expected command, so the message was to us.
  // Publish a help message.
  //publishTinamousStatus("Hello! Please use the topic '/Tinamous/V1/Commands/" DEVICE_USERNAME "' to control the fans.");
} 

bool handleStatusMessage(String payload) {
  if (payload.indexOf("fans on")> 0) {
    Serial.println("Turn on the fans!");
    publishTinamousStatus("Will you switch the fans on please.");
    return true;
  }

  if (payload.indexOf("fans off")> 0) {
    Serial.println("Turn off the fans!");
    publishTinamousStatus("fans go off please.");
    return true;
  }


  if (payload.indexOf("help")> 0) {
    Serial.println("Sending help...");
    publishTinamousStatus(
    "Send a message to me (@" DEVICE_USERNAME ") then: \n* 'Fans On' to turn the fans on," 
    " or \n* 'Fans Off' to turn the fans off,"
    " or \n* 'Fans Speed 6' to set the speed to 6 (max 11),"
    " or \n* Lights Off' to turn of the lights,");
    //" or \n* Lights On' to turn of the lights");    
    return true;
  }
  
  return false;
}

