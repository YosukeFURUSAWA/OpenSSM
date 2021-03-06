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
�}�N����`
=================================================================================================*/
#define RTC_LOWMEM


/*================================================================================================
�\����
=================================================================================================*/
typedef struct RTC {
	unsigned int msec;					/* �~���b */
	unsigned char sec;					/* �b */
	unsigned char min;					/* �� */
	unsigned char hour;					/* �� */
	unsigned int day;					/* �� */

	unsigned int tick;					/* �N����̍��v����(�~���b) */

#ifndef RTC_LOWMEM
	unsigned int secmeter;				/* �N����̍��v����(�b) */
	unsigned int minmeter;				/* �N����̍��v����(��) */
	unsigned int hourmeter;				/* �N����̍��v����(��) */
#endif

} RTC_T;
typedef RTC_T* pRTC_T;


/*=================================================================================================
�O���[�o���ϐ�
=================================================================================================*/
extern RTC_T rtc;


/*================================================================================================
�v���g�^�C�v�錾
=================================================================================================*/
extern void RTC_init(void);
extern void RTC_init_timer(void);
extern unsigned int RTC_get_ticks(unsigned int start, unsigned int end);


#endif
