const mqtt = require('mqtt');
const mqttClient = mqtt.connect('mqtt://broker.hivemq.com'); // Replace with your MQTT broker

mqttClient.on('connect', function () {
    console.log('Connected to MQTT broker');
});

mqttClient.on('error', function (error) {
    console.error('MQTT connection error:', error);
});

module.exports = mqttClient;