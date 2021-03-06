/**************************************************************************************************
	Title			: Screen Task
	Programmer		: Yosuke FURUSAWA.
	Copyright		: Copyright (C) 2008-2010 Yosuke FURUSAWA.
	License			: 4-clause BSD License
	Since			: 2008/12/15

	Filename		: screen.c
	Last up date	: 2010/11/21
	Kanji-Code		: Shift-JIS
	TAB Space		: 4
**************************************************************************************************/


/* Note:
F1 ～ F12, PageUp, PageDownは、画面切り替えキーとしてシステムの予約とする
DELは、キーリピートの変更キーとして予約

キー入力処理については、再設計の余地あり...
作りこむ時間がありませんでした...
*/


/*================================================================================================
ヘッダファイルをインクルード
=================================================================================================*/
#include <p24FJ64GA002.h>

#include "types.h"

#include "libvideo.h"
#include "libuart.h"
#include "librtc.h"
#include "libadc.h"
#include "libdac.h"
#include "libeeprom.h"
#include "libps2.h"

#include "table.h"
#include "ssm.h"
#include "main.h"
#include "extmeter.h"

#include "screen.h"
#include "config.h"


/*================================================================================================
マクロ定義
=================================================================================================*/
#define abs(a)						(((a)>0) ? (a) : -(a))


/*================================================================================================
グローバル変数
=================================================================================================*/
/* 起動時の表示画面を保存するための変数. cf SCREEN_main(), SCREEN_setup_config() */
static unsigned int backup;

SCREEN_T screen;


static METER_T meter[6];
static GRAPH_T graph;
static TRACK_T track;


/*================================================================================================
プロトタイプ宣言
=================================================================================================*/
static  void SCREEN_meter1(void);
static  void SCREEN_meter2(void);
static  void SCREEN_meter3(void);
static  void SCREEN_meter4(void);
static  void SCREEN_meter5(void);
static  void SCREEN_meter6(void);
static  void SCREEN_setup_ssm(void);
static  void SCREEN_setup_video(void);
static  void SCREEN_setup_extmeter(void);
static  void SCREEN_setup_config(void);
static  void SCREEN_debug(void);
static  void SCREEN_version(void);
static  void SCREEN_lifegame(void);

static  void SCREEN_keybuf_clear(void);
static  unsigned char SCREEN_key_get(void);


/**************************************************************************************************
画面描画タスクの初期化
**************************************************************************************************/
void SCREEN_init(void)
{
	screen.screen = 0x01;
	screen.screen_flag = SCREEN_INIT;
	screen.fps = 7;

	screen.track[0] = 2;
	screen.track[1] = 1;

	return;
}


/**************************************************************************************************
画面描画タスクの初期化

関数ポインタを使うと、ちょっと速くなる.
**************************************************************************************************/
BOOL SCREEN_main(void)
{
	unsigned char key;
	static unsigned char watch = 0;
	static unsigned int tick = 0;

	if(RTC_get_ticks(tick, rtc.tick) < (10000 / screen.fps)) return(FALSE);
	tick = rtc.tick;

	/* 画面セレクト. F1 ～ F12の 12画面. うぉ、DOSっぽい */
	key = PS2_key_check();
	if(key >= KEY_F1 && key <= KEY_F12){
		screen.screen_flag = SCREEN_INIT;
		switch(PS2_key_get()){
		case KEY_F1:	screen.screen = 0x01;	break;		/* 3min NG */
		case KEY_F2:	screen.screen = 0x02;	break;		/* 9min OK */
		case KEY_F3:	screen.screen = 0x03;	break;		/* 21sec NG */
		case KEY_F4:	screen.screen = 0x04;	break;		/* 10min OK */
		case KEY_F5:	screen.screen = 0x05;	break;		/* 11min OK */
		case KEY_F6:	screen.screen = 0x06;	break;		/* 10min OK */
		case KEY_F7:	screen.screen = 0x07;	break;		/* 7sec NG */
		case KEY_F8:	screen.screen = 0x08;	break;
		case KEY_F9:	screen.screen = 0x09;	break;
		case KEY_F10:	screen.screen = 0x0a;	break;
		case KEY_F11:	screen.screen = 0x0b;	break;
		case KEY_F12:	screen.screen = 0x0c;	break;
		}

	/* テンキー・キーボード用 */
	} else if(key == KEY_PAGEDOWN){
		PS2_key_get();
		screen.screen_flag = SCREEN_INIT;
		screen.screen--;
		if(screen.screen < 0x00) screen.screen = 0x00;

	} else if(key == KEY_PAGEUP){
		PS2_key_get();
		screen.screen_flag = SCREEN_INIT;
		screen.screen++;
		if(screen.screen > 0x0c) screen.screen = 0x0c;

	/* おまけ : キーリピートを切り替える */
	} else if(key == KEY_DEL){
		if(ps2.wait != 2000){
			ps2.wait = 2000;
			VIDEO_locate(0,1);
			VIDEO_putstr("Slow");
		} else {
			ps2.wait =  500;
			VIDEO_locate(0,1);
			VIDEO_putstr("Fast");
		}
		PS2_key_buf_clear();

	/* おまけ : クイックセーブ */
	} else if(key == KEY_HOME){
		PS2_key_get();
		VIDEO_locate(0,1);
		VIDEO_putstr("Saving...");
		VIDEO_locate(0,1);
		screen.screen_flag = SCREEN_INIT;
		if(CONFIG_save()){
			VIDEO_putstr("         ");
		} else {
			VIDEO_putstr("Error... ");
		}
		screen.screen_flag = SCREEN_VIEW;

	/* おまけ : 内部時計 */
	} else if(key == KEY_END){
		PS2_key_get();
		watch++;
	}

	/* 時計表示 */
	if(watch % 2){
		VIDEO_locate( 14,0);
		VIDEO_putuint(rtc.day, 5);
		VIDEO_putch(' ');
		VIDEO_putuint(rtc.hour, 2);
		VIDEO_putch(':');
		VIDEO_putuint(rtc.min, 2);
		VIDEO_putch(':');
		VIDEO_putuint(rtc.sec, 2);
		VIDEO_putch('.');
		VIDEO_putuint(rtc.msec, 3);
	}

	/* 画面を表示する */
	switch(screen.screen){
	case 0x01:	SCREEN_meter1();				break;
	case 0x02:	SCREEN_meter2();				break;
	case 0x03:	SCREEN_meter3();				break;
	case 0x04:	SCREEN_meter4();				break;
	case 0x05:	SCREEN_meter5();				break;
	case 0x06:	SCREEN_meter6();				break;
	case 0x07:	SCREEN_setup_ssm();				break;
	case 0x08:	SCREEN_setup_video();			break;
	case 0x09:	SCREEN_setup_extmeter();		break;
	case 0x0a:	SCREEN_setup_config();			break;
	case 0x0b:	SCREEN_debug();					break;
	case 0x0c:	SCREEN_version();				break;
	default:	screen.screen = 0x01;			break;			/* ここにきたらバグ */
	}

	/* 起動時の表示画面保存用 */
	if(screen.screen < 0x07) backup = screen.screen;

	return(TRUE);
}


