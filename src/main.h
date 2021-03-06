/**************************************************************************************************
	Title			: Startup function
	Programmer		: Yosuke FURUSAWA.
	Copyright		: Copyright (C) 2008-2010 Yosuke FURUSAWA.
	License			: 4-clause BSD License
	Since			: 2008/12/15

	Filename		: main.h
	Last up date	: 2010/11/21
	Kanji-Code		: Shift-JIS
	TAB Space		: 4
**************************************************************************************************/


#ifndef _MAIN_H_
#define _MAIN_H_


/*================================================================================================
マクロ定義
=================================================================================================*/
#define GPIO_LED		LATAbits.LATA4
#define GPIO_KXM		LATBbits.LATB1

#define LED_ON()		GPIO_LED = 0
#define LED_OFF()		GPIO_LED = 1

#define KXM_ON()		GPIO_KXM = 1
#define KXM_OFF()		GPIO_KXM = 0


/*================================================================================================
構造体
=================================================================================================*/
typedef struct INFO {
	char *serial;

	char *board_name;
	unsigned int  board_year;
	unsigned char board_month;
	unsigned char board_day;
	unsigned char board_major;
	unsigned char board_minor;
	unsigned char board_revision;
	char *board_designed;

	char *firmware_name;
	unsigned int  firmware_year;
	unsigned char firmware_month;
	unsigned char firmware_day;
	unsigned char firmware_major;
	unsigned char firmware_minor;
	unsigned char firmware_revision;
	char *firmware_designed;

	char *project;
	char *web;
	char *mail;
	char *copyright;
} INFO_T;
typedef INFO_T* pINFO_T;


/*=================================================================================================
グローバル変数
=================================================================================================*/
extern const INFO_T info;


#endif
