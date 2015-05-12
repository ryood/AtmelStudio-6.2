/*
 * SPLC792_I2C_Test.c
 *
 * Created: 2015/05/12 7:04:04
 *  Author: gizmo
 */ 

#define F_CPU	16000000UL

#include <avr/io.h>
#include <util/delay.h>
#include "SPLC792-I2C.h"

int main(void)
{
	I2C_LCD_init();
	
	I2C_LCD_puts("SPLC792-I2C");
	I2C_LCD_setpos(1, 1);
	I2C_LCD_puts("Testing...");
	
    while(1)
    {
        I2C_LCD_setpos(1, 1);
		_delay_ms(1000);
		I2C_LCD_puts("Blinking...");
		_delay_ms(1000);
		I2C_LCD_setpos(1, 1);
		I2C_LCD_puts("Testing...");
    }
}