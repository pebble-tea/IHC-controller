#define F_CPU 16000000UL
#define BAUDRATE 9600

#define COMPUTE_BAUDRATE(baudrate) (F_CPU/16/baudrate)
#define COMPUTE_MICROSEC(us) (us*16)

#define SET_OVERFLOW(ovf) (TCNT1=ovf)

#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#define T_4100  0
// steps are all defined relative to full step,
#define T_600   (0xFFFF - COMPUTE_MICROSEC(600)) // full step offset (600us)
#define T_300   (0xFFFF - COMPUTE_MICROSEC(300)) // middle step offset (300us)
#define T_450   (0xFFFF - COMPUTE_MICROSEC(450)) // quarter step offset (150us)

// #region Declarations ~

// -----------------------------------------------------------------------------
// Serial UART I/O
// -----------------------------------------------------------------------------
void      uart_init(uint16_t);
// TX (OUTPUT)
void      uart_putc(uint8_t);
void      uart_print(char*);
// RX (INPUT)
uint8_t   uart_getc();
void      uart_readLine(char*, uint8_t);

// -----------------------------------------------------------------------------
// Timer1
// -----------------------------------------------------------------------------
void timer1_init();

// #endregion

int main()
{
    timer1_init(); // Initialize Timer (TIMER1)
    uart_init(COMPUTE_BAUDRATE(9600)); // Initialize Serial (UART)
    uart_print("Connected\n\r");
    return 0;
}

// #region Interrupt Service Routines  ~

ISR(TIMER1_OVF_vect)
{
}

ISR(TIMER1_COMPA_vect)
{
}

ISR(TIMER1_COMPB_vect)
{
}

// #endregion
// #region Serial Communication (UART) ~

void uart_init(uint16_t ubrr)
{
    UBRR0L = (ubrr & 0xFF); // config UBRR0L with 8 lower bits of ubrr
    UBRR0H = (ubrr << 8);   // config UBRROH with 8 higher bits of ubrr

    UCSR0C = _BV(UCSZ01) | _BV(UCSZ00); // Configure data frame size to 8-bits.
    UCSR0B = _BV(RXEN0) | _BV(TXEN0) ;  // Enable RX/TX
}

void uart_putc(uint8_t c)
{
    loop_until_bit_is_set(UCSR0A, UDRE0); // check if available
    UDR0 = c; // define char
}

void uart_print(char *data)
{
    while(*data>0){ uart_putc(*data++); } // loop until NULL terminated
}

uint8_t uart_getc()
{
    loop_until_bit_is_set(UCSR0A, RXC0); /* Wait until data exists. */
    return UDR0;
}

void uart_readLine(char* buffer, uint8_t size)
{
    uint8_t index = 0;
    char c;
    // Loop getchar while not the end or not ended (\r)
    do
    {
        c = uart_getc();      // get char
        buffer[index++] = c;  // Add char to buffer
    }
    while((index < size) && (c != '\r'));
    buffer[index] = 0;  // ensure NULL terminated string
}

// #endregion
// #region AVR Timer/Counter (Timer1)  ~

void timer1_init()
{
  cli(); // clear interrupts

  TCNT1 = 0;  // reset overflow register
  TCCR1A = 0; // reset TCCR1A register

  // enable Timer/Counter in CtC mode without prescaler (prescale=1)
  TCCR1B |= _BV(WGM12) | _BV(WGM13) | _BV(CS10);

  OCR1A = T_450; // quarter step compare (COMPA)
  OCR1B = T_300; // middle step compare (COMPB)

  // enable Timer Compare and Overflow
  TIMSK1 |= _BV(TOIE1) | _BV(OCIE1B) | _BV(OCIE1A);

  sei(); // enable interrupts
}

// #endregion
