/*
 *	I2C LCD controll sample
 *	Copyright (c) 2009 Strawberry Linux Co.,Ltd.
 *	2009.5.19
 *
 *	���̃v���O�����𗘗p���邱�ƂŊȒP��I2C�t������舵�����Ƃ��ł��܂��B
 *	�Ȃ����̃v���O������I2C�t�����W���[���̕]����ړI�Ƃ��Ă��܂��B
 *	���i�E���@�ł��g�����������ꍇ�̓^�C�~���O�E�}�[�W�����\�����m�F�̏ゲ���p���������B
 *	���̃v���O�����������ŕs����N���Ă����Ђł͈�ؐӔC�𕉂��܂���B
 *	avr-gcc 3.4.3�œ���m�F
 *
 *	AVR�̃n�[�h�E�F�AI2C���W���[�����g�p���Ă��܂��B
 *	ATMEGA168�p �����N���b�N8MHz�œ��삵�܂��i�W�����͂n�e�e�j
 *	I2C�N���b�N�͗]�T���݂�100kHz�Ƃ��Ă��܂��B
 *
 *	�s���A�T�C��
 *		PC3....RST(���Z�b�g�s���j���ȗ��\
 *		PC4....SDA
 *		PC5....SCL
 *
 */


#include <avr/io.h>
#include <avr/boot.h>
#include <avr/interrupt.h>
#include <avr/signal.h>
#include <compat/twi.h>


// ���̊֐��Ńʕb�̃E�F�C�g�����܂��B���� 100 = 100�ʕb�E�F�C�g
// �N���b�N�X�s�[�h�ɂ�蒲�����Ă��������B
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



//////////////////////////////////////////////////////////////////////////////

void i2c_init()
{
	// TWBR = {(CLOCK(8MHz) / I2C_CLK) - 16} / 2;
	// I2C_CLK = 100kHz, CLOCK = 8MHz, TWBR = 32
	// I2C_CLK = 100kHz, CLOCK = 20MHz, TWBR = 92
	TWBR = 72;
	TWSR = 0;
}



unsigned char wait_stat()
{
	while(!(TWCR & _BV(TWINT)));
	
	return TW_STATUS;
}


void i2c_stop()
{
	
	TWCR = _BV(TWINT) | _BV(TWSTO) | _BV(TWEN);
	while(TWCR & _BV(TWSTO));
}



unsigned char i2c_start(unsigned char addr, unsigned char eeaddr)
{
i2c_restart:
i2c_start_retry:
	TWCR = _BV(TWINT) | _BV(TWSTA) | _BV(TWEN);
	switch(wait_stat()){
		case TW_REP_START:
		case TW_START:
			break;
		case TW_MT_ARB_LOST:
			goto i2c_start_retry;
		default:
			return 0;
	}
	TWDR = addr | TW_WRITE;
	TWCR = _BV(TWINT) | _BV(TWEN);
	switch(wait_stat()){
		case TW_MT_SLA_ACK:
			break;
		case TW_MT_SLA_NACK:
			goto i2c_restart;
		case TW_MT_ARB_LOST:
			goto i2c_start_retry;
		default:
			return 0;
	}
	TWDR = eeaddr;
	TWCR = _BV(TWINT) | _BV(TWEN);
	switch(wait_stat()){
		case TW_MT_DATA_ACK:
			break;
		case TW_MT_DATA_NACK:
			i2c_stop();
			return 0;
		case TW_MT_ARB_LOST:
			goto i2c_start_retry;
		default:
			return 0;
	}
	return 1;
}



unsigned char i2c_write(unsigned addr, unsigned char eeaddr, unsigned char dat)
{
	unsigned char rv=0;

restart:
begin:
	if(!i2c_start(addr, eeaddr))	goto quit;
	
	TWDR = dat;
	TWCR = _BV(TWINT) | _BV(TWEN);
	switch(wait_stat()){
		case TW_MT_DATA_ACK:
			break;
		case TW_MT_ARB_LOST:
			goto begin;
		case TW_MT_DATA_NACK:
		default:
			goto quit;
	}
	rv = 1;
quit:
	i2c_stop();
	wait_us(50);	// �P���߂��Ƃɗ]�T������50us�E�F�C�g���܂��B
	
	return rv;
}






// �R�}���h�𑗐M���܂��BHD44780�ł���RS=0�ɑ���
void i2c_cmd(unsigned char db)
{
	i2c_write(0b01111100, 0b00000000, db);
}

// �f�[�^�𑗐M���܂��BHD44780�ł���RS=1�ɑ���
void i2c_data(unsigned char db)
{
	i2c_write(0b01111100, 0b01000000, db);
}

// �i��Ɂj�������A�����M���܂��B
void i2c_puts(unsigned char *s)
{
	while(*s){
		i2c_data(*s++);
	}
}




