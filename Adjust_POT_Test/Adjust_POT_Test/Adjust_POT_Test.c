/*
 * Adjust_POT_Test.c
 *
 * Created: 2015/04/18 17:33:25
 *  Author: gizmo
 *
 *
 */

//	ATmega328P

#include <avr/io.h>
#include <avr/sfr_defs.h>

#define sbi(BYTE,BIT) (BYTE|=_BV(BIT))  // BYTEの指定BITに1をセット
#define cbi(BYTE,BIT) (BYTE&=~_BV(BIT)) // BYTEの指定BITをクリア

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

void adc_init(void)
{
	// ADC使用ピンをデジタル入力禁止 PC0, PC1
	DIDR0 |= (1 << ADC0D) | (1 << ADC1D);
	
	//ADCの基準電圧(VREF)
	ADMUX =	(0 << REFS0);
	
	//ADC動作設定
	ADCSRA = 0b10000111		//bit2-0: 111 = 128分周	16MHz/128=125kHz (50k～200kHzであること)
		| (1 << ADSC);		//1回目変換開始(調整)
}

/*==============================================
 * 引数：変換するADCチャンネル
 * 返値：ADCの読み取り値(0..1023)
 *==============================================*/
int16_t adc_convert(int8_t channel)
{
	ADMUX = (0 << REFS0) | channel; // AD変換チャンネル
	
	//cbi(ADCSRA, ADIF);
	sbi(ADCSRA, ADSC); // 変換開始
	
	loop_until_bit_is_set(ADCSRA, ADIF); // 変換完了まで待つ
	
	return ADC;
}


int main(void)
{
	int interval = 10;
		
	DDRD = 0x01;
	
	adc_init();
	
    while(1)
    {
		interval = adc_convert(0);
		
		PORTD ^= 0x01;
		wait_ms(interval);
    }
}
