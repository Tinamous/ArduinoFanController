#include <MQTTClient.h>
#include <system.h>
#include "secrets.h"

// Be sure to use WiFiSSLClient for an SSL connection.
// for a non ssl (port 1883) use regular WiFiClient.
//WiFiClient networkClient; 
WiFiSSLClient networkClient; 

// MQTT Settings defined in secrets.h
// Set buffer size as the default is WAY to small!.
MQTTClient mqttClient(4096); 

// If we have been connected since powered up 
bool was_connected = false;
String senml = "";
unsigned long nextSendMeasurementsAt = 0;

// converted to lower case in setup.
String lowerDeviceAtName = "@"DEVICE_USERNAME;

// ===============================================
// Setup the connection to the MQTT server.
// ===============================================
bool setupMqtt() {
  senml.reserve(4096);
  lowerDeviceAtName.toLowerCase();
  
  Serial.print("Connecting to Tinamous MQTT Server on port:");
  Serial.println(MQTT_SERVERPORT);
  mqttClient.begin(MQTT_SERVER, MQTT_SERVERPORT, networkClient);

  // Handle received messages.
  mqttClient.onMessage(messageReceived);

  connectToMqttServer();
}

// ===============================================
// Connect/reconnect to the MQTT servier.
// This may be called repeatedly and does nothing
// if already connected.
// ===============================================
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
    Serial.println("Failed to connect to MQTT Server."); 
    Serial.print("Error: "); 
    Serial.print(mqttClient.lastError()); 
    Serial.print(", Return Code: "); 
    Serial.print(mqttClient.returnCode()); 
    Serial.println(); 

    if (mqttClient.lastError() == LWMQTT_CONNECTION_DENIED) {
      Serial.println("Access denied. Check your username and password. Username should be 'DeviceName.AccountName' e.g. MySensor.MyHome"); 
    }

    if (mqttClient.lastError() == -6) {
      Serial.println("Check your Arduino has the SSL Certificate loaded for Tinmaous.com"); 
      // Load the Firmware Updater sketch onto the Arduino.
      // Use the Tools -> WiFi Firmware Updater utility
    }
    
    delay(10000); 
    return false;
  } 
 
  Serial.println("Connected!"); 

  // Subscribe to status messages sent to this device.
  mqttClient.subscribe("/Tinamous/V1/Status.To/" DEVICE_USERNAME); 

  // Subscribe to any additional data feeds for our displays.
  for (int i=0; i<4; i++) {
    if (mqttFeedsTopic[i] != "") {
      Serial.println("Subscribing to " + mqttFeedsTopic[i]);
      mqttClient.subscribe(mqttFeedsTopic[i]);
    }
  }

  // Subsribe to all of the command's for this device.
  mqttClient.subscribe("/Tinamous/V1/Commands/" DEVICE_USERNAME "/#");
  Serial.println("Subscribed."); 

  // Say Hi.
  if (was_connected) {
    // If we were previously conneced, give a reconnect message.
    Serial.println("****** Reconnect! *****************");
    publishTinamousStatus("Ouch. Appears we got disconnected, well I'm back now.");
  } else {
    // for the first connect, give a "Hello" message.
    publishTinamousStatus("Hello! I'm your biggest (and brightest) fan ;-) #SeeWhatIDidThere. Use '@" DEVICE_USERNAME " Help' for help.");
  }

  was_connected = true;
  return true;
} 

// ===============================================
// Loop processor for MQTT functions
// ===============================================
void mqttLoop() {
  // Call anyway, does nothing if already connected.
  connectToMqttServer();

  // Check inbound and keep alive.
  mqttClient.loop(); 

  // Send measurements (if the time interval is appropriate).
  sendMeasurements();
}