/*-------------------------------------------------------------------------------------------------
メータ表示1
-------------------------------------------------------------------------------------------------*/
static  void SCREEN_meter1(void)
{
	unsigned int tmp;
	int x, y;

	/* 画面初期化 */
	if(screen.screen_flag == SCREEN_INIT){
		screen.screen_flag = SCREEN_VIEW;
		screen.fps = 10;
		VIDEO_vram_clear(0x00);

		VIDEO_locate( 0, 0);	VIDEO_putstr("SSM Multi Monitor");

		VIDEO_locate( 7, 2);	VIDEO_putstr("rpm");
		VIDEO_locate( 7, 3);	VIDEO_putstr("km/h");
		VIDEO_locate( 7, 4);	VIDEO_putch ('%');
		VIDEO_locate( 7, 5);	VIDEO_putch ('s');

		VIDEO_locate(17, 2);	VIDEO_putstr("WAT");
		VIDEO_locate(17, 3);	VIDEO_putstr("AIR");
		VIDEO_locate(17, 4);	VIDEO_putch ('V');
		VIDEO_locate(17, 5);	VIDEO_putstr("km/L");

		VIDEO_locate(29, 2);	VIDEO_putstr("MAF");
		VIDEO_locate(29, 3);	VIDEO_putstr("AFR");
		VIDEO_locate(29, 4);	VIDEO_putstr("KNO");
		VIDEO_locate(29, 5);	VIDEO_putstr("IGN");

		VIDEO_locate( 6, 9);	VIDEO_putstr("kg/cm2");

		METER_init(&meter[0],  50, 50, 50);
		TRACK_init(&track,    105,  0, NTSC_WIDTH - 105, 90);
	}

	/* キーボード入力処理 */
	switch(SCREEN_key_get()){
	case KEY_UP:
	case KEY_DOWN:	screen.track[0] = (screen.track[0] + 1) % 3;	break;
	case KEY_LEFT:
	case KEY_RIGHT:	screen.track[1] = (screen.track[1] + 1) % 3;	break;
	default:														break;
	}

	/* ±2G -> ±1Gへ変換. あんまり賢くないコード... */
	x = (adc.adc[ screen.track[0] ] << 1) - 1024;
	y = (adc.adc[ screen.track[1] ] << 1) - 1024;
	x = -1 * x;
	if(x >  512) x =  512;
	if(x < -512) x = -512;
	if(y >  512) y =  512;
	if(y < -512) y = -512;
	x = (x + 512) / 10;
	y = (y + 512) / 10;

	/* 表示 */
	TRACK_putdata(&track, x, y);
	TRACK_draw_point(&track);
/*
	VIDEO_locate(31,10);	VIDEO_putuint(screen.track[0], 1);
	VIDEO_locate(31,11);	VIDEO_putuint(screen.track[1], 1);
*/
	VIDEO_locate( 2, 2);	VIDEO_putuint(ssm_data.engine, 4);
	VIDEO_locate( 3, 3);	VIDEO_putuint(ssm_data.speed, 3);
	VIDEO_locate( 0, 4);	VIDEO_putdouble(ssm_data.throttle, 3, 1);
	VIDEO_locate( 5, 5);	VIDEO_putuint(ssm_data.shift, 1);

	VIDEO_locate(12, 2);	VIDEO_putint(ssm_data.coolant, 3);
	VIDEO_locate(12, 3);	VIDEO_putint(ssm_data.intakeair, 3);
	VIDEO_locate(11, 4);	VIDEO_putdouble(ssm_data.battery, 2, 1);
	VIDEO_locate(10, 5);	VIDEO_putdouble(ssm_data.fuel, 3, 1);

	VIDEO_locate(21, 2);	VIDEO_putdouble(ssm_data.maf, 4, 1);
	VIDEO_locate(21, 3);	VIDEO_putdouble(ssm_data.afr, 4, 1);
	VIDEO_locate(23, 4);	VIDEO_putdouble(ssm_data.knock, 2, 1);
	VIDEO_locate(23, 5);	VIDEO_putdouble(ssm_data.ignition, 2, 1);

	VIDEO_locate( 4, 8);	VIDEO_putdouble(ssm_data.boost, 1, 2);

	if(ssm_data.boost <= 0)	tmp = (ssm_data.boost + 1.0) * 33.333;
	else					tmp = 33.0 + ssm_data.boost  * 44.666;
	METER_draw(&meter[0], tmp);

	return;
}


/*-------------------------------------------------------------------------------------------------
メータ表示2
-------------------------------------------------------------------------------------------------*/
static  void SCREEN_meter2(void)
{
	unsigned int tmp;

	/* 画面初期化 */
	if(screen.screen_flag == SCREEN_INIT){
		screen.screen_flag = SCREEN_VIEW;
		screen.fps = 10;
		VIDEO_vram_clear(0x00);

		VIDEO_locate( 0,  0);	VIDEO_putstr("Multi Meter");
		VIDEO_locate( 6,  4);	VIDEO_putstr("rpm");
		VIDEO_locate(18,  4);	VIDEO_putch ('%');
		VIDEO_locate(25,  4);	VIDEO_putstr("kg/cm2");
		VIDEO_locate( 4, 10);	VIDEO_putstr("km/h");
		VIDEO_locate(15, 10);	VIDEO_putstr("Water");
		VIDEO_locate(28, 10);	VIDEO_putstr("Air");

		METER_init(&meter[0],  40, 128, 40);
		METER_init(&meter[1], 128, 128, 40);
		METER_init(&meter[2], 215, 128, 40);
		METER_init(&meter[3],  40,  40, 40);
		METER_init(&meter[4], 128,  40, 40);
		METER_init(&meter[5], 215,  40, 40);
	}


	/* キーボード入力処理 */
	SCREEN_keybuf_clear();

	VIDEO_locate( 4, 3);	VIDEO_putuint(ssm_data.engine, 4);
	tmp = ((unsigned long)ssm_data.engine * 100) / 9999;
	METER_draw(&meter[0],    tmp);

	VIDEO_locate(16, 3);	VIDEO_putuint(ssm_data.throttle, 3);
	METER_draw(&meter[1],    ssm_data.throttle);

	VIDEO_locate(25, 3);	VIDEO_putdouble(ssm_data.boost, 0, 2);
	if(ssm_data.boost <= 0)	tmp = (ssm_data.boost + 1.0) * 33.333;
	else					tmp = 33.0 + ssm_data.boost  * 44.666;
	METER_draw(&meter[2],    tmp);

	VIDEO_locate( 5, 9);	VIDEO_putuint(ssm_data.speed, 3);
	tmp = ((unsigned int)ssm_data.speed * 100) / 255;
	METER_draw(&meter[3],    tmp);


	VIDEO_locate(15, 9);	VIDEO_putint(ssm_data.coolant, 3);
	tmp = (((unsigned int)ssm_data.coolant + 40) * 100) / 255;
	METER_draw(&meter[4],    tmp);

	VIDEO_locate(26, 9);	VIDEO_putint(ssm_data.intakeair, 3);
	tmp = (((unsigned int)ssm_data.intakeair + 40) * 100) / 255;
	METER_draw(&meter[5],    tmp);

	return;
}


