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
�}�N����`
=================================================================================================*/
/* �����\���������}�N���B�����w�肵�Ȃ��ꍇ�́A�ǂ�ȃt�H���g�T�C�Y�ł��\���\ */
#define FONT_FAST8											/*  ���� 8px �t�H���g�p */


/*=================================================================================================
�v���g�^�C�v�錾
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
