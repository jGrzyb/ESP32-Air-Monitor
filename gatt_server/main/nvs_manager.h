#ifndef NVS_MANAGER_H
#define NVS_MANAGER_H

#include "nvs_flash.h"
#include "nvs.h"
#include <string.h>
#include <stdio.h>

#define SSID_KEY "ssid"
#define PASSWORD_KEY "pass"

void save_to_nvs(const char* key, const char* value);
void read_from_nvs(const char* key, char* value, size_t max_len);
void remove_from_nvs(const char* key);

#endif // NVS_MANAGER_H