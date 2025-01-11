#include "main.h"

uint8_t ssid[32] = {0};
uint8_t password[64] = {0};

bool was_ssid_set = false;
bool was_password_set = false;



static const char *TAG_MQTT = "mqtt_example";
static uint32_t sendFreq = 10000;
static bool isMqttConnected = false;
static esp_mqtt_client_handle_t client;
static char topic[128] = {0};
static uint8_t mac[6] = {0};
static uint64_t msg_id = 0;
static bool wifiAfterFirstConnect = false;




void initialize_sntp(void) {
    ESP_LOGI("SNTP", "Initializing SNTP");
    esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
    esp_sntp_setservername(0, "pool.ntp.org");
    esp_sntp_init();
}


static void setFreq(uint8_t f) {
    switch(f) {
        case '0': sendFreq = 500; break;
        case '1': sendFreq = 1000; break;
        case '2': sendFreq = 5000; break;
        case '3': sendFreq = 10000; break;
        case '4': sendFreq = 20000; break;
        default: sendFreq = 10000; break;
    }
    ESP_LOGI(TAG_MQTT, "Send frequency set to %lu", sendFreq);
}
static void setPressureFilter(uint8_t f) {
    uint8_t config;
    switch(f) {
        case '0': config = 0x80; break;
        case '1': config = 0xA0; break;
        case '2': config = 0xC0; break;
        case '3': config = 0xE0; break;
        default: config = 0x80; break;
    }
    esp_err_t ret = i2c_set_freq(config);
    if(ret == ESP_OK) {
        ESP_LOGI(TAG_MQTT, "Measure frequency set to 0x%X", config);
    }

}



static void if_wifi_configured() {
    if(was_ssid_set && was_password_set) {
        save_to_nvs(SSID_KEY, (char*)ssid);
        save_to_nvs(PASSWORD_KEY, (char*)password);
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
    if(!wifiAfterFirstConnect) {
        time_t now;
        initialize_sntp();
        while(time(&now) < 24 * 3600) {
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            ESP_LOGI("SNTP", "Waiting for time to be set...");
        }
        mqtt_start();
        wifiAfterFirstConnect = true;
    }
}
void onWifiFailded(void) {
    // esp_restart();
}




void onMqttConnected(esp_mqtt_client_handle_t c) {
    client = c;
    isMqttConnected = true;
}
void onMqttDisconnected() {
    isMqttConnected = false;
}

void onMqttMessageReceived(char* data, int data_len, char* topic, int topic_len) {
    char data_arr[data_len + 1];
    strncpy(data_arr, data, data_len);
    data_arr[data_len] = '\0';

    char topic_arr[topic_len + 1];
    strncpy(topic_arr, topic, topic_len);
    topic_arr[topic_len] = '\0';

    if(data_len < 2) {
        ESP_LOGE(TAG_MQTT, "Invalid data received: %s", data_arr);
        return;
    }

    switch(data_arr[0]) {
        case 'f': setFreq(data_arr[1]); break;
        case 'r': esp_restart(); break;
        case 'p': setPressureFilter(data_arr[1]); break;
        default: ESP_LOGE(TAG_MQTT, "Invalid command received: %s", data_arr); break;
    }

    
}

void getSendData() {
    while(true) {
        float temperature, pressure, humidity;
        esp_err_t ret = read_sensor_data(&temperature, &pressure, &humidity);
        ESP_LOGI("I2C", "Temperature: %.2f °C, Pressure: %.2f hPa, Humidity: %.2f %%RH", temperature, pressure, humidity);
        if(ret == ESP_OK && isMqttConnected) {
            char message[128];
            time_t now;
            time(&now);
            snprintf(message, sizeof(message), "{\"time\":%lld, \"temperature\":%f, \"pressure\":%f, \"moisture\": %f}", (long long)now, temperature, pressure, humidity);
            msg_id++;
            esp_mqtt_client_publish(client, topic, message, 0, 1, 0);
            ESP_LOGI(TAG_MQTT, "sent publish successful, msg_id=%llu, freq=%lu", msg_id, sendFreq);
        }
        vTaskDelay(sendFreq / portTICK_PERIOD_MS);
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
        "/esp/%02X:%02X:%02X:%02X:%02X:%02X/out", 
        mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]
    );



    i2c_start();
    xTaskCreate(&getSendData, "getSendData", 4096, NULL, 5, NULL);



    read_from_nvs(SSID_KEY, (char*)ssid, sizeof(ssid) - 1);
    read_from_nvs(PASSWORD_KEY, (char*)password, sizeof(password) - 1);

    wifi_init_sta();
    bluetoothInit();

    esp_rom_gpio_pad_select_gpio(LED_PIN);
    gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);
}