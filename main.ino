#include <WiFi.h>
#include <PubSubClient.h>
#include <cstdlib>
#include <ClosedCube_MAX30205.h>
#include <Wire.h>
#include <NewPing.h>
const char* ssid = "ONEPLUS_co_apzhrb";
const char* password = "dwdyssnagua";
#define channelID 2508761
const char mqttUserName[] = "PAk0EggFFSEfOCkGEgYUKDM";
const char clientID[] = "PAk0EggFFSEfOCkGEgYUKDM";
const char mqttPass[] = "8kUN65gHNv7ve9eo8efEzhaI";
// #define USESECUREMQTT
#define BUZZER_PIN 13 // Define the pin for the buzzer
#define WATER_LEVEL_PIN 34 // Define the pin for the water level sensor signal
#define WATER_LEVEL_THRESHOLD 500 // Adjust this threshold based on your water level sensor and setup

#define TRIGGER_PIN 25 // Define the trigger pin for the ultrasonic sensor
#define ECHO_PIN 26 // Define the echo pin for the ultrasonic sensor
#define MAX_DISTANCE 200 // Define the maximum distance to measure in centimeters
WiFiClient client;
ClosedCube_MAX30205 tempSensor; // Corrected the class name
NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE); // Create an instance of the NewPing library for the ultrasonic sensor
const char* server = "mqtt3.thingspeak.com";
int status = WL_IDLE_STATUS;
long lastPublishMillis = 0;
int connectionDelay = 1;
int updateInterval = 30;
PubSubClient mqttClient(client);
void mqttSubscriptionCallback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}
void mqttSubscribe(long subChannelID) {
  String myTopic = "channels/" + String(subChannelID) + "/subscribe";
  mqttClient.subscribe(myTopic.c_str());
}
void mqttPublish(String message) {
  String topicString = "channels/" + String(channelID) + "/publish";
  Serial.println(topicString);
  mqttClient.publish(topicString.c_str(), message.c_str());
}
void connectWifi() {
  Serial.print("Connecting to Wi-Fi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("Connected to Wi-Fi.");
}
void mqttConnect() {
  while (!mqttClient.connected()) {
    if (mqttClient.connect(clientID, mqttUserName, mqttPass)) {
      Serial.print("MQTT to ");
      Serial.print(server);
      Serial.print(" at port ");
      Serial.print(1883);
      Serial.println(" successful.");
    } 
    else {
      Serial.print("MQTT connection failed, rc = ");
      Serial.print(mqttClient.state());
      Serial.println(" Will try again in a few seconds");
      delay(2000);
    }
  }
}
void setup() {
  Wire.begin();
  Serial.begin(115200);
  delay(3000);
  connectWifi();
  mqttClient.setServer(server, 1883);
  mqttClient.setCallback(mqttSubscriptionCallback);
  mqttClient.setBufferSize(2048);
  tempSensor.begin(0x48);   // set continuous mode, active mode
  pinMode(BUZZER_PIN, OUTPUT); // Set the buzzer pin as an output
  digitalWrite(BUZZER_PIN, HIGH); // Turn off the buzzer initially
}
void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    connectWifi();
  }
  if (!mqttClient.connected()) {
    mqttConnect();
    mqttSubscribe(channelID);
  }
  mqttClient.loop();
  
  float temp = tempSensor.readTemperature(); // Read temperature every loop iteration
  int waterLevel = analogRead(WATER_LEVEL_PIN); // Read water level
  unsigned int distance = sonar.ping_cm(); // Measure distance using the ultrasonic sensor

  // Print temperature, water level, and distance to serial monitor
  Serial.print("Temperature: ");
  Serial.print(temp, 2);
  Serial.println("°C");
  Serial.print("Water Level: ");
  Serial.println(waterLevel);
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");


  // If temperature is above 35°C, turn on the buzzer for 1 second
  if (temp > 35.0) {
    digitalWrite(BUZZER_PIN, LOW); // Turn on the buzzer
    delay(1000); // Keep the buzzer on for 1 second
    digitalWrite(BUZZER_PIN, HIGH); // Turn off the buzzer
  }

  // If water level falls below the threshold, buzz with a delay of 2 seconds
  if (waterLevel < WATER_LEVEL_THRESHOLD || distance < 50) {
    digitalWrite(BUZZER_PIN, LOW); // Turn on the buzzer
    delay(2000); // Keep the buzzer on for 2 seconds
    digitalWrite(BUZZER_PIN, HIGH); // Turn off the buzzer
  }
  if (abs(long(millis()) - lastPublishMillis) > updateInterval * 1000) {
    // Publish the random number to the channel
    mqttPublish((String("field1=") + String(temp))(String("field2=") + String(waterLevel)));
    mqttPublish((String("field2=") + String(waterLevel)));
    // Update the last publish time
    lastPublishMillis = millis();
  }
  delay(1000); // Delay for 1 second
}
