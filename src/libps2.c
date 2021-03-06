/**************************************************************************************************
	Title			: PIC24F Series PS/2 Keyboard Driver
	Programmer		: Yosuke FURUSAWA
	Copyright		: Copyright (C) 2010 Yosuke FURUSAWA.
	License			: 4-clause BSD License
	Since			: 2010/05/29

	Filename		: libps2.c
	Last up date	: 2010/05/30
	Kanji-Code		: Shift-JIS
	TAB Space		: 4

	Note			: In this version, do not support multi byte scan code and mouse protocol.
**************************************************************************************************/


/*
** スキャンコードセット 2 と仮定する
*/ 


/*================================================================================================
ヘッダファイルをインクルード
=================================================================================================*/
#include <p24FJ64GA002.h>

#include "types.h"

#include "librtc.h"
#include "libps2.h"


/*=================================================================================================
マクロ定義
================================================================================================*/
#define GPIO_PS2_CLK			PORTBbits.RB8
#define GPIO_PS2_DAT			PORTBbits.RB9

#define PS2_KEY_BUFFER_SIZE		5				/* 高速にタイプするなら、大きめに... */
#define PS2_CODE_BUFFER_SIZE	16				/* PS2_main()のポーリング間隔とタイプ速度に依存 */


/*=================================================================================================
グローバル変数
=================================================================================================*/
PS2_T ps2;

/* キーバッファ(アプリケーション参照用) */
static unsigned char key_buf[ PS2_KEY_BUFFER_SIZE ];
static unsigned char key_stptr;
static unsigned char key_enptr;

/* スキャンコードバッファ(プロトコル処理用) */
static unsigned char code_buf[ PS2_CODE_BUFFER_SIZE ];
static unsigned char code_stptr;
static unsigned char code_enptr;

/* スキャンコード -> キー変換テーブル cf : Fujitsu FMV-KB322, 109 keyboard */
/* 規則性が全くない. なんで？ (==; */
static const unsigned char key[] = {
	KEY_NOP,			/* 0x00 */
	KEY_F9,				/* 0x01 */
	KEY_NOP,			/* 0x02 */
	KEY_F5,				/* 0x03 */
	KEY_F3,				/* 0x04 */
	KEY_F1,				/* 0x05 */
	KEY_F2,				/* 0x06 */
	KEY_F12,			/* 0x07 */
	KEY_NOP,			/* 0x08 */
	KEY_F10,			/* 0x09 */
	KEY_F8,				/* 0x0a */
	KEY_F6,				/* 0x0b */
	KEY_F4,				/* 0x0c */
	KEY_TAB,			/* 0x0d */
	KEY_HANKAKU,		/* 0x0e */
	KEY_NOP,			/* 0x0f */
	KEY_NOP,			/* 0x10 */
	KEY_ALT,			/* 0x11 */
	KEY_RSHIFT,			/* 0x12 :  KEY_LSHIFTに該当するが、コードを単純化するために RSHIFT */
	KEY_HIRAGANA,		/* 0x13 */
	KEY_CTRL,			/* 0x14 */
	'q',				/* 0x15 */
	'1',				/* 0x16 */
	KEY_NOP,			/* 0x17 */
	KEY_NOP,			/* 0x18 */
	KEY_NOP,			/* 0x19 */
	'z',				/* 0x1a */
	's',				/* 0x1b */
	'a',				/* 0x1c */
	'w',				/* 0x1d */
	'2',				/* 0x1e */
	KEY_LWIN,			/* 0x1f */
	KEY_NOP,			/* 0x20 */
	'c',				/* 0x21 */
	'x',				/* 0x22 */
	'd',				/* 0x23 */
	'e',				/* 0x24 */
	'4',				/* 0x25 */
	'3',				/* 0x26 */
	KEY_RWIN,			/* 0x27 */
	KEY_NOP,			/* 0x28 */
	' ',				/* 0x29 */
	'v',				/* 0x2a */
	'f',				/* 0x2b */
	't',				/* 0x2c */
	'r',				/* 0x2d */
	'5',				/* 0x2e */
	KEY_APP,			/* 0x2f */
	KEY_NOP,			/* 0x30 */
	'n',				/* 0x31 */
	'b',				/* 0x32 */
	'h',				/* 0x33 */
	'g',				/* 0x34 */
	'y',				/* 0x35 */
	'6',				/* 0x36 */
	KEY_NOP,			/* 0x37 */
	KEY_NOP,			/* 0x38 */
	KEY_NOP,			/* 0x39 */
	'm',				/* 0x3a */
	'j',				/* 0x3b */
	'u',				/* 0x3c */
	'7',				/* 0x3d */
	'8',				/* 0x3e */
	KEY_NOP,			/* 0x3f */
	KEY_NOP,			/* 0x40 */
	',',				/* 0x41 */
	'k',				/* 0x42 */
	'i',				/* 0x43 */
	'o',				/* 0x44 */
	'0',				/* 0x45 */
	'9',				/* 0x46 */
	KEY_NOP,			/* 0x47 */
	KEY_NOP,			/* 0x48 */
	'.',				/* 0x49 */
	'/',				/* 0x4a */
	'l',				/* 0x4b */
	';',				/* 0x4c */
	'p',				/* 0x4d */
	'-',				/* 0x4e */
	KEY_NOP,			/* 0x4f */
	KEY_NOP,			/* 0x50 */
	'\\',				/* 0x51 */
	':',				/* 0x52 */
	KEY_NOP,			/* 0x53 */
	'@',				/* 0x54 */
	'^',				/* 0x55 */
	KEY_NOP,			/* 0x56 */
	KEY_NOP,			/* 0x57 */
	KEY_CAPS,			/* 0x58 */
	KEY_RSHIFT,			/* 0x59 */
	KEY_ENTER,			/* 0x5a */
	'[',				/* 0x5b */
	KEY_NOP,			/* 0x5c */
	']',				/* 0x5d */
	KEY_NOP,			/* 0x5e */
	KEY_NOP,			/* 0x5f */
	KEY_NOP,			/* 0x60 */
	KEY_NOP,			/* 0x61 */
	KEY_NOP,			/* 0x62 */
	KEY_NOP,			/* 0x63 */
	KEY_HENKAN,			/* 0x64 */
	KEY_NOP,			/* 0x65 */
	KEY_BACKSPACE,		/* 0x66 */
	KEY_MUHENKAN,		/* 0x67 */
	KEY_NOP,			/* 0x68 */
	KEY_END,			/* 0x69 */
	'\\',				/* 0x6a */
	KEY_LEFT,			/* 0x6b */
	KEY_HOME,			/* 0x6c */
	KEY_NOP,			/* 0x6d */
	KEY_NOP,			/* 0x6e */
	KEY_NOP,			/* 0x6f */
	KEY_INS,			/* 0x70 */
	KEY_DEL,			/* 0x71 */
	KEY_DOWN,			/* 0x72 */
	KEY_NOP,			/* 0x73 */
	KEY_RIGHT,			/* 0x74 */
	KEY_UP,				/* 0x75 */
	KEY_ESC,			/* 0x76 */
	KEY_NUM,			/* 0x77 */
	KEY_F11,			/* 0x78 */
	KEY_NOP,			/* 0x79 */
	KEY_PAGEDOWN,		/* 0x7a */
	KEY_NOP,			/* 0x7b */
	KEY_PRINT,			/* 0x7c */
	KEY_PAGEUP,			/* 0x7d */
	KEY_SCROLL,			/* 0x7e */
	KEY_NOP,			/* 0x7f */
	KEY_NOP,			/* 0x80 */
	KEY_NOP,			/* 0x81 */
	KEY_NOP,			/* 0x82 */
	KEY_F7,				/* 0x83 */
};


