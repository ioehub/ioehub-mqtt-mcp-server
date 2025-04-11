// MQTT 토픽 설정
const char* mqtt_subscribe_topic = "ioehub/mcp/command";
const char* mqtt_publish_topic = "ioehub/mcp/response";

//#define DHTPIN 13     // Digital pin connected to the DHT sensor
// Feather HUZZAH ESP8266 note: use pins 3, 4, 5, 12, 13 or 14 --
// Pin 15 can work but DHT must be disconnected during program upload.

// DHT 센서 설정
//#define DHTTYPE DHT22  // DHT22 (AM2302) 센서 사용
#define DHTTYPE DHT11  // DHT22 (AM2302) 센서 사용
DHT* dhtSensors[20];   // 최대 10개의 DHT 센서 지원

// 서보모터 설정
Servo servos[16];      // 최대 16개의 서보모터 지원
bool servoAttached[16] = {false};

// WiFi 및 MQTT 클라이언트 설정
//WiFiClient espClient;
//PubSubClient client(espClient);

// 타임스탬프 가져오기
String getTimestamp() {
  // ESP32는 NTP 서버를 통해 시간을 가져올 수 있지만, 간단한 구현을 위해 현재 밀리초로 대체
  unsigned long currentMillis = millis();
  char timestamp[20];
  sprintf(timestamp, "%lu", currentMillis);
  return String(timestamp);
}

// JSON 응답 생성
template <typename T>
String createJsonResponse(const char* function, const T& result) {
  StaticJsonDocument<256> doc;
  doc["function"] = function;
  doc["result"] = result;
  doc["timestamp"] = getTimestamp();
  
  String response;
  serializeJson(doc, response);
  return response;
}

// LED 제어 함수
bool ioehub_mqtt_set_led(uint8_t pin, bool state) 
{
	int pin2;
	if(pin==0)
		pin2=2;
	else
		pin2=2;
	  pinMode(pin2, OUTPUT);
	  digitalWrite(pin2, state ? HIGH : LOW);
	  return true;
}

// 온도 읽기 함수
float ioehub_mqtt_get_temperature(uint8_t pin) 
{
 
  if (dhtSensors[pin] == NULL) {
    dhtSensors[pin] = new DHT(pin, DHTTYPE);
    dhtSensors[pin]->begin();
    delay(2000); // DHT 센서 안정화 시간
  }
   return dhtSensors[pin]->readTemperature();
 
//	sensor.read();

//	return 1.0;
}

// 습도 읽기 함수
float ioehub_mqtt_get_humidity(uint8_t pin) 
{
/*
  if (dhtSensors[pin] == NULL) {
    dhtSensors[pin] = new DHT(pin, DHTTYPE);
    dhtSensors[pin]->begin();
    delay(2000); // DHT 센서 안정화 시간
  }
  return dhtSensors[pin]->readHumidity();
*/
	return 2.0;
}

// 서보모터 제어 함수
bool ioehub_mqtt_set_servo(uint8_t pin, uint8_t angle) {
  if (angle > 180) angle = 180;
  
  if (!servoAttached[pin]) {
    servos[pin].attach(pin);
    servoAttached[pin] = true;
  }
  
  servos[pin].write(angle);
  return true;
}

// 다중 LED 제어 함수
int ioehub_mqtt_set_multi_led(JsonArray pins, JsonArray states) {
  int successCount = 0;
  
  for (size_t i = 0; i < pins.size() && i < states.size(); i++) {
    uint8_t pin = pins[i];
    bool state = states[i];
    
    if (ioehub_mqtt_set_led(pin, state)) {
      successCount++;
    }
  }
  
  return successCount;
}

// 다중 서보모터 제어 함수
int ioehub_mqtt_set_multi_servo(JsonArray pins, JsonArray angles) {
  int successCount = 0;
  
  for (size_t i = 0; i < pins.size() && i < angles.size(); i++) {
    uint8_t pin = pins[i];
    uint8_t angle = angles[i];
    
    if (ioehub_mqtt_set_servo(pin, angle)) {
      successCount++;
    }
  }
  
  return successCount;
}

// 핀 상태 확인 함수
int ioehub_mqtt_get_pin_status(uint8_t pin) {
  pinMode(pin, INPUT);
  return digitalRead(pin);
}