/*-------------------------------------------------------------------------------------------------
メータ表示3
-------------------------------------------------------------------------------------------------*/
static  void SCREEN_meter3(void)
{
	unsigned int tmp;

	/* 画面初期化 */
	if(screen.screen_flag == SCREEN_INIT){
		screen.screen_flag = SCREEN_VIEW;
		screen.fps = 10;
		VIDEO_vram_clear(0x00);

		VIDEO_locate( 0, 0);	VIDEO_putstr("Fuel Consumption Monitor");

		VIDEO_locate( 5, 4);	VIDEO_putstr("rpm");
		VIDEO_locate(13, 4);	VIDEO_putstr("km/h");
		VIDEO_locate( 2, 9);	VIDEO_putstr("kg/cm2");
		VIDEO_locate(15, 9);	VIDEO_putstr("%");
		VIDEO_locate( 5,11);	VIDEO_putstr("WAT");
		VIDEO_locate(14,11);	VIDEO_putstr("AIR");

		METER_init(&meter[0],  36, 132, 36);
		METER_init(&meter[1], 112, 132, 36);
		METER_init(&meter[2],  36,  57, 36);
		METER_init(&meter[3], 112,  57, 36);

		VIDEO_locate(28, 2);	VIDEO_putstr("Fe/h");
		VIDEO_locate(28, 3);	VIDEO_putstr("ml/s");
		VIDEO_locate(28, 4);	VIDEO_putstr("km/L");
		GRAPH_init(&graph, 152,   0, GRAPH_SIZE, 100);
	}

	/* キーボード入力処理 */
	SCREEN_keybuf_clear();

	VIDEO_locate( 3, 3);	VIDEO_putuint(ssm_data.engine, 4);
	tmp = ((unsigned long)ssm_data.engine * 100) / 9999;
	METER_draw(&meter[0],    tmp);

	VIDEO_locate(13, 3);	VIDEO_putuint(ssm_data.speed, 3);
	tmp = ((unsigned int)ssm_data.speed * 100) / 255;
	METER_draw(&meter[1],    tmp);

	VIDEO_locate( 2, 8);	VIDEO_putdouble(ssm_data.boost, 0, 2);
	if(ssm_data.boost <= 0)	tmp = (ssm_data.boost + 1.0) * 33.333;
	else					tmp = 33.0 + ssm_data.boost  * 44.666;
	METER_draw(&meter[2],    tmp);
	
	VIDEO_locate(13, 8);	VIDEO_putuint(ssm_data.throttle, 3);
	METER_draw(&meter[3],    ssm_data.throttle);

	VIDEO_locate( 0,11);	VIDEO_putint(ssm_data.coolant, 3);
	VIDEO_locate( 9,11);	VIDEO_putint(ssm_data.intakeair, 3);

	VIDEO_locate(18, 2);	VIDEO_putdouble((ssm_data.fuel_rate * (double)ssm.price * 3.6), 4, 3);
	VIDEO_locate(19, 3);	VIDEO_putdouble( ssm_data.fuel_rate, 3, 3);
	VIDEO_locate(19, 4);	VIDEO_putdouble( ssm_data.fuel, 3, 3);
	GRAPH_putdata(&graph, (unsigned int)(ssm_data.fuel * 3.0));
	GRAPH_draw_line(&graph);

	return;
}


/*-------------------------------------------------------------------------------------------------
メータ表示4
-------------------------------------------------------------------------------------------------*/
static  void SCREEN_meter4(void)
{
	unsigned int tmp;

	/* 画面初期化 */
	if(screen.screen_flag == SCREEN_INIT){
		screen.screen_flag = SCREEN_VIEW;
		screen.fps = 10;
		VIDEO_vram_clear(0x00);

		VIDEO_locate(20, 9);	VIDEO_putstr("rpm");
		VIDEO_locate(20,10);	VIDEO_putstr("km/h");
		VIDEO_locate(20,11);	VIDEO_putstr("km/L");

		VIDEO_locate(31, 9);	VIDEO_putch('%');
		VIDEO_locate(31,10);	VIDEO_putch('C');
		VIDEO_locate(31,11);	VIDEO_putch('s');

		METER_init(&meter[0], 50, 50, 50);
		VIDEO_locate( 5,10);	VIDEO_putstr("kg/cm2");
	}

	/* キーボード入力処理 */
	SCREEN_keybuf_clear();

	/* 表示 */
	if(ssm_data.boost <= 0)	tmp = (ssm_data.boost + 1.0) * 33.333;
	else					tmp = 33.0 + ssm_data.boost  * 44.666;
	METER_draw(&meter[0], tmp);

	VIDEO_locate( 4, 9);	VIDEO_putdouble(ssm_data.boost, 0, 2);


	VIDEO_locate(15, 9);	VIDEO_putuint(ssm_data.engine, 4);
	VIDEO_locate(16,10);	VIDEO_putuint(ssm_data.speed, 3);
	VIDEO_locate(13,11);	VIDEO_putdouble(ssm_data.fuel, 3, 1);


	VIDEO_locate(24, 9);	VIDEO_putdouble(ssm_data.throttle, 3, 1);
	VIDEO_locate(27,10);	VIDEO_putuint(ssm_data.coolant, 3);
	VIDEO_locate(29,11);	VIDEO_putuint(ssm_data.shift, 1);

	return;
}


/*-------------------------------------------------------------------------------------------------
メータ表示5
-------------------------------------------------------------------------------------------------*/
static  void SCREEN_meter5(void)
{
	int x, y;

	/* 画面初期化 */
	if(screen.screen_flag == SCREEN_INIT){
		screen.screen_flag = SCREEN_VIEW;
		screen.fps = 10;
		VIDEO_vram_clear(0x00);

		VIDEO_locate(20, 9);	VIDEO_putstr("rpm");
		VIDEO_locate(20,10);	VIDEO_putstr("km/h");
		VIDEO_locate(20,11);	VIDEO_putstr("kg/cm2");

		VIDEO_locate(31, 9);	VIDEO_putch('%');
		VIDEO_locate(31,10);	VIDEO_putch('C');
		VIDEO_locate(31,11);	VIDEO_putch('s');

		TRACK_init(&track, 0, 0, 100, 50);
	}

	/* キーボード入力処理 */
	switch(SCREEN_key_get()){
	case KEY_UP:
	case KEY_DOWN:	screen.track[0] = (screen.track[0] + 1) % 3;	break;
	case KEY_LEFT:
	case KEY_RIGHT:	screen.track[1] = (screen.track[1] + 1) % 3;	break;
	default:											break;
	}

	/* ±2G -> ±1Gへ変換. あんまり賢くないコード... */
	x = (adc.adc[ screen.track[0] ] << 1) - 1024;
	y = (adc.adc[ screen.track[1] ] << 1) - 1024;
	x = -1 * x;
	if(x >  512) x =  512;
	if(x < -512) x = -512;
	if(y >  512) y =  512;
	if(y < -512) y = -512;
	x = (x + 512) / 10;
	y = (y + 512) / 10;

	/* 表示 */
	TRACK_putdata(&track, (1024 - adc.adc[ screen.track[0] ]) / 10, adc.adc[ screen.track[1] ] / 10);
	TRACK_draw_point(&track);

	VIDEO_locate(13, 9);	VIDEO_putuint(screen.track[0], 1);
	VIDEO_locate(13,10);	VIDEO_putuint(screen.track[1], 1);

	VIDEO_locate(15, 9);	VIDEO_putuint(ssm_data.engine, 4);
	VIDEO_locate(16,10);	VIDEO_putuint(ssm_data.speed, 3);
	VIDEO_locate(14,11);	VIDEO_putdouble(ssm_data.boost, 0, 2);

	VIDEO_locate(24, 9);	VIDEO_putdouble(ssm_data.throttle, 3, 1);
	VIDEO_locate(27,10);	VIDEO_putuint(ssm_data.coolant, 3);
	VIDEO_locate(29,11);	VIDEO_putuint(ssm_data.shift, 1);


	return;
}


