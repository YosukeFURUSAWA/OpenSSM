/**************************************************************************************************
	Title			: SSM Protocol
	Programer		: Yosuke FURUSAWA.
	Copyright		: Copyright (C) 2007-2010 Yosuke FURUSAWA.
	License			: 4-clause BSD License
	Since			: 2007/11/xx

	Filename		: ssm.c
	Last up date	: 2010/08/22
	Kanji-Code		: Shift-JIS
	TAB Space		: 4
**************************************************************************************************/


/*=================================================================================================
ヘッダファイルをインクルード
=================================================================================================*/
#include <p24FJ64GA002.h>

#include "types.h"
#include "table.h"
#include "ssm.h"

#include "libadc.h"
#include "librtc.h"
#include "libuart.h"


/*=================================================================================================
マクロ定義
=================================================================================================*/
/* 各ギア比の中間をもって、シフトの境目とする */
#define SSM_SHIFT(x,y)		(x - (x - y) / 2.0)

/* タイヤの基本スペック */
#define TIRE_WIDTH			235			/* タイヤの幅 (mm) */
#define TIRE_FLAT			45			/* タイヤの扁平率 (%) */
#define TIRE_INCH			17			/* タイヤの直径 (inch) */

/* ギア比 */
#define GEAR_F				3.900
#define GEAR_1				3.636
#define GEAR_2				2.375
#define GEAR_3				1.761
#define GEAR_4				1.346
#define GEAR_5				1.062
#define GEAR_6				0.842
#define GEAR_7				0.001


/*=================================================================================================
グローバル変数
=================================================================================================*/
SSM_T ssm;
SSM_DATA_T ssm_data;


/*=================================================================================================
プロトタイプ宣言
=================================================================================================*/
static  BOOL SSM_openssm(void);
static  void SSM_log(void);
static  BOOL SSM_openport(void);
static  void SSM_write_packet(void);
static  BOOL SSM_read_packet(pSSM_T ssm, pSSM_DATA_T data);
static  unsigned char SSM_calc_gear(pSSM_T ssm, pSSM_DATA_T data);


/**************************************************************************************************
SSM初期化
**************************************************************************************************/
void SSM_init(void)
{
	/* 初期化する */
	ssm.tire_width		= TIRE_WIDTH;
	ssm.tire_flat		= TIRE_FLAT;
	ssm.tire_inch		= TIRE_INCH;
	ssm.gear_ratio[0]	= GEAR_F;
	ssm.gear_ratio[1]	= GEAR_1;
	ssm.gear_ratio[2]	= GEAR_2;
	ssm.gear_ratio[3]	= GEAR_3;
	ssm.gear_ratio[4]	= GEAR_4;
	ssm.gear_ratio[5]	= GEAR_5;
	ssm.gear_ratio[6]	= GEAR_6;
	ssm.gear_ratio[7]	= GEAR_7;
	ssm.price			= 100;
	ssm.tire_circle		= SSM_TIRE_R(ssm.tire_width, ssm.tire_flat, ssm.tire_inch);
	ssm.mode			= SSM_MODE_OPENSSM;

	ssm.wait  = 1500;
	ssm.last  = rtc.tick;
	ssm.cycle = 0xffff;
	ssm.error = 0x0000;

	ssm_data.engine		= 800;
	ssm_data.throttle	= 0;
	ssm_data.speed		= 0;
	ssm_data.boost		= 0.0;
	ssm_data.coolant	= 20.0;
	ssm_data.intakeair	= 20.0;
	ssm_data.battery	= 12.0;
	ssm_data.maf		= 5;
	ssm_data.afr		= 14.7;
	ssm_data.ignition	= 0;
	ssm_data.knock		= 0;
	ssm_data.fuel		= 0;
	ssm_data.fuel_rate	= 0;
	ssm_data.shift		= 0;

	UART1_buf_clear();
	UART2_buf_clear();
	return;
}


/**************************************************************************************************
SSM通信処理
**************************************************************************************************/
BOOL SSM_main(void)
{
	static unsigned char mode = 0xff;

	/* 通信モードによって動作を変える */
	switch(ssm.mode){


	/* OpenSSMモード */
	case SSM_MODE_OPENSSM:

		/* 前回のコールと異なるとき、UARTを初期化する */
		if(mode != ssm.mode){
			UART1_init(115200);
			UART2_init(  4800);
		}

		mode = ssm.mode;
		return(SSM_openssm());
		break;


	/* OpenPort下位互換モード */
	case SSM_MODE_OPENPORT:
	default:

		/* 前回のコールと異なるとき、UARTを初期化する */
		if(mode != ssm.mode){
			UART1_init(  4800);
			UART2_init(  4800);
		}

		mode = ssm.mode;
		return(SSM_openport());
		break;
	}

	/* ここにきたらバグ */
	return(FALSE);
}