// 모든 센서 데이터 수집 함수
String ioehub_mqtt_get_all_sensors() {
  StaticJsonDocument<512> doc;
  JsonObject sensors = doc.createNestedObject("sensors");
  
  // 간단한 예시: DHT 센서가 연결된 핀 4, 5의 데이터를 수집
  if (dhtSensors[4] != NULL) {
    JsonObject dht4 = sensors.createNestedObject("dht4");
    dht4["temperature"] = ioehub_mqtt_get_temperature(4);
    dht4["humidity"] = ioehub_mqtt_get_humidity(4);
  }
  
  if (dhtSensors[5] != NULL) {
    JsonObject dht5 = sensors.createNestedObject("dht5");
    dht5["temperature"] = ioehub_mqtt_get_temperature(5);
    dht5["humidity"] = ioehub_mqtt_get_humidity(5);
  }
  
  String result;
  serializeJson(doc, result);
  return result;
}

// MQTT 메시지 처리 콜백 함수
void callback(char* topic, byte* payload, unsigned int length) 
{
  // 수신된 메시지를 문자열로 변환
  char message[length + 1];
  for (unsigned int i = 0; i < length; i++) {
    message[i] = (char)payload[i];
  }
  message[length] = '\0';
  
  // JSON 파싱
  StaticJsonDocument<512> doc;
  DeserializationError error = deserializeJson(doc, message);
  
  // 파싱 오류 확인
  if (error) {
    String errorResponse = createJsonResponse("error", "Invalid JSON format");
    client.publish(mqtt_publish_topic, errorResponse.c_str());
    return;
  }
  
  // 함수 이름 및 파라미터 추출
  const char* function = doc["function"];
  JsonObject params = doc["params"];
  
  // 응답 변수
  String response;
  Serial.printf("function[%s]",(char*)function);
  Serial.println("");
  // 함수 실행
  if (strcmp(function, "ioehub_mqtt_set_led") == 0) 
  {
    uint8_t pin = params["pin"];
    Serial.printf("pin[%d]",pin);
    Serial.println("");
	bool state = params["state"];
    bool result = ioehub_mqtt_set_led(pin, state);
    response = createJsonResponse(function, result ? 1 : 0);
  }
  else if (strcmp(function, "ioehub_mqtt_get_temperature") == 0) {
    uint8_t pin = params["pin"];
    float temp = ioehub_mqtt_get_temperature(pin);
    response = createJsonResponse(function, temp);
  }
  else if (strcmp(function, "ioehub_mqtt_get_humidity") == 0) {
    uint8_t pin = params["pin"];
    float humidity = ioehub_mqtt_get_humidity(pin);
    response = createJsonResponse(function, humidity);
  }
  else if (strcmp(function, "ioehub_mqtt_set_servo") == 0) {
    uint8_t pin = params["pin"];
    uint8_t angle = params["angle"];
    bool result = ioehub_mqtt_set_servo(pin, angle);
    response = createJsonResponse(function, result ? 1 : 0);
  }
  else if (strcmp(function, "ioehub_mqtt_set_multi_led") == 0) {
    JsonArray pins = params["pins"];
    JsonArray states = params["states"];
    int successCount = ioehub_mqtt_set_multi_led(pins, states);
    response = createJsonResponse(function, successCount);
  }
  else if (strcmp(function, "ioehub_mqtt_set_multi_servo") == 0) {
    JsonArray pins = params["pins"];
    JsonArray angles = params["angles"];
    int successCount = ioehub_mqtt_set_multi_servo(pins, angles);
    response = createJsonResponse(function, successCount);
  }
  else if (strcmp(function, "ioehub_mqtt_get_pin_status") == 0) {
    uint8_t pin = params["pin"];
    int status = ioehub_mqtt_get_pin_status(pin);
    response = createJsonResponse(function, status);
  }
  else if (strcmp(function, "ioehub_mqtt_get_all_sensors") == 0) {
    String sensorData = ioehub_mqtt_get_all_sensors();
    // JSON 내의 JSON 문자열을 전송하기 위해 특별 처리
    StaticJsonDocument<768> responseDoc;
    responseDoc["function"] = function;
    responseDoc["result"] = serialized(sensorData);
    responseDoc["timestamp"] = getTimestamp();
    String response;
    serializeJson(responseDoc, response);
  }
  else {
    response = createJsonResponse("error", "Unknown function");
  }
  
  // 응답 전송
  client.publish(mqtt_publish_topic, response.c_str());
}
