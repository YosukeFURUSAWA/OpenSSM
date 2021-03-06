/**************************************************************************************************
	Title			: I2C Bus Serial EEPROM Driver
	Programer		: Yosuke FURUSAWA.
	Copyright		: Copyright (C) 2003-2008 Yosuke FURUSAWA.
	License			: 4-clause BSD License
	Since			: 2003/08/13 ?

	Filename		: libeeprom.c
	Last up date	: 2010/05/27
	Kanji-Code		: Shift-JIS
	TAB Space		: 4
**************************************************************************************************/


/*
TODO :
1. 高速化. Block read / write
*/


/*=================================================================================================
ヘッダファイルをインクルード
=================================================================================================*/
#include <p24FJ64GA002.h>

#include "types.h"

#include "librtc.h"
#include "libeeprom.h"


/*=================================================================================================
マクロ定義
=================================================================================================*/
/* ポート */
#define EEPROM_TX_SET()			TRISBbits.TRISB3 = 0; TRISBbits.TRISB2 = 0;
#define EEPROM_RX_SET()			TRISBbits.TRISB3 = 0; TRISBbits.TRISB2 = 1;
#define EEPROM_SCL				LATBbits.LATB3
#define EEPROM_SDA_READ			PORTBbits.RB2
#define EEPROM_SDA_WRITE		LATBbits.LATB2
#define EEPROM_AD_PIN			LATAbits.LATA3
#define EEPROM_AE_PIN			LATAbits.LATA2
//#define EEPROM_AF_PIN			0

/* 論理レベル */
#define EEPROM_HIGH				1
#define EEPROM_LOW				0

/* ビットパターン と マスク */
#define EEPROM_A0			0x0001			/* 0000 0000 0000 0000 0001 */
#define EEPROM_A1			0x0002			/* 0000 0000 0000 0000 0010 */
#define EEPROM_A2			0x0004			/* 0000 0000 0000 0000 0100 */
#define EEPROM_A3			0x0008			/* 0000 0000 0000 0000 1000 */
#define EEPROM_A4			0x0010			/* 0000 0000 0000 0001 0000 */
#define EEPROM_A5			0x0020			/* 0000 0000 0000 0010 0000 */
#define EEPROM_A6			0x0040			/* 0000 0000 0000 0100 0000 */
#define EEPROM_A7			0x0080			/* 0000 0000 0000 1000 0000 */
#define EEPROM_A8			0x0100			/* 0000 0000 0001 0000 0000 */
#define EEPROM_A9			0x0200			/* 0000 0000 0010 0000 0000 */
#define EEPROM_AA			0x0400			/* 0000 0000 0100 0000 0000 */
#define EEPROM_AB			0x0800			/* 0000 0000 1000 0000 0000 */
#define EEPROM_AC			0x1000			/* 0000 0001 0000 0000 0000 */
#define EEPROM_AD			0x2000			/* 0000 0010 0000 0000 0000 */
#define EEPROM_AE			0x4000			/* 0000 0100 0000 0000 0000 */
#define EEPROM_AF			0x8000			/* 0000 1000 0000 0000 0000 */

#define EEPROM_MASK(x,y)	((x & y) ? EEPROM_HIGH : EEPROM_LOW)


/*=================================================================================================
プロトタイプ宣言
=================================================================================================*/
static void EEPROM_start(void);
static void EEPROM_stop(void);
static void EEPROM_TX_pulse(unsigned char sda);
static unsigned char EEPROM_RX_pulse(void);
static void EEPROM_wait(void);


/**************************************************************************************************
初期化
**************************************************************************************************/
void EEPROM_init(void)
{
	EEPROM_RX_SET();
	EEPROM_SCL = EEPROM_HIGH;

	/* Bank */
	EEPROM_AD_PIN = EEPROM_LOW;
	EEPROM_AE_PIN = EEPROM_LOW;
	//EEPROM_AF_PIN = EEPROM_LOW;

	return;
}


