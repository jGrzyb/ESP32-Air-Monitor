#ifndef STATION_EXAMPLE_MAIN_H
#define STATION_EXAMPLE_MAIN_H

#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "esp_system.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "driver/gpio.h"

#define WIFI_SSID "GRZYYB"
#define WIFI_PASS "12345678"
#define MAX_RETRY 10
#define LED_PIN GPIO_NUM_2 // LED na GPIO2 w ESP32

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1

#define WEB_SERVER "www.columbia.edu"
#define WEB_PORT "80"
#define REQUEST "GET /~fdc/sample.html HTTP/1.1\r\nHost: www.columbia.edu\r\n\r\n"

void blink_led_task(void *pvParameters);
void wifi_init_sta(void);
void http_get_task(void *pvParameters);

#endif // STATION_EXAMPLE_MAIN_H