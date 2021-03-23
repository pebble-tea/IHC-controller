#define F_CPU 16000000UL
#define SERIAL_RX_TIMEOUT 100

#define COMPUTE_BAUDRATE(baudrate) (F_CPU/16/baudrate)
#define COMPUTE_MICROSEC(us) (us*16)

#define SET(PIN,N) (PIN |=  (1<<N))
#define CLR(PIN,N) (PIN &= ~(1<<N))
#define MOD(PIN, N, V) (PIN = ((PIN & ~(1<<N)) | (V << N)))
#define CHECK(PIN, N) (PIN & (1 << N))

#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>

// steps are all relative to full step,
#define T_4100  0                                // no offset
#define T_600   (0xFFFF - COMPUTE_MICROSEC(600)) // full step offset (600us)
#define T_300   (0xFFFF - COMPUTE_MICROSEC(300)) // middle step offset (300us)
#define T_450   (0xFFFF - COMPUTE_MICROSEC(450)) // quarter step offset (150us)

#define SET_OVERFLOW(ovf) (TCNT1=ovf)

enum STATE { START_STATE, DATA_STATE, END_STATE } FRAME_STATE; // frame state machine

#define OUTPUT_SIZE 8

uint8_t  OFFSET = 0;
uint16_t outputs[OUTPUT_SIZE] = {0};

// #region Declarations ~

void writeData(uint16_t*, uint8_t);
void writeParity(uint16_t*);

// -----------------------------------------------------------------------------
// Atmega328p I/O
// -----------------------------------------------------------------------------
void  init_io();
void  io_writeAll(uint8_t);
void  io_write(uint8_t, uint8_t);
// -----------------------------------------------------------------------------
// Serial UART I/O
// -----------------------------------------------------------------------------
void      uart_init(uint16_t);
// TX (OUTPUT)
void      uart_putc(uint8_t);
void      uart_print(char*);
// RX (INPUT)
uint8_t   uart_getc();
uint8_t   uart_getc_timeout(uint8_t*, uint16_t);
void      uart_readLine(char*, uint8_t);

// -----------------------------------------------------------------------------
// Timer1
// -----------------------------------------------------------------------------
void timer1_init();

// -----------------------------------------------------------------------------
// I/O
// -----------------------------------------------------------------------------
uint8_t get_parity(uint16_t);

// #endregion

int main()
{
    timer1_init(); // Initialize Timer (TIMER1)
    uart_init(COMPUTE_BAUDRATE(9600)); // Initialize Serial (UART)
    uart_print("Connected\n\r");

    // init I/O pins and write HIGH to all
    init_io();
    io_writeAll(1);

    outputs[5] = 0b00011111;
    outputs[6] = 0b00010110;

    FRAME_STATE=START_STATE; // start state machine

    while(1){} // infinite loop busy
    return 0;
}

// write data for each array element with a bit offset
void writeData(uint16_t* array, uint8_t offset)
{
  for(int i=0;i<OUTPUT_SIZE;i++)
  {
      uint16_t val = array[i];
      io_write(i+2, (((val & (1<<offset))>>offset)==0)); // OUTPUT 1 if we have a zero
  }
}

// write parity for each array element
void writeParity(uint16_t* array)
{
  for(int i=0;i<OUTPUT_SIZE;i++)
  {
      uint16_t val = array[i];
      io_write(i+2, (get_parity(val)==0));  // OUTPUT 1 if we have a zero
  }
}

// #region Atmega328p I/O ~

void init_io()
{
    DDRD = 0b11111100; // all pins are OUTPUTS except from PD0 & PD1
    DDRB = 0b00000011; // only PB0 & PB1 are OUTPUTS
}

void  io_writeAll(uint8_t value)
{
  for(int i=2;i<OUTPUT_SIZE+2;i++)
  {
    io_write(i, value);
  }
}

void io_write(uint8_t pin, uint8_t value)
{
    if(pin<OUTPUT_SIZE)
    {
        MOD(PORTD, pin, value);
    }
    else
    {
        MOD(PORTB,(pin - OUTPUT_SIZE), value);
    }
}

// #endregion
// #region Interrupt Service Routines  ~

ISR(TIMER1_OVF_vect)
{
    switch(FRAME_STATE)
    {
        case START_STATE:
          // PB0 == LOW and OVF=300us
          // STATE is now DATA
          io_writeAll(0);
          SET_OVERFLOW(T_300);
          FRAME_STATE=DATA_STATE;
        break;
        case DATA_STATE:
          // PB0 == HIGH and OVF=600us
          // STATE is still DATA
          io_writeAll(1);
          SET_OVERFLOW(T_600);
        break;
        case END_STATE:
          // PB0 == HIGH and OVF=0
          // STATE is now returning to START
          io_writeAll(1);
          SET_OVERFLOW(T_4100);

          //TODO : Serial READ

          uint8_t timeout = 0;
          uint8_t index = uart_getc_timeout(&timeout, SERIAL_RX_TIMEOUT);
          uint16_t value = uart_getc_timeout(&timeout, SERIAL_RX_TIMEOUT);

          if(timeout==0) { outputs[index]=value; }

          FRAME_STATE=START_STATE;
        break;
    }
}

ISR(TIMER1_COMPA_vect)
{
    if(FRAME_STATE==DATA_STATE)
    {
        if(OFFSET<16)
        {
            // TODO : write data
            writeData(outputs, OFFSET++);
        }else
        {
           // TODO : write parity
           FRAME_STATE=END_STATE;
           writeParity(outputs);
           OFFSET = 0;
        }
    }
}

ISR(TIMER1_COMPB_vect)
{
    if(FRAME_STATE==DATA_STATE || FRAME_STATE==END_STATE)
    {
      io_writeAll(0);
      //CLR(PORTB, PB0);
    }
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

uint8_t uart_getc_timeout(uint8_t *timeout, uint16_t ms)
{
    uint16_t cycle = COMPUTE_MICROSEC(ms);
    do { cycle--; } while (bit_is_clear(UCSR0A, RXC0) && cycle>0);
    *timeout = (cycle<=0);
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
// #region Utils ~

uint8_t get_parity(uint16_t n)
{
    uint8_t parity = 0;
    while (n)
    {
        parity = !parity;
        n = n & (n - 1);
    }
    return parity;
}

// #endregion
