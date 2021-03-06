/**************************************************************************************************
	Title			: I2C Bus Serial EEPROM Driver
	Programer		: Yosuke FURUSAWA.
	Copyright		: Copyright (C) 2003-2010 Yosuke FURUSAWA.
	License			: 4-clause BSD License
	Since			: 2003/08/13 ?

	Filename		: libeeprom.c
	Last up date	: 2010/05/27
	Kanji-Code		: Shift-JIS
	TAB Space		: 4
**************************************************************************************************/


#ifndef _LIBEEPROM_H_
#define _LIBEEPROM_H_

#include "types.h"


/*=================================================================================================
プロトタイプ宣言
=================================================================================================*/
extern void EEPROM_init(void);
extern int EEPROM_read (unsigned int ptr);
extern BOOL EEPROM_write(unsigned int ptr, unsigned char buf);


#endif
