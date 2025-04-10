import time
import sys
from mcp_server import ioehub_mqtt_get_temperature, connect_mqtt, mqtt_thread

def test_mqtt_temperature():
    print("Starting MQTT temperature test")
    
    # Make sure MQTT connection is established
    if not mqtt_thread.is_alive():
        print("Starting MQTT connection...")
        mqtt_thread.start()
    
    # Wait for connection to be established
    time.sleep(3)
    
    # Test the function multiple times
    for i in range(3):
        try:
            print(f"Test {i+1}: Requesting temperature...")
            temperature = ioehub_mqtt_get_temperature()
            print(f"Test {i+1}: Current temperature is {temperature}°C")
        except Exception as e:
            print(f"Test {i+1}: Error getting temperature: {e}")
        
        # Wait between tests
        time.sleep(2)
    
    print("MQTT temperature test completed")

if __name__ == "__main__":
    test_mqtt_temperature() 