#ifndef LEDS_H
#define LEDS_H


#include <stdio.h>
#include "esp_log.h"
#include "driver/gpio.h"

#define RED_LED_PIN GPIO_NUM_17
#define BLUE_LED_PIN GPIO_NUM_5

void init_leds();

void turn_on_RED_led();
void turn_off_RED_led();

void turn_on_BLUE_led();
void turn_off_BLUE_led();

#endif // LEDS_H