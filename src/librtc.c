/**************************************************************************************************
	Title			: PIC24F Series Virtual Real Time Clock Driver
	Programmer		: Yosuke FURUSAWA
	Copyright		: Copyright (C) 2008-2010 Yosuke FURUSAWA.
	License			: 4-clause BSD License
	Since			: 2008/11/12

	Filename		: librtc.c
	Last up date	: 2010/08/13
	Kanji-Code		: Shift-JIS
	TAB Space		: 4
**************************************************************************************************/


/*================================================================================================
�w�b�_�t�@�C�����C���N���[�h
=================================================================================================*/
#include <p24FJ64GA002.h>

#include "table.h"
#include "librtc.h"


/*=================================================================================================
�}�N����`
=================================================================================================*/
#define RTC_init_timer()		PR1 = cpu_fcy[ OSCTUN ] / 10000


/*=================================================================================================
�O���[�o���ϐ�
=================================================================================================*/
RTC_T rtc;


/**************************************************************************************************
�^�C�}�[1 ���荞��
**************************************************************************************************/
void __attribute__((interrupt, auto_psv)) _T1Interrupt(void)
{
	IFS0bits.T1IF = 0;

	/* tick is 0.1ms */
	rtc.tick++;
	if(rtc.tick % 10 != 0) return;

	/* �~���b */
	rtc.msec++;
	rtc.msec = rtc.msec % 1000;
	if(rtc.msec != 0) return;

	/* �b */
#ifndef RTC_LOWMEM
	rtc.secmeter++;
#endif
	rtc.sec++;
	rtc.sec = rtc.sec % 60;
	if(rtc.sec != 0) return;

	/* �� */
#ifndef RTC_LOWMEM
	rtc.minmeter++;
#endif
	rtc.min++;
	rtc.min = rtc.min % 60;
	if(rtc.min != 0) return;

	/* �� */
#ifndef RTC_LOWMEM
	rtc.hourmeter++;
#endif
	rtc.hour++;
	rtc.hour = rtc.hour % 24;
	if(rtc.hour != 0) return;

	/* �� */
	rtc.day++;

	return;
}


/**************************************************************************************************
RTC������
**************************************************************************************************/
void RTC_init(void)
{
	rtc.msec = 0;
	rtc.sec = 0;
	rtc.min = 0;
	rtc.hour = 0;
	rtc.day = 0;
	rtc.tick = 0;

#ifndef RTC_LOWMEM
	rtc.secmeter = 0;
	rtc.minmeter = 0;
	rtc.hourmeter = 0;
#endif

	RTC_init_timer();

	T1CON = 0b1000000000000000;
	IPC0bits.T1IP = 5;
	IFS0bits.T1IF = 0;
	IEC0bits.T1IE = 1;
	return;
}


/**************************************************************************************************
tick�̒����𓾂�
**************************************************************************************************/
 unsigned int RTC_get_ticks(unsigned int start, unsigned int end)
{
	if(start <= end){
		return(end - start);
	} else {
		return(end + (0xffff - start) + 1);
	}

	/* �����ɂ�����o�O */
	return(0);
}
