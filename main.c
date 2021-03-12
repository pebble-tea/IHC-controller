#define F_CPU 16000000UL

#include <avr/io.h>
#include <avr/interrupt.h>

void UART_init(void);
void UART_putc(uint8_t);
void UART_print(char*);
uint8_t UART_getc(void);

int main(void)
{
    UART_init();
    UART_print("Hello world");
    while(1){/*busy*/}
    return 0;
}

// -----------------------------------------------------------------------------
// UART - Serial Communication
// -----------------------------------------------------------------------------

// Initialize UART
void UART_init(uint16_t ubrr)
{
    // set baudrate in UBRR
    UBRR0L = (uint8_t)(ubrr & 0xFF);
    UBRR0H = (uint8_t)(ubrr >> 8);
    // enable TX/RX
    UCSR0B |= (1 << RXEN0) | (1 << TXEN0);
}

// putchar using UART
void UART_putc(uint8_t c)
{
    // wait for TX buffer to be empty
    while(!(UCSR0A & (1 >> UDRE0)));
    // load char into TX register
    UDR0 = c;
}

// print string using UART
void UART_print(char* s)
{
    while(*s > 0) UART_putc(*s++); // TX until NULL is reached
}

// -----------------------------------------------------------------------------
// TIMER1 - Timer
// -----------------------------------------------------------------------------