/**************************************************************************************************
読み込み
---------------------------------------------------------------------------------------------------
Arguments :
unsigned char	ptr		読み込みアドレス  A7 A6 A5 A4 A3 A2 A1 A0
---------------------------------------------------------------------------------------------------
Returns :
int						読み込んだデータ
**************************************************************************************************/
int EEPROM_read(unsigned int ptr)
{
	unsigned char data;

//	EEPROM_AF_PIN = EEPROM_MASK(ptr, EEPROM_AF);
	EEPROM_AE_PIN = EEPROM_MASK(ptr, EEPROM_AE);
	EEPROM_AD_PIN = EEPROM_MASK(ptr, EEPROM_AD);

	EEPROM_start();

	/* コントロール : 1010 AF AE AD R/W */
	EEPROM_TX_pulse(EEPROM_HIGH);
	EEPROM_TX_pulse(EEPROM_LOW);
	EEPROM_TX_pulse(EEPROM_HIGH);
	EEPROM_TX_pulse(EEPROM_LOW);
//	EEPROM_TX_pulse(EEPROM_MASK(ptr, EEPROM_AF));
	EEPROM_TX_pulse(EEPROM_LOW);
	EEPROM_TX_pulse(EEPROM_MASK(ptr, EEPROM_AE));
	EEPROM_TX_pulse(EEPROM_MASK(ptr, EEPROM_AD));
	EEPROM_TX_pulse(EEPROM_LOW);
	if(EEPROM_RX_pulse() == EEPROM_HIGH) goto READ_ERROR;		/* Acknowledge */

	/* アドレス : * * * AC AB AA A9 A8 */
	EEPROM_TX_pulse(EEPROM_LOW);
	EEPROM_TX_pulse(EEPROM_LOW);
	EEPROM_TX_pulse(EEPROM_LOW);
	EEPROM_TX_pulse(EEPROM_MASK(ptr, EEPROM_AC));
	EEPROM_TX_pulse(EEPROM_MASK(ptr, EEPROM_AB));
	EEPROM_TX_pulse(EEPROM_MASK(ptr, EEPROM_AA));
	EEPROM_TX_pulse(EEPROM_MASK(ptr, EEPROM_A9));
	EEPROM_TX_pulse(EEPROM_MASK(ptr, EEPROM_A8));
	if(EEPROM_RX_pulse() == EEPROM_HIGH) goto READ_ERROR;		/* Acknowledge */

	/* アドレス : A7 A6 A5 A4 A3 A2 A1 A0 */
	EEPROM_TX_pulse(EEPROM_MASK(ptr, EEPROM_A7));
	EEPROM_TX_pulse(EEPROM_MASK(ptr, EEPROM_A6));
	EEPROM_TX_pulse(EEPROM_MASK(ptr, EEPROM_A5));
	EEPROM_TX_pulse(EEPROM_MASK(ptr, EEPROM_A4));
	EEPROM_TX_pulse(EEPROM_MASK(ptr, EEPROM_A3));
	EEPROM_TX_pulse(EEPROM_MASK(ptr, EEPROM_A2));
	EEPROM_TX_pulse(EEPROM_MASK(ptr, EEPROM_A1));
	EEPROM_TX_pulse(EEPROM_MASK(ptr, EEPROM_A0));
	if(EEPROM_RX_pulse() == EEPROM_HIGH) goto READ_ERROR;		/* Acknowledge */

	/* ここから読み込み処理 */
	EEPROM_start();

	/* デバイスアドレス 1010 AF AE AD R/W */
	EEPROM_TX_pulse(EEPROM_HIGH);
	EEPROM_TX_pulse(EEPROM_LOW);
	EEPROM_TX_pulse(EEPROM_HIGH);
	EEPROM_TX_pulse(EEPROM_LOW);
//	EEPROM_TX_pulse(EEPROM_MASK(ptr, EEPROM_AF));
	EEPROM_TX_pulse(EEPROM_LOW);
	EEPROM_TX_pulse(EEPROM_MASK(ptr, EEPROM_AE));
	EEPROM_TX_pulse(EEPROM_MASK(ptr, EEPROM_AD));
	EEPROM_TX_pulse(EEPROM_HIGH);
	if(EEPROM_RX_pulse() == EEPROM_HIGH) goto READ_ERROR;		/* Acknowledge */

	/* データ読み出し */
	data = 0;
	data +=  EEPROM_RX_pulse() << 7;
	data +=  EEPROM_RX_pulse() << 6;
	data +=  EEPROM_RX_pulse() << 5;
	data +=  EEPROM_RX_pulse() << 4;
	data +=  EEPROM_RX_pulse() << 3;
	data +=  EEPROM_RX_pulse() << 2;
	data +=  EEPROM_RX_pulse() << 1;
	data +=  EEPROM_RX_pulse();
	EEPROM_TX_pulse(EEPROM_HIGH);								/* No Acknowledge */

	EEPROM_stop();
	return(data);


READ_ERROR:
	EEPROM_stop();
	return(-1);

}



