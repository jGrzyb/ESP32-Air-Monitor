#ifndef MAIN_H
#define MAIN_H

#include "app_main.h"

void onMqttConnected();
void onMqttDisconnected();
void onMqttMessageReceived(char* data, int len);

void app_main(void);

#endif // MAIN_H