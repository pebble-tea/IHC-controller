#define F_CPU 16000000UL

#include <avr/io.h>
#include <avr/interrupt.h>

#define us2val(us) (us*16)

#define t_4100  0                     // 4100us before Overflow, 65536 and overlflow is 4.1ms, 65536-65536=0
#define t_300   0xFFFF - us2val(300)  // 300us before Overflow (65536-(300*16)) (could be 300us after t_600)
#define t_600   0xFFFF - us2val(600)  // 600us before Overflow (65536-(600*16))
#define t_150   0xFFFF - us2val(450)  // 150us after t_600, 450us before Overflow (65536-(450*16))

// -----------------------------------------------------------------------------
// MAIN
// -----------------------------------------------------------------------------

void TIMER1_init(void);

ISR(TIMER1_COMPA_vect);
ISR(TIMER1_COMPB_vect);
ISR(TIMER1_OVF_vect);

int main()
{
  TIMER1_init();
  return 0;
}

// -----------------------------------------------------------------------------
// Interrupt Service Routines (ISR)
// -----------------------------------------------------------------------------

ISR(TIMER1_COMPA_vect)
{
}

ISR(TIMER1_COMPB_vect)
{
}

ISR(TIMER1_OVF_vect)
{
}

// -----------------------------------------------------------------------------
// TIMER1
// -----------------------------------------------------------------------------

// Initalize TIMER1 in CTC mode then set compare and overflow registers
void TIMER1_init()
{
  cli(); // clear interrupts

  // reset TIMER1 counters
  TCCR1A  = 0;  // timer1
  TCCR1B  = 0;  // timer1
  TCNT1   = 0;  // ovf offset

  // enable TIMER1 in CTC mode without prescaler (prescaler=1)
  TCCR1B  |= (1<<WGM12) | (1<<WGM13) | (1<<CS10);

  // define COMPARE registers
  OCR1A = t_150;
  OCR1B = t_300;

  // enable COMPARE and OVERFLOW registers
  TIMSK1 |= (1<<TOIE1) | (1<<OCIE1B) | (1<<OCIE1A);

  sei(); // enable interrupts
}
