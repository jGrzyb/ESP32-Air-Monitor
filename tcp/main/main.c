#include "main.h"

static const char *TAG_MQTT = "mqtt_example";
static uint8_t freq = 10;
static bool isMqttConnected = false;
static esp_mqtt_client_handle_t client;
static char topic[128] = {0};
static uint8_t mac[6] = {0};

static void whenMqttConnected() {
    while(isMqttConnected) {
        char message[128];
        snprintf(message, sizeof(message), "%d", rand());

        int msg_id = esp_mqtt_client_publish(client, topic, message, 0, 1, 0);
        ESP_LOGI(TAG_MQTT, "sent publish successful, msg_id=%d", msg_id);
        ESP_LOGI(TAG_MQTT, "FREQ: %d", freq);
        vTaskDelay(freq * 1000 / portTICK_PERIOD_MS);
    }
    vTaskDelete(NULL);
}

void onMqttConnected(esp_mqtt_client_handle_t c) {
    client = c;
    isMqttConnected = true;
    xTaskCreate(&whenMqttConnected, "mqttConnectedTask", 4096, NULL, 5, NULL);
}

void onMqttDisconnected() {
    isMqttConnected = false;
}

void onMqttMessageReceived(char* data, int len) {
    char temp[len + 1];
    strncpy(temp, data, len);
    temp[len] = '\0';

    char* end;
    long value = strtol(temp, &end, 10);
    if(value <= 0) {
        value = 10;
    }
    freq = value;
}

void app_main(void)
{
    // Get MAC address
    esp_read_mac(mac, CONFIG_ESP_MAC_ADDR_UNIVERSE_WIFI_STA);
    snprintf(
        topic, sizeof(topic), 
        "/user1/out/%02X:%02X:%02X:%02X:%02X:%02X", 
        mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]
    );

    app_start();
}