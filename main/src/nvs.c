#include "esp_log.h"
#include "esp_err.h"
#include "nvs_flash.h"

#include "nvs.h"

static const char *TAG = "NVS";

esp_err_t init_nvs_flash() {
    esp_err_t err = nvs_flash_init();

    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_LOGW(TAG, "init: erasing flash (reason: %s)", esp_err_to_name(err));
      ESP_ERROR_CHECK(nvs_flash_erase());
      err = nvs_flash_init();
    }

    if(err != ESP_OK) {
        ESP_LOGE(TAG, "init failed: %s", esp_err_to_name(err));
        return err;
    } 

    ESP_LOGI(TAG, "NVS flash initialized successfully");
    return ESP_OK;
}
