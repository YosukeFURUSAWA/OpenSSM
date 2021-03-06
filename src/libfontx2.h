/**************************************************************************************************
	Title			: FONTX2 Driver
	Programmer		: Yosuke FURUSAWA
	Copyright		: Copyright (C) 2010 Yosuke FURUSAWA.
	License			: 4-clause BSD License
	Since			: 2010/05/08

	Filename		: libfontx2.h
	Last up date	: 2010/08/09
	Kanji-Code		: Shift-JIS
	TAB Space		: 4

	Note			: In this version, do not support Kanji and Font Selector.
**************************************************************************************************/


#ifndef _LIBFONTX2_H_
#define _LIBFONTX2_H_


/*=================================================================================================
マクロ定義
=================================================================================================*/
/* 文字表示高速化マクロ。何も指定しない場合は、どんなフォントサイズでも表示可能 */
#define FONT_FAST8											/*  横幅 8px フォント用 */


/*=================================================================================================
プロトタイプ宣言
=================================================================================================*/
extern BOOL FONTX2_init(void);
extern  unsigned char FONTX2_get_ascii_width(void);
extern  unsigned char FONTX2_get_ascii_width_byte(void);
extern  unsigned char FONTX2_get_ascii_height(void);
extern  unsigned char FONTX2_get_ascii_height_byte(void);
extern  unsigned char FONTX2_get_ascii_size(void);
extern  unsigned char *FONTX2_get_ascii_font(const unsigned char ascii);
extern  unsigned char FONTX2_get_ascii_font_data(const unsigned char ascii, unsigned char x, unsigned char y);


#endif