/*-------------------------------------------------------------------------------------------------
メータ表示6
-------------------------------------------------------------------------------------------------*/
static  void SCREEN_meter6(void)
{
	/* 画面初期化 */
	if(screen.screen_flag == SCREEN_INIT){
		screen.screen_flag = SCREEN_VIEW;
		screen.fps = 10;
		VIDEO_vram_clear(0x00);

		VIDEO_locate( 9,11);	VIDEO_putstr("km/L");

		VIDEO_locate(20, 9);	VIDEO_putstr("rpm");
		VIDEO_locate(20,10);	VIDEO_putstr("km/h");
		VIDEO_locate(20,11);	VIDEO_putstr("kg/cm2");

		VIDEO_locate(31, 9);	VIDEO_putch('%');
		VIDEO_locate(31,10);	VIDEO_putch('C');
		VIDEO_locate(31,11);	VIDEO_putch('s');

		GRAPH_init(&graph, 0, 16, GRAPH_SIZE, 32);
	}

	/* キーボード入力処理 */
	SCREEN_keybuf_clear();

	/* 表示 */
	GRAPH_putdata(&graph, (unsigned int)ssm_data.fuel * 3);
	GRAPH_draw_line(&graph);
	VIDEO_locate(2,11);		VIDEO_putdouble(ssm_data.fuel, 3, 1);

	VIDEO_locate(15, 9);	VIDEO_putuint(ssm_data.engine, 4);
	VIDEO_locate(16,10);	VIDEO_putuint(ssm_data.speed, 3);
	VIDEO_locate(14,11);	VIDEO_putdouble(ssm_data.boost, 0, 2);

	VIDEO_locate(24, 9);	VIDEO_putdouble(ssm_data.throttle, 3, 1);
	VIDEO_locate(27,10);	VIDEO_putuint(ssm_data.coolant, 3);
	VIDEO_locate(29,11);	VIDEO_putuint(ssm_data.shift, 1);

	return;
}


/*-------------------------------------------------------------------------------------------------
SSMの設定
-------------------------------------------------------------------------------------------------*/
static  void SCREEN_setup_ssm(void)
{
	static char cursol;
	char run = 0;
	char i;

	/* 画面初期化 */
	if(screen.screen_flag == SCREEN_INIT){
		screen.screen_flag = SCREEN_VIEW;
		screen.fps = 20;
		VIDEO_vram_clear(0x00);
		cursol = 0;

		VIDEO_locate( 0, 0);	VIDEO_putstr("SSM Setup");
		VIDEO_locate(16, 0);	VIDEO_putstr("UART1");
		VIDEO_locate(24, 0);	VIDEO_putstr("UART2");

		VIDEO_locate( 1, 2);	VIDEO_putstr("MODE");

		VIDEO_locate( 1, 4);	VIDEO_putstr("TIRE Width");
		VIDEO_locate( 1, 5);	VIDEO_putstr("TIRE Flat");
		VIDEO_locate( 1, 6);	VIDEO_putstr("TIRE Inch");
		VIDEO_locate( 1, 7);	VIDEO_putstr("TIRE Circle");
		VIDEO_locate( 1, 8);	VIDEO_putstr("Fuel Price");
		VIDEO_locate( 1, 9);	VIDEO_putstr("SSM Wait");
		VIDEO_locate( 1,10);	VIDEO_putstr("SSM Cycle");
		VIDEO_locate( 1,11);	VIDEO_putstr("SSM Error");

		VIDEO_locate(20, 4);	VIDEO_putstr("Final");
		VIDEO_locate(20, 5);	VIDEO_putstr("1st");
		VIDEO_locate(20, 6);	VIDEO_putstr("2nd");
		VIDEO_locate(20, 7);	VIDEO_putstr("3rd");
		VIDEO_locate(20, 8);	VIDEO_putstr("4th");
		VIDEO_locate(20, 9);	VIDEO_putstr("5th");
		VIDEO_locate(20,10);	VIDEO_putstr("6th");
		VIDEO_locate(20,11);	VIDEO_putstr("7th");
	}

	/* キーボード入力処理 */
	switch(SCREEN_key_get()){
	case KEY_UP:		cursol--;		break;
	case KEY_DOWN:		cursol++;		break;
	case KEY_LEFT:		run = -1;		break;
	case KEY_RIGHT:		run =  1;		break;
	default:							break;
	}

	/* カーソル確認/表示 */
	if(cursol <  0) cursol =  0;
	if(cursol > 16) cursol = 16;

	VIDEO_locate(0, 2);
	if(cursol == 0)		VIDEO_putch('>');
	else				VIDEO_putch(' ');

	for(i = 1; i < 9; i++){
		VIDEO_locate(0, 3 + i);
		if(cursol == i) VIDEO_putch('>');
		else			VIDEO_putch(' ');
	}
	for(; i < 17; i++){
		VIDEO_locate(19, i - 5);
		if(cursol == i) VIDEO_putch('>');
		else			VIDEO_putch(' ');
	}

	/* 実行 */
	if(run == -1 || run == 1){
		switch(cursol){
		case  0:
			if(run == -1)	ssm.mode = SSM_MODE_OPENSSM;
			else			ssm.mode = SSM_MODE_OPENPORT;
			break;

		case  1:	ssm.tire_width += run;						break;
		case  2:	ssm.tire_flat += run;						break;
		case  3:	ssm.tire_inch += run;						break;

		case  5:	ssm.price += run;							break;
		case  6:	ssm.wait += run;							break;

		case  9:	ssm.gear_ratio[0] += (double)run * 0.001;	break;
		case 10:	ssm.gear_ratio[1] += (double)run * 0.001;	break;
		case 11:	ssm.gear_ratio[2] += (double)run * 0.001;	break;
		case 12:	ssm.gear_ratio[3] += (double)run * 0.001;	break;
		case 13:	ssm.gear_ratio[4] += (double)run * 0.001;	break;
		case 14:	ssm.gear_ratio[5] += (double)run * 0.001;	break;
		case 15:	ssm.gear_ratio[6] += (double)run * 0.001;	break;
		case 16:	ssm.gear_ratio[7] += (double)run * 0.001;	break;
		default:												break;
		}

		ssm.tire_circle = SSM_TIRE_R(ssm.tire_width, ssm.tire_flat, ssm.tire_inch);
	}


	/* 表示 */
	VIDEO_locate( 6, 2);
	switch(ssm.mode){
	case SSM_MODE_OPENSSM:	VIDEO_putstr("OpenSSM ");	break;
	case SSM_MODE_OPENPORT:
	default:
							VIDEO_putstr("OpenPort");	break;
	}

	VIDEO_locate(16, 1);	VIDEO_putdouble(UART1_get_baud() / 1000.0, 3, 1);	VIDEO_putch('k');
	VIDEO_locate(25, 1);	VIDEO_putdouble(UART2_get_baud() / 1000.0, 3, 1);	VIDEO_putch('k');

	VIDEO_locate(16, 2);	VIDEO_putuint(UART1_get_sendbuf(), 3);	VIDEO_putch('/');	VIDEO_putuint(UART1_TX_BUFFER_SIZE, 3);
	VIDEO_locate(16, 3);	VIDEO_putuint(UART1_get_recvbuf(), 3);	VIDEO_putch('/');	VIDEO_putuint(UART1_RX_BUFFER_SIZE, 3);

	VIDEO_locate(25, 2);	VIDEO_putuint(UART2_get_sendbuf(), 3);	VIDEO_putch('/');	VIDEO_putuint(UART2_TX_BUFFER_SIZE, 3);
	VIDEO_locate(25, 3);	VIDEO_putuint(UART2_get_recvbuf(), 3);	VIDEO_putch('/');	VIDEO_putuint(UART2_RX_BUFFER_SIZE, 3);

	VIDEO_locate(14, 4);	VIDEO_putuint(ssm.tire_width, 4);
	VIDEO_locate(15, 5);	VIDEO_putuint(ssm.tire_flat, 3);
	VIDEO_locate(15, 6);	VIDEO_putuint(ssm.tire_inch, 3);
	VIDEO_locate(13, 7);	VIDEO_putuint(ssm.tire_circle, 5);
	VIDEO_locate(13, 8);	VIDEO_putuint(ssm.price, 5);
	VIDEO_locate(13, 9);	VIDEO_putuint(ssm.wait, 5);
	VIDEO_locate(13,10);	VIDEO_putuint(ssm.cycle, 5);
	VIDEO_locate(13,11);	VIDEO_putuint(ssm.error, 5);

	VIDEO_locate(26, 4);	VIDEO_putdouble(ssm.gear_ratio[0], 1, 3);
	VIDEO_locate(26, 5);	VIDEO_putdouble(ssm.gear_ratio[1], 1, 3);
	VIDEO_locate(26, 6);	VIDEO_putdouble(ssm.gear_ratio[2], 1, 3);
	VIDEO_locate(26, 7);	VIDEO_putdouble(ssm.gear_ratio[3], 1, 3);
	VIDEO_locate(26, 8);	VIDEO_putdouble(ssm.gear_ratio[4], 1, 3);
	VIDEO_locate(26, 9);	VIDEO_putdouble(ssm.gear_ratio[5], 1, 3);
	VIDEO_locate(26,10);	VIDEO_putdouble(ssm.gear_ratio[6], 1, 3);
	VIDEO_locate(26,11);	VIDEO_putdouble(ssm.gear_ratio[7], 1, 3);

	return;
}


