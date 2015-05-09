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

int readRE(void) 
{
	static uint8_t index;
	int retVal = 0;
	
	index = (index << 2) | (RE_PIN & _BV(RE_A)) | (RE_PIN & _BV(RE_B));
	index &= 0b1111;
	
	switch (index) {
	// 時計回り
	case 0b0001:	// 00 -> 01
	case 0b1110:	// 11 -> 10
		retVal = -1;
		break;
	// 反時計回り
	case 0b0010:	// 00 -> 10
	case 0b1101:	// 11 -> 01
		retVal = 1;
		break;
	}
	
	_delay_ms(5);	// (とりあえず)チャタリング防止
	
	return retVal;	 
}

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
		int sw;
		
		switch (readRE()) {
		case 1:
			LED_PORT = 0;
			_delay_ms(50);
			LED_PORT |= _BV(LED_GREEN);
			LED_PORT &= ~_BV(LED_RED);
			break;
		case -1:
			LED_PORT = 0;
			_delay_ms(50);
			LED_PORT |= _BV(LED_RED);
			LED_PORT &= ~_BV(LED_GREEN);
			break;
		}
		
		sw = RE_PIN & _BV(RE_SW);
		
		if (sw)
			LED_PORT |= _BV(LED_BLUE);
		else
			LED_PORT &= ~_BV(LED_BLUE);
		
		_delay_ms(5);
	}
}
