#define F_CPU 16000000UL
#define LED_PIN 0

#include <avr/io.h>
#include <util/delay.h>


//added comments !

int main()
{
  DDRB ^= (1 << LED_PIN); // set pin output
  while(1)
  {
    PORTB ^= (1<<LED_PIN);  // Toggle pin
    _delay_ms((500));       // Delay 500ms
  }

  return 0;
}
