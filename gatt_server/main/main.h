#ifndef MAIN_H
#define MAIN_H

#include "station_example_main.h"
#include "gatts_demo.h"
#include "nvs_manager.h"

// on bluetooth write event
void onBluetoothWrite_a(uint8_t* arrPtr, int len);
void onBluetoothRead_a(esp_gatt_rsp_t* rsp);

void onBluetoothWrite_b(uint8_t* arrPtr, int len);
void onBluetoothRead_b(esp_gatt_rsp_t* rsp);

void onWifiConnected(void);
void onWifiFailded(void);


void app_main(void);

#endif // MAIN_H