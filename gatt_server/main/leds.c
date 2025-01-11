#include "leds.h"

static const char *TAG_RED_LED = "RED_LED";
static const char *TAG_BLUE_LED = "BLUE_LED";

void init_leds() {
    gpio_reset_pin(RED_LED_PIN);
    gpio_set_direction(RED_LED_PIN, GPIO_MODE_OUTPUT);
    gpio_reset_pin(BLUE_LED_PIN);
    gpio_set_direction(BLUE_LED_PIN, GPIO_MODE_OUTPUT);
}

void turn_on_RED_led() {
    ESP_LOGE(TAG_RED_LED,"Turn on red diode led!");
    gpio_set_level(RED_LED_PIN, 1);
}

void turn_off_RED_led() {
    ESP_LOGE(TAG_RED_LED,"Turn off red diode led!");
    gpio_set_level(RED_LED_PIN, 0);
}

void turn_on_BLUE_led() {
    ESP_LOGE(TAG_BLUE_LED,"Turn on blue diode led!");
    gpio_set_level(BLUE_LED_PIN,1);
}

void turn_off_BLUE_led() {
    ESP_LOGE(TAG_BLUE_LED,"Turn off blue diode led!");
    gpio_set_level(BLUE_LED_PIN, 0);
}