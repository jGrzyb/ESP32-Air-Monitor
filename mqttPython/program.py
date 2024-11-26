import paho.mqtt.client as mqtt
import json
import threading

# The callback for when the client receives a CONNACK response from the server.
def on_connect(client, userdata, flags, reason_code, properties):
    print(f"Connected with result code {reason_code}")
    # Subscribing in on_connect() means that if we lose the connection and
    # reconnect then subscriptions will be renewed.
    client.subscribe("/user1/out/#")

# The callback for when a PUBLISH message is received from the server.
def on_message(client, userdata, msg):
    d = json.loads(str(msg.payload.decode('utf-8')))
    for k in d:
        print(k, ":", d[k])
    print()

mqttc = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2)
mqttc.on_connect = on_connect
mqttc.on_message = on_message

mqttc.connect("mqtt.eclipseprojects.io", 1883, 60)

# Blocking call that processes network traffic, dispatches callbacks and
# handles reconnecting.
# Other loop*() functions are available that give a threaded interface and a
# manual interface.
mqtt_thread = threading.Thread(target=mqttc.loop_forever)
mqtt_thread.start()

while True:
    user_input = input()
    try:
        freq = int(user_input)
        mqttc.publish("/user1/in/F8:B3:B7:21:2C:7D", freq)
    except ValueError:
        print("Invalid input. Please enter an integer.")