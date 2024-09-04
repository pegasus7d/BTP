#include <WiFi.h>
#include <PubSubClient.h>

// Wi-Fi credentials
const char* ssid = "Different";         // Replace with your SSID
const char* password = "champion";      // Replace with your Wi-Fi password

// MQTT broker IP address
const char* mqttServer = "192.168.149.222"; // Replace with your MQTT broker IP address
const int mqttPort = 1883; // Default MQTT port, adjust if necessary

// Initialize PubSubClient with the Ethernet client
WiFiClient wifiClient;
PubSubClient client(wifiClient);

// MQTT topics
const char* publishTopic = "sensor/reading";    // Topic to publish messages
const char* subscribeTopic = "device/time_update"; // Topic to subscribe to for receiving time

// Variable for the interval duration (in milliseconds)
unsigned long interval = 2000; // Default to 2 seconds

// Variable to store the last time a message was published
unsigned long previousMillis = 0;

void setup() {
  // Start serial communication at 115200 baud
  Serial.begin(115200);

  // Connect to Wi-Fi
  setup_wifi();

  // Set MQTT server and port
  client.setServer(mqttServer, mqttPort);
  
  // Set the callback function for incoming messages
  client.setCallback(callback);

  // Connect to the MQTT broker
  if (connectToMQTT()) {
    Serial.println("Connected to MQTT broker!");
    // Subscribe to the time update topic
    client.subscribe(subscribeTopic);
    Serial.println("Subscribed to time update topic.");
  } else {
    Serial.println("Failed to connect to MQTT broker.");
  }
}

void loop() {
  // Reconnect if the connection is lost
  if (!client.connected()) {
    if (connectToMQTT()) {
      Serial.println("Reconnected to MQTT broker!");
      // Re-subscribe to the time update topic
      client.subscribe(subscribeTopic);
    } else {
      Serial.println("Failed to reconnect to MQTT broker.");
    }
  }
  
  client.loop(); // Handle incoming messages

  unsigned long currentMillis = millis();

  // Check if the interval has passed
  if (currentMillis - previousMillis >= interval) {
    // Save the last publish time
    previousMillis = currentMillis;

    // Publish a message to the MQTT topic
    static int counter = 0;
    counter++;

    // Get Wi-Fi connection status
    String wifiStatus = WiFi.isConnected() ? "Connected" : "Disconnected";
    
    // Create message with counter, Wi-Fi status, and IP address
    String message = "Hello from ESP32! Counter: " + String(counter) + 
                     ", Wi-Fi Status: " + wifiStatus +
                     ", IP: " + WiFi.localIP().toString();

    // Print the message to the Serial Monitor
    Serial.println(message);
    Serial.println(message.length());

    // Publish the message to the MQTT topic
    bool success = client.publish(publishTopic, message.c_str());
    Serial.println(success ? "Message published successfully!" : "Failed to publish message.");
  }
}

// Callback function for incoming messages
void callback(char* topic, byte* payload, unsigned int length) {
  // Check if the received message is from the subscribed time update topic
  if (strcmp(topic, subscribeTopic) == 0) {
    Serial.print("Received time update: ");
    String receivedTime;
    for (unsigned int i = 0; i < length; i++) {
      receivedTime += (char)payload[i];
    }
    Serial.println(receivedTime);

    // Convert the received time to a long integer if needed
    long timeValue = receivedTime.toInt();
    Serial.print("Time as integer: ");
    Serial.println(timeValue*1000);


    // Update the interval duration using the received time value
    interval = timeValue * 1000;  // Convert seconds to milliseconds
    Serial.print("Updated interval: ");
    Serial.println(interval);
  }
}

void setup_wifi() {
  // Connect to the Wi-Fi network
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("Wi-Fi connected!");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

bool connectToMQTT() {
  Serial.print("Connecting to MQTT broker...");
  if (client.connect("ESP32Client")) {
    Serial.println("Connected!");
    return true;
  } else {
    Serial.print("Failed. Error code: ");
    Serial.println(client.state());
    return false;
  }
}
