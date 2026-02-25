#include <string.h>
#include "esp_http_client.h"
#include "esp_log.h"
#include "driver/uart.h"
#include "cJSON.h"

#include "uart.h"
#include "http.h"

// Buffer size for storing HTTP response data
#define RESPONSE_BUFFER_SIZE 4096

static const char *TAG = "HTTP";

// Buffer to accumulate HTTP response data and its current length
static char response_buffer[RESPONSE_BUFFER_SIZE];
static int response_len = 0;

// Helper function to parse JSON response and send it over UART
static void print_output_json(const char *json_str)
{
    cJSON *json = cJSON_Parse(json_str);
    if (json == NULL) {
        ESP_LOGW(TAG, "failed to parse JSON response");
        uart_write_bytes(UART_PORT, json_str, strlen(json_str));
        uart_write_bytes(UART_PORT, "\r\n", 2);
        return;
    }

    char *formatted_json = cJSON_PrintUnformatted(json);
    if (formatted_json != NULL) {
        uart_write_bytes(UART_PORT, formatted_json, strlen(formatted_json));
        uart_write_bytes(UART_PORT, "\r\n", 2);
        cJSON_free(formatted_json);
    } else {
        ESP_LOGE(TAG, "response: cJSON_PrintUnformatted failed");
        uart_send("ERROR: failed to format response\r\n");
    }

    cJSON_Delete(json);
}

// HTTP event handler to accumulate response data and handle errors
static esp_err_t http_event_handler(esp_http_client_event_t *evt) {
    switch (evt->event_id) {
        case HTTP_EVENT_ON_DATA:
            if (response_len + evt->data_len < RESPONSE_BUFFER_SIZE - 1) {
                memcpy(response_buffer + response_len, evt->data, evt->data_len);
                response_len += evt->data_len;
            } else {
                ESP_LOGW(TAG, "event: buffer full, truncating");
            }
            break;

        case HTTP_EVENT_ERROR:
            ESP_LOGE(TAG, "event: HTTP_EVENT_ERROR");
            response_len = 0;
            break;

        default:
            break;
    }
    return ESP_OK;
}

// Helper function to handle the final response after HTTP request
static void handle_response(esp_err_t err, const char *method)
{
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "%s failed: %s", method, esp_err_to_name(err));
        char msg[64];
        snprintf(msg, sizeof(msg), "ERROR: %s failed: %s\r\n",
                 method, esp_err_to_name(err));
        uart_send(msg);
        return;
    }

    ESP_LOGI(TAG, "%s ok: response_len=%d", method, response_len);

    if (response_len == 0) {
        ESP_LOGW(TAG, "%s: empty response", method);
        uart_send("OK: empty response\r\n");
        return;
    }

    response_buffer[response_len] = '\0';
    print_output_json(response_buffer);
}

// Public function to perform HTTP GET request
// @param url Full URL to send the GET request, example: "http://httpbin.org/get"
// @return ESP_OK on success, or an error code on failure
esp_err_t http_get(const char *url)
{
    response_len = 0;

    esp_http_client_config_t config = {
        .url = url,
        .event_handler = http_event_handler,
        .method = HTTP_METHOD_GET,
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);
    if (client == NULL) {
        ESP_LOGE(TAG, "GET: esp_http_client_init failed");
        uart_send("ERROR: failed to init HTTP client\r\n");
        return ESP_FAIL;
    }

    esp_err_t err = esp_http_client_perform(client);
    esp_http_client_cleanup(client);
    handle_response(err, "GET");
    response_len = 0;
    return err;
}

// Public function to perform HTTP POST request
// @param url Full URL to send the POST request, example: "http://httpbin.org/post"
// @param body The body of the POST request, nullable, example: "body_param" or ""
// @return ESP_OK on success, or an error code on failure
esp_err_t http_post(const char *url, const char *body)
{
    response_len = 0;

    esp_http_client_config_t config = {
        .url = url,
        .event_handler = http_event_handler,
        .method = HTTP_METHOD_POST,
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);
    if (client == NULL) {
        ESP_LOGE(TAG, "POST: esp_http_client_init failed");
        uart_send("ERROR: failed to init HTTP client\r\n");
        return ESP_FAIL;
    }

    // Set POST body and headers
    // Body send json
    esp_http_client_set_post_field(client, body, strlen(body));
    esp_http_client_set_header(client, "Content-Type", "application/json");
    esp_http_client_set_header(client, "Accept", "application/json");

    esp_err_t err = esp_http_client_perform(client);
    esp_http_client_cleanup(client);
    handle_response(err, "POST");
    response_len = 0;
    return err;
}
