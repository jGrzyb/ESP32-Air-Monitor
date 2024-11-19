#ifndef MAIN_H
#define MAIN_H

#include "station_example_main.h"
#include "gatts_demo.h"
#include "nvs_manager.h"

#define WIFI_CONFIG_KEY "wifi_config"

// what bluetoothRead returns
static uint8_t* bluetoothMessage = NULL;
static int bluetoothMessage_len = 0;

// on bluetooth write event
void onBluetoothWrite(uint8_t* arrPtr, int len);
void onBluetoothRead(esp_gatt_rsp_t* rsp);

void onWifiConnected(void);
void onWifiFailded(void);


void app_main(void);

#endif // MAIN_H