/**************************************************************************************************
	Title			: External Meter
	Programmer		: Yosuke FURUSAWA.
	Copyright		: Copyright (C) 2010 Yosuke FURUSAWA.
	License			: 4-clause BSD License
	Since			: 2010/06/19

	Filename		: extmeter.c
	Last up date	: 2010/11/21
	Kanji-Code		: Shift-JIS
	TAB Space		: 4
**************************************************************************************************/


/*================================================================================================
ヘッダファイルをインクルード
=================================================================================================*/
#include <p24FJ64GA002.h>

#include "types.h"
#include "ssm.h"
#include "extmeter.h"

#include "libdac.h"


/*=================================================================================================
マクロ定義
=================================================================================================*/


/*=================================================================================================
構造体
=================================================================================================*/


/*=================================================================================================
グローバル変数
=================================================================================================*/
EXTMETER_T extmeter;


/*=================================================================================================
プロトタイプ宣言
=================================================================================================*/


/**************************************************************************************************
初期化
**************************************************************************************************/
void EXTMETER_init(unsigned char target)
{
	extmeter.target = target;

	/* extmeter.mapを 2次元配列にする方法もあるが... */
	switch(target){
	case EXTMETER_SPEED:
		extmeter.map[  0 ] =     0;
		extmeter.map[  1 ] =    12;
		extmeter.map[  2 ] =    24;
		extmeter.map[  3 ] =    36;
		extmeter.map[  4 ] =    48;
		extmeter.map[  5 ] =    60;
		extmeter.map[  6 ] =    72;
		extmeter.map[  7 ] =    84;
		extmeter.map[  8 ] =    96;
		extmeter.map[  9 ] =   108;
		extmeter.map[ 10 ] =   120;
		extmeter.map[ 11 ] =   132;
		extmeter.map[ 12 ] =   144;
		extmeter.map[ 13 ] =   156;
		extmeter.map[ 14 ] =   168;
		extmeter.map[ 15 ] =   180;
		break;

	case EXTMETER_ENGINE:
		extmeter.map[  0 ] =     0;
		extmeter.map[  1 ] =   600;
		extmeter.map[  2 ] =  1200;
		extmeter.map[  3 ] =  2800;
		extmeter.map[  4 ] =  3400;
		extmeter.map[  5 ] =  4000;
		extmeter.map[  6 ] =  4600;
		extmeter.map[  7 ] =  5200;
		extmeter.map[  8 ] =  5800;
		extmeter.map[  9 ] =  6400;
		extmeter.map[ 10 ] =  7000;
		extmeter.map[ 11 ] =  7600;
		extmeter.map[ 12 ] =  8200;
		extmeter.map[ 13 ] =  8800;
		extmeter.map[ 14 ] =  9400;
		extmeter.map[ 15 ] = 10000;
		break;

	case EXTMETER_BOOST:
		extmeter.map[  0 ] = -1.000;
		extmeter.map[  1 ] = -0.900;
		extmeter.map[  2 ] = -0.700;
		extmeter.map[  3 ] = -0.500;
		extmeter.map[  4 ] = -0.300;
		extmeter.map[  5 ] = -0.180;
		extmeter.map[  6 ] = -0.000;
		extmeter.map[  7 ] =  0.200;
		extmeter.map[  8 ] =  0.350;
		extmeter.map[  9 ] =  0.510;
		extmeter.map[ 10 ] =  0.700;
		extmeter.map[ 11 ] =  0.900;
		extmeter.map[ 12 ] =  1.050;
		extmeter.map[ 13 ] =  1.200;
		extmeter.map[ 14 ] =  1.400;
		extmeter.map[ 15 ] =  1.600;
		break;

	case EXTMETER_THROTTLE:
		extmeter.map[  0 ] =     00;
		extmeter.map[  1 ] =   6.60;
		extmeter.map[  2 ] =  13.20;
		extmeter.map[  3 ] =  19.80;
		extmeter.map[  4 ] =  26.40;
		extmeter.map[  5 ] =  33.00;
		extmeter.map[  6 ] =  39.60;
		extmeter.map[  7 ] =  46.20;
		extmeter.map[  8 ] =  52.80;
		extmeter.map[  9 ] =  59.40;
		extmeter.map[ 10 ] =  66.00;
		extmeter.map[ 11 ] =  72.60;
		extmeter.map[ 12 ] =  79.20;
		extmeter.map[ 13 ] =  85.80;
		extmeter.map[ 14 ] =  92.40;
		extmeter.map[ 15 ] = 100.00;
		break;

	case EXTMETER_COOLANT:
	case EXTMETER_INTAKEAIR:
	default:
		extmeter.map[  0 ] =   -40;
		extmeter.map[  1 ] =   -28;
		extmeter.map[  2 ] =   -16;
		extmeter.map[  3 ] =    -4;
		extmeter.map[  4 ] =     8;
		extmeter.map[  5 ] =    20;
		extmeter.map[  6 ] =    32;
		extmeter.map[  7 ] =    44;
		extmeter.map[  8 ] =    56;
		extmeter.map[  9 ] =    68;
		extmeter.map[ 10 ] =    80;
		extmeter.map[ 11 ] =    92;
		extmeter.map[ 12 ] =   104;
		extmeter.map[ 13 ] =   116;
		extmeter.map[ 14 ] =   128;
		extmeter.map[ 15 ] =   140;
		break;
	}

	return;
}


/**************************************************************************************************
メイン

高速化する場合、dataをポインタ変数にして data[ extmeter.target ]で対象を選択すると良いが、
そのあとの forループはポインタ変数にしない方が速いかもしれない？　未確認。
**************************************************************************************************/
BOOL EXTMETER_main(void)
{
	unsigned int i;
	double data;

	/* 出力するデータを選ぶ */
	switch(extmeter.target){
	case EXTMETER_SETTING:		DAC_setvalue(extmeter.setting);	return(TRUE);

	case EXTMETER_SPEED:		data = ssm_data.speed;			break;
	case EXTMETER_ENGINE:		data = ssm_data.engine;			break;
	case EXTMETER_BOOST:		data = ssm_data.boost;			break;
	case EXTMETER_THROTTLE:		data = ssm_data.throttle;		break;
	case EXTMETER_COOLANT:		data = ssm_data.coolant;		break;
	case EXTMETER_INTAKEAIR:	data = ssm_data.intakeair;		break;
	default:													break;
	}

	/* マップから出力値を選ぶ */
	for(i = 1; i < EXTMETER_DIV; i++){
		if(extmeter.map[i - 1] <= data && data < extmeter.map[i]){
			DAC_setvalue(((i << 4) & 0xf0) | ((unsigned char)(((data - extmeter.map[i - 1]) / (extmeter.map[i] - extmeter.map[i - 1])) * EXTMETER_DIV) & 0x0f));
		}
	}

	return(TRUE);
}


