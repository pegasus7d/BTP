const mqtt = require('mqtt');

// MQTT broker details
const brokerIP = '192.168.149.222'; // Replace with your MQTT broker IP
const port = 1883; // Default MQTT port
const topic = 'sensor/reading'; // Topic to subscribe to

// Connect to the MQTT broker
const client = mqtt.connect(`mqtt://${brokerIP}:${port}`);

// When connected to the broker
client.on('connect', () => {
  console.log('Connected to MQTT broker!');
  // Subscribe to the topic
  client.subscribe(topic, (err) => {
    if (err) {
      console.error('Failed to subscribe to topic:', err);
    } else {
      console.log(`Subscribed to topic '${topic}'`);
    }
  });
});

// When a message is received on the subscribed topic
client.on('message', (receivedTopic, message) => {
  if (receivedTopic === topic) {
    console.log(`Received message on topic '${receivedTopic}': '${message.toString()}'`);
  }
});

// Handle errors
client.on('error', (err) => {
  console.error('MQTT Client Error:', err);
});
