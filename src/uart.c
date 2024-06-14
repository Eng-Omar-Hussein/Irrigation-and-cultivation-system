#include "uart.h"
void uart_init (uint16_t ubrr){
    UCSRC &= ~(1<<URSEL);
    UBRRH = (uint8_t) (ubrr >> 8);
    UBRRL = (uint8_t) ubrr ;
    UCSRB = (1<<RXEN) | (1 << TXEN);
    UCSRC = (1<<URSEL) | (1<<UCSZ1) | (1 << UCSZ0);
}

void uart_tx(uint8_t d){
    while (!(UCSRA & (1<<UDRE)));
    UDR = d;
}

uint8_t uart_rx(){
    while (!(UCSRA & (1<<RXC)));
    return UDR;
}