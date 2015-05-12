/*
 * Debouncer_Test.c
 *
 * Created: 2015/05/12 3:23:58
 *  Author: gizmo
 */ 

#define F_CPU	16000000UL

#include <avr/io.h>
#include <util/delay.h>

// Rotary Encoder
//
#define RE_DIR	DDRD
#define RE_PORT	PORTD
#define RE_PIN	PIND
#define RE_A	0
#define RE_B	1
#define RE_SW	2

// GND Switch
//
#define GNDSW_DIR	DDRB
#define GNDSW_PORT	PORTB
#define GNDSW_GND	3
#define GNDSW_VGND	4

#define RE_SW_PRESS	(~(RE_PIN)&_BV(RE_SW))

int8_t readRE_SW(void)
{
	if (RE_SW_PRESS) {
		_delay_ms(10);
		while(RE_SW_PRESS)
			;
		_delay_ms(10);
		return 1;
	}
	else return 0;
}

int main(void)
{
	int8_t RE_SWval = 0;
		
	// Rotary Encoder
	//
	// PullUp
	RE_PORT |= _BV(RE_A) | _BV(RE_B) | _BV(RE_SW);
		
	// GND Level Switch
	//
	GNDSW_DIR |= _BV(GNDSW_GND) | _BV(GNDSW_VGND);
	GNDSW_PORT |= _BV(GNDSW_VGND);
		
    while(1)
    {
		// GND Level
		//
		if (readRE_SW()) {
			if (RE_SWval) {
				RE_SWval = 0;
				GNDSW_PORT |= _BV(GNDSW_GND);
				GNDSW_PORT &= ~_BV(GNDSW_VGND);
			}
			else {
				RE_SWval = 1;
				GNDSW_PORT |= _BV(GNDSW_VGND);
				GNDSW_PORT &= ~_BV(GNDSW_GND);
			}
		}         
    }
}