// ===============================================
// Send measurements (Temperature/RH/etc) to the MQTT server
// ===============================================
void sendMeasurements() {
  if (!mqttClient.connected()) {
    Serial.println("Not connected, not sending sensor measurements");
    nextSendMeasurementsAt = millis() + (60 * 1000);
    return;
  }

  beginSenML();
  
  if (nextSendMeasurementsAt < millis()) {
    // Send the measurements
    Serial.println("Sending sensor measurements to Tinamous");

    if (hasBme280 || hasBme680) {
      // Temperature
      appendFloatSenML("Temperature", temperature);
    
      // Humidity
      appendFloatSenML("humidity", humidity);
    
      // pressure
      appendFloatSenML("pressure", pressure);
    }

    if (hasCCS811) {
      // TVOC
      appendFloatSenML("TVOC", tVOC);
      // eCO2
      appendFloatSenML("eCO2", eCO2);

      // it's actually uint8_t
      appendUInt8SenML("ccsLastError", ccsLastStatusError);
    }

    if (hasBme680) {
      // Gas resistance (BME680)
      appendFloatSenML("GasR", gas_resistance);
    }
    
    // Light
    if (hasLightSensor) {
      appendUInt16SenML("light", lightLevelLux);
      appendUInt16SenML("red", redLightLevel);
      appendUInt16SenML("green", greenLightLevel);
      appendUInt16SenML("blue", blueLightLevel);
      appendUInt16SenML("clear", clearLightLevel);
      appendUInt16SenML("colorTemp", colorTemperature);
    }

    // Set color
    // TODO: Use the set HSV value.
    appendStringSenML("color", "12,0.1,0.2");
    //ledsSetColor
    
    // RSSI
    appendFloatSenML("rssi", rssi);

    // Fan Supply Voltage
    appendFloatSenML("fanv", voltage);
    
    // Speed selected
    appendIntSenML("setSpeed", fanInfos[0].speedSet);
    
    // Master power state
    int power = isMasterPowerEnabled() ? 1 : 0;
    appendIntSenML("masterPower", power);

    String fieldname = "";
    
    // Fan speed (RPM)
    // and Display mode for the fans
    for (int fanId=0; fanId<4; fanId++) {
      fieldname = "FanRpm-";
      fieldname = fieldname + fanId;
      appendIntSenML(fieldname, fanDisplayModes[fanId]);
      fieldname = "Display-";
      fieldname = fieldname + fanId;
      appendIntSenML(fieldname, fanDisplayModes[fanId]);
    }
    
    // ledBrightnessPercent (Last)
    appendLastIntSenML("ledBrightness", ledBrightnessPercent);

    sendSenML();

    // Schedule the next publish of sensor measurements
    // to be in n seconds time...
    nextSendMeasurementsAt = millis() + (30 * 1000);
  }
}

void beginSenML() {
  senml = "{'e':[";
}

void appendIntSenML(String name, int value) {
  senml = senml + constructSenMLFieldStart(name);
  senml = senml + value;
  senml = senml + "},";
}

// The last measurement...
void appendLastIntSenML(String name, int value) {
    senml = senml + constructSenMLFieldStart(name);
    senml = senml + value;
    senml = senml + "}";
}

void appendFloatSenML(String name, float value) {
    senml = senml + constructSenMLFieldStart(name);
    senml = senml + value;
    senml = senml + "},";
}

void appendUInt8SenML(String name, uint8_t value) {
    senml = senml + constructSenMLFieldStart(name);
    senml = senml + value;
    senml = senml + "},";
}

void appendUInt16SenML(String name, uint16_t value) {
    senml = senml + constructSenMLFieldStart(name);
    senml = senml + value;
    senml = senml + "},";
}

void appendStringSenML(String name, String value) {
  senml = senml + "{'n':'" + name + "', 'sv':'";
  senml = senml + value;
  senml = senml + "'},";
}

String constructSenMLFieldStart(String name) {
  return "{'n':'" + name + "', 'v':";
}

void sendSenML() {
  // Terminate the json and send the senml to Tinamous
  senml = senml +  "]}";
  Serial.println("Senml:");
  Serial.println(senml);

  if (senml.length() > 2048) {
    Serial.println("*** SENML too long. It will overflow the buffer ***");
  }
  publishTinamousSenMLMeasurements(senml);
}

// ===============================================
// Pubish a status message to the Tinamous MQTT
// topic.
// ===============================================
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
    Serial.print("Status message sent.");
  }
}

void publishTinamousSenMLMeasurements(String senml) {
  if (senml.length()> 4096) {
    Serial.println("senml longer than buffer. Ignoring!!!");
    return;
  }
  
  mqttClient.publish("/Tinamous/V1/Measurements/SenML", senml); 
}



