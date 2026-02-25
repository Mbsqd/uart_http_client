#ifndef WIFI_H
#define WIFI_H

#include "esp_err.h"

// Function prototype for initializing Wi-Fi in station mode
// Function blocks until the Wi-Fi connection is established or fails after maximum retries
// Returns ESP_OK on success, or an error code on failure

 esp_err_t wifi_init_sta();

#endif
