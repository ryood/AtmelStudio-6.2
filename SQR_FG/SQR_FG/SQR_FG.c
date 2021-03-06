/*
 * SQR_FG.c
 *
 * Created: 2015/05/07 15:41:32
 *  Author: gizmo
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sfr_defs.h>
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

// Potentiometer
//
#define POT_DUTY	0

const uint16_t cycle_table[] = {
	20000, 10000, 5000, 2000, 1000, 500, 200, 100, 50, 20, 10, 5, 2
};

#define CYCLE_TABLE_ELEMENTS	(sizeof(cycle_table)/sizeof(uint16_t))
#define INITIAL_FREQ	(4)
#define INITIAL_DUTY	(64)

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
int8_t readRE(void)
{
	static uint8_t index;
	int8_t retVal = 0;
	
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
 * Potentiometer
 *
 ------------------------------------------------------------------------*/
void adc_init(void)
{
	// ADC使用ピンをデジタル入力禁止 PC0, PC1
	DIDR0 |= _BV(ADC0D) | _BV(ADC1D);
	
	// ADCの基準電圧(AVCC)
	ADMUX =	_BV(REFS0);
	
	// ADC動作設定
	ADCSRA = 0b10000111		// bit2-0: 111 = 128分周	16MHz/128=125kHz (50k〜200kHzであること)
		| _BV(ADSC);		// 1回目変換開始(調整)
}

uint8_t adc_convert8(uint8_t channel)
{
	ADMUX = _BV(REFS0)		// ADCの基準電圧(AVCC)
		| _BV(ADLAR)		// 左詰め
		| channel;			// AD変換チャンネル
	
	ADCSRA |= _BV(ADSC);	// 変換開始
	
	loop_until_bit_is_set(ADCSRA, ADIF); // 変換完了まで待つ
	
	return ADCH;
}

// 7bit 0..127
uint8_t readDuty(void)
{
	// 8bit -> 7bit
	return adc_convert8(POT_DUTY) >> 1;
}

/*------------------------------------------------------------------------/
 * main routine
 *
 ------------------------------------------------------------------------*/
#if 0
static void wait_us(short t)
{
	while(t-->=0){
		asm volatile ("nop");
		asm volatile ("nop");
	}
}

// この関数でミリ秒のウェイトを入れます。引数 100 = 100ミリ秒ウェイト
// クロックスピードにより調整してください。
static void wait_ms(short t)
{
	while(t-->=0){
		wait_us(1000);
	}
}
#endif

int main(void)
{
	uint8_t freq = INITIAL_FREQ;
	int8_t REval;
	uint16_t cycle;
	uint8_t duty;
	uint8_t old_duty = INITIAL_DUTY; 
	uint16_t duty_val;

	// PWM
	//
	PWM_DIR = _BV(PWM_A_PORT) | _BV(PWM_B_PORT);
	
	// Rotary Encoder
	//
	RE_DIR = 0;
	// PullUp
	RE_PORT = _BV(RE_A) | _BV(RE_B) | _BV(RE_SW);
	
	// Initialize ADC
	//
	adc_init();
	
	// Initialize PWM
	//
	cycle = cycle_table[freq];
	timer1_init_PWM(cycle,  cycle / 2);
	
	//sei();
	
    while(1)
    {
		duty = readDuty();		
		REval = readRE();
		
		if (REval != 0 || duty != old_duty) {
			old_duty = duty;
			
			freq += REval;
			freq = freq < 0 ? 0 : (freq >= CYCLE_TABLE_ELEMENTS ? CYCLE_TABLE_ELEMENTS - 1 : freq);
			
			cycle = cycle_table[freq];
			duty_val = ((uint32_t)cycle * duty >> 7);
			if (duty_val == 0)
				duty_val = 1;
			
			timer1_set_cycle_duty(cycle, duty_val);
		}
    }
}