/*-------------------------------------------------------------------------------------------------
ビデオの設定
-------------------------------------------------------------------------------------------------*/
static  void SCREEN_setup_video(void)
{
	static char cursol;
	char run = 0;
	unsigned int i;

	/* 画面初期化 */
	if(screen.screen_flag == SCREEN_INIT){
		screen.screen_flag = SCREEN_VIEW;
		screen.fps = 5;
		VIDEO_vram_clear(0x00);
		cursol = 0;

		VIDEO_locate( 1, 1);	VIDEO_putstr("NTSC Setup");

		VIDEO_locate( 1, 2);	VIDEO_putstr("line");
		VIDEO_locate( 1, 3);	VIDEO_putstr("line_sync");
		VIDEO_locate( 1, 4);	VIDEO_putstr("line_space");
		VIDEO_locate( 1, 5);	VIDEO_putstr("line_video");
		VIDEO_locate( 1, 6);	VIDEO_putstr("horizon");
		VIDEO_locate( 1, 7);	VIDEO_putstr("serration");
		VIDEO_locate( 1, 8);	VIDEO_putstr("equalizing");
		VIDEO_locate( 1, 9);	VIDEO_putstr("left_space");
		VIDEO_locate( 1,10);	VIDEO_putstr("width");

		VIDEO_locate(12, 1);	VIDEO_putstr("Monitor");
		VIDEO_locate(20, 1);	VIDEO_putstr("Superimpose");

		/* 白枠 */
		VIDEO_line(             1,               1, NTSC_WIDTH - 2,               1);
		VIDEO_line(             1,               1,              1, NTSC_HEIGHT - 1);
		VIDEO_line(NTSC_WIDTH - 2,               1, NTSC_WIDTH - 2, NTSC_HEIGHT - 1);
		VIDEO_line(             1, NTSC_HEIGHT - 1, NTSC_WIDTH - 2, NTSC_HEIGHT - 1);

		VIDEO_line(             2,               2, NTSC_WIDTH - 3,               2);
		VIDEO_line(             2,               2,              2, NTSC_HEIGHT - 2);
		VIDEO_line(NTSC_WIDTH - 3,               2, NTSC_WIDTH - 3, NTSC_HEIGHT - 2);
		VIDEO_line(             2, NTSC_HEIGHT - 2, NTSC_WIDTH - 3, NTSC_HEIGHT - 2);

		VIDEO_line(             3,               3, NTSC_WIDTH - 4,               3);
		VIDEO_line(             3,               3,              3, NTSC_HEIGHT - 3);
		VIDEO_line(NTSC_WIDTH - 4,               3, NTSC_WIDTH - 4, NTSC_HEIGHT - 3);
		VIDEO_line(             3, NTSC_HEIGHT - 3, NTSC_WIDTH - 4, NTSC_HEIGHT - 3);

		VIDEO_line(             4,               4, NTSC_WIDTH - 5,               4);
		VIDEO_line(             4,               4,              4, NTSC_HEIGHT - 4);
		VIDEO_line(NTSC_WIDTH - 5,               4, NTSC_WIDTH - 5, NTSC_HEIGHT - 4);
		VIDEO_line(             4, NTSC_HEIGHT - 4, NTSC_WIDTH - 5, NTSC_HEIGHT - 4);
	}


	/* キーボード入力処理 */
	switch(SCREEN_key_get()){
	case KEY_UP:		cursol--;		break;
	case KEY_DOWN:		cursol++;		break;
	case KEY_LEFT:		run = -1;		break;
	case KEY_RIGHT:		run =  1;		break;
	default:							break;
	}

	/* カーソル確認/表示 */
	if(cursol <  0) cursol =  0;
	if(cursol > 17) cursol = 17;
	for(i = 0; i < 9; i++){
		VIDEO_locate(13, 2 + i);
		if(cursol == i) VIDEO_putch('>');
		else			VIDEO_putch(' ');
	}
	for(; i < 18; i++){
		VIDEO_locate(25, i - 7);
		if(cursol == i) VIDEO_putch('>');
		else			VIDEO_putch(' ');
	}

	/* 実行 */
	if(run == -1 || run == 1){
		switch(cursol){
		case  0:	ntsc.monitor.line += run;					break;
		case  1:	ntsc.monitor.line_sync += run;				break;
		case  2:	ntsc.monitor.line_space_top += run;			break;
		case  3:	ntsc.monitor.line_video += run;				break;
		case  4:	ntsc.monitor.horizon_pulse += run;			break;
		case  5:	ntsc.monitor.serration_pulse += run;		break;
		case  6:	ntsc.monitor.equalizing_pulse += run;		break;
		case  7:	ntsc.monitor.left_space += run;				break;
		case  8:	ntsc.monitor.video_width += run;			break;
		case  9:	ntsc.superimpose.line += run;				break;
		case 10:	ntsc.superimpose.line_sync += run;			break;
		case 11:	ntsc.superimpose.line_space_top += run;		break;
		case 12:	ntsc.superimpose.line_video += run;			break;
		case 13:	ntsc.superimpose.horizon_pulse += run;		break;
		case 14:	ntsc.superimpose.serration_pulse += run;	break;
		case 15:	ntsc.superimpose.equalizing_pulse += run;	break;
		case 16:	ntsc.superimpose.left_space += run;			break;
		case 17:	ntsc.superimpose.video_width += run;		break;
		default:												break;
		}
	}

	/* 表示 */
	VIDEO_locate(14, 2);	VIDEO_putuint(ntsc.monitor.line, 5);
	VIDEO_locate(14, 3);	VIDEO_putuint(ntsc.monitor.line_sync, 5);
	VIDEO_locate(14, 4);	VIDEO_putuint(ntsc.monitor.line_space_top, 5);
	VIDEO_locate(14, 5);	VIDEO_putuint(ntsc.monitor.line_video, 5);
	VIDEO_locate(14, 6);	VIDEO_putuint(ntsc.monitor.horizon_pulse, 5);
	VIDEO_locate(14, 7);	VIDEO_putuint(ntsc.monitor.serration_pulse, 5);
	VIDEO_locate(14, 8);	VIDEO_putuint(ntsc.monitor.equalizing_pulse, 5);
	VIDEO_locate(14, 9);	VIDEO_putuint(ntsc.monitor.left_space, 5);
	VIDEO_locate(14,10);	VIDEO_putuint(ntsc.monitor.video_width, 5);

	VIDEO_locate(26, 2);	VIDEO_putuint(ntsc.superimpose.line, 5);
	VIDEO_locate(26, 3);	VIDEO_putuint(ntsc.superimpose.line_sync, 5);
	VIDEO_locate(26, 4);	VIDEO_putuint(ntsc.superimpose.line_space_top, 5);
	VIDEO_locate(26, 5);	VIDEO_putuint(ntsc.superimpose.line_video, 5);
	VIDEO_locate(26, 6);	VIDEO_putuint(ntsc.superimpose.horizon_pulse, 5);
	VIDEO_locate(26, 7);	VIDEO_putuint(ntsc.superimpose.serration_pulse, 5);
	VIDEO_locate(26, 8);	VIDEO_putuint(ntsc.superimpose.equalizing_pulse, 5);
	VIDEO_locate(26, 9);	VIDEO_putuint(ntsc.superimpose.left_space, 5);
	VIDEO_locate(26,10);	VIDEO_putuint(ntsc.superimpose.video_width, 5);

	return;
}