/*-------------------------------------------------------------------------------------------------
OpenSSMモード
-------------------------------------------------------------------------------------------------*/
static  BOOL SSM_openssm(void)
{
	static unsigned int tick = 0;
	static unsigned char flag = 0;

	/* 実行周期 */
	if(RTC_get_ticks(tick, rtc.tick) < ssm.wait) return(FALSE);
	tick = rtc.tick;

	if(!SSM_read_packet(&ssm, &ssm_data)){
		ssm.error++;

		/* エラーが発生したときは、1サイクルのウェイトを入れてからパケットを送る */
		if(ssm.error >> 1){
			SSM_write_packet();
			ssm.cycle = 0xffff;
		}
	} else {
		ssm.cycle = RTC_get_ticks(ssm.last, rtc.tick);
		ssm.last = rtc.tick;
		SSM_write_packet();
	}

	/* スペースキーの入力があったならログを出す/止める */
	if(UART1_getch() == ' ')	flag++;
	if(flag % 2)				SSM_log();

	return(TRUE);
}


/*-------------------------------------------------------------------------------------------------
UART1へログを出す
-------------------------------------------------------------------------------------------------*/
static  void SSM_log(void)
{
	double sec;

	/* バッファから溢れそうなときは、飛ばす... */
	if(UART1_get_sendbuf() != UART1_TX_BUFFER_SIZE) return;
	UART1_buf_clear();

	/* 時刻 */
	sec = rtc.sec + (double)rtc.msec / 1000.0;
	UART1_putint(rtc.hour);
	UART1_putch(':');
	UART1_putint(rtc.min);
	UART1_putch(':');
	UART1_putdouble(sec, 3);
	UART1_putch(',');

	/* SSMデータ */
	UART1_putint(ssm_data.engine);
	UART1_putch(',');
	UART1_putint(ssm_data.speed);
	UART1_putch(',');
	UART1_putint(ssm_data.throttle);
	UART1_putch(',');
	UART1_putint(ssm_data.shift);
	UART1_putch(',');
	UART1_putint(ssm_data.coolant);
	UART1_putch(',');
	UART1_putint(ssm_data.intakeair);
	UART1_putch(',');
	UART1_putdouble(ssm_data.battery, 1);
	UART1_putch(',');
	UART1_putdouble(ssm_data.fuel, 1);
	UART1_putch(',');
	UART1_putint(ssm_data.maf);
	UART1_putch(',');
	UART1_putdouble(ssm_data.afr, 1);
	UART1_putch(',');
	UART1_putint(ssm_data.knock);
	UART1_putch(',');
	UART1_putint(ssm_data.ignition);
	UART1_putch(',');

	/* 加速度 */
	UART1_putint(adc.adc[0]);
	UART1_putch(',');
	UART1_putint(adc.adc[1]);
	UART1_putch(',');
	UART1_putint(adc.adc[2]);

	UART1_putch('\r');
	UART1_putch('\n');

	return;
}


/*-------------------------------------------------------------------------------------------------
UART1 と UART2 をクロス接続
OpenPort下位互換の通信モード
-------------------------------------------------------------------------------------------------*/
static  BOOL SSM_openport(void)
{
	int buf;

	while((buf = UART1_getch()) > 0){
		UART2_putch(buf);
	}

	while((buf = UART2_getch()) > 0){
		UART1_putch(buf);
	}

	return(TRUE);
}


/*-------------------------------------------------------------------------------------------------
SSMブロック読み込み
-------------------------------------------------------------------------------------------------*/
static  void SSM_write_packet(void)
{
	unsigned char i;
	const unsigned char packet[] = { 
		0x80, 0x10, 0xf0, 0x29, 0xa8, 0x00, 0x00, 0x00, 0x0e, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x15,
		0x00, 0x00, 0x10, 0x00, 0x00, 0x24, 0x00, 0x00, 0x08, 0x00, 0x00, 0x12, 0x00, 0x00, 0x1c,
		0x00, 0x00, 0x13, 0x00, 0x00, 0x14, 0x00, 0x00, 0x46, 0x00, 0x00, 0x11, 0x00, 0x00, 0x22,
		0x8d,
	};

	UART2_buf_clear();
	for(i = 0; i < sizeof(packet); i++){
		UART2_putch(packet[i]);								/* 送信エラーは無視する... */
	}

	return;
}