static unsigned char icon_data[]={
	// �A�C�R���A�h���X, �Y���r�b�g
	0x00, 0b10000,
	0x02, 0b10000,
	0x04, 0b10000,
	0x06, 0b10000,
	
	0x07, 0b10000,
	0x07, 0b01000,
	0x09, 0b10000,
	0x0B, 0b10000,
	
	0x0D, 0b01000,
	0x0D, 0b00100,
	0x0D, 0b00010,
	0x0D, 0b10000,
	
	0x0F, 0b10000, // �A���e�i�}�[�N
};





/*
 *	�A�C�R���̕\�����[�`��
 *
 *	���ꂼ��̃A�C�R���ɊY������r�b�g�𗧂Ă�i�N���A�j���܂��B
 *	���݂̃A�C�R���̕\����Ԃ͓ǂݎ��Ȃ����߁A�}�C�R�����Ńf�[�^��
 *	�ҏW���Ă���܂Ƃ߂đ��M����K�v������܂��Bicon_buff[]
 */


// �������F�e�r�b�g�����ꂼ��̃A�C�R���ɑ������܂��Bbit0�`bit12�i�S�P�R�r�b�g�j
// bit0���E�[�̃A�C�R��, bit13�����̃A�C�R���i�A���e�i�j�ɊY�����܂��B
// puticon(0x1FFF)�őS�A�C�R���_��

void puticon(unsigned short flg)
{
	static unsigned char icon_buff[16];	// �A�C�R���̕ҏW�p
	unsigned char i;
	
	for(i=0;i<sizeof(icon_data)/2;i++){
		if(flg & (0x1000>>i)){	// �Y���r�b�g�������Ă�����
			icon_buff[icon_data[i*2]] |= icon_data[i*2+1];	// �o�b�t�@�𗧂Ă܂��B
		} else {
			icon_buff[icon_data[i*2]] &= ~icon_data[i*2+1];	// �o�b�t�@���N���A���܂��B
		}
	}
	// �ꊇ��LCD�ɏ������݂܂��B
	for(i=0;i<16;i++){
		i2c_cmd(0b00111001);	// �R�}���h
		i2c_cmd(0b01000000+i);	// �A�C�R���̈�̃A�h���X��ݒ�
		i2c_data(icon_buff[i]);	// �A�C�R���f�[�^
	}
}


void main(void)
{
	// �R���g���X�g�̐ݒ�
	unsigned char contrast = 0b111111;	// 3.0V�� ���l���グ��ƔZ���Ȃ�܂��B
										// 2.7V�ł�0b111000���炢�ɂ��Ă��������B�B
										// �R���g���X�g�͓d���d���C���x�ɂ�肩�Ȃ�ω����܂��B���ۂ̉t�����݂Ē������Ă��������B
	char i;
	
	DDRD = _BV(0);
	
	DDRC = _BV(3);			 // RST�s�����o�͂�
	//PORTC = _BV(4) | _BV(5); // SCL, SDA�����v���A�b�v��L��
	
	i2c_init();				 // AVR����I2C���W���[���̏�����
	wait_ms(500);
	
	PORTC &= ~_BV(3);		 // RST��L�ɂ��܂��B���Z�b�g
	wait_ms(1);
	PORTC |= _BV(3);		 // RST��H�ɂ��܂��B���Z�b�g����
	wait_ms(10);
	
	// ��������I2C LCD�̏��������s���܂��B
	wait_ms(40);
	i2c_cmd(0b00111000); // function set
	i2c_cmd(0b00111001); // function set
	i2c_cmd(0b00010100); // interval osc
	i2c_cmd(0b01110000 | (contrast & 0xF)); // contrast Low
	
	i2c_cmd(0b01011100 | ((contrast >> 4) & 0x3)); // contast High/icon/power
	i2c_cmd(0b01101100); // follower control
	wait_ms(300);

	i2c_cmd(0b00111000); // function set
	i2c_cmd(0b00001100); // Display On
	
	i2c_cmd(0b00000001); // Clear Display
	wait_ms(2);			 // Clear Display�͒ǉ��E�F�C�g���K�v
	
	
	// �P�s�ڂ̕\��
	i2c_puts("I2C LCD");
	
	// �Q�s�ڂɃJ�[�\�����ړ�
	i2c_cmd(0b11000000);	// ADDR=0x40
	
	// �J�i�̕\���i���̃\�[�X�v���O������SJIS�ŋL�q����Ă��Ȃ��Ƃ��܂��\������܂���j
	i2c_puts("SPLC792-I2C");
	
	
	// �A�C�R����_�ł�����
	while(1){
		//PORTD ^= 0b00000001;
		
		i2c_cmd(0b11000000);	// ADDR=0x40
		i2c_puts("Testing... ");
		wait_ms(1000);
		
		i2c_cmd(0b11000000);	// ADDR=0x40
		i2c_puts("SPLC792-I2C");
		wait_ms(1000);
		
		/*
		for(i=0;i<=12;i++){
			// �P�ӏ������r�b�g�𔽓]�����ăA�C�R�����\���ɂ��܂��B
			// �r�b�g�𗧂Ă�ƕ\���n�m�ɂȂ�܂��B
			puticon(0x1FFF ^ (1<<i));	// �P�r�b�g�������]
			wait_ms(500);
		}
		*/
		
	}
	while(1);

}