/*-------------------------------------------------------------------------------------------------
外付けメータの設定
-------------------------------------------------------------------------------------------------*/
static  void SCREEN_setup_extmeter(void)
{
	static unsigned char target = 0;
	static char cursol;
	char run = 0;
	unsigned int i;

	/* 画面初期化 */
	if(screen.screen_flag == SCREEN_INIT){
		screen.screen_flag = SCREEN_VIEW;
		screen.fps = 10;
		VIDEO_vram_clear(0x00);
		cursol = 0;

		target = extmeter.target;

		VIDEO_locate( 0, 0);	VIDEO_putstr("External Meter Setup");

		VIDEO_locate( 1, 2);	VIDEO_putstr("TARGET");
		VIDEO_locate( 1, 4);	VIDEO_putstr("0x0f");
		VIDEO_locate( 1, 5);	VIDEO_putstr("0x1f");
		VIDEO_locate( 1, 6);	VIDEO_putstr("0x2f");
		VIDEO_locate( 1, 7);	VIDEO_putstr("0x3f");
		VIDEO_locate( 1, 8);	VIDEO_putstr("0x4f");
		VIDEO_locate( 1, 9);	VIDEO_putstr("0x5f");
		VIDEO_locate( 1,10);	VIDEO_putstr("0x6f");
		VIDEO_locate( 1,11);	VIDEO_putstr("0x7f");
		VIDEO_locate(16, 4);	VIDEO_putstr("0x8f");
		VIDEO_locate(16, 5);	VIDEO_putstr("0x9f");
		VIDEO_locate(16, 6);	VIDEO_putstr("0xaf");
		VIDEO_locate(16, 7);	VIDEO_putstr("0xbf");
		VIDEO_locate(16, 8);	VIDEO_putstr("0xcf");
		VIDEO_locate(16, 9);	VIDEO_putstr("0xdf");
		VIDEO_locate(16,10);	VIDEO_putstr("0xef");
		VIDEO_locate(16,11);	VIDEO_putstr("0xff");
	}

	/* キーボード入力処理 */
	switch(SCREEN_key_get()){
	case KEY_UP:		cursol--;		break;
	case KEY_DOWN:		cursol++;		break;
	case KEY_LEFT:		run = -1;		break;
	case KEY_RIGHT:		run =  1;		break;
	default:							break;
	}

	/* カーソル確認/表示 */
	if(cursol <  0) cursol =  0;
	if(cursol > 17) cursol = 17;
	for(i = 0; i < 10; i++){
		VIDEO_locate( 0, 2 + i);
		if(cursol == i) VIDEO_putch('>');
		else			VIDEO_putch(' ');
	}
	for(; i < 18; i++){
		VIDEO_locate(15, 4 + i - 10);
		if(cursol == i) VIDEO_putch('>');
		else			VIDEO_putch(' ');
	}

	/* 実行 */
	switch(cursol){
	case  0:
		extmeter.target = target;
		if(run == -1 || run == 1){
			target += run;
			if(target < 1) target = 1;
			if(target > 6) target = 6;
			EXTMETER_init(target);
		}
		break;

	case  1:
		extmeter.target = target;
		break;

	default:
		extmeter.target  = EXTMETER_SETTING;
		extmeter.setting = ((cursol - 2) << 4) + 0x0f;
		if(run == -1 || run == 1){
			switch(target){
			case EXTMETER_ENGINE:		extmeter.map[ cursol - 2 ] += 100 * run;		break;
			case EXTMETER_BOOST:		extmeter.map[ cursol - 2 ] += 0.01 * run;		break;
			case EXTMETER_THROTTLE:		extmeter.map[ cursol - 2 ] += 0.1 * run;		break;
			case EXTMETER_SPEED:
			case EXTMETER_COOLANT:
			case EXTMETER_INTAKEAIR:
										extmeter.map[ cursol - 2 ] += 1 * run;			break;
			default:																	break;
			}
		}

		break;
	}

	/* 表示 */
	VIDEO_locate( 8, 2);
	switch(extmeter.target){
	case EXTMETER_SETTING:		VIDEO_putstr("-SETTING MODE-");		break;
	case EXTMETER_SPEED:		VIDEO_putstr("Vechile Speed ");		break;
	case EXTMETER_ENGINE:		VIDEO_putstr("Engine Speed  ");		break;
	case EXTMETER_BOOST:		VIDEO_putstr("Boost Meter   ");		break;
	case EXTMETER_THROTTLE:		VIDEO_putstr("Throttle      ");		break;
	case EXTMETER_COOLANT:		VIDEO_putstr("Coolant Temp  ");		break;
	case EXTMETER_INTAKEAIR:	VIDEO_putstr("IntakeAir Temp");		break;
	default:														break;
	}
	for(i = 0; i < 8; i++){
		VIDEO_locate( 6, 4 + i);
		VIDEO_putdouble(extmeter.map[i], 4, 2);
	}
	for(; i < 16; i++){
		VIDEO_locate(21, 4 + i - 8);
		VIDEO_putdouble(extmeter.map[i], 4, 2);
	}

	return;
}