// =========================================================================================

// ===============================================
// Process messages received from the MQTT server
// ===============================================
void messageReceived(String &topic, String &payload) { 
  Serial.println("Message from Tinamous on topic: " + topic + " - " + payload); 

  // Show a little light display to show Alexa interaction.
  showAlexaConnectionActive();
  
  // If the payload starts with "/Tinamous/V1/Commands/" DEVICE_USERNAME 
  // then we should handle that...
  if (topic.startsWith("/Tinamous/V1/Commands/" DEVICE_USERNAME)) {
    handleCommand(topic, payload);
    return;
  }

  payload.toLowerCase();

  // If it starts with an @ it's a status post to this deivce.
  if (payload.startsWith(lowerDeviceAtName)) {
    
    // Clean up the status message.
    // replace it with a space so our index of
    // isn't 0 for the start of the line.    
    payload.replace(lowerDeviceAtName, " ");

    if (handleStatusMessage(payload)) {
      return;
    }
  } 

  // Otherwise check to see if the data has come from a custom MQTT subscription
  if (checkCustomMqttFeeds(topic, payload)) {
    return;
  }

  // And finally, reply with unknown message.
  Serial.print("Unknown message: '");
  Serial.print(payload);
  Serial.println("'");
  publishTinamousStatus("Unknown message. use help.");
} 

// =================================================
// Custom MQTT topic handling
// =================================================

bool checkCustomMqttFeeds(String &topic, String &payload) {
  for (int i=0; i<4; i++) {
    if (mqttFeedsTopic[i] == topic) {
      handleCustomMqttFeed(i, topic, payload);
      return true;
    }
  }
  return false;
}

// Expect this to be a simple value containing payload
void handleCustomMqttFeed(int index, String &topic, String &payload) {
int value = payload.toInt();
  Serial.println("Setting custom mqtt feed value to " + value);
  mqttFeedsValue[index] = payload.toInt();
}

