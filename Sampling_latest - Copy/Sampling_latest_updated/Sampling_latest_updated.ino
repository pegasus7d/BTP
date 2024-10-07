#define TINY_GSM_MODEM_SIM7600  
#define SerialAT Serial1
#define SerialMon Serial
#define TINY_GSM_USE_GPRS true

#include <TinyGsmClient.h>
#include <PubSubClient.h>
#include <Adafruit_ADS1X15.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <Ticker.h>

#define RXD2 16    
#define TXD2 17   
#define powerPin 4

int LED_BUILTIN = 2;
int ledStatus = LOW;

const char *broker = "test.mosquitto.org"; 
const char *publishTopic = "sensor/reading";    
const char *subscribeTopic = "device/time_update"; 

const char apn[] = "";

TinyGsm modem(SerialAT);
TinyGsmClient client(modem);
PubSubClient mqtt(client);
Adafruit_ADS1115 ads;
AsyncWebServer server(80);

bool adsConnected = false;
int16_t ch0 = 0, ch1 = 0, ch2 = 0;

unsigned long interval = 2000;
unsigned long previousMillis = 0;

// Setup web server route
void setupWebServer() {
  WiFi.softAP("ESP32_AP");

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    String html = "<html><body><h1>ESP32 Sensor Status</h1>";
    html += "<p>Network: " + String(modem.isNetworkConnected() ? "Connected" : "Disconnected") + "</p>";
    html += "<p>MQTT: " + String(mqtt.connected() ? "Connected" : "Disconnected") + "</p>";
    html += "<p>ADS1115: " + String(adsConnected ? "Connected" : "Disconnected") + "</p>";
    html += "<p>Ch0: " + String(ch0) + "</p>";
    html += "<p>Ch1: " + String(ch1) + "</p>";
    html += "<p>Ch2: " + String(ch2) + "</p>";
    html += "</body></html>";
    request->send(200, "text/html", html);
  });

  server.begin();
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(powerPin, OUTPUT);
  
  digitalWrite(powerPin, LOW);
  delay(100);
  digitalWrite(powerPin, HIGH);
  delay(1000);
  digitalWrite(powerPin, LOW);

  SerialAT.begin(115200, SERIAL_8N1, RXD2, TXD2);

  if (!modem.init() || !modem.restart()) {
    Serial.println("Modem init/restart failed");
    return;
  }

  if (!modem.waitForNetwork()) {
    Serial.println("Network connection failed.");
    return;
  }

  if (!modem.gprsConnect(apn)) {
    Serial.println("Failed to connect to GPRS.");
    return;
  }

  adsConnected = ads.begin();

  mqtt.setServer(broker, 1883);
  mqtt.setCallback(callback);

  if (connectToMQTT()) {
    mqtt.subscribe(subscribeTopic);
  }

  setupWebServer();
}

void loop() {
  if (!mqtt.connected()) {
    if (connectToMQTT()) {
      mqtt.subscribe(subscribeTopic);
    }
  }

  mqtt.loop();

  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    if (adsConnected) {
      ch0 = ads.readADC_SingleEnded(0);
      ch1 = ads.readADC_SingleEnded(1);
      ch2 = ads.readADC_SingleEnded(2);
      String message = String(ch0) + String(",") + String(ch1) + String(",") + String(ch2);
      mqtt.publish(publishTopic, message.c_str());
    }
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
    String receivedMessage;
    for (unsigned int i = 0; i < length; i++) {
      receivedMessage += (char)payload[i];
    }
    long timeValue = receivedMessage.toInt();
    if (timeValue > 0) {
      interval = timeValue * 1000;
      Serial.print("Updated interval to: ");
      Serial.println(interval);
    }
  }
}
