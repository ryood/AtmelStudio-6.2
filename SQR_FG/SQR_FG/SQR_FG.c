/*
 * SQR_FG.c
 *
 * Created: 2015/05/07 15:41:32
 *  Author: gizmo
 */ 


#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>

#define F_CPU	16000000UL
#include <util/delay.h>

// PWM
//
#define PWM_DIR		DDRB
#define PWM_A_PORT	1
#define PWM_B_PORT	2

// Rotary Encoder
//
#define RE_DIR	DDRD
#define RE_PORT	PORTD
#define RE_PIN	PIND
#define RE_A	0
#define RE_B	1
#define RE_SW	2

const uint16_t cycle_table[] = {
	20000, 10000, 5000, 2000, 1000, 500, 200, 100, 50, 20, 10, 5, 2
};

#define CYCLE_TABLE_ELEMENTS	(sizeof(cycle_table)/sizeof(uint16_t))
#define INITIAL_FREQ	4

/*------------------------------------------------------------------------/
 * PWM
 *
 ------------------------------------------------------------------------*/
void timer1_set_cycle_duty(uint16_t cycle, uint16_t duty)
{
	// Output Compare Register
	OCR1A = cycle;
	OCR1B = duty;
}

void timer1_init_PWM(uint16_t cycle, uint16_t duty)
{
	timer1_set_cycle_duty(cycle, duty);
	
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

/*------------------------------------------------------------------------/
 * Rotary Encoder
 *
 ------------------------------------------------------------------------*/
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
		retVal = 1;
		break;
		// 反時計回り
		case 0b0010:	// 00 -> 10
		case 0b1101:	// 11 -> 01
		retVal = -1;
		break;
	}
	
	_delay_ms(5);	// (とりあえず)チャタリング防止
	
	return retVal;
}

/*------------------------------------------------------------------------/
 * main routine
 *
 ------------------------------------------------------------------------*/
int main(void)
{
	int freq = INITIAL_FREQ;
	int REval;
	int cycle;
	int duty;
	
	// PWM
	PWM_DIR = _BV(PWM_A_PORT) | _BV(PWM_B_PORT);
	
	// Rotary Encoder
	RE_DIR = 0;
	// PullUp
	RE_PORT = _BV(RE_A) | _BV(RE_B) | _BV(RE_SW);

	// Initialize PWM
	cycle = cycle_table[freq];
	duty = cycle / 2;
	timer1_init_PWM(cycle, duty);
	
	//sei();
	
    while(1)
    {		
		REval = readRE();
		if (REval != 0) {
			freq += REval;
			freq = freq < 0 ? 0 : (freq >= CYCLE_TABLE_ELEMENTS ? CYCLE_TABLE_ELEMENTS - 1 : freq);
			
			cycle = cycle_table[freq];
			duty = cycle / 2;
			
			timer1_set_cycle_duty(cycle, duty);
		}
    }
}