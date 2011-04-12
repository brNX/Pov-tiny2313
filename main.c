/*
 * main.c
 *
 *  Created on: Jan 14, 2011
 *      Author: bgouveia
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

volatile unsigned int counter = 0;

#define THRESHOLD 30

//uint8_t currentRPS=0;

//uint8_t A[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,255,17,49,81,142,0,255,17,17,17,14,0,134,137,137,137,113,};
//uint8_t A[] = {255,0,127,0,63,0,31,0,15,0,7,0,3,0,1,0,0,128,0,192,0,224,0,240,0,248,0,252,0,254,0,255,};
uint8_t A[] = {126,66,66,66,0,126,18,50,78,0,126,66,66,126,0,126,18,18,30,0,0,30,184,125,54,60,60,60,54,125,184,30,0};
uint8_t B[] = {126,66,66,66,0,126,18,50,78,0,126,66,66,126,0,126,18,18,30,0,0,120,184,125,54,60,60,60,54,125,184,120,0};

#define  VD_PIN  4
#define  VD_THRESHOLD  750
#define VIEWFIELD  30  //em percentagem
#define POV_WIDTH  32
#define DELAYCONSTANT VIEWFIELD*10/POV_WIDTH
#define START_ESTIMATE 1000U

unsigned int start_estimate_low = 0;
unsigned int start_estimate_high = START_ESTIMATE;

unsigned int _niterations = 0;
unsigned int _iterationpair = 0;
uint8_t *_dance;
unsigned int _dancesteps;
int _dancecurstep;

volatile unsigned int per_dance_delay=START_ESTIMATE*DELAYCONSTANT;

void resetDance() {
  _dancecurstep = _dancesteps;
  _niterations = _niterations+1;

  if (_niterations >= 8) {

     _iterationpair = (_iterationpair+1)%2;
      if (_iterationpair % 2 == 0){

    	  _dance = B;
    	  _dancesteps=sizeof(B)/sizeof(uint8_t);
      }
      else{
    	  _dance = A;
    	  _dancesteps=sizeof(A)/sizeof(uint8_t);

      }

      _niterations = 0;
  }
}

void tick ()
{
  start_estimate_high = (start_estimate_high + counter)>>1;
  per_dance_delay = start_estimate_high*DELAYCONSTANT;
  resetDance();
}

void danceStep () {
  if (_dancecurstep-1 < 0){
	  PORTB = 0;
	  return;
  }
  _dancecurstep--;

  PORTB = _dance[_dancecurstep] ;
  _delay_loop_2(per_dance_delay>>2);
}

static void initTimers(){
	//enable interrupt TimerCounter0 compare match A and TimerCounter1 compare match A
	TIMSK = _BV(OCIE0A) | _BV(OCIE1A);

	//setting CTC
	TCCR0A = _BV(WGM01);

	//Timer0 Settings: Timer Prescaler /8,
	TCCR0B = _BV(CS01);

	//top of the counters
	OCR0A=0x7C;

	//interrupt every 1ms timer0 and every 1 us timer1
	sei();//enable interrupts
}

//timer interrupt 0
ISR(TIMER0_COMPA_vect)
{
    counter++;
}

void main(void) __attribute__((noreturn));

void main(void){

   /*leds  PB0-PB7*/
    DDRB = 0xFF;

    /*pin PD3 input*/
    DDRD &= ~_BV(PD3);

    initTimers();

    _dance=A;
    _dancesteps=sizeof(A)/sizeof(uint8_t);

    resetDance();

    for(;;)
    {
    	if ((PIND & _BV(PD3))==0){
    	  if (counter > THRESHOLD){
    		cli();
    	    tick();
            counter=0;
            sei();
    	  }
        }
    	danceStep();
    }
}
