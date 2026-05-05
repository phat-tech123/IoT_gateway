## Configuration

### mosquitto.conf
listener 1883
allow_anonymous true
---

### Open Firewall (Windows)

```bash
netsh advfirewall firewall add rule name="MQTT_1883" dir=in action=allow protocol=TCP localport=1883
```

## Run
### Terminal 1
.\mosquitto.exe -c mosquitto.conf -v

### Terminal 2
Terminal 2 — Subscribe (test MQTT)

### Terminal 3 — Publish test message
.\mosquitto_pub.exe -h 192.168.137.1 -t test -m "hello"

## Terminal 4
python .\mqtt_gateway.py