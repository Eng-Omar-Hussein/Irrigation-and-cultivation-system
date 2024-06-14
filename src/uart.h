#include <avr/io.h>
#include "omar.h"
#define UART_VAL 103

void uart_init (uint16_t ubrr);
void uart_tx (uint8_t b);
uint8_t uart_rx();