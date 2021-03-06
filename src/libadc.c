/**************************************************************************************************
	Title			: PIC24F Series ADC Driver
	Programmer		: Yosuke FURUSAWA
	Copyright		: Copyright (C) 2008-2010 Yosuke FURUSAWA.
	License			: 4-clause BSD License
	Since			: 2008/12/10

	Filename		: libadc.c
	Last up date	: 2010/08/13
	Kanji-Code		: Shift-JIS
	TAB Space		: 4
**************************************************************************************************/


/*================================================================================================
ヘッダファイルをインクルード
=================================================================================================*/
#include <p24FJ64GA002.h>

#include "types.h"

#include "librtc.h"
#include "libadc.h"


/*=================================================================================================
マクロ定義
=================================================================================================*/


/*=================================================================================================
グローバル変数
=================================================================================================*/
ADC_T adc;


/*=================================================================================================
プロトタイプ宣言
=================================================================================================*/


/**************************************************************************************************
内蔵ADC 割り込み
**************************************************************************************************/
void __attribute__((interrupt, auto_psv)) _ADC1Interrupt(void)
{
	IFS0bits.AD1IF = 0;

	/* LPF */
	adc.adc[0] = (ADC1BUF0 + ADC1BUF3 + ADC1BUF6 + ADC1BUF9) >> 2;
	adc.adc[1] = (ADC1BUF1 + ADC1BUF4 + ADC1BUF7 + ADC1BUFA) >> 2;
	adc.adc[2] = (ADC1BUF2 + ADC1BUF5 + ADC1BUF8 + ADC1BUFB) >> 2;

#ifndef ADC_LOWMEM
	adc.cycle = RTC_get_ticks(adc.last, rtc.tick);
	adc.last = rtc.tick;
#endif

	return;
}


/**************************************************************************************************
ADC 初期化
**************************************************************************************************/
void ADC_init(void)
{
	
#ifndef ADC_LOWMEM
	adc.last = rtc.tick;
	adc.cycle = 0xffff;
#endif

	/* 内蔵ADC */
	/*          FEDCBA9876543210 */
	AD1CON1 = 0b1000000011100101;
	AD1CON2 = 0b0000010000110000;
	AD1CON3 = 0b0001111111111111;
	AD1PCFG = 0b1111111111111000;
	AD1CSSL = 0b0000000000000111;
	AD1CHS  = 0b0000000000000000;

	IPC3bits.AD1IP = 1;
	IEC0bits.AD1IE = 1;
	IFS0bits.AD1IF = 0;

	return;
}
