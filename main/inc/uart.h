#ifndef UART_H
#define UART_H

#include "driver/uart.h"

// UART port configuration
extern const uart_port_t UART_PORT;

// UART initialization function
void uart_init();

// Functiom to send a string message over UART
void uart_send(const char *msg);

// UART data reading function
void uart_read_data(void *arg);

#endif
