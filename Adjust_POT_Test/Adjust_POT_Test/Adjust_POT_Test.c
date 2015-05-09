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

#define sbi(BYTE,BIT) (BYTE|=_BV(BIT))  // BYTE�̎w��BIT��1���Z�b�g
#define cbi(BYTE,BIT) (BYTE&=~_BV(BIT)) // BYTE�̎w��BIT���N���A

static void wait_us(short t)
{
	while(t-->=0){
		asm volatile ("nop");
		asm volatile ("nop");
	}
}

// ���̊֐��Ń~���b�̃E�F�C�g�����܂��B���� 100 = 100�~���b�E�F�C�g
// �N���b�N�X�s�[�h�ɂ�蒲�����Ă��������B
static void wait_ms(short t)
{
	while(t-->=0){
		wait_us(1000);
	}
}

void adc_init(void)
{
	// ADC�g�p�s�����f�W�^�����͋֎~ PC0, PC1
	DIDR0 |= (1 << ADC0D) | (1 << ADC1D);
	
	//ADC�̊�d��(VREF)
	ADMUX =	(0 << REFS0);
	
	//ADC����ݒ�
	ADCSRA = 0b10000111		//bit2-0: 111 = 128����	16MHz/128=125kHz (50k�`200kHz�ł��邱��)
		| (1 << ADSC);		//1��ڕϊ��J�n(����)
}

/*==============================================
 * �����F�ϊ�����ADC�`�����l��
 * �Ԓl�FADC�̓ǂݎ��l(0..1023)
 *==============================================*/
int16_t adc_convert(int8_t channel)
{
	ADMUX = (0 << REFS0) | channel; // AD�ϊ��`�����l��
	
	//cbi(ADCSRA, ADIF);
	sbi(ADCSRA, ADSC); // �ϊ��J�n
	
	loop_until_bit_is_set(ADCSRA, ADIF); // �ϊ������܂ő҂�
	
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