static  BOOL SSM_read_packet(pSSM_T ssm, pSSM_DATA_T data)
{
	int buf;
	unsigned char i;
	double fuel;

	static unsigned char read_packet[ 18 ];

	/* 一旦、バッファに入れる */
	i = 0;
	while((buf = UART2_getch()) != -1 && i < 45){
		/* 空読み */
		i++;
	}
	i = 0;
	while((buf = UART2_getch()) != -1 && i < 18){
		read_packet[i] = (unsigned char)buf;
		i++;
	}

	/* パケットの確認 */
	if(read_packet[ 0] != 0x80) return(FALSE);				/* コマンドヘッダ */
	if(read_packet[ 1] != 0xf0) return(FALSE);				/* 通信方向 MSB */
	if(read_packet[ 2] != 0x10) return(FALSE);				/* 通信方向 LSB */
	if(read_packet[ 3] != 0x0e) return(FALSE);				/* コマンド + データのサイズ */
	if(read_packet[ 4] != 0xe8) return(FALSE);				/* コマンド */

	/* 変換 */
	data->engine	= ((read_packet[ 5] << 8) + read_packet[ 6]) >> 2;
	data->throttle	= (double)((int)read_packet[ 7] * 100) / 255.0;
	data->speed		= read_packet[ 8];
	data->boost		= (((double)read_packet[ 9] - 128.0) * 37.0) / 3570.0;
	data->coolant	= (int)read_packet[10] - 40;
	data->intakeair	= (int)read_packet[11] - 40;
	data->battery	= (double)read_packet[12] * 0.08;
	data->shift		= SSM_calc_gear(ssm, data);
	data->maf		= (double)(((unsigned int)read_packet[13] << 8) + (unsigned int)read_packet[14]) / 100.0;
	data->afr		= ((double)read_packet[15] / 128.0) * 14.7;
	data->ignition	= (read_packet[16] - 128) >> 1;
	data->knock		= (read_packet[17] - 128) >> 1;


	if(data->engine > 0){
		fuel		= (data->maf / data->afr) / 761.0;
	} else {
		fuel		= 0;
	}

	data->fuel		= ((double)data->speed / 3600.0) / fuel;
	data->fuel_rate  = fuel * 1000;

	return(TRUE);
}


/*-------------------------------------------------------------------------------------------------
ギアを推測する
-------------------------------------------------------------------------------------------------*/
static  unsigned char SSM_calc_gear(pSSM_T ssm, pSSM_DATA_T data)
{
	double ratio;
	unsigned char shift;

	/* 走行中でないときは、ニュートラルとする */
	if(data->engine  <  1000) return(0);
	if(data->speed   ==    0) return(0);

	/* タイヤサイズ、車速、エンジン回転数からギア比を求める */
	ratio  = (data->engine / (data->speed * ssm->gear_ratio[ SSM_GEAR_FINAL ]) * ssm->tire_circle * 60.0);
	ratio /= 1000000.0;

	/* シフトを求める */
	if(SSM_SHIFT(ssm->gear_ratio[SSM_GEAR_1], ssm->gear_ratio[SSM_GEAR_2]) <= ratio) shift = SSM_GEAR_1;
	if(SSM_SHIFT(ssm->gear_ratio[SSM_GEAR_1], ssm->gear_ratio[SSM_GEAR_2]) >  ratio) shift = SSM_GEAR_2;
	if(SSM_SHIFT(ssm->gear_ratio[SSM_GEAR_2], ssm->gear_ratio[SSM_GEAR_3]) >  ratio) shift = SSM_GEAR_3;
	if(SSM_SHIFT(ssm->gear_ratio[SSM_GEAR_3], ssm->gear_ratio[SSM_GEAR_4]) >  ratio) shift = SSM_GEAR_4;
	if(SSM_SHIFT(ssm->gear_ratio[SSM_GEAR_4], ssm->gear_ratio[SSM_GEAR_5]) >  ratio) shift = SSM_GEAR_5;
	if(SSM_SHIFT(ssm->gear_ratio[SSM_GEAR_5], ssm->gear_ratio[SSM_GEAR_6]) >  ratio) shift = SSM_GEAR_6;
	if(SSM_SHIFT(ssm->gear_ratio[SSM_GEAR_6], ssm->gear_ratio[SSM_GEAR_7]) >  ratio) shift = SSM_GEAR_7;

	return(shift);
}
