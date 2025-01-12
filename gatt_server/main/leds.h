#ifndef LEDS_H
#define LEDS_H


#include <stdio.h>
#include "esp_log.h"
#include "driver/gpio.h"

#define RED_LED_GPIO GPIO_NUM_17
#define GREEN_LED_GPIO GPIO_NUM_16

void init_leds();

void red_led_on();
void red_led_off();

void green_led_on();
void green_led_off();

void blink_led_task(void *pvParameters);

#endif // LEDS_H