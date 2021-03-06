/**************************************************************************************************
	Title			: External Meter
	Programmer		: Yosuke FURUSAWA.
	Copyright		: Copyright (C) 2010 Yosuke FURUSAWA.
	License			: 4-clause BSD License
	Since			: 2010/06/19

	Filename		: extmeter.h
	Last up date	: 2010/11/21
	Kanji-Code		: Shift-JIS
	TAB Space		: 4
**************************************************************************************************/


#ifndef _EXTMETER_H_
#define _EXTMETER_H_


/*================================================================================================
マクロ定義
=================================================================================================*/
#define EXTMETER_SETTING		(0)
#define EXTMETER_SPEED			(1)
#define EXTMETER_ENGINE			(2)
#define EXTMETER_BOOST			(3)
#define EXTMETER_THROTTLE		(4)
#define EXTMETER_COOLANT		(5)
#define EXTMETER_INTAKEAIR		(6)

#define EXTMETER_DIV			16								/* 変更不可 */


/*================================================================================================
構造体
=================================================================================================*/
typedef struct EXTMETER {
	unsigned char target;
	unsigned char setting;				/* target == EXTMETER_SETTINGのときにのみ使われる */
	double map[ EXTMETER_DIV ];
} EXTMETER_T;
typedef EXTMETER_T* pEXTMETER_T;


/*=================================================================================================
グローバル変数
=================================================================================================*/
extern EXTMETER_T extmeter;


/*================================================================================================
プロトタイプ宣言
=================================================================================================*/
void EXTMETER_init(unsigned char target);
extern BOOL EXTMETER_main(void);


#endif
