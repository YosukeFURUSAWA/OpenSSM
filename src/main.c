/**************************************************************************************************
	Title			: Startup function
	Programmer		: Yosuke FURUSAWA.
	Copyright		: Copyright (C) 2008-2010 Yosuke FURUSAWA.
	License			: 4-clause BSD License
	Since			: 2008/12/10

	Filename		: main.c
	Last up date	: 2010/11/21
	Kanji-Code		: Shift-JIS
	TAB Space		: 4
**************************************************************************************************/


/*================================================================================================
�w�b�_�t�@�C�����C���N���[�h
=================================================================================================*/
#include <p24FJ64GA002.h>

#include "types.h"
#include "table.h"
#include "ssm.h"
#include "extmeter.h"
#include "main.h"

#include "libadc.h"
#include "libdac.h"
#include "librtc.h"
#include "libuart.h"
#include "libps2.h"
#include "libfontx2.h"
#include "libvideo.h"


/*================================================================================================
�R���t�B�O���[�V����
=================================================================================================*/
_CONFIG1(	JTAGEN_OFF &
			GCP_OFF &
			GWRP_OFF &
			BKBUG_OFF &
			WINDIS_OFF &
			COE_OFF &
			ICS_PGx1 &
			FWDTEN_ON &
			WDTPS_PS256) 

_CONFIG2(	IESO_OFF &
			FNOSC_FRCPLL &
			FCKSM_CSDCMD &
			OSCIOFNC_ON &
			IOL1WAY_OFF &
			I2C1SEL_PRI &
			POSCMOD_NONE)


/*=================================================================================================
�O���[�o���ϐ�
=================================================================================================*/

/* ���� */
const INFO_T info = {
	"OS10A708M01",							/* ��V���A���ԍ� */

	"OpenSSM Rev.A",						/* ��^�� */
	2010, 5, 12,							/* ��݌v������ (������/���b�g�Ƃ͖��֌W) */
	1, 0, 0,								/* ��o�[�W���� */
	"Yosuke FURUSAWA",						/* ��J���� */

	"OpenSSM Firmware",						/* �t�@�[���E�F�A���� */
	2010, 11, 21,							/* �t�@�[���E�F�A�ŏI�X�V�N���� */
	1, 0, 5,								/* �t�@�[���E�F�A�o�[�W���� */
	"Yosuke FURUSAWA",						/* �t�@�[���E�F�A�J���� */

	"Project WinSSM & OpenSSM",
	"http://ssm.nextfoods.jp/",
	"g-ssm@nextfoods.jp",
	"(C) 2007 - 2010 Y.FURUSAWA",
};


/**************************************************************************************************
�X�^�[�g�A�b�v�֐�
**************************************************************************************************/
int main(void){
	unsigned int tick;

	/* �|�[�g�̓��o�̓��[�h�ݒ� */
	/*          FEDCBA9876543210 */
	TRISA	= 0b0000000000000011;
	TRISB	= 0b0000111101100001;
	LED_OFF();
	KXM_OFF();

	/* CPU�N���b�N��ݒ肷��. �I�[�o�[�N���b�N���́A��ʕ`�悪�኱�����Ȃ� */
	/* CPU�̌̍�������Ǝv����̂ŁA�\���Ɍ��؂��s������ */
	CLKDIV = 0;
//	OSCTUN = 0b0000000000011111;	/* Fosc = 8.96MHz, Fcy = 17.92MHz, Core 35.84MHz */
	OSCTUN = 0b0000000000000000;	/* Fosc = 8.00MHz, Fcy = 16.00MHz, Core 32.00MHz */

	/* CPU�̊��荞�݃��x���ݒ� */
	SRbits.IPL = 0;

	/* �d�������肷��܂ő҂� */
	RTC_init();
	ADC_init();
	tick = rtc.tick;
	while(RTC_get_ticks(tick, rtc.tick) < 1000) ClrWdt();

	KXM_ON();

	/* �e�h���C�o/���W���[��/�����֐� libxxx �̏����� */
	FONTX2_init();
	VIDEO_init();
	UART1_init(115200);
	UART2_init(  4800);
	DAC_init();
	PS2_init();

	/* �A�v���P�[�V�����w�̏����� */
	SSM_init();
	SCREEN_init();
	EXTMETER_init(EXTMETER_BOOST);
	CONFIG_init();

	/* WDT���Z�b�g���ꂽ�Ƃ��A�������[�h���Ȃ� */
	if (RCONbits.WDTO){
		VIDEO_locate( 2, 1);
		VIDEO_putstr("WDT Error!");
		VIDEO_locate( 2, 2);
		VIDEO_putstr("Initializing systems...");
		while(RTC_get_ticks(tick, rtc.tick) < 50000) ClrWdt();

	/* �����x�Z���T�������Ƃ��A�������[�h���Ȃ� */
	} else if (adc.adc[0] < 20 && adc.adc[1] < 20 && adc.adc[2] < 20) {
		VIDEO_locate( 2, 1);
		VIDEO_putstr("Can't find Accelerometer.");
		VIDEO_locate( 2, 2);
		VIDEO_putstr("Initializing systems...");
		while(RTC_get_ticks(tick, rtc.tick) < 50000) ClrWdt();


	/* ���ݒ�̎������[�h */
	} else {
		VIDEO_locate( 2, 1);
		VIDEO_putstr("Now loading configurations...");
		VIDEO_locate( 2, 2);
		if(CONFIG_load()){
			VIDEO_putstr("Success");
			while(RTC_get_ticks(tick, rtc.tick) <  5000) ClrWdt();
		} else {
			VIDEO_putstr("Error");
			while(RTC_get_ticks(tick, rtc.tick) < 50000) ClrWdt();
		}
	}

	/* ���C�����[�v */
	while(1){
		ClrWdt();

		/* LED��_�ł�����BCPU���ׂ������Ƃ��A�t���b�V�����x���Ȃ� */
		GPIO_LED = ~GPIO_LED;

		/* �V���v���ȃ��E���h���r���̃}���`�^�X�N (^^; */
		SSM_main();
		SCREEN_main();
		PS2_main();
		EXTMETER_main();
	}

	/* �E�H�b�`�h�b�N�^�C�}�ɂ�郊�Z�b�g�𔭐������� */
	while(1);
	return(0);
}


