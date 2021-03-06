/**************************************************************************************************
	Title			: Configuration Save / Load Interface
	Programmer		: Yosuke FURUSAWA.
	Copyright		: Copyright (C) 2010 Yosuke FURUSAWA.
	License			: 4-clause BSD License
	Since			: 2010/07/03

	Filename		: config.c
	Last up date	: 2010/08/31
	Kanji-Code		: Shift-JIS
	TAB Space		: 4
**************************************************************************************************/


/*================================================================================================
ヘッダファイルをインクルード
=================================================================================================*/
#include <p24FJ64GA002.h>

#include "types.h"
#include "table.h"

#include "libeeprom.h"
#include "libuart.h"
#include "libvideo.h"

#include "ssm.h"
#include "main.h"
#include "extmeter.h"
#include "screen.h"


#include "config.h"


/*================================================================================================
マクロ定義
=================================================================================================*/


/*================================================================================================
構造体
=================================================================================================*/
typedef struct CONFIG {
	void *ptr;
	unsigned char size;
} CONFIG_T;
typedef CONFIG_T* pCONFIG_T;


/*================================================================================================
グローバル変数
=================================================================================================*/
/* 保存対象を設定する */
static const CONFIG_T config[] = {
	{ &extmeter,			sizeof(EXTMETER_T),		},
	{ &screen,				sizeof(SCREEN_T),		},
	{ &ssm,					sizeof(SSM_T),			},
//	{ &ntsc,				sizeof(NTSC_T),			},		/* 問題があったとき表示されなくなる */
	{ NULL,					0x00,					},
};


/*================================================================================================
プロトタイプ宣言
=================================================================================================*/
static  BOOL CONFIG_write_and_verify(const unsigned int ptr, const unsigned char data);
static  unsigned int CONFIG_make_checksum(void);
static  unsigned int CONFIG_read_checksum(void);


/**************************************************************************************************
初期化
**************************************************************************************************/
BOOL CONFIG_init(void)
{
	return(TRUE);
}


/**************************************************************************************************
保存する
---------------------------------------------------------------------------------------------------
復帰処理が面倒なので、何かあったら WDTリセットさせる
**************************************************************************************************/
BOOL CONFIG_save(void)
{
	unsigned char *target_ptr;
	unsigned int i, j, eeprom_ptr, checksum;

	eeprom_ptr = 0;

	/* CONFIGリストの処理 */
	for(i = 0; config[i].size != 0; i++){
		ClrWdt();
		target_ptr = config[i].ptr;

		/* 保存対象の構造体を書き込む */
		for(j = 0; j < config[i].size; j++){
			while(!CONFIG_write_and_verify(eeprom_ptr, *target_ptr));
			target_ptr++;
			eeprom_ptr++;
		}
	}

	/* チェックサムの記録 */
	checksum = CONFIG_make_checksum();
	while(!CONFIG_write_and_verify(eeprom_ptr, (checksum >> 8)));
	eeprom_ptr++;
	while(!CONFIG_write_and_verify(eeprom_ptr, (checksum     )));

	return(TRUE);
}


/**************************************************************************************************
読み込む
---------------------------------------------------------------------------------------------------
復帰処理が面倒なので、何かあったら WDTリセットさせる
**************************************************************************************************/
BOOL CONFIG_load(void)
{
	unsigned char *target_ptr;
	unsigned int i, j, eeprom_ptr, checksum, verify, buf;

	eeprom_ptr = 0;

	/* チェックサムを確認する */
	checksum = CONFIG_make_checksum();
	verify   = CONFIG_read_checksum();

	/* チェックサムが異なるときはエラーを返す */
	if(checksum != verify) return(FALSE);


	/* メモリに書き込む */
	for(i = 0; config[i].size != 0; i++){
		ClrWdt();
		target_ptr = config[i].ptr;

		/* 保存対象の構造体を読込 */
		for(j = 0; j < config[i].size; j++){
			while((buf = EEPROM_read(eeprom_ptr)) == -1);
			*target_ptr = buf;
			target_ptr++;
			eeprom_ptr++;
		}
	}



	return(TRUE);
}


/**************************************************************************************************
データを覗く
**************************************************************************************************/
void CONFIG_dump_config(void)
{
	unsigned char *ptr;
	unsigned int i, j;

	for(i = 0; config[i].size != NULL; i++){
		ptr = config[i].ptr;

		UART1_puthex(i);
		UART1_puthex(config[i].size);
		UART1_putch(' ');

		for(j = 0; j < config[i].size; j++){
			ClrWdt();
			UART1_puthex(*ptr);
			UART1_putch(' ');
			ptr++;
			while(UART1_get_sendbuf() < 10);
		}

		UART1_putch('\r');
		UART1_putch('\n');
	}

	return;
}


/*-------------------------------------------------------------------------------------------------
ベリファイ付書込
---------------------------------------------------------------------------------------------------
復帰処理が面倒なので、何かあったら WDTリセットさせる
-------------------------------------------------------------------------------------------------*/
static  BOOL CONFIG_write_and_verify(const unsigned int ptr, const unsigned char data)
{
	int buf;

	do {
		EEPROM_write(ptr, data);
		buf = EEPROM_read(ptr);
	} while (buf != data || buf == -1);

	return(TRUE);
}


/*-------------------------------------------------------------------------------------------------
EEPROMのデータからチェックサムを生成する
-------------------------------------------------------------------------------------------------*/
static  unsigned int CONFIG_make_checksum(void)
{
	int buf;
	unsigned int i, j, eeprom_ptr, checksum;

	eeprom_ptr = 0;
	checksum = 0;

	for(i = 0; config[i].size != 0; i++){
		ClrWdt();

		for(j = 0; j < config[i].size; j++){
			while((buf = EEPROM_read(eeprom_ptr)) == -1);
			checksum += buf;
			eeprom_ptr++;
		}
	}

	return(checksum);
}


/*-------------------------------------------------------------------------------------------------
EEPROMのチェックサムを読み込む
-------------------------------------------------------------------------------------------------*/
static  unsigned int CONFIG_read_checksum(void)
{
	int buf;
	unsigned int i, eeprom_ptr, checksum;

	/* チェックサムのアドレスを求める */
	eeprom_ptr = 0;
	for(i = 0; config[i].size != 0; i++){
		eeprom_ptr += config[i].size;
	}

	/* チェックサムを読み込む */
	while((buf = EEPROM_read(eeprom_ptr)) == -1);
	checksum = buf << 8;
	eeprom_ptr++;
	while((buf = EEPROM_read(eeprom_ptr)) == -1);
	checksum = checksum + buf;

	return(checksum);
}
