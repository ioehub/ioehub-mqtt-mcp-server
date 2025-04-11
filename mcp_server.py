from mcp.server.fastmcp import FastMCP
from datetime import datetime

import paho.mqtt.client as mqtt
import json
import time
import threading

# MQTT Configuration
MQTT_BROKER = "172.30.1.100"
MQTT_PORT = 1883
MQTT_USERNAME = "ioehub"
MQTT_PASSWORD = "password"
MQTT_PUBLISH_TOPIC = "ioehub/mcp/command"
MQTT_SUBSCRIBE_TOPIC = "ioehub/mcp/response"

# Create the FastMCP instance with stdio transport
mcp = FastMCP()

# Global variable to store the latest temperature reading
latest_temperature = 0.0
mqtt_response_received = False
mqtt_response_data = None

# MQTT client callbacks

def on_connect(client, userdata, flags, rc):
    print(f"Connected to MQTT broker with result code {rc}")
    client.subscribe(MQTT_SUBSCRIBE_TOPIC)

def on_message(client, userdata, msg):
    global latest_temperature, mqtt_response_received, mqtt_response_data
    print(f"Received message on topic {msg.topic}: {msg.payload.decode()}")
    try:
        data = json.loads(msg.payload.decode())
        if 'function' in data and data['function'] == 'ioehub_mqtt_get_temperature' and 'result' in data:
            latest_temperature = float(data['result'])
        mqtt_response_received = True
        mqtt_response_data = data
    except json.JSONDecodeError:
        print("Failed to decode JSON message")
    except Exception as e:
        print(f"Error processing message: {e}")

# Setup MQTT client
mqtt_client = mqtt.Client()
mqtt_client.username_pw_set(MQTT_USERNAME, MQTT_PASSWORD)
mqtt_client.on_connect = on_connect
mqtt_client.on_message = on_message

# Connect to MQTT broker in a separate thread to avoid blocking
def connect_mqtt():
    try:
        mqtt_client.connect(MQTT_BROKER, MQTT_PORT, 60)
        mqtt_client.loop_start()
    except Exception as e:
        print(f"Failed to connect to MQTT broker: {e}")

# Start MQTT connection in background
mqtt_thread = threading.Thread(target=connect_mqtt)
mqtt_thread.daemon = True
mqtt_thread.start()


# Define the tool using the @mcp.tool() decorator
@mcp.tool()
def ioehub_mqtt_get_temperature() -> str:
    """
    Returns current temperature from MQTT sensor
    
    :return: Current temperature in Celsius
    """
    global mqtt_response_received, mqtt_response_data
    
    # Reset response flags
    mqtt_response_received = False
    mqtt_response_data = None
    
    # Send temperature request command via MQTT using the specified JSON format
    request = {
        "function": "ioehub_mqtt_get_temperature",
        "params": {
            "pin": 13
        }
    }
    
    mqtt_client.publish(MQTT_PUBLISH_TOPIC, json.dumps(request))
    
    # Wait for response (with timeout)
    timeout = time.time() + 5.0  # 5 seconds timeout
    while not mqtt_response_received and time.time() < timeout:
        time.sleep(0.1)
    
    # Check if we received a valid response
    if mqtt_response_received and mqtt_response_data and 'result' in mqtt_response_data:
        return str(mqtt_response_data['result'])
    
    
    # Return the latest temperature reading as fallback
    return str(latest_temperature)

@mcp.tool()
def ioehub_mqtt_set_led(pin: int = 0, state: int = 0) -> bool:
    """
    Sets the LED state via MQTT
    
    :param pin: The LED pin number (default: 0)
    :param state: The state to set (1 for ON, 0 for OFF)
    :return: True if the operation was successful
    """
    global mqtt_response_received, mqtt_response_data
    
    # Reset response flags
    mqtt_response_received = False
    mqtt_response_data = None
    
    # Send LED control command via MQTT using the specified JSON format
    request = {
        "function": "ioehub_mqtt_set_led",
        "params": {
            "pin": pin,
            "state": state
        }
    }
    
    mqtt_client.publish(MQTT_PUBLISH_TOPIC, json.dumps(request))
    
    # Wait for response (with timeout)
    timeout = time.time() + 5.0  # 5 seconds timeout
    while not mqtt_response_received and time.time() < timeout:
        time.sleep(0.1)
    
    # Check if we received a valid response
    if mqtt_response_received and mqtt_response_data and 'result' in mqtt_response_data:
        # Assume result is a boolean success flag or can be converted to boolean
        return bool(mqtt_response_data['result'])
    
    # Return False as fallback if no valid response
    return False

# Run the server if the script is executed directly
if __name__ == "__main__":
    print("Starting MCP server...")
    mcp.run(transport="stdio") 