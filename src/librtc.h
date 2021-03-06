/**************************************************************************************************
	Title			: PIC24F Series Virtual Real Time Clock Driver
	Programmer		: Yosuke FURUSAWA
	Copyright		: Copyright (C) 2008-2010 Yosuke FURUSAWA.
	License			: 4-clause BSD License
	Since			: 2008/11/12

	Filename		: librtc.h
	Last up date	: 2010/08/13
	Kanji-Code		: Shift-JIS
	TAB Space		: 4
**************************************************************************************************/


#ifndef _LIBRTC_H_
#define _LIBRTC_H_


/*=================================================================================================
マクロ定義
=================================================================================================*/
#define RTC_LOWMEM


/*================================================================================================
構造体
=================================================================================================*/
typedef struct RTC {
	unsigned int msec;					/* ミリ秒 */
	unsigned char sec;					/* 秒 */
	unsigned char min;					/* 分 */
	unsigned char hour;					/* 時 */
	unsigned int day;					/* 日 */

	unsigned int tick;					/* 起動後の合計時間(ミリ秒) */

#ifndef RTC_LOWMEM
	unsigned int secmeter;				/* 起動後の合計時間(秒) */
	unsigned int minmeter;				/* 起動後の合計時間(分) */
	unsigned int hourmeter;				/* 起動後の合計時間(時) */
#endif

} RTC_T;
typedef RTC_T* pRTC_T;


/*=================================================================================================
グローバル変数
=================================================================================================*/
extern RTC_T rtc;


/*================================================================================================
プロトタイプ宣言
=================================================================================================*/
extern void RTC_init(void);
extern void RTC_init_timer(void);
extern unsigned int RTC_get_ticks(unsigned int start, unsigned int end);


#endif
