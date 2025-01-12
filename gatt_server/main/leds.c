// leds.c
#include "leds.h"
#include "driver/gpio.h"
#include "esp_log.h"

#define CONFIG_LOG_MAXIMUM_LEVEL ESP_LOG_INFO


static const char *TAG = "leds";

// Initialize the LEDs
void leds_init(void) {
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << RED_LED_GPIO) | (1ULL << GREEN_LED_GPIO),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    ESP_ERROR_CHECK(gpio_config(&io_conf));
    ESP_LOGI(TAG, "LEDs initialized on GPIO %d and GPIO %d", RED_LED_GPIO, GREEN_LED_GPIO);
}

// Turn on the red LED
void red_led_on(void) {
    gpio_set_level(RED_LED_GPIO, 1);
    ESP_LOGI(TAG, "Red LED turned ON");
}

// Turn off the red LED
void red_led_off(void) {
    gpio_set_level(RED_LED_GPIO, 0);
    ESP_LOGI(TAG, "Red LED turned OFF");
}

// Turn on the green LED
void green_led_on(void) {
    gpio_set_level(GREEN_LED_GPIO, 1);
    ESP_LOGI(TAG, "Green LED turned ON");
}

// Turn off the green LED
void green_led_off(void) {
    gpio_set_level(GREEN_LED_GPIO, 0);
    ESP_LOGI(TAG, "Green LED turned OFF");
}