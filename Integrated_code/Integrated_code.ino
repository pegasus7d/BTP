// Use TinyGSM and PubSubClient libraries to connect to MQTT via 4G LTE (VVM501 module)
#define TINY_GSM_MODEM_SIM7600  // SIM7600 AT instruction is compatible with A7670
#define SerialAT Serial1
#define SerialMon Serial
#define TINY_GSM_USE_GPRS true
#include <TinyGsmClient.h>
#include <PubSubClient.h>

#define RXD2 16    // VVM501 MODULE RXD INTERNALLY CONNECTED
#define TXD2 17    // VVM501 MODULE TXD INTERNALLY CONNECTED
#define powerPin 4 // VVM501 MODULE ESP32 PIN D4 CONNECTED TO POWER PIN OF A7670C CHIPSET, INTERNALLY CONNECTED

int LED_BUILTIN = 2;
int ledStatus = LOW;

const char *broker         = "broker.hivemq.com"; // Using public HiveMQ broker
const char *publishTopic   = "sensor/reading";    // Topic to publish messages
const char *subscribeTopic = "device/time_update"; // Topic to subscribe to for receiving time

const char apn[] = ""; // Keep APN blank for automatic detection for 4G SIM

#ifdef DUMP_AT_COMMANDS
#include <StreamDebugger.h>
StreamDebugger debugger(SerialAT, SerialMon);
TinyGsm        modem(debugger);
#else
TinyGsm        modem(SerialAT);
#endif

TinyGsmClient client(modem);
PubSubClient mqtt(client);

unsigned long interval = 2000;  // Default to 2 seconds
unsigned long previousMillis = 0;  // Store the last time a message was published

void setup() {
  Serial.begin(115200);
  
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(powerPin, OUTPUT);
  digitalWrite(powerPin, LOW);
  delay(100);
  digitalWrite(powerPin, HIGH);
  delay(1000);
  digitalWrite(powerPin, LOW);

  Serial.println("\nConfiguring VVM501 Module. Please wait...");
  delay(10000);  // Wait for the module to stabilize

  SerialAT.begin(115200, SERIAL_8N1, RXD2, TXD2);

  // Initialize modem
  if (!modem.init() || !modem.restart()) {
    Serial.println("Failed to initialize/restart modem.");
    return;
  }

  Serial.println("Waiting for network...");
  if (!modem.waitForNetwork()) {
    Serial.println("Network connection failed.");
    delay(10000);
    return;
  }
  Serial.println("Network connected.");

  if (!modem.gprsConnect(apn)) {
    Serial.println("Failed to connect to GPRS.");
    return;
  }
  Serial.println("LTE module connected to network.");

  // MQTT setup
  mqtt.setServer(broker, 1883);
  mqtt.setCallback(callback);

  // Connect to MQTT broker
  if (connectToMQTT()) {
    Serial.println("Connected to MQTT broker.");
    mqtt.subscribe(subscribeTopic);
  }
}

void loop() {
  if (!mqtt.connected()) {
    if (connectToMQTT()) {
      mqtt.subscribe(subscribeTopic);  // Re-subscribe to the topic
    }
  }

  mqtt.loop();  // Handle incoming messages

  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    static int counter = 0;
    counter++;

    // Create the message
    String message = "Hello from VVM501! Counter: " + String(counter);
    Serial.println(message);

    // Publish the message
    mqtt.publish(publishTopic, message.c_str());
  }
}

bool connectToMQTT() {
  Serial.print("Connecting to MQTT broker...");
  if (mqtt.connect("VVM501Client")) {
    Serial.println("Connected!");
    return true;
  } else {
    Serial.print("Failed. Error code: ");
    Serial.println(mqtt.state());
    return false;
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  if (strcmp(topic, subscribeTopic) == 0) {
    Serial.print("Received message: ");
    String receivedMessage;
    for (unsigned int i = 0; i < length; i++) {
      receivedMessage += (char)payload[i];
    }
    Serial.println(receivedMessage);

    // Update the interval duration if it's a valid number
    long timeValue = receivedMessage.toInt();
    if (timeValue > 0) {
      interval = timeValue * 1000;  // Convert seconds to milliseconds
      Serial.print("Updated interval to: ");
      Serial.println(interval);
    }
  }
}
