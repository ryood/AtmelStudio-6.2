/*
 * Adjust_POT_Test.c
 *
 * Created: 2015/04/18 17:33:25
 *  Author: gizmo
 *
 *
 */

//	ATmega328P

/*
ポート対応	[ATmega328P]	x:存在しない(PC6はRESET) -:空き
bit		7	6	5	4	3	2	1	0
portB	-	-	-	-	-	-	-	CKO		PB0:システムクロック出力
portC	x	x	SCL	SDA	-	-	Fm	FM		I2C(PC5:クロック PC4:データ) ADC(PC1:周波数adj PC0:周波数)
portD	-	-	-	-	-	RST	-	-		LCD

【ヒューズビット】 16MHz外部クリスタル
Low: B7 High: D9 Ext: 07

ビルド環境
AtmelStudio 6.2		最適化オプション:-O1	数学ライブラリを追加:libm.a
ATmega328P	16MHz（外部クリスタル）
*/ 


#include <avr/io.h>
#include <avr/sfr_defs.h>

#define sbi(BYTE,BIT) (BYTE|=_BV(BIT)) // BYTEの指定BITに1をセット
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
 * 波形生成割込み
 *==============================================*/

//******************************************************************
// timer0 setup
// set prescaler to 1, 16,000,000Hz / 16 clock
//

//******************************************************************
// Timer2 Interrupt Service at 16,000,000Hz = 1uSec
//
ISR(TIMER0_OVF_vect)

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
	int adjust = 10;
	
	DDRD = 0x01;
	
	adc_init();
	
    while(1)
    {
		//interval = adc_convert(0);
		//adjust = adc_convert(1);
		
		PORTD ^= 0x01;
		//wait_ms(interval/10);
		wait_us(adjust);
    }
}
