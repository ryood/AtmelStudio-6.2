/*
 *	I2C LCD controll sample
 *	Copyright (c) 2009 Strawberry Linux Co.,Ltd.
 *	2009.5.19
 *
 *	このプログラムを利用することで簡単にI2C液晶を取り扱うことができます。
 *	なおこのプログラムはI2C液晶モジュールの評価を目的としています。
 *	製品・実機でお使いいただく場合はタイミング・マージンを十分ご確認の上ご利用ください。
 *	このプログラムが原因で不具合が起きても弊社では一切責任を負いません。
 *	avr-gcc 3.4.3で動作確認
 *
 *	AVRのハードウェアI2Cモジュールを使用しています。
 *	ATMEGA168用 内蔵クロック8MHzで動作します（８分周はＯＦＦ）
 *	I2Cクロックは余裕をみて100kHzとしています。
 *
 *	ピンアサイン
 *		PC3....RST(リセットピン）※省略可能
 *		PC4....SDA
 *		PC5....SCL
 *
 */


#include <avr/io.h>
#include <avr/boot.h>
#include <avr/interrupt.h>
#include <avr/signal.h>
#include <compat/twi.h>


// この関数でμ秒のウェイトを入れます。引数 100 = 100μ秒ウェイト
// クロックスピードにより調整してください。
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
	wait_us(50);	// １命令ごとに余裕を見て50usウェイトします。
	
	return rv;
}






// コマンドを送信します。HD44780でいうRS=0に相当
void i2c_cmd(unsigned char db)
{
	i2c_write(0b01111100, 0b00000000, db);
}

// データを送信します。HD44780でいうRS=1に相当
void i2c_data(unsigned char db)
{
	i2c_write(0b01111100, 0b01000000, db);
}

// （主に）文字列を連続送信します。
void i2c_puts(unsigned char *s)
{
	while(*s){
		i2c_data(*s++);
	}
}




static unsigned char icon_data[]={
	// アイコンアドレス, 該当ビット
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
	
	0x0F, 0b10000, // アンテナマーク
};





/*
 *	アイコンの表示ルーチン
 *
 *	それぞれのアイコンに該当するビットを立てる（クリア）します。
 *	現在のアイコンの表示状態は読み取れないため、マイコン側でデータを
 *	編集してからまとめて送信する必要があります。icon_buff[]
 */


// 引き数：各ビットがそれぞれのアイコンに相当します。bit0～bit12（全１３ビット）
// bit0が右端のアイコン, bit13が左のアイコン（アンテナ）に該当します。
// puticon(0x1FFF)で全アイコン点灯

void puticon(unsigned short flg)
{
	static unsigned char icon_buff[16];	// アイコンの編集用
	unsigned char i;
	
	for(i=0;i<sizeof(icon_data)/2;i++){
		if(flg & (0x1000>>i)){	// 該当ビットが立っていたら
			icon_buff[icon_data[i*2]] |= icon_data[i*2+1];	// バッファを立てます。
		} else {
			icon_buff[icon_data[i*2]] &= ~icon_data[i*2+1];	// バッファをクリアします。
		}
	}
	// 一括でLCDに書き込みます。
	for(i=0;i<16;i++){
		i2c_cmd(0b00111001);	// コマンド
		i2c_cmd(0b01000000+i);	// アイコン領域のアドレスを設定
		i2c_data(icon_buff[i]);	// アイコンデータ
	}
}


void main(void)
{
	// コントラストの設定
	unsigned char contrast = 0b111111;	// 3.0V時 数値を上げると濃くなります。
										// 2.7Vでは0b111000くらいにしてください。。
										// コントラストは電源電圧，温度によりかなり変化します。実際の液晶をみて調整してください。
	char i;
	
	DDRD = _BV(0);
	
	DDRC = _BV(3);			 // RSTピンを出力に
	//PORTC = _BV(4) | _BV(5); // SCL, SDA内蔵プルアップを有効
	
	i2c_init();				 // AVR内蔵I2Cモジュールの初期化
	wait_ms(500);
	
	PORTC &= ~_BV(3);		 // RSTをLにします。リセット
	wait_ms(1);
	PORTC |= _BV(3);		 // RSTをHにします。リセット解除
	wait_ms(10);
	
	// ここからI2C LCDの初期化を行います。
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
	wait_ms(2);			 // Clear Displayは追加ウェイトが必要
	
	
	// １行目の表示
	i2c_puts("I2C LCD");
	
	// ２行目にカーソルを移動
	i2c_cmd(0b11000000);	// ADDR=0x40
	
	// カナの表示（このソースプログラムはSJISで記述されていないとうまく表示されません）
	i2c_puts("SPLC792-I2C");
	
	
	// アイコンを点滅させる
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
			// １箇所だけビットを反転させてアイコンを非表示にします。
			// ビットを立てると表示ＯＮになります。
			puticon(0x1FFF ^ (1<<i));	// １ビットだけ反転
			wait_ms(500);
		}
		*/
		
	}
	while(1);

}