/*=================================================================================================
プロトタイプ宣言
=================================================================================================*/
 static BOOL PS2_key_put(unsigned char buf);
 static void PS2_code_buf_clear(void);
 static BOOL PS2_code_put(unsigned char buf);
 static unsigned char PS2_code_get(void);
 static unsigned char PS2_code_get_buf(void);
 static BOOL PS2_code_get_key(void);


/**************************************************************************************************
I/Oピン状態変化割り込み : 割り込みが足りない... INT0に配線すれば良かった...
**************************************************************************************************/

/* アセンブラ命令で MSB <--> LSBを変えられたはずだけど、調べるのが面倒なのでパス */
 static unsigned char reverse (unsigned char buf)
{
	return(
			((buf & 0b00000001) << 7)
		+	((buf & 0b00000010) << 5)
		+	((buf & 0b00000100) << 3)
		+	((buf & 0b00001000) << 1)
		+	((buf & 0b00010000) >> 1)
		+	((buf & 0b00100000) >> 3)
		+	((buf & 0b01000000) >> 5)
		+	((buf & 0b10000000) >> 7)
	);

}

void __attribute__((interrupt, auto_psv)) _CNInterrupt(void)
{
	static unsigned int  last  = 0x0000;
	static unsigned int  buf   = 0x0000;
	static unsigned char count = 0x00;

	IFS1bits.CNIF = 0;

	/* クロックが送られてきたら、処理を行う */
	if(GPIO_PS2_CLK == 0){

		/* 久しぶりのクロックならば、バッファをクリアする */
		if(RTC_get_ticks(last, rtc.tick) > 5){
			buf = 0x0000;
			count = 0x00;
		}
		last = rtc.tick;

		/* データ受信処理 */
		buf = (buf << 1) + GPIO_PS2_DAT;
		count++;

		/* 正しいパケットならば、スキャンコードバッファへ送る */
		if(count > 10){
			PS2_code_put(reverse(buf >> 2));		/* パリティビットは無視 */
			buf = 0x0000;
			count = 0x00;
		}

	} else {
		/* 立ち上がりエッジでは、何もしない */
	}


	return;
}


