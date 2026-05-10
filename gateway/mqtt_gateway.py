import json
import time
import paho.mqtt.client as mqtt

# --------------------------------------
# CONFIG
# --------------------------------------
LOCAL_BROKER = "192.168.137.1"
LOCAL_PORT = 1883

CORE_IOT_HOST = "app.coreiot.io"
CORE_IOT_PORT = 1883
ACCESS_TOKEN = "g5fqNKTTwyhgEkO9NVKC"
CORE_IOT_TOPIC = "v1/gateway/telemetry"
CORE_IOT_RPC_TOPIC = "v1/gateway/rpc"
LOCAL_TOPICS = [
    ("esp32/+/telemetry", 0),
    ("esp32/+/anomaly", 0),
    ("esp32/+/status", 0),
]
STATUS_TIMEOUT_SECONDS = 20
STATUS_CHECK_INTERVAL = 5
DEBUG_LOCAL_MESSAGES = True
last_status_timestamps = {
    "esp1": 0,
    "esp2": 0,
}
reported_offline = {
    "esp1": False,
    "esp2": False,
}
last_status_check = time.time()

# --------------------------------------
# CoreIoT client
# --------------------------------------
tb_client = mqtt.Client(client_id="CoreIoT_Gateway")
tb_client.username_pw_set(ACCESS_TOKEN)


def on_connect_tb(client, userdata, flags, rc):
    if rc == 0:
        print("Connected to CoreIoT:", rc)
        tb_client.subscribe(CORE_IOT_RPC_TOPIC)
        print("Subscribed to CoreIoT RPC topic:", CORE_IOT_RPC_TOPIC)
    else:
        print("Failed to connect to CoreIoT, rc=", rc)


def handle_rpc_request(topic, payload_text):
    try:
        data = json.loads(payload_text)
    except json.JSONDecodeError as exc:
        print("Invalid RPC JSON payload:", exc)
        return

    # Handle nested structure: {"device":"esp1","data":{"id":4,"method":"setStateLed1","params":true}}
    rpc_data = data.get("data", data)  # Fallback to root if no "data" key
    
    method = rpc_data.get("method")
    params = rpc_data.get("params")
    request_id = rpc_data.get("id", 0)
    device_name = data.get("device", "esp1")  # Get device from root level

    print(f"RPC Request: method={method}, params={params}, id={request_id}, device={device_name}")

    if method == "setStateLed1":
        device_name = "esp1"
    elif method == "setStateLed2":
        device_name = "esp2"
    elif method is None:
        print(f"Unknown RPC method: None - full payload: {payload_text}")
        return
    else:
        print(f"Unknown RPC method: {method}")
        return

    state = "on" if params else "off"
    led_cmd = {"state": state}
    cmd_topic = f"esp32/{device_name}/cmd/led"
    cmd_payload = json.dumps(led_cmd)

    result = local_client.publish(cmd_topic, cmd_payload)
    print(f"Published LED command to {cmd_topic}: {cmd_payload}, result={result}")

    response = {
        "device": device_name,
        "id": request_id,
        "data": {
            "result": "ok",
            "state": state
        }
    }

    tb_client.publish(
        "v1/gateway/rpc",
        json.dumps(response)
    )

    print("Published gateway RPC response")



tb_client.on_connect = on_connect_tb

# --------------------------------------
# Local MQTT client
# --------------------------------------
local_client = mqtt.Client(client_id="Local_MQTT_Gateway")


def on_connect_local(client, userdata, flags, rc):
    if rc == 0:
        print("Connected to local MQTT broker")
        for topic, qos in LOCAL_TOPICS:
            client.subscribe(topic, qos)
            print("Subscribed to local topic:", topic)
    else:
        print("Failed to connect to local MQTT broker, rc=", rc)


def publish_coreiot_payload(device_name, values):
    telemetry = {
        device_name: [
            {
                "ts": int(time.time() * 1000),
                "values": values,
            }
        ]
    }
    payload = json.dumps(telemetry)
    result = tb_client.publish(CORE_IOT_TOPIC, payload)
    print("Forwarded to CoreIoT topic:", CORE_IOT_TOPIC, "result:", result)
    print("CoreIoT payload:", payload)


def publish_offline(device_name):
    values = {"status": "offline"}
    publish_coreiot_payload(device_name, values)
    reported_offline[device_name] = True
    print("Published offline status for", device_name)


def on_message_local(client, userdata, msg):
    payload_text = msg.payload.decode(errors="replace")
    if DEBUG_LOCAL_MESSAGES:
        print("Received local message:", msg.topic, payload_text)

    try:
        data = json.loads(payload_text)
    except json.JSONDecodeError as exc:
        print("Invalid JSON payload:", exc)
        return

    parts = msg.topic.split("/")
    if len(parts) != 3 or parts[0] != "esp32":
        print("Unexpected topic format, ignoring:", msg.topic)
        return

    device_name = parts[1]
    topic_type = parts[2]

    if topic_type not in ["telemetry", "anomaly", "status"]:
        print("Topic not forwarded to CoreIoT:", msg.topic)
        return

    if topic_type == "status":
        if device_name in last_status_timestamps:
            last_status_timestamps[device_name] = time.time()
            if reported_offline.get(device_name, False):
                reported_offline[device_name] = False
                print(device_name, "returned online")

    publish_coreiot_payload(device_name, data)


def on_message_coreiot(client, userdata, msg):
    payload_text = msg.payload.decode(errors="replace")
    print(f"📥 CoreIoT Topic: {msg.topic}")
    print(f"📦 Payload: {payload_text}")

    if "rpc" in msg.topic:
        handle_rpc_request(msg.topic, payload_text)


local_client.on_connect = on_connect_local
local_client.on_message = on_message_local

tb_client.on_message = on_message_coreiot

# --------------------------------------
# RUN
# --------------------------------------
print("Starting gateway...")

try:
    tb_client.connect(CORE_IOT_HOST, CORE_IOT_PORT, 60)
except Exception as exc:
    print("CoreIoT connect failed:", exc)

try:
    local_client.connect(LOCAL_BROKER, LOCAL_PORT, 60)
except Exception as exc:
    print("Local MQTT connect failed:", exc)


tb_client.loop_start()
local_client.loop_start()

while True:
    time.sleep(1)
    current_time = time.time()
    if current_time - last_status_check >= STATUS_CHECK_INTERVAL:
        for device_name, last_time in last_status_timestamps.items():
            if current_time - last_time >= STATUS_TIMEOUT_SECONDS and not reported_offline.get(device_name, False):
                publish_offline(device_name)
        last_status_check = current_time

    if not local_client.is_connected():
        try:
            local_client.reconnect()
        except Exception as exc:
            print("Local MQTT reconnect failed:", exc)
            time.sleep(2)
    if not tb_client.is_connected():
        try:
            tb_client.reconnect()
        except Exception as exc:
            print("CoreIoT reconnect failed:", exc)
            time.sleep(2)
