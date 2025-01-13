#ifndef NVS_MANAGER_H
#define NVS_MANAGER_H

#include "nvs_flash.h"
#include "nvs.h"
#include <string.h>
#include <stdio.h>

#define SSID_KEY "ssid"
#define PASSWORD_KEY "pass"
#define SERVER_IP "127.0.0.1"

#define LIMITS_TEMP "20.0 24.0"

#define LIMITS_PRESSURE "1000.0 1020.0"

#define LIMITS_HUMIDITY "0.3 0.6"

void save_to_nvs(const char* key, const char* value);
void read_from_nvs(const char* key, char* value, size_t max_len);
void remove_from_nvs(const char* key);

#endif // NVS_MANAGER_H