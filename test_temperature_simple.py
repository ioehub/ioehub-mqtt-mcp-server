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

# Global variable to store the latest temperature reading
latest_temperature = 25.5
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

# Connect to MQTT broker
def connect_mqtt():
    try:
        mqtt_client.connect(MQTT_BROKER, MQTT_PORT, 60)
        mqtt_client.loop_start()
        return True
    except Exception as e:
        print(f"Failed to connect to MQTT broker: {e}")
        return False

def get_mqtt_temperature():
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
        return float(mqtt_response_data['result'])
    
    # Return the latest temperature reading as fallback
    return latest_temperature

def test_temperature():
    print("Starting MQTT temperature test")
    
    # Connect to MQTT broker
    if connect_mqtt():
        # Allow time for connection to establish
        time.sleep(3)
        
        # Test the function multiple times
        for i in range(3):
            try:
                print(f"Test {i+1}: Requesting temperature...")
                temperature = get_mqtt_temperature()
                print(f"Test {i+1}: Current temperature is {temperature}°C")
            except Exception as e:
                print(f"Test {i+1}: Error getting temperature: {e}")
            
            # Wait between tests
            time.sleep(2)
        
        # Cleanup
        mqtt_client.loop_stop()
        mqtt_client.disconnect()
    else:
        print("Failed to establish MQTT connection. Test aborted.")
    
    print("MQTT temperature test completed")

if __name__ == "__main__":
    test_temperature() 