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
ヘッダファイルをインクルード
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
コンフィグレーション
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
グローバル変数
=================================================================================================*/

/* 基板情報 */
const INFO_T info = {
	"OS10A708M01",							/* 基板シリアル番号 */

	"OpenSSM Rev.A",						/* 基板型番 */
	2010, 5, 12,							/* 基板設計完了日 (製造日/ロットとは無関係) */
	1, 0, 0,								/* 基板バージョン */
	"Yosuke FURUSAWA",						/* 基板開発者 */

	"OpenSSM Firmware",						/* ファームウェア名称 */
	2010, 11, 21,							/* ファームウェア最終更新年月日 */
	1, 0, 5,								/* ファームウェアバージョン */
	"Yosuke FURUSAWA",						/* ファームウェア開発者 */

	"Project WinSSM & OpenSSM",
	"http://ssm.nextfoods.jp/",
	"g-ssm@nextfoods.jp",
	"(C) 2007 - 2010 Y.FURUSAWA",
};


/**************************************************************************************************
スタートアップ関数
**************************************************************************************************/
int main(void){
	unsigned int tick;

	/* ポートの入出力モード設定 */
	/*          FEDCBA9876543210 */
	TRISA	= 0b0000000000000011;
	TRISB	= 0b0000111101100001;
	LED_OFF();
	KXM_OFF();

	/* CPUクロックを設定する. オーバークロック時は、画面描画が若干早くなる */
	/* CPUの個体差があると思われるので、十分に検証を行うこと */
	CLKDIV = 0;
//	OSCTUN = 0b0000000000011111;	/* Fosc = 8.96MHz, Fcy = 17.92MHz, Core 35.84MHz */
	OSCTUN = 0b0000000000000000;	/* Fosc = 8.00MHz, Fcy = 16.00MHz, Core 32.00MHz */

	/* CPUの割り込みレベル設定 */
	SRbits.IPL = 0;

	/* 電源が安定するまで待つ */
	RTC_init();
	ADC_init();
	tick = rtc.tick;
	while(RTC_get_ticks(tick, rtc.tick) < 1000) ClrWdt();

	KXM_ON();

	/* 各ドライバ/モジュール/割込関数 libxxx の初期化 */
	FONTX2_init();
	VIDEO_init();
	UART1_init(115200);
	UART2_init(  4800);
	DAC_init();
	PS2_init();

	/* アプリケーション層の初期化 */
	SSM_init();
	SCREEN_init();
	EXTMETER_init(EXTMETER_BOOST);
	CONFIG_init();

	/* WDTリセットされたとき、自動ロードしない */
	if (RCONbits.WDTO){
		VIDEO_locate( 2, 1);
		VIDEO_putstr("WDT Error!");
		VIDEO_locate( 2, 2);
		VIDEO_putstr("Initializing systems...");
		while(RTC_get_ticks(tick, rtc.tick) < 50000) ClrWdt();

	/* 加速度センサが無いとき、自動ロードしない */
	} else if (adc.adc[0] < 20 && adc.adc[1] < 20 && adc.adc[2] < 20) {
		VIDEO_locate( 2, 1);
		VIDEO_putstr("Can't find Accelerometer.");
		VIDEO_locate( 2, 2);
		VIDEO_putstr("Initializing systems...");
		while(RTC_get_ticks(tick, rtc.tick) < 50000) ClrWdt();


	/* 環境設定の自動ロード */
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

	/* メインループ */
	while(1){
		ClrWdt();

		/* LEDを点滅させる。CPU負荷が高いとき、フラッシュが遅くなる */
		GPIO_LED = ~GPIO_LED;

		/* シンプルなラウンドロビンのマルチタスク (^^; */
		SSM_main();
		SCREEN_main();
		PS2_main();
		EXTMETER_main();
	}

	/* ウォッチドックタイマによるリセットを発生させる */
	while(1);
	return(0);
}


