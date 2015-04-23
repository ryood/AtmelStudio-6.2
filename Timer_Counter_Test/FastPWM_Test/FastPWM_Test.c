/*
 * FastPWM_Test.c
 *
 * Created: 2015/04/21 12:52:41
 *  Author: gizmo
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>

#define F_CPU	16000000UL
#include <util/delay.h>

void timer1_init_FastPWM(uint16_t cycle, uint16_t duty)
{
	// Output Compare Register
	OCR1A = cycle;
	OCR1B = duty;

	// Initialize Counter
	TCNT1 = 0;
	
	// Phase and Frequency Correct PWM Mode, TOP = OCR1A
	TCCR1B |= (1 << WGM13) | (0 << WGM12);
	TCCR1A |= (0 << WGM11) | (1 << WGM10);

	
	// Compare Output OC1A, OC1B
	TCCR1A |= (0 << COM1A1) | (1 << COM1A0);
	TCCR1A |= (1 << COM1B1) | (0 << COM1B0);
	
	
	// Timer1 Start, Set Prescaler to 8
	TCCR1B |= (1 << CS11);
	
}

int main(void)
{
	DDRB  = 0xFF;  // PB1(OC1A) o—Í

	timer1_init_FastPWM(500, 375);
	sei();
	
    while(1)
    {
        //PORTB ^= 0x06;
        //_delay_ms(50);
    }
}
