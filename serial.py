import paho.mqtt.client as mqtt
import serial
import time

# Set up MQTT client
client = mqtt.Client()
client.username_pw_set("spotlight", "tracker")  # Optional, depending on your MQTT broker
client.connect("your_broker_ip", 1883, 60)  # Connect to your MQTT broker

# Set up the serial port for the bridge node (receiving data from tag)
ser = serial.Serial("/dev/ttyACM0", 115200, timeout=1)  # Adjust port as needed

# Define the topic to publish the location data
mqtt_topic = "location/tag_data"

def parse_data(line):
    # Parse the line received from serial port to extract location data
    if "POS:" in line:
        try:
            # Extract position data (x, y, z) from the received line
            data = line.split("POS:")[1].split(",")
            x, y, z = map(float, data)
            return x, y, z
        except Exception as e:
            print(f"Error parsing location data: {e}")
    return None

while True:
    line = ser.readline().decode("utf-8").strip()
    if line:
        print(f"Received data: {line}")

        # Parse the data to extract position (x, y, z)
        position = parse_data(line)
        if position:
            x, y, z = position
            print(f"Publishing position: {x}, {y}, {z}")

            # Publish the position data to MQTT broker
            message = f"{{'x': {x}, 'y': {y}, 'z': {z}}}"
            client.publish(mqtt_topic, message)
            
    time.sleep(1)  # Adjust delay as needed
