#include "main.h"


uint8_t ssid[32] = {0};
uint8_t password[64] = {0};
uint8_t server_ip[65] = {0};

bool was_ssid_set = false;
bool was_password_set = false;
bool was_server_ip_set = false;


#define portTICK_PERIOD_MS ( ( TickType_t ) 1000 / CONFIG_FREERTOS_HZ )
#define CONFIG_FREERTOS_HZ 1000


static const char *TAG_MQTT = "mqtt_example";
static uint32_t sendFreq = 10000;
static bool isMqttConnected = false;
static esp_mqtt_client_handle_t client;
static char topic[128] = {0};
static uint8_t mac[6] = {0};
static uint64_t msg_id = 0;
static bool wifiAfterFirstConnect = false;

static float min_temp = 20.0f;
static float max_temp = 24.0f;

static float min_pressure = 1000.0f;
static float max_pressure = 1020.0f;

static float min_humidity = 0.3f;
static float max_humidity = 0.6f;

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

static void setTemperatureLimits(char* data) {
    if (sscanf(data, "%f %f", &min_temp, &max_temp) == 2) {
        printf("Temperature should be between %.2f and %.2f\n", min_temp,max_temp);
    } else {
        printf("Error: Could not parse the input string.\n");
    }
}

static void setPressureLimits(char* data) {
    if (sscanf(data, "%f %f", &min_pressure, &max_pressure) == 2) {
        printf("Pressure should be between %.2f and %.2f\n", min_pressure,max_pressure);
    } else {
        printf("Error: Could not parse the input string.\n");
    }
}

static void setHumidityLimits(char* data) {
    if (sscanf(data, "%f %f", &min_humidity, &max_humidity) == 2) {
        printf("Humidity should be between %.2f and %.2f\n", min_humidity,max_humidity);
    } else {
        printf("Error: Could not parse the input string.\n");
    }
}

static void writeLimits() {
    char result[12] = {0};
    sprintf(result, "%.2f %.2f", min_temp, max_temp);
    save_to_nvs(LIMITS_TEMP,result);
    sprintf(result, "%.2f %.2f", min_pressure, max_pressure);
    save_to_nvs(LIMITS_PRESSURE,result);
    sprintf(result, "%.2f %.2f", min_humidity, max_humidity);
    save_to_nvs(LIMITS_HUMIDITY,result);
}

static void readLimits() {
    char *data[12] = {0};
    read_from_nvs(LIMITS_TEMP, data, sizeof(data) - 1);
    setTemperatureLimits(data);
    read_from_nvs(LIMITS_PRESSURE, data, sizeof(data) - 1);
    setPressureLimits(data);
    read_from_nvs(LIMITS_HUMIDITY, data, sizeof(data) - 1);
    setHumidityLimits(data);
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
    //if_wifi_configured();
}

void onBluetoothRead_a(esp_gatt_rsp_t* rsp) {
    rsp->attr_value.len = strlen((char*)ssid);
    memcpy(rsp->attr_value.value, ssid, strlen((char*)ssid));
}

void onBluetoothWrite_b(uint8_t* arrPtr, int len) {
    memcpy(password, arrPtr, len);
    password[len] = '\0';
    was_password_set = true;
    //if_wifi_configured();
}

void onBluetoothRead_b(esp_gatt_rsp_t* rsp) {
    rsp->attr_value.len = strlen((char*)password);
    memcpy(rsp->attr_value.value, password, strlen((char*)password));
}

void onBluetoothWrite_c(uint8_t* arrPtr, int len) {
    // printf("onBluetoothWrite_c\n");
    memcpy(server_ip, arrPtr, len);
    server_ip[len] = '\0';
    printf("%s\n",(char*)server_ip);
    was_server_ip_set = true;
}

void onBluetoothRead_c(esp_gatt_rsp_t* rsp) {
    // printf("onBluetoothRead_c\n");
    rsp->attr_value.len = strlen((char*)server_ip);
    memcpy(rsp->attr_value.value, server_ip, strlen((char*)server_ip));
    printf("%s\n",(char*)server_ip);
}

void onBluetoothWrite_d(uint8_t* arrPtr, int len) {
    if (len == 0) return;
    // printf("onBluetoothWrite_d\n");
    if (was_ssid_set)
        save_to_nvs(SSID_KEY,(char*)ssid);
    if (was_password_set)
        save_to_nvs(PASSWORD_KEY,(char*)password);
    if (was_server_ip_set)
        save_to_nvs(SERVER_IP,(char*)server_ip);

    if (was_password_set || was_server_ip_set || was_ssid_set)
        esp_restart();
}

void onBluetoothRead_d(esp_gatt_rsp_t* rsp) {
    return;
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
        case 't': setTemperatureLimits(data_arr[1]); break;//"t0.3 0.6"
        case 'c': setPressureLimits(data_arr[1]); break; //"c0.3 0.6"
        case 'w': setHumidityLimits(data_arr[1]); break; //"w0.3 0.6"
        default: ESP_LOGE(TAG_MQTT, "Invalid command received: %s", data_arr); break;
    }

    
}

void getSendData() {
    while(true) {
        float temperature, pressure, humidity;
        esp_err_t ret = read_sensor_data(&temperature, &pressure, &humidity);

        bool is_temperature_good = (temperature >= min_temp || temperature <= max_temp);
        bool is_pressure_good = (pressure >= min_pressure || pressure <= max_pressure);
        bool is_humidity_good = (humidity >= min_humidity || humidity <= max_humidity);

        if (is_temperature_good && is_pressure_good  && is_humidity_good) {
            red_led_off();
            green_led_on();
        }
        else if (!is_temperature_good && !is_pressure_good  && !is_humidity_good) {
            red_led_on();
            green_led_off();
        }
        else {
            red_led_on();
            green_led_on();
        }

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

// void app_main(void) {
//     printf("Hello, world!\n");
// }

void app_main(void)
{
    leds_init();

    red_led_on();
    green_led_on();
    // vTaskDelay(1000);
    // red_led_off();
    // green_led_off();

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
    read_from_nvs(SERVER_IP, (char*)server_ip, sizeof(server_ip) - 1);
    wifi_init_sta();
    bluetoothInit();
    esp_rom_gpio_pad_select_gpio(LED_PIN);
    gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);

    red_led_off();
    green_led_off();
}