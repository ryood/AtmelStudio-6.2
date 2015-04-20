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
�|�[�g�Ή�	[ATmega328P]	x:���݂��Ȃ�(PC6��RESET) -:��
bit		7	6	5	4	3	2	1	0
portB	-	-	-	-	-	-	-	CKO		PB0:�V�X�e���N���b�N�o��
portC	x	x	SCL	SDA	-	-	Fm	FM		I2C(PC5:�N���b�N PC4:�f�[�^) ADC(PC1:���g��adj PC0:���g��)
portD	-	-	-	-	-	RST	-	-		LCD

�y�q���[�Y�r�b�g�z 16MHz�O���N���X�^��
Low: B7 High: D9 Ext: 07

�r���h��
AtmelStudio 6.2		�œK���I�v�V����:-O1	���w���C�u������ǉ�:libm.a
ATmega328P	16MHz�i�O���N���X�^���j
*/ 


#include <avr/io.h>
#include <avr/sfr_defs.h>

#define sbi(BYTE,BIT) (BYTE|=_BV(BIT)) // BYTE�̎w��BIT��1���Z�b�g
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
 * �g�`����������
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