/**************************************************************************************************
書き込み
---------------------------------------------------------------------------------------------------
Arguments :
unsigned int	ptr		書き込みアドレス A10 A9 A8 A7 A6 A5 A4 A3 A2 A1 A0
unsigned char	buf		書き込むデータ
---------------------------------------------------------------------------------------------------
Returns :
unsigned char			TRUE or FALSE
**************************************************************************************************/
BOOL EEPROM_write(unsigned int ptr, unsigned char buf)
{
	unsigned int wait;

//	EEPROM_AF_PIN = EEPROM_MASK(ptr, EEPROM_AF);
	EEPROM_AE_PIN = EEPROM_MASK(ptr, EEPROM_AE);
	EEPROM_AD_PIN = EEPROM_MASK(ptr, EEPROM_AD);

	EEPROM_start();

	/* コントロール : 1010 AF AE AD R/W */
	EEPROM_TX_pulse(EEPROM_HIGH);
	EEPROM_TX_pulse(EEPROM_LOW);
	EEPROM_TX_pulse(EEPROM_HIGH);
	EEPROM_TX_pulse(EEPROM_LOW);
//	EEPROM_TX_pulse(EEPROM_MASK(ptr, EEPROM_AF));
	EEPROM_TX_pulse(EEPROM_LOW);
	EEPROM_TX_pulse(EEPROM_MASK(ptr, EEPROM_AE));
	EEPROM_TX_pulse(EEPROM_MASK(ptr, EEPROM_AD));
	EEPROM_TX_pulse(EEPROM_LOW);
	if(EEPROM_RX_pulse() == EEPROM_HIGH) goto WRITE_ERROR;		/* Acknowledge */

	/* アドレス : * * * AC AB AA A9 A8 */
	EEPROM_TX_pulse(EEPROM_LOW);
	EEPROM_TX_pulse(EEPROM_LOW);
	EEPROM_TX_pulse(EEPROM_LOW);
	EEPROM_TX_pulse(EEPROM_MASK(ptr, EEPROM_AC));
	EEPROM_TX_pulse(EEPROM_MASK(ptr, EEPROM_AB));
	EEPROM_TX_pulse(EEPROM_MASK(ptr, EEPROM_AA));
	EEPROM_TX_pulse(EEPROM_MASK(ptr, EEPROM_A9));
	EEPROM_TX_pulse(EEPROM_MASK(ptr, EEPROM_A8));
	if(EEPROM_RX_pulse() == EEPROM_HIGH) goto WRITE_ERROR;		/* Acknowledge */

	/* アドレス : A7 A6 A5 A4 A3 A2 A1 A0 */
	EEPROM_TX_pulse(EEPROM_MASK(ptr, EEPROM_A7));
	EEPROM_TX_pulse(EEPROM_MASK(ptr, EEPROM_A6));
	EEPROM_TX_pulse(EEPROM_MASK(ptr, EEPROM_A5));
	EEPROM_TX_pulse(EEPROM_MASK(ptr, EEPROM_A4));
	EEPROM_TX_pulse(EEPROM_MASK(ptr, EEPROM_A3));
	EEPROM_TX_pulse(EEPROM_MASK(ptr, EEPROM_A2));
	EEPROM_TX_pulse(EEPROM_MASK(ptr, EEPROM_A1));
	EEPROM_TX_pulse(EEPROM_MASK(ptr, EEPROM_A0));
	if(EEPROM_RX_pulse() == EEPROM_HIGH) goto WRITE_ERROR;		/* Acknowledge */

	/* データ書き込み */
	EEPROM_TX_pulse(EEPROM_MASK(buf, EEPROM_A7));
	EEPROM_TX_pulse(EEPROM_MASK(buf, EEPROM_A6));
	EEPROM_TX_pulse(EEPROM_MASK(buf, EEPROM_A5));
	EEPROM_TX_pulse(EEPROM_MASK(buf, EEPROM_A4));
	EEPROM_TX_pulse(EEPROM_MASK(buf, EEPROM_A3));
	EEPROM_TX_pulse(EEPROM_MASK(buf, EEPROM_A2));
	EEPROM_TX_pulse(EEPROM_MASK(buf, EEPROM_A1));
	EEPROM_TX_pulse(EEPROM_MASK(buf, EEPROM_A0));
	if(EEPROM_RX_pulse() == EEPROM_HIGH) goto WRITE_ERROR;		/* Acknowledge */

	EEPROM_stop();

	/* Write Cycle Time */
	wait = rtc.tick;
	while(RTC_get_ticks(wait, rtc.tick) < 100);

	return(TRUE);

WRITE_ERROR:
	EEPROM_stop();
	return(FALSE);
}


