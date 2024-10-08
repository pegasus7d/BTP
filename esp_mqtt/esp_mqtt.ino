
// //THIS EXAMPLE SHOWS HOW VVM501 ESP32 4G LTE MODULE CAN CONNECT TO MQTT PUBLIC BROKER HIVEMQ USING TINYGSMCLIENT AND PUBSUBCLIENT LIBRARY
// //THE DEVICE CAN PUBLISH AS WELL AS SUBSCRIBE TOPICS VIA 4G MQTT
// //FOR VVM501 PRODUCT DETAILS VISIT www.vv-mobility.com

// #define TINY_GSM_MODEM_SIM7600  // SIM7600 AT instruction is compatible with A7670
// #define SerialAT Serial1
// #define SerialMon Serial
// #define TINY_GSM_USE_GPRS true
// #include <TinyGsmClient.h>
// #define RXD2 17    //VVM501 MODULE RXD INTERNALLY CONNECTED
// #define TXD2 16    //VVM501 MODULE TXD INTERNALLY CONNECTED
// #define powerPin 4 ////VVM501 MODULE ESP32 PIN D4 CONNECTED TO POWER PIN OF A7670C CHIPSET, INTERNALLY CONNECTED

// char a, b;



// const char apn[]      = ""; //APN automatically detects for 4G SIM, NO NEED TO ENTER, KEEP IT BLANK

// #ifdef DUMP_AT_COMMANDS
// #include <StreamDebugger.h>
// StreamDebugger debugger(SerialAT, SerialMon);
// TinyGsm        modem(debugger);
// #else
// TinyGsm        modem(SerialAT);
// #endif
// TinyGsmClient client(modem);


// void setup()
// {
//   Serial.begin(115200);

//   pinMode(powerPin, OUTPUT);
//   digitalWrite(powerPin, LOW);
//   delay(100);
//   digitalWrite(powerPin, HIGH);
//   delay(1000);
//   digitalWrite(powerPin, LOW);




//   Serial.println("\nconfiguring VVM501 Module. Kindly wait");

//   delay(10000);

//   SerialAT.begin(115200, SERIAL_8N1, RXD2, TXD2);


//   // Restart takes quite some time
//   // To skip it, call init() instead of restart()
//   DBG("Initializing modem...");
//   if (!modem.init()) {
//     DBG("Failed to restart modem, delaying 10s and retrying");
//     return;
//   }
//   // Restart takes quite some time
//   // To skip it, call init() instead of restart()
//   DBG("Initializing modem...");
//   if (!modem.restart()) {
//     DBG("Failed to restart modem, delaying 10s and retrying");
//     return;
//   }

//   String name = modem.getModemName();
//   DBG("Modem Name:", name);

//   String modemInfo = modem.getModemInfo();
//   DBG("Modem Info:", modemInfo);


//   Serial.println("Waiting for network...");
//   if (!modem.waitForNetwork()) {
//     Serial.println(" fail");
//     delay(10000);
//     return;
//   }
//   Serial.println(" success");

//   if (modem.isNetworkConnected()) {
//     Serial.println("Network connected");
//   }


//   // GPRS connection parameters are usually set after network registration
//   Serial.print(F("Connecting to "));
//   Serial.print(apn);
//   if (!modem.gprsConnect(apn)) {
//     Serial.println(" fail");
//     delay(10000);
//     return;
//   }
//   Serial.println(" success");

//   if (modem.isGprsConnected()) {
//     Serial.println("LTE module connected");
//   }

//   Serial.println("Enter Standard AT commands like AT, AT+CPIN?, AT+CCLK?, etc.");
//   Serial.println("SELECT SERIAL PORT MONITOR AS \"BOTH NL & CR\" TO VIEW COMMAND RESPONSE CORRECTLY IF YOU ARE USING ARDUINO IDE");
//   Serial.println("Refer A7600 series datasheet for entire list of commands");
//   Serial.println("Understand the AT Commands properly");
//   Serial.println("Incorrect AT commands can corrupt the 4G module memory!!!");


//   // MQTT Broker setup
//   //mqtt.setServer(broker, 1883);
//   //mqtt.setCallback(callback);
// }



// void loop()
// {
//   if (Serial.available() > 0) // read AT commands from user Serial port and send to the Module
//   {
//     a = Serial.read();
//     SerialAT.write(a);
//   }
//   if (SerialAT.available() > 0) //read Response commands from module and send to user Serial Port
//   {
//     b = SerialAT.read();
//     Serial.write(b);
//   }
// }
void setup() {
  Serial.begin(115200); // Initialize Serial Monitor
  Serial2.begin(115200, SERIAL_8N1, 25, 26); // Initialize hardware Serial for ESP32 communication
  Serial.setTimeout(1000); // Set a timeout for Serial input
  Serial.print("Start");
}

void loop() {
  // Check if any data is available on Serial Monitor
  if (Serial.available() > 0) {
    // Read the input from Serial Monitor
    String input = Serial.readStringUntil('\n');
    // Send the input to Serial2
    Serial2.println(input);
    // Wait for ESP32 to respond
    delay(1000); // Adjust delay according to response time
    // Print the response from ESP32 to Serial Monitor
    waitForResponse();
  }
}

void waitForResponse() {
  // Wait for response from ESP32
  unsigned long timeout = millis() + 10000; // Timeout after 10 seconds
  while (!Serial2.available() && millis() < timeout) {
    // Wait for data to be available or timeout
  }
  // Print the response from ESP32 to Serial Monitor
  while (Serial2.available()) {
    Serial.write(Serial2.read());
  }
}

// AT+CMQTTSTART
// AT+CMQTTACCQ=0,"LAB"
// 
// AT+CMQTTCONNECT=0,"tcp://test.mosquitto.org:1883",20,1
// AT+CMQTTTOPIC=0,9     //     mqtt/test1
// AT+CMQTTPAYLOAD=0,6
// AT+CMQTTPUB=0,1,60