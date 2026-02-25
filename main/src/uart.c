#include "driver/uart.h"
#include "esp_log.h"

#include "uart.h"
#include "http.h"

const uart_port_t UART_PORT = UART_NUM_2;
const uint32_t BAUD_RATE = 115200;
const uint8_t TX_PIN = 17; 
const uint8_t RX_PIN = 16; 
const uint16_t RX_BUFFER = 2048; // UART buffer sizes
const uint8_t DATA_BUFFER_SIZE = 254; // Max size for URL and body in HTTP commands

static const char *TAG = "UART";

// UART initialization function
void uart_init()
{
    uart_config_t uart_config = {
        .baud_rate = BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };

    ESP_ERROR_CHECK(uart_driver_install(UART_PORT, RX_BUFFER, 0, 0, NULL, 0));
    ESP_ERROR_CHECK(uart_param_config(UART_PORT, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(UART_PORT, TX_PIN, RX_PIN,
                                 UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));

    ESP_LOGI(TAG, "UART initialized successfully");

    const char *ready_msg = "UART is ready to receive data. Usage:\r\n"
                            "http GET <url>\r\n"
                            "http POST <url> <body>\r\n";
    uart_write_bytes(UART_PORT, ready_msg, strlen(ready_msg));
}


// Function to parse received UART data and execute corresponding HTTP commands
void parse_and_execute_command(char *input) {
    // Remove \r \n from the end of the input
    uint8_t len = strlen(input); 
    while(len > 0 && (input[len - 1] == '\n' || input[len - 1] == '\r')) {
        len--;
        input[len] = '\0';
    }
    ESP_LOGI(TAG, "Received input: %s", input);

    char *command = strtok(input, " ");
    if (command == NULL || strcmp(command, "http") != 0) {
        ESP_LOGW(TAG, "Received invalid command: %s", input);
        char *return_msg = "Invalid command\r\n";
        uart_write_bytes(UART_PORT, return_msg, strlen(return_msg));
        return;
    }

    char *method = strtok(NULL, " ");
    if(method == NULL) {
        ESP_LOGW(TAG, "HTTP method not specified");
        char *return_msg = "HTTP method not specified\r\n";
        uart_write_bytes(UART_PORT, return_msg, strlen(return_msg));
        return;
    }

    char *url = strtok(NULL, " ");
    if(url == NULL) {
        ESP_LOGW(TAG, "URL not specified");
        char *return_msg = "URL not specified\r\n";
        uart_write_bytes(UART_PORT, return_msg, strlen(return_msg));
        return;
    }

    if(strcmp(method, "GET") == 0) {
        ESP_LOGI(TAG, "Executing HTTP GET: %s", url);
        esp_err_t err = http_get(url);
        if(err != ESP_OK) {
            ESP_LOGI(TAG, "Failed HTTP GET: %s", url);
            char *return_msg = "HTTP GET failed\r\n";
            uart_write_bytes(UART_PORT, return_msg, strlen(return_msg));
        }
    } else if(strcmp(method, "POST") == 0) {
        char *body = strtok(NULL, "");
        if(body == NULL) body = "";
        ESP_LOGI(TAG, "Executing HTTP POST: %s with body: %s", url, body);
        
        if(http_post(url, body) != ESP_OK) {
            ESP_LOGW(TAG, "Failed HTTP POST: %s", url);
            char *return_msg = "HTTP POST failed\r\n";
            uart_write_bytes(UART_PORT, return_msg, strlen(return_msg));
            return;
        }

        char *return_msg = "HTTP POST executed\r\n";
        uart_write_bytes(UART_PORT, return_msg, strlen(return_msg));
    } else {
        ESP_LOGW(TAG, "Received unsupported HTTP method: %s", method);
        char *return_msg = "Unsupported HTTP method\r\n";
        uart_write_bytes(UART_PORT, return_msg, strlen(return_msg));
        return;
    }

}

// Function to send a string message over UART
void uart_send(const char *msg) {
    uart_write_bytes(UART_PORT, msg, strlen(msg));
}


// FreeRTOS task to continuously read data from UART and execute commands
void uart_read_data(void *arg) {
    ESP_LOGI(TAG, "task started, stack=%d", uxTaskGetStackHighWaterMark(NULL));
    uint8_t data[DATA_BUFFER_SIZE];

    while(1) {
        int length = uart_read_bytes(UART_PORT, data, DATA_BUFFER_SIZE, pdMS_TO_TICKS(100));
        if (length > 0) {
            data[length] = '\0';
            ESP_LOGI(TAG, "Received data: %s", data);

            parse_and_execute_command((char *)data);

            ESP_LOGI(TAG, "free stack after request: %d", uxTaskGetStackHighWaterMark(NULL));
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