/*-------------------------------------------------------------------------------------------------
スタートビット
-------------------------------------------------------------------------------------------------*/
static void EEPROM_start(void)
{
	EEPROM_TX_SET();

	EEPROM_SDA_WRITE = EEPROM_HIGH;
	EEPROM_SCL = EEPROM_HIGH;
	EEPROM_wait();
	EEPROM_SDA_WRITE = EEPROM_LOW;
	EEPROM_SCL = EEPROM_LOW;
	EEPROM_wait();

	EEPROM_RX_SET();
	return;
}


/*-------------------------------------------------------------------------------------------------
ストップビット
-------------------------------------------------------------------------------------------------*/
static void EEPROM_stop(void)
{
	EEPROM_TX_SET();

	EEPROM_SCL = EEPROM_LOW;
	EEPROM_SDA_WRITE = EEPROM_LOW;
	EEPROM_wait();
	EEPROM_SCL = EEPROM_HIGH;
	EEPROM_SDA_WRITE = EEPROM_HIGH;
	EEPROM_wait();

	EEPROM_RX_SET();
	return;
}


/*-------------------------------------------------------------------------------------------------
シリアルクロック出力、データ出力
---------------------------------------------------------------------------------------------------
Arguments :
unsigned char   sda      SDA の bit
-------------------------------------------------------------------------------------------------*/
static void EEPROM_TX_pulse(unsigned char sda)
{
	EEPROM_TX_SET();

	EEPROM_SDA_WRITE = sda;
	EEPROM_SCL = EEPROM_HIGH;
	EEPROM_wait();
	EEPROM_SCL = EEPROM_LOW;
	EEPROM_wait();

	EEPROM_RX_SET();
	return;
}


/*-------------------------------------------------------------------------------------------------
シリアルクロック出力、データ入力
---------------------------------------------------------------------------------------------------
Arguments :
unsigned char   sda      SDA の bit
-------------------------------------------------------------------------------------------------*/
static unsigned char EEPROM_RX_pulse(void)
{
	unsigned char buf;

	EEPROM_RX_SET();

	EEPROM_SCL = EEPROM_HIGH;
	EEPROM_wait();
	buf = EEPROM_SDA_READ;
	EEPROM_SCL = EEPROM_LOW;
	EEPROM_wait();

	return(buf);
}


/*-------------------------------------------------------------------------------------------------
ウェイト
-------------------------------------------------------------------------------------------------*/
static void EEPROM_wait(void)
{
	Nop();
	Nop();
	Nop();
	Nop();
	Nop();
	Nop();
	Nop();
	Nop();
	Nop();
	Nop();
	Nop();
	Nop();
	Nop();
	Nop();
	Nop();
	Nop();
	Nop();
	Nop();
	return;
}
