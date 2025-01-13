#ifndef NVS_MANAGER_H
#define NVS_MANAGER_H

#include "nvs_flash.h"
#include "nvs.h"
#include <string.h>
#include <stdio.h>

#define SSID_KEY "ssid"
#define PASSWORD_KEY "pass"
#define SERVER_IP "127.0.0.1"

#define MIN_TEMP 20.0f
#define MAX_TEMP 24.0f

#define MIN_PRESSURE 1000.0f
#define MAX_PRESSURE 1020.0f

#define MIN_HUMIDITY 0.3f
#define MAX_HUMIDITY 0.6f

void save_to_nvs(const char* key, const char* value);
void read_from_nvs(const char* key, char* value, size_t max_len);
void remove_from_nvs(const char* key);

#endif // NVS_MANAGER_H