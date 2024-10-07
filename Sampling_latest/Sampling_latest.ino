#define TINY_GSM_MODEM_SIM7600  
#define SerialAT Serial1
#define SerialMon Serial
#define TINY_GSM_USE_GPRS true
#include <TinyGsmClient.h>
#include <PubSubClient.h>

#include <WiFi.h>
#include <AsyncTCP.h>                // Added for Async WebServer support
#include <ESPAsyncWebServer.h>       // Added the Async WebServer library

#include <Adafruit_ADS1X15.h>

#define RXD2 16    
#define TXD2 17   
#define powerPin 4

bool check=false;

int LED_BUILTIN = 2;
int ledStatus = LOW;

const char *broker         = "test.mosquitto.org"; 
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
Adafruit_ADS1115 ads;

AsyncWebServer server(80); 

bool adsConnected = false;
int16_t ch0 = 0;
int16_t ch1 = 0;
int16_t ch2 = 0;

unsigned long interval = 2000;  // Default to 2 seconds
unsigned long previousMillis = 0;  // Store the last time a message was published
unsigned long serverStartMillis = 0;
const unsigned long serverTimeout = 60000*15;

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
    // delay(10000);
    return;
  }
  Serial.println("Network connected.");

  if (!modem.gprsConnect(apn)) {
    Serial.println("Failed to connect to GPRS.");
    return;
  }
  Serial.println("LTE module connected to network.");


  // Initialize ADS1115
  adsConnected = ads.begin();
  if (!adsConnected) {
    Serial.println("Failed to initialize ADS1115");
  }

  // MQTT setup
  mqtt.setServer(broker, 1883);
  mqtt.setCallback(callback);

  // Connect to MQTT broker
  if (connectToMQTT()) {
    Serial.println("Connected to MQTT broker.");
    mqtt.subscribe(subscribeTopic);
  }
  startWebServer();  // Start the web server
  serverStartMillis = millis();  // Record the time the server started
 

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
    if(adsConnected){
      try{
        ch0 = ads.readADC_SingleEnded(0);
        ch1 = ads.readADC_SingleEnded(1);
        ch2 = ads.readADC_SingleEnded(2);
        Serial.print("ADS1115 Value: ");
        Serial.println(String(ch0) + String(ch1) + String(ch2));
        String message = String(ch0) + String(ch1) + String(ch2);
        mqtt.publish(publishTopic, message.c_str());
        

      }catch(...){
        Serial.println("Error reading ADS1115");
      }
    }

  
  }
  // Stop the web server after 1 minute
  if (currentMillis - serverStartMillis >= serverTimeout and check==false) {
    stopWebServer();  // Shut down the web server
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

void startWebServer() {
  WiFi.softAP("ESP32_AP");
  // Route to handle "/" page
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", "Welcome to ESP32 Async WebServer");
  });

  // Route to return ADS1115 readings
  server.on("/readings", HTTP_GET, [](AsyncWebServerRequest *request){
    // Return sensor readings as a JSON object
    String jsonResponse = "{\"ch0\":" + String(ch0) + ", \"ch1\":" + String(ch1) + ", \"ch2\":" + String(ch2) + "}";
    request->send(200, "application/json", jsonResponse);
  });

  // Start server
  server.begin();  // New: Begin the async web server to listen for requests
}

void stopWebServer() {
  server.end();  // Shut down the async web server
  WiFi.softAPdisconnect(true);
  check=true;
  // Serial.println("Web server stopped after 30 seconds.");
}


String getSignalStrength() {
  int signalQuality = modem.getSignalQuality();
  //Serial.println(signalQuality);
  return String(signalQuality);
}