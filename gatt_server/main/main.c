#include "main.h"

uint8_t ssid[32] = {0};
uint8_t password[64] = {0};

bool was_ssid_set = false;
bool was_password_set = false;



static const char *TAG_MQTT = "mqtt_example";
static uint8_t freq = 10;
static bool isMqttConnected = false;
static esp_mqtt_client_handle_t client;
static char topic[128] = {0};
static uint8_t mac[6] = {0};
static uint64_t msg_id = 0;




static void if_wifi_configured() {
    if(was_ssid_set && was_password_set) {
        save_to_nvs(SSID_KEY, (char*)ssid);
        save_to_nvs(PASSWORD_KEY, (char*)password);
        // vTaskDelay(1000 / portTICK_PERIOD_MS);
        esp_restart();
    }
}

void onBluetoothWrite_a(uint8_t* arrPtr, int len) {
    memcpy(ssid, arrPtr, len);
    ssid[len] = '\0';
    was_ssid_set = true;
    if_wifi_configured();
}
void onBluetoothRead_a(esp_gatt_rsp_t* rsp) {
    rsp->attr_value.len = strlen((char*)ssid);
    memcpy(rsp->attr_value.value, ssid, strlen((char*)ssid));
}

void onBluetoothWrite_b(uint8_t* arrPtr, int len) {
    memcpy(password, arrPtr, len);
    password[len] = '\0';
    was_password_set = true;
    if_wifi_configured();
}
void onBluetoothRead_b(esp_gatt_rsp_t* rsp) {
    rsp->attr_value.len = strlen((char*)password);
    memcpy(rsp->attr_value.value, password, strlen((char*)password));
}

void onWifiConnected(void) {
    // xTaskCreate(&http_get_task, "http_get_task", 4096, NULL, 5, NULL);
    mqtt_start();
}
void onWifiFailded(void) {
    // esp_restart();
}




void onMqttConnected(esp_mqtt_client_handle_t c) {
    client = c;
    isMqttConnected = true;
    // xTaskCreate(&whenMqttConnected, "mqttConnectedTask", 4096, NULL, 5, NULL);
}

void onMqttDisconnected() {
    isMqttConnected = false;
}

void onMqttMessageReceived(char* data, int data_len) {
    char temp[data_len + 1];
    strncpy(temp, data, data_len);
    temp[data_len] = '\0';

    char* end;
    long value = strtol(temp, &end, 10);
    if(value <= 0) {
        value = 10;
    }
    freq = value;
}

// void mqttSendMessage() {
//     char message[128];
//     snprintf(message, sizeof(message), "{\"id\":%llu, \"temperature\":%d, \"pressure\":%d, \"moisture\": %d}", msg_id, rand(), rand(), rand());
//     msg_id++;
//     esp_mqtt_client_publish(client, topic, message, 0, 1, 0);
    
//     ESP_LOGI(TAG_MQTT, "sent publish successful, msg_id=%llu", msg_id);
//     ESP_LOGI(TAG_MQTT, "FREQ: %d", freq);
// }

// void i2c_task() {
//     while(true) {
//         float temperature, pressure, humidity;
//         read_sensor_data(&temperature, &pressure, &humidity);
//         ESP_LOGI("I2C", "Temperature: %.2f °C, Pressure: %.2f hPa, Humidity: %.2f %%RH", temperature, pressure, humidity);
//         vTaskDelay(1000 / portTICK_PERIOD_MS);
//     }
// }


void getSendData() {
    while(true) {
        float temperature, pressure, humidity;
        esp_err_t ret = read_sensor_data(&temperature, &pressure, &humidity);
        ESP_LOGI("I2C", "Temperature: %.2f °C, Pressure: %.2f hPa, Humidity: %.2f %%RH", temperature, pressure, humidity);

        char message[128];
        snprintf(message, sizeof(message), "{\"id\":%llu, \"temperature\":%f, \"pressure\":%f, \"moisture\": %f}", msg_id, temperature, pressure, humidity);
        msg_id++;
        esp_mqtt_client_publish(client, topic, message, 0, 1, 0);
        ESP_LOGI(TAG_MQTT, "sent publish successful, msg_id=%llu, freq=%d", msg_id, freq);

        vTaskDelay(10000 / portTICK_PERIOD_MS);
    }
}



void app_main(void)
{
    esp_err_t ret;

    ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK( ret );



    esp_read_mac(mac, CONFIG_ESP_MAC_ADDR_UNIVERSE_WIFI_STA);
    snprintf(
        topic, sizeof(topic), 
        "/user1/out/%02X:%02X:%02X:%02X:%02X:%02X", 
        mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]
    );



    i2c_start();
    xTaskCreate(&getSendData, "getSendData", 4096, NULL, 5, NULL);
    // xTaskCreate(&i2c_task, "i2c_task", 4096, NULL, 5, NULL);



    read_from_nvs(SSID_KEY, (char*)ssid, sizeof(ssid) - 1);
    read_from_nvs(PASSWORD_KEY, (char*)password, sizeof(password) - 1);

    wifi_init_sta();
    bluetoothInit();

    esp_rom_gpio_pad_select_gpio(LED_PIN);
    gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);
}