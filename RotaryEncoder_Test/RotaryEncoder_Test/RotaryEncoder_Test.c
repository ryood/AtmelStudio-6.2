/*
 * RotaryEncoder_Test.c
 *
 * Created: 2015/05/07 16:11:07
 *  Author: gizmo
 */ 


#include <avr/io.h>

#define F_CPU	8000000UL
#include <util/delay.h>

#define RE_DIR	DDRD
#define RE_PORT	PORTD
#define RE_PIN	PIND
#define RE_A	0
#define RE_B	1
#define RE_SW	2

#define LED_DIR		DDRB
#define LED_PORT	PORTB
#define LED_RED		1
#define LED_GREEN	2
#define LED_BLUE	6 

int main(void)
{
	// Rotary Encoder
	RE_DIR = 0;
	// PullUp
	RE_PORT = _BV(RE_A) | _BV(RE_B) | _BV(RE_SW);
	
	// LED
	LED_DIR = _BV(LED_RED) | _BV(LED_GREEN) | _BV(LED_BLUE);
		
    while(1)
    {
		int rotA, rotB, sw;

		rotA = RE_PIN & _BV(RE_A);
		rotB = RE_PIN & _BV(RE_B);
		sw = RE_PIN & _BV(RE_SW);
		
		if (rotA)
			LED_PORT |= _BV(LED_RED);
		else
			LED_PORT &= ~_BV(LED_RED);
			
		if (rotB)
			LED_PORT |= _BV(LED_GREEN);
		else
			LED_PORT &= ~_BV(LED_GREEN);
			
		if (sw)
			LED_PORT |= _BV(LED_BLUE);
		else
			LED_PORT &= ~_BV(LED_BLUE);
    }
}