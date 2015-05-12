/*
 * MOS_FET_Switching_Test.c
 *
 * Created: 2015/05/11 22:52:38
 *  Author: gizmo
 */ 


#include <avr/io.h>

#define F_CPU	16000000UL
#include <util/delay.h>

int main(void)
{
	DDRB  = 0b00111000;
	PORTB = 0b00110000;
	
    while(1)
    {
		PORTB = 0b00101000;
		_delay_ms(100);
		PORTB = 0b00110000;
		_delay_ms(100);
    }
}
