#ifndef MAIN_H
#define MAIN_H

#include "station_example_main.h"
#include "gatts_demo.h"

// what bluetoothRead returns
static uint8_t* bluetoothMessage = NULL;
static int bluetoothMessage_len = 0;

// on bluetooth write event
void bluetoothWrite(uint8_t* arrPtr, int len);
void bluetoothRead(esp_gatt_rsp_t* rsp);


void app_main(void);

#endif // MAIN_H