/*-------------------------------------------------------------------------------------------------
環境設定の保存と読み出し
-------------------------------------------------------------------------------------------------*/
static  void SCREEN_setup_config(void)
{
	static char cursol;
	char run = 0;
	unsigned int i;

	/* 画面初期化 */
	if(screen.screen_flag == SCREEN_INIT){
		screen.screen_flag = SCREEN_VIEW;
		screen.fps = 5;
		VIDEO_vram_clear(0x00);

		cursol = 0;

		VIDEO_locate( 0, 0);	VIDEO_putstr("Configuration Setup");

		VIDEO_locate( 1, 2);	VIDEO_putstr("Load");
		VIDEO_locate( 1, 3);	VIDEO_putstr("Save");
		VIDEO_locate( 1, 4);	VIDEO_putstr("Initialize");
	}

	/* キーボード入力処理 */
	switch(SCREEN_key_get()){
	case KEY_UP:		cursol--;		break;
	case KEY_DOWN:		cursol++;		break;
	case KEY_ENTER:		run = 1;		break;
	default:							break;
	}

	/* カーソル確認/表示 */
	if(cursol < 0) cursol = 0;
	if(cursol > 2) cursol = 2;
	for(i = 0; i < 3; i++){
		VIDEO_locate( 0, 2 + i);
		if(cursol == i) VIDEO_putch('>');
		else			VIDEO_putch(' ');
	}

	/* 実行 */
	if(run == 1){
		switch(cursol){
		case 0:
			VIDEO_locate( 5, 6);		VIDEO_putstr("Now Loading...");
			if(CONFIG_load()){
				VIDEO_locate( 5, 6);	VIDEO_putstr("Load Success  ");
			} else {
				VIDEO_locate( 5, 6);	VIDEO_putstr("Load Failed   ");
			}
			screen.screen = 0x0a;
			screen.screen_flag = SCREEN_VIEW;
			break;

		case 1:
			screen.screen = backup;
			screen.screen_flag = SCREEN_INIT;
			VIDEO_locate( 5, 6);		VIDEO_putstr("Now Saving... ");
			if(CONFIG_save()){
				VIDEO_locate( 5, 6);	VIDEO_putstr("Save Success  ");
			} else {
				VIDEO_locate( 5, 6);	VIDEO_putstr("Save Failed   ");
			}
			screen.screen = 0x0a;
			screen.screen_flag = SCREEN_VIEW;
			break;

		case 2:
			SSM_init();
			SCREEN_init();
			VIDEO_init();
			screen.screen = 0x0a;
			screen.screen_flag = SCREEN_INIT;
			EXTMETER_init(EXTMETER_BOOST);
			VIDEO_locate( 5, 6);		VIDEO_putstr("Init Success  ");
			break;

		default:
			break;
		}
	}


	return;
}


/*-------------------------------------------------------------------------------------------------
デバック用情報表示画面
-------------------------------------------------------------------------------------------------*/
static  void SCREEN_debug(void)
{
	/* 画面初期化 */
	if(screen.screen_flag == SCREEN_INIT){
		screen.screen_flag = SCREEN_VIEW;
		screen.fps = 10;
		VIDEO_vram_clear(0x00);

		VIDEO_locate( 0, 0);	VIDEO_putstr("DEBUG Monitor");

		VIDEO_locate( 0, 2);	VIDEO_putstr("PortA");
		VIDEO_locate( 0, 3);	VIDEO_putstr("PortB");
		VIDEO_locate( 0, 4);	VIDEO_putstr("DAC");
		VIDEO_locate( 0, 5);	VIDEO_putstr("ADC-0");
		VIDEO_locate( 0, 6);	VIDEO_putstr("ADC-1");
		VIDEO_locate( 0, 7);	VIDEO_putstr("ADC-2");
		VIDEO_locate( 0, 9);	VIDEO_putstr("Last");
		VIDEO_locate( 0,10);	VIDEO_putstr("Cycle");
		VIDEO_locate( 0,11);	VIDEO_putstr("Error");

		VIDEO_locate(11, 2);	VIDEO_putstr("Speed");
		VIDEO_locate(11, 3);	VIDEO_putstr("Engine");
		VIDEO_locate(11, 4);	VIDEO_putstr("Throttle");
		VIDEO_locate(11, 5);	VIDEO_putstr("Boost");
		VIDEO_locate(11, 6);	VIDEO_putstr("Gear");
		VIDEO_locate(11, 7);	VIDEO_putstr("Temp");
		VIDEO_locate(11, 8);	VIDEO_putstr("Battery");
		VIDEO_locate(11, 9);	VIDEO_putstr("MAF");
		VIDEO_locate(11,10);	VIDEO_putstr("AF Rate");
		VIDEO_locate(11,11);	VIDEO_putstr("IG/Knock");

		VIDEO_locate(26, 2);	VIDEO_putstr("km/h");
		VIDEO_locate(26, 3);	VIDEO_putstr("rpm");
		VIDEO_locate(26, 4);	VIDEO_putch('%');
		VIDEO_locate(26, 5);	VIDEO_putstr("kg/cm2");
		VIDEO_locate(26, 6);	VIDEO_putstr("Shift");
		VIDEO_locate(26, 7);	VIDEO_putstr("C");
		VIDEO_locate(26, 8);	VIDEO_putstr("V");
		VIDEO_locate(26, 9);	VIDEO_putstr("g/s");
		VIDEO_locate(26,10);	VIDEO_putstr("A/F");
		VIDEO_locate(26,11);	VIDEO_putstr("deg");
	}

	SCREEN_keybuf_clear();

	/* 描画 */
	VIDEO_locate( 6, 2);
	VIDEO_puthex(PORTA >> 8);	VIDEO_puthex(PORTA);

	VIDEO_locate( 6, 3);
	VIDEO_puthex(PORTB >> 8);	VIDEO_puthex(PORTB);

	VIDEO_locate( 6, 4);
	VIDEO_putch('0');	VIDEO_putch('0');	VIDEO_puthex(dac);

	VIDEO_locate( 6, 5);
	VIDEO_puthex(adc.adc[0] >> 8);	VIDEO_puthex(adc.adc[0]);

	VIDEO_locate( 6, 6);
	VIDEO_puthex(adc.adc[1] >> 8);	VIDEO_puthex(adc.adc[1]);

	VIDEO_locate( 6, 7);
	VIDEO_puthex(adc.adc[2] >> 8);	VIDEO_puthex(adc.adc[2]);

	VIDEO_locate( 6, 9);
	VIDEO_puthex(ssm.last >> 8);	VIDEO_puthex(ssm.last);

	VIDEO_locate( 6,10);
	VIDEO_puthex(ssm.cycle >> 8);	VIDEO_puthex(ssm.cycle);

	VIDEO_locate( 6,11);
	VIDEO_puthex(ssm.error >> 8);	VIDEO_puthex(ssm.error);

	VIDEO_locate(22, 2);
	VIDEO_putuint(ssm_data.speed, 3);

	VIDEO_locate(21, 3);
	VIDEO_putuint(ssm_data.engine, 4);

	VIDEO_locate(19, 4);
	VIDEO_putdouble(ssm_data.throttle, 3,1);

	VIDEO_locate(20, 5);
	VIDEO_putdouble(ssm_data.boost, 0, 2);

	VIDEO_locate(24, 6);
	VIDEO_putuint(ssm_data.shift, 1);

	VIDEO_locate(20, 7);
	VIDEO_putuint(ssm_data.coolant, 2);
	VIDEO_putch('/');
	VIDEO_putuint(ssm_data.intakeair, 2);

	VIDEO_locate(20, 8);
	VIDEO_putdouble(ssm_data.battery, 2, 1);

	VIDEO_locate(20, 9);
	VIDEO_putuint(ssm_data.maf, 5);

	VIDEO_locate(20,10);
	VIDEO_putdouble(ssm_data.afr, 2, 1);

	VIDEO_locate(20,11);
	VIDEO_putuint(ssm_data.ignition, 2);
	VIDEO_putch('/');
	VIDEO_putuint(ssm_data.knock, 2);

	return;
}


