import paho.mqtt.client as mqtt
import time

# MQTT broker details
broker_ip = 'broker.hivemq.com'  # Replace with your MQTT broker IP
port = 1883  # Default MQTT port
subscribe_topic = 'sensor/reading'  # Topic to subscribe to
publish_topic = 'device/time_update'  # Topic to publish the time

# Callback when the client connects to the broker
def on_connect(client, userdata, flags, rc):
    print('Connected to MQTT broker!')
    # Subscribe to the topic
    client.subscribe(subscribe_topic)
    print(f'Subscribed to topic "{subscribe_topic}"')

# Callback when a message is received on a subscribed topic
def on_message(client, userdata, message):
    print(f'Received message on topic "{message.topic}": "{message.payload.decode()}"')

# Callback when the client encounters an error
def on_log(client, userdata, level, buf):
    print(f'MQTT Client Log: {buf}')

# Create an MQTT client instance
client = mqtt.Client()

# Set the callbacks
client.on_connect = on_connect
client.on_message = on_message
client.on_log = on_log

# Connect to the MQTT broker
client.connect(broker_ip, port, 60)

# Start the network loop in a separate thread
client.loop_start()

try:
    while True:
        # Get the current time as a string
        current_time = str(5)  # Get current time as a string
        
        # Publish the current time to the new topic
        client.publish(publish_topic, current_time)
        print(f'Sent time: {current_time} to topic "{publish_topic}"')

        # Wait for a specific interval (e.g., 5 seconds) before sending the next time
        time.sleep(500000)

except KeyboardInterrupt:
    print('Disconnected from MQTT broker')
    client.loop_stop()
    client.disconnect()