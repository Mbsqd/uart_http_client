#include <stdio.h>
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "uart.h"
#include "nvs.h"
#include "wifi.h"

static const char *TAG = "MAIN";

void app_main(void)
{
    // Initialize NVS
    ESP_ERROR_CHECK(init_nvs_flash());

    // Initialize Wi-Fi
    ESP_ERROR_CHECK(wifi_init_sta());

    // Initialize UART
    uart_init();

    BaseType_t ret = xTaskCreate(uart_read_data, "uart_read_data", 4096, NULL, 10, NULL);
    if (ret != pdPASS) {
        ESP_LOGE(TAG, "task create failed: uart_read_data");
    }
}