/*-------------------------------------------------------------------------------------------------
基板情報表示画面
-------------------------------------------------------------------------------------------------*/
static  void SCREEN_version(void)
{
	/* 画面初期化 */
	if(screen.screen_flag == SCREEN_INIT){
		screen.screen_flag = SCREEN_VIEW;
		screen.fps = 5;
		VIDEO_vram_clear(0x00);

		VIDEO_locate(0, 0);
		VIDEO_putstr("Product Infomation");

		VIDEO_locate(0, 2);
		VIDEO_putstr("Board name    : ");
		VIDEO_putstr(info.board_name);

		VIDEO_locate(0, 3);
		VIDEO_putstr("Board build   : Koki ");
		VIDEO_putuint(info.board_year, 4);	VIDEO_putch('/');	VIDEO_putuint(info.board_month, 2);	VIDEO_putch('/');	VIDEO_putuint(info.board_day, 2);

		VIDEO_locate(0, 4);
		VIDEO_putstr("Firmware Ver. : ");
		VIDEO_putuint(info.firmware_major, 2);	VIDEO_putch('.');	VIDEO_putuint(info.firmware_minor, 2);	VIDEO_putch('.');	VIDEO_putuint(info.firmware_revision, 3);

		VIDEO_locate(0, 5);
		VIDEO_putstr("Firmware build: Koki ");
		VIDEO_putuint(info.firmware_year, 4);	VIDEO_putch('/');	VIDEO_putuint(info.firmware_month, 2);	VIDEO_putch('/');	VIDEO_putuint(info.firmware_day, 2);

		VIDEO_locate(0, 6);
		VIDEO_putstr("Board S/N     : ");
		VIDEO_putstr(info.serial);

		VIDEO_locate(0, 8);	VIDEO_putstr(info.project);
		VIDEO_locate(0, 9);	VIDEO_putstr(info.web);
		VIDEO_locate(0,10);	VIDEO_putstr(info.mail);
		VIDEO_locate(0,11);	VIDEO_putstr(info.copyright);

	}

	SCREEN_keybuf_clear();
	return;
}


/*-------------------------------------------------------------------------------------------------
おまけ、23/3 ライフゲームもどき 兼 スクリーンセーバー
-------------------------------------------------------------------------------------------------*/
static  void SCREEN_lifegame(void)
{
	static unsigned int y, stage;
	unsigned int x, check;

	/* 画面初期化 */
	if(screen.screen_flag == SCREEN_INIT){
		screen.screen_flag = SCREEN_VIEW;
		screen.fps = 60;
		y = 1;
		stage = 0;
	}

	SCREEN_keybuf_clear();

	if(y >= NTSC_HEIGHT - 1){
		y = 1;
		stage++;
	}
	if(y < (unsigned int)FONTX2_get_ascii_height()){
		VIDEO_locate(27, 0);
		VIDEO_putuint(stage, 5);
	}

	/* 探索 */
	for(x = 1; x < NTSC_WIDTH - 1; x++){
		/* メモリに余裕がないので、ラインを捨てながら描く */
		check = VIDEO_get_point(x - 1, y - 1)
			  + VIDEO_get_point(x - 1, y    )
			  + VIDEO_get_point(x - 1, y + 1)
			  + VIDEO_get_point(x    , y - 1)
			  + VIDEO_get_point(x    , y + 1)
			  + VIDEO_get_point(x + 1, y - 1)
			  + VIDEO_get_point(x + 1, y    )
			  + VIDEO_get_point(x + 1, y + 1);

		if(check == 3){
			VIDEO_point(x,y);
		} else {
			if(check != 2) VIDEO_point_(x,y);
		}
	}

	y++;

	return;
}


/*-------------------------------------------------------------------------------------------------
使わないキーを読み込み、バッファを捨てる.
-------------------------------------------------------------------------------------------------*/
static  void SCREEN_keybuf_clear(void)
{
	switch(PS2_key_check()){
	case KEY_F1:
	case KEY_F2:
	case KEY_F3:
	case KEY_F4:
	case KEY_F5:
	case KEY_F6:
	case KEY_F7:
	case KEY_F8:
	case KEY_F9:
	case KEY_F10:
	case KEY_F11:
	case KEY_F12:
	case KEY_PAGEUP:
	case KEY_PAGEDOWN:
	case KEY_DEL:
	case KEY_HOME:
		break;

	default:
		PS2_key_get();
		break;
	}

	return;
}


/*-------------------------------------------------------------------------------------------------
各スクリーンでキー入力が必要なときに使う
-------------------------------------------------------------------------------------------------*/
static  unsigned char SCREEN_key_get(void)
{
	switch(PS2_key_check()){
	case KEY_F1:
	case KEY_F2:
	case KEY_F3:
	case KEY_F4:
	case KEY_F5:
	case KEY_F6:
	case KEY_F7:
	case KEY_F8:
	case KEY_F9:
	case KEY_F10:
	case KEY_F11:
	case KEY_F12:
	case KEY_PAGEUP:
	case KEY_PAGEDOWN:
	case KEY_DEL:
	case KEY_HOME:
		return(KEY_NOP);
		break;

	default:
		return(PS2_key_get());
		break;
	}

	/* ここにきたらバグ */
	return(KEY_NOP);
}
