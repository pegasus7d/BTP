import paho.mqtt.client as mqtt
import time

# MQTT broker details
broker_ip = 'test.mosquitto.org'
port = 1883
subscribe_topic = 'sensor/reading'
publish_topic = 'device/time_update'

# Callback when the client connects to the broker
def on_connect(client, userdata, flags, rc):
    print('Connected to MQTT broker!')
    client.subscribe(subscribe_topic)
    print(f'Subscribed to topic "{subscribe_topic}"')

# Callback when a message is received on a subscribed topic
def on_message(client, userdata, message):
    print(f'Received message on topic "{message.topic}": "{message.payload.decode()}"')

# Create an MQTT client instance
client = mqtt.Client()

# Set the callbacks
client.on_connect = on_connect
client.on_message = on_message

# Connect to the MQTT broker
client.connect(broker_ip, port, 60)

# Start the network loop in a separate thread
client.loop_start()

try:
    while True:
        current_time = str(10)  # Simulate sending time in seconds
        client.publish(publish_topic, current_time)
        print(f'Sent time: {current_time} to topic "{publish_topic}"')
        time.sleep(5)  # Send every 5 seconds
except KeyboardInterrupt:
    print('Disconnected from MQTT broker')
    client.loop_stop()
    client.disconnect()