// =================================================
// Status message handling
// This is where the device had an "@DeviceName Do Stuff
// message posted to it.
// =================================================
bool handleStatusMessage(String payload) {
int payloadValue;

  // This does FANS only, not leds.
  if (payload.indexOf("fans on")> 0 || payload.indexOf("turn on the fans")> 0) {
    Serial.println("Turn on the fans!");
    setFansSpeed(50);
    setPower(1);
    publishTinamousStatus("Fans on.");
    return true;
  }

  // This does FANS only, not leds.
  if (payload.indexOf("fans off")> 0 || payload.indexOf("turn off the fans")> 0) {
    Serial.println("Turn off the fans!");
    setPower(0);
    setFansSpeed(0);
    publishTinamousStatus("fans off.");
    return true;
  }

  // Alexa, Turn off the fans
  // "turn off" from alexa mapped to turning off the fans and the lights
  if (payload.indexOf("sleep")> 0 || payload.indexOf("turn off")> 0) {
    sleepNow();
    return true;
  }

  // Alexa, Turn on the fans
  // "turn on" from alexa mapped to waking the fans and the lights.
  if (payload.indexOf("wake")> 0 || payload.indexOf("turn on") > 0) {
    wakeNow();
    return true;
  }   

  if (payload.indexOf("leds off")> 0) {
    ledBrightnessPercent = 0;
    return true;
  } 

  if (payload.indexOf("leds on")> 0) {
    ledBrightnessPercent = 50;
    return true;
  } 

  // Alexa command to dim the LEDs.
  if (payload.indexOf("dim leds by")> 0) {
    // TODO: Get the value...
    Serial.println("Dim the leds by x!");
    payload.replace("dim leds by", "");
    payload.trim();
    Serial.println(payload);
    payloadValue = payload.toInt();
    ledBrightnessPercent = ledBrightnessPercent - payloadValue;
    if (ledBrightnessPercent < 0) {
      ledBrightnessPercent = 0;
    }
  }

  // Alexa, set brightness of ___ to 20%
  if (payload.indexOf("set brightness")> 0) {
    // TODO: Get the value...
    Serial.println("Set leds brightness!");
    payload.replace("set brightness", "");
    payload.trim();
    Serial.println(payload);
    payloadValue = payload.toInt();
    ledBrightnessPercent = payloadValue;
    if (ledBrightnessPercent < 0) {
      ledBrightnessPercent = 0;
    }
    if (ledBrightnessPercent > 100) {
      ledBrightnessPercent = 100;
    }

    return true;
  }

  if (payload.indexOf("adjust brightness")> 0) {
    // TODO: Get the value...
    Serial.println("Set leds brightness!");
    payload.replace("adjust brightness", "");
    payload.trim();
    Serial.println(payload);
    payloadValue = payload.toInt();
    ledBrightnessPercent += payloadValue;
    if (ledBrightnessPercent < 0) {
      ledBrightnessPercent = 0;
    }
    if (ledBrightnessPercent > 100) {
      ledBrightnessPercent = 100;
    }
    
    return true;
  }

  // Alexa, Set Powerlevel to 20% for the fans.
  // If the message is "fans speed ##"
  if (payload.indexOf("fan speed")> 0 || payload.indexOf("set powerlevel") > 0) {
    payload.replace("fan speed", "");
    payload.replace("set powerlevel", "");
    payload.trim();
    Serial.print("Fan Speed: '");
    Serial.println(payload);
    // Lets hope it's just an int...
    int speed = payload.toInt();

    if (speed < 0 || speed > 100) {
      return false;
    }
    
    Serial.print("Setting fans speed to ");
    Serial.println(speed, DEC);
    publishTinamousStatus("Fans speed set.");
    if (speed < 2) {
      // PWM fans don't stop at 0 they
      // just run at min speed
      // kill the power for anything 
      // less than 2%.
      setPower(0);
      setFansSpeed(speed);
    } else {
      setPower(1);
      setFansSpeed(speed);
    }
    return true;
  }

  // Alexa, set the fan to red
  // Alexa, change the fan to the color blue
  if (payload.indexOf("set color hsv") > 0) {
    payload.replace("set color hsv", "");
    payload.trim();

    // Now we need to split the hsv values "34.2,0.1, 0.2" =>
    Serial.print("Set color: ");
    Serial.println(payload);

    int commaIndex = payload.indexOf(',');
    int secondCommaIndex = payload.indexOf(',', commaIndex + 1);

    String stringValue;
    float hue;
    float saturation;
    float brightness;
    
    stringValue = payload.substring(0, commaIndex);
    hue = stringValue.toFloat();
    
    stringValue = payload.substring(commaIndex + 1, secondCommaIndex);
    saturation = stringValue.toFloat();
    
    stringValue = payload.substring(secondCommaIndex + 1);
    brightness = stringValue.toFloat();

    Serial.print("HSV: ");
    Serial.print(hue);
    Serial.print(", ");
    Serial.print(saturation);
    Serial.print(", ");
    Serial.print(brightness);
    Serial.println();

    ledsSetColor = CHSV(hue, saturation * 255, 40);
      
    return true;
  }
  
  if (payload.indexOf("help")> 0) {
    Serial.println("Sending help...");
    publishTinamousStatus(
    "Send a message to me (@" DEVICE_USERNAME ") then: \n"
    "'Turn On' to turn the fans and leds on \n" 
    "'wake'  to turn on the lights and fans \n"
    "\n"
    "'Turn Off' to turn the fans and leds off \n"
    "'sleep'  to turn off the lights and fans \n"
    "\n"
    "'fan speed 60' to set the speed to 60% \n"
    "'fans on' to switch on the fans (not leds) \n"
    "'fans off' to switch off the fans (not leds) \n"
    "\n"
    "'leds on'  to turn on the lights \n"
    "'leds off'  to turn off the lights \n"
    "\n"
    "'bright'  to turn the leds to bright \n"
    "'dim'  to dim the lights \n"
    "'set leds to 40' to set the LEDs to 40% brightness \n"
    "'dim leds by 20' to dim the LEDs to 20% \n"
    );
    return true;
  }
  
  return false;
}

// ==============================================
// Command handlers.
// ===============================================

// Handle a command that's come in via the MQTT subscription
void handleCommand(String &topic, String &payload) {
  Serial.println("Handle command: " + topic);
   
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

  Serial.print("Unknown command. Topic:");    
  Serial.println(topic);    
  publishTinamousStatus("Sorry I don't know that command.");
}