/**************************************************************************************************
初期化
**************************************************************************************************/
void PS2_init(void)
{
	PS2_key_buf_clear();
	PS2_code_buf_clear();

	ps2.wait = 3000;

	CNEN2bits.CN22IE = 1;
	IPC4bits.CNIP = 5;
	IEC1bits.CNIE = 1;
	IFS1bits.CNIF = 0;

	return;
}


/**************************************************************************************************
プロトコル処理
**************************************************************************************************/
BOOL PS2_main(void)
{
	/* スキャンコード -> キー変換処理 */
	PS2_code_get_key();
	return(TRUE);
}

/**************************************************************************************************
キーバッファクリア
**************************************************************************************************/
void PS2_key_buf_clear(void)
{
	key_stptr = 0x00;
	key_enptr = 0x00;

	PS2_code_buf_clear();

	return;
}


/*-------------------------------------------------------------------------------------------------
スキャンコードバッファクリア
-------------------------------------------------------------------------------------------------*/
 static void PS2_code_buf_clear(void)
{
	code_stptr = 0x00;
	code_enptr = 0x00;
	return;
}


/*-------------------------------------------------------------------------------------------------
キーバッファへ入れる
-------------------------------------------------------------------------------------------------*/
 static BOOL PS2_key_put(unsigned char buf)
{
	unsigned char nxptr;

	nxptr = key_enptr + 1;
	if(nxptr >= PS2_KEY_BUFFER_SIZE) nxptr = 0;

	/* Buffer is Full */
	if(key_stptr == nxptr){
		return(FALSE);
	}

	key_buf[ key_enptr ] = buf;
	key_enptr = nxptr;

	return(TRUE);
}


/*-------------------------------------------------------------------------------------------------
スキャンコードバッファへ入れる
-------------------------------------------------------------------------------------------------*/
 static BOOL PS2_code_put(unsigned char buf)
{
	unsigned char nxptr;

	nxptr = code_enptr + 1;
	if(nxptr >= PS2_CODE_BUFFER_SIZE) nxptr = 0;

	/* Buffer is Full */
	if(code_stptr == nxptr){
		return(FALSE);
	}

	code_buf[ code_enptr ] = buf;
	code_enptr = nxptr;

	return(TRUE);
}


/**************************************************************************************************
キーバッファから取得する
**************************************************************************************************/
unsigned char PS2_key_get(void)
{
	unsigned char buf, nxptr;

	/* Buffer is Empty */
	if(key_stptr == key_enptr){
		return(0x00);
	}

	buf = key_buf[ key_stptr ];

	nxptr = key_stptr + 1;
	if(nxptr >= PS2_KEY_BUFFER_SIZE) nxptr = 0;
	key_stptr = nxptr;

	return(buf);
}

/* 確認するだけ... */
unsigned char PS2_key_check(void)
{
	/* Buffer is Empty */
	if(key_stptr == key_enptr){
		return(0x00);
	}

	return(key_buf[ key_stptr ]);
}


/*-------------------------------------------------------------------------------------------------
スキャンコードバッファから取得する
-------------------------------------------------------------------------------------------------*/
 static unsigned char PS2_code_get(void)
{
	unsigned char buf, nxptr;

	/* Buffer is Empty */
	if(code_stptr == code_enptr){
		return(0x00);
	}

	buf = code_buf[ code_stptr ];

	nxptr = code_stptr + 1;
	if(nxptr >= PS2_CODE_BUFFER_SIZE) nxptr = 0;
	code_stptr = nxptr;

	return(buf);
}


/*-------------------------------------------------------------------------------------------------
使用済バッファサイズを得る
-------------------------------------------------------------------------------------------------*/
 unsigned char PS2_code_get_buf(void)
{
	if(code_stptr >= code_enptr)	return(code_stptr - code_enptr);
	else							return(code_stptr + code_enptr);
}


/*-------------------------------------------------------------------------------------------------
スキャンコード -> キーコードへ変換する
---------------------------------------------------------------------------------------------------
データの順番 : [CODE][RELEASE][CODE][CODE][RELEASE], etc...
まじめにリリース処理を入れるのは面倒なので、インターバルでみる (^^;
-------------------------------------------------------------------------------------------------*/
 static BOOL PS2_code_get_key(void)
{
	unsigned char buf;
	static unsigned char last = 0x00;
	static unsigned int tick = 0x0000;

	while((buf = PS2_code_get()) != 0x00){
		if(RTC_get_ticks(tick, rtc.tick) > ps2.wait){
			last = 0x00;
		}

		/* 特定のコマンドを無視する. F0, E0, E1 など */
		if(buf != last && buf < sizeof(key)){
			if(key[buf] != KEY_NOP){
				if(!PS2_key_put(key[buf])){
					PS2_key_buf_clear();
					PS2_key_put(key[buf]);
				}
				last = buf;
				tick = rtc.tick;
			}
		}
	}

	return(TRUE);
}
