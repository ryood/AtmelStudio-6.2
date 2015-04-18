/*
 * GccApplication1.c
 *
 * Created: 2015/04/13 2:15:13
 *  Author: gizmo
 */ 


#include <avr/io.h>
#include <util/delay.h>

int main(void)
{
	DDRD = 0b00000001;
	
    while(1)
    {
		PORTD ^= 0b00000001;
		_delay_ms(1000);    
	}
}