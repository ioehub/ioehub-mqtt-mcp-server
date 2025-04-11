# IoEHub MQTT MCP 서버

## 1. 개요

이 문서는 MQTT를 통해 온도 센서 데이터를 읽고 LED를 제어하는 MCP(Model Context Protocol) 서버를 설명합니다. 이 서버는 FastMCP 프레임워크를 기반으로 하며, JSON-RPC 프로토콜을 사용하여 통신합니다.

## 2. 시스템 구성

- **프레임워크**: FastMCP
- **통신 프로토콜**: MQTT, JSON-RPC 2.0
- **구현 언어**: Python
- **지원 기능**: 온도 측정, LED 제어

## 3. MQTT 설정

```
MQTT_BROKER = "172.30.1.100"
MQTT_PORT = 1883
MQTT_USERNAME = "ioehub"
MQTT_PASSWORD = "password"
MQTT_PUBLISH_TOPIC = "ioehub/mcp/command"
MQTT_SUBSCRIBE_TOPIC = "ioehub/mcp/response"
```

## 4. 구현된 기능

### 4.1 온도 측정 (`ioehub_mqtt_get_temperature`)

- **설명**: MQTT를 통해 온도 센서에서 현재 온도 데이터를 읽어옵니다.
- **반환 값**: 현재 온도(섭씨)를 문자열로 반환
- **기본 핀**: 13번

### 4.2 LED 제어 (`ioehub_mqtt_set_led`)

- **설명**: MQTT를 통해 지정된 핀의 LED 상태를 제어합니다.
- **파라미터**:
  - `pin` (정수): LED 핀 번호 (기본값: 0)
  - `state` (정수): LED 상태 (1=켜짐, 0=꺼짐)
- **반환 값**: 작업 성공 여부(True/False)

## 5. 메시지 형식

### 5.1 온도 측정 요청

```json
{
  "function": "ioehub_mqtt_get_temperature",
  "params": {
    "pin": 13
  }
}
```

### 5.2 온도 측정 응답

```json
{
  "function": "ioehub_mqtt_get_temperature",
  "result": 26.5,
  "timestamp": "749817"
}
```

### 5.3 LED 제어 요청

```json
{
  "function": "ioehub_mqtt_set_led",
  "params": {
    "pin": 0,
    "state": 1
  }
}
```

### 5.4 LED 제어 응답

```json
{
  "function": "ioehub_mqtt_set_led",
  "result": true,
  "timestamp": "749820"
}
```

## 6. 작동 방식

1. **서버 초기화**:
   - MQTT 클라이언트 생성 및 설정
   - 구독 토픽 설정 (ioehub/mcp/response)
   - FastMCP 인스턴스 생성

2. **연결 프로세스**:
   - MQTT 브로커에 연결
   - 백그라운드 스레드에서 메시지 수신 대기

3. **함수 호출 처리**:
   - MCP 도구 호출 시 해당 함수 실행
   - MQTT를 통해 요청 전송
   - 응답 대기 (최대 5초)
   - 결과 반환

4. **메시지 수신 처리**:
   - 구독 토픽에서 메시지 수신
   - JSON 파싱 및 데이터 추출
   - 응답 플래그 설정

## 7. 오류 처리

- **응답 타임아웃**: 5초 이내에 응답이 없으면 기본값 반환
- **JSON 파싱 오류**: 잘못된 형식의 메시지 처리
- **MQTT 연결 오류**: 연결 실패 시 오류 메시지 출력

## 8. 사용 방법

### 8.1 서버 실행

```bash
python mcp_server.py
```

### 8.2 클라이언트에서 함수 호출

```python
# MCP 클라이언트 예시
temperature = client.invoke("ioehub_mqtt_get_temperature")
print(f"현재 온도: {temperature}°C")

# LED 켜기
result = client.invoke("ioehub_mqtt_set_led", {"pin": 0, "state": 1})
print(f"LED 켜기 결과: {'성공' if result else '실패'}")
```

## 9. mcp server 설정
```
{
  "mcpServers": {
    "IoEHubMqttMcpServer": {
        "command": "C:\\vang\\project\\2025\\ioehub\\mqtt-mcp-server\\.venv\\Scripts\\python.exe",
        "args": ["C:\\vang\\project\\2025\\ioehub\\mqtt-mcp-server\\mcp_server.py"]

    }    
  }
}
```

## 10. 제한 사항 및 참고사항

- MQTT 브로커가 실행 중이어야 합니다.
- 응답 처리 시 'function' 필드를 확인하여 적절한 응답을 식별합니다.
- 모든 디버그 메시지는 stdout이 아닌 stderr로 출력되어야 합니다.
- 실제 하드웨어와 통신하기 위해서는 해당 장치에서 MQTT 클라이언트가 실행되어야 합니다.
