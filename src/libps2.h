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


#ifndef _LIBPS2_H_
#define _LIBPS2_H_


/*=================================================================================================
マクロ定義
=================================================================================================*/
/* 特殊キーのコード */
#define KEY_NOP					0x00
#define KEY_BACKSPACE			0x08
#define KEY_TAB					0x09
#define KEY_ENTER				0x0d
#define KEY_ESC					0x1b
#define KEY_DEL					0x7f
#define KEY_CTRL				0x90
#define KEY_LSHIFT				0x91
#define KEY_ALT					0x92
#define KEY_RSHIFT				0x94
#define KEY_INS					0xa0
#define KEY_END					0xa1
#define KEY_DOWN				0xa2
#define KEY_PAGEDOWN			0xa3
#define KEY_LEFT				0xa4
#define KEY_RIGHT				0xa6
#define KEY_HOME				0xa7
#define KEY_UP					0xa8
#define KEY_PAGEUP				0xa9
#define KEY_PRINT				0xab
#define KEY_F1					0xc0
#define KEY_F2					0xc1
#define KEY_F3					0xc2
#define KEY_F4					0xc3
#define KEY_F5					0xc4
#define KEY_F6					0xc5
#define KEY_F7					0xc6
#define KEY_F8					0xc7
#define KEY_F9					0xc8
#define KEY_F10					0xc9
#define KEY_F11					0xca
#define KEY_F12					0xcb
#define KEY_NUM					0xd0
#define KEY_CAPS				0xd1
#define KEY_SCROLL				0xd2
#define KEY_HANKAKU				0xe0
#define KEY_MUHENKAN			0xe2
#define KEY_HENKAN				0xe3
#define KEY_HIRAGANA			0xe4
#define KEY_LWIN				0xf0
#define KEY_RWIN				0xf1
#define KEY_APP					0xf2


/*=================================================================================================
構造体
=================================================================================================*/
typedef struct PS2 {
	unsigned int wait;
} PS2_T;
typedef PS2_T* pPS2_T;


/*=================================================================================================
グローバル変数
=================================================================================================*/
extern PS2_T ps2;


/*=================================================================================================
プロトタイプ宣言
=================================================================================================*/
extern void PS2_init(void);
extern BOOL PS2_main(void);
extern void PS2_key_buf_clear(void);
extern unsigned char PS2_key_get(void);
extern unsigned char PS2_key_check(void);


#endif
