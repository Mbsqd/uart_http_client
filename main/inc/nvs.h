#ifndef NVS_H
#define NVS_H

#include "esp_err.h"

// Function to initialize NVS flash
// This function initializes the NVS flash storage, need to wifi.
// Returns ESP_OK on success, or an error code on failure

esp_err_t init_nvs_flash();

#endif 