// ===============================================
// /Fans/Power + value in payload (1 = on, 0 = off).
// ===============================================
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
    Serial.print(value); // 0..100
    Serial.print(", payload: "); // 0..100
    Serial.print(payload); // 0..100
    Serial.println();
    setFansSpeed(value);
  } else {
    Serial.println("Unknown FAN command!");    
    publishTinamousStatus("Hello! Sorry I don't know that command. Please check your MQTT topic. Command: ");
  }
}

// ===============================================
// Handle Commands:
// ===============================================
// /Leds/Power + power in payload (1 on, 0 off)
// /Leds/Brightness + brigthness in payload (0..255)
// /Leds/Fanx/DisplayMode x = fan (1-4) + display mode in payload.
// /Leds/Strip/DisplayMode + display mode in payload.
void handleLedsCommands(String &topic, String &payload) {
int value;
value = payload.toInt();

  if (topic == "/Leds/Power") {
    // turn on/off the LEDs.
    if (value > 0) {
      ledBrightnessPercent = 50;
    }
  } else if (topic == "/Leds/Brightness") {
    // Set the brightness
    if (value > 0 && value <= 100) {
      ledBrightnessPercent =  value;
    } else {
      Serial.println("Invalid brightness");
       publishTinamousStatus("Invalid brightness. Range is 0..100. Thanks.");
    }
    Serial.print("Setting leds brightness: " );
    Serial.println(ledBrightnessPercent, DEC);
  } else if (topic == "/Leds/Fan1/DisplayMode") {
    // Set display type for LED1
    Serial.print("Handle fan 1 led DisplayMode: ");
    Serial.println(value);
    fanDisplayModes[0] =  (DisplayMode)value;
  } else if (topic == "/Leds/Fan2/DisplayMode") {
    Serial.print("Handle fan 2 led DisplayMode: ");
    Serial.println(value);
    fanDisplayModes[1] =  (DisplayMode)value;
  } else if (topic == "/Leds/Fan3/DisplayMode") {
    Serial.print("Handle fan 3 led DisplayMode: ");
    Serial.println(value);
    fanDisplayModes[2] =  (DisplayMode)value;
  } else if (topic == "/Leds/Fan4/DisplayMode") {
    Serial.print("Handle fan 4 led DisplayMode: ");
    Serial.println(value);
    fanDisplayModes[3] = (DisplayMode)value;
  } else if (topic == "/Leds/Strip/DisplayMode") {
    Serial.println("Handle led strip DisplayMode");
    // TODO: When we know how to handle this...
  } else {
    Serial.println("Unknown LED command!");    
    publishTinamousStatus("Hello! Sorry I don't know that command. Please check your MQTT topic. Command: ");
  } 
}



// ===============================================
// Handle Sleep request command
// ===============================================
// Commands:
// /Sleep/Now - sleeps NOW Fans and LEDs full off.
// /Sleep/At + value (hh:mm:ss) in payload Sleeps daily at that time
void handleSleepCommand(String &topic, String &payload) {
 
  if (topic == "/Sleep/At") {
    // Expect payload to be hh:mm:ss (or hh:mm)
    String sleepAt = payload;
    // Setup sleep mode at this tine
    Serial.print("Sleep at: "); 
    Serial.println(sleepAt); 
    Serial.println("*** Not implemented ***");
  } else if (topic == "/Sleep/Now") {
    sleepNow();
  }
}

// ===============================================
// Handle wake request command
// ===============================================
// Commands:
// /Sleep/Now - sleeps NOW Fans and LEDs full off.
// /Sleep/At + value (hh:mm:ss) in payload wakes daily at that time
void handleWakeCommand(String &topic, String &payload) {
 
  if (topic == "/Wake/At") {
    // Expect payload to be hh:mm:ss (or hh:mm)
    String wakeAt = payload;
    // Setup sleep mode at this tine
    Serial.print("Wake at: "); 
    Serial.println(wakeAt); 
    Serial.println("*** Not implemented ***");
  } else if (topic == "/Wake/Now") {
    wakeNow();
  }
}


