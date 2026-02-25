#include "driver/uart.h"
#include "esp_log.h"

#include "parser.h"
#include "uart.h"
#include "http.h"

const uart_port_t UART_PORT = UART_NUM_2;
const uint32_t BAUD_RATE = 115200;
const uint8_t TX_PIN = 17; 
const uint8_t RX_PIN = 16; 
const uint16_t RX_BUFFER = 2048; // UART buffer sizes
const uint8_t DATA_BUFFER_SIZE = 255; // Max size for URL and body in HTTP commands, save last byte for null terminator

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

// Function to send a string message over UART
void uart_send(const char *msg) {
    uart_write_bytes(UART_PORT, msg, strlen(msg));
}

// Function to parse received UART data and execute corresponding HTTP commands
void parse_and_execute_command(char *input)
{
    strip_line_ending(input);

    if (strlen(input) == 0) return;

    ESP_LOGI(TAG, "rx: \"%s\"", input);

    parsed_cmd_t cmd;
    cmd_parse_result_t result = parse_command(input, &cmd);

    switch (result) {
        case CMD_ERR_UNKNOWN:
            ESP_LOGW(TAG, "parse: unknown command");
            uart_send("ERROR: unknown command\r\n");
            return;

        case CMD_ERR_NO_METHOD:
            ESP_LOGW(TAG, "parse: missing method");
            uart_send("ERROR: missing method\r\n");
            return;

        case CMD_ERR_NO_URL:
            ESP_LOGW(TAG, "parse: missing url");
            uart_send("ERROR: missing url\r\n");
            return;

        case CMD_ERR_BAD_METHOD:
            ESP_LOGW(TAG, "parse: unsupported method");
            uart_send("ERROR: unsupported method, use GET or POST\r\n");
            return;

        case CMD_OK:
            break; 
    }

    if (cmd.method == HTTP_GET) {
        ESP_LOGI(TAG, "exec: GET %s", cmd.url);
        http_get(cmd.url);

    } else if (cmd.method == HTTP_POST) {
        const char *body = cmd.body ? cmd.body : "";
        ESP_LOGI(TAG, "exec: POST %s body=\"%s\"", cmd.url, body);
        http_post(cmd.url, body);
    }
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
