/**************************************************************************************************
	Title			: FONTX2 Driver
	Programmer		: Yosuke FURUSAWA
	Copyright		: Copyright (C) 2010 Yosuke FURUSAWA.
	License			: 4-clause BSD License
	Since			: 2010/05/08

	Filename		: libfontx2.c
	Last up date	: 2010/05/13
	Kanji-Code		: Shift-JIS
	TAB Space		: 4

	Note			: In this version, do not support Kanji and Font Selector.
**************************************************************************************************/


/*================================================================================================
ヘッダファイルをインクルード
=================================================================================================*/
#include "types.h"

#include "libfontx2.h"


/*=================================================================================================
構造体
=================================================================================================*/
/* ASCIIコード用半角文字フォントのヘッダ */
typedef struct FONTX2_HEADER_ASCII {
	char ident[6];
	char fontname[8];

	unsigned char width;
	unsigned char height;
	unsigned char codetype;

} FONTX2_HEADER_ASCII_T;
typedef FONTX2_HEADER_ASCII_T* pFONTX2_HEADER_ASCII_T;


/*=================================================================================================
グローバル変数
=================================================================================================*/
const unsigned char fontx2_ident[] = "FONTX2";


/**************************************************************************************************
*****                                                                                         *****
*****           製品組み込み時には、フォントのライセンスを十分に確認すること!                 *****
*****                                                                                         *****
**************************************************************************************************/

/* 1つだけインクルードすること */
const unsigned char fontx2_ascii_data[] = {
//#include "font/4x8.txt"			/* 美咲フォント */
//#include "font/akagi11a.txt"		/* 赤城フォント */
//#include "font/gonhn12x.txt"		/* 細ゴシック体「小伝馬町12」フォント */
//#include "font/gothn12x.txt"		/* 細ゴシック体「小伝馬町12」フォント2 */
//#include "font/jpnhn8x.txt"		/* NEC PC-98  8dot フォント (PC-9821As2から抜きました...) */
//#include "font/jpnhn16x.txt"		/* NEC PC-98 16dot フォント (PC-9821As2から抜きました...) */
//#include "font/k6x10.txt"			/* k12x10 (k6x10) フォント */
//#include "font/kyohn16x.txt"		/* 教科書体「人形町16」フォント */
//#include "font/noho12a.txt"		/* のほ 12dot フォント */
//#include "font/noho16a.txt"		/* のほ 16dot フォント */
//#include "font/mgohn16x.txt"		/* 丸ゴシック体「秋葉原16」フォント */
//#include "font/minhn12x.txt"		/* 太明朝体「道玄坂 12」フォント */
//#include "font/minhn14x.txt"		/* 太明朝体「道玄坂 14」フォント */
//#include "font/minhn16x.txt"		/* 太明朝体「道玄坂 16」フォント */
//#include "font/mplhn10.txt"		/* M+ BITMAP 10dot フォント */
//#include "font/mplhn11.txt"		/* M+ BITMAP 11dot フォント */
//#include "font/mplhn12.txt"		/* M+ BITMAP 12dot フォント */
//#include "font/mplhn13.txt"		/* M+ BITMAP 13dot フォント */
//#include "font/paw16a.txt"		/* ぱうフォント */
//#include "font/reihn16x.txt"		/* 隷書体「八丁堀 16」フォント */
//#include "font/xbghn16x.txt"		/* ファンテール「兜町 16」フォント */
#include "font/shnhn16.txt"			/* 東雲フォント */
//#include "font/8x16rk.txt"		/* X11フォント */
};

const pFONTX2_HEADER_ASCII_T fontx2_ascii = (pFONTX2_HEADER_ASCII_T)&fontx2_ascii_data;


/**************************************************************************************************
初期化
**************************************************************************************************/
BOOL FONTX2_init(void)
{
	/* 内蔵フォントデータを確認する. strcmp()相当 */
	if(fontx2_ident[0] != fontx2_ascii->ident[0])	return(FALSE);
	if(fontx2_ident[1] != fontx2_ascii->ident[1])	return(FALSE);
	if(fontx2_ident[2] != fontx2_ascii->ident[2])	return(FALSE);
	if(fontx2_ident[3] != fontx2_ascii->ident[3])	return(FALSE);
	if(fontx2_ident[4] != fontx2_ascii->ident[4])	return(FALSE);
	if(fontx2_ident[5] != fontx2_ascii->ident[5])	return(FALSE);


	/* フォントサイズが高速化マクロの処理と合っているか確認する */
#ifdef FONT_FAST8
	if(FONTX2_get_ascii_width() != 8)				return(FALSE);
#endif


	return(TRUE);
}


/**************************************************************************************************
1文字の横幅pxを取得する
**************************************************************************************************/
 unsigned char FONTX2_get_ascii_width(void)
{
	return(fontx2_ascii->width);
}
 unsigned char FONTX2_get_ascii_width_byte(void)
{
	return((((fontx2_ascii->width - 1) >> 3) + 1));
}


/**************************************************************************************************
1文字の縦幅pxを取得する
**************************************************************************************************/
 unsigned char FONTX2_get_ascii_height(void)
{
	return(fontx2_ascii->height);
}
 unsigned char FONTX2_get_ascii_height_byte(void)
{
	return(fontx2_ascii->height);
}


/**************************************************************************************************
1文字のフォントサイズ(byte)を取得する
**************************************************************************************************/
 unsigned char FONTX2_get_ascii_size(void)
{
	return( FONTX2_get_ascii_width_byte() * FONTX2_get_ascii_height_byte() );
}


/**************************************************************************************************
1文字のフォントデータのポインタを取得する
**************************************************************************************************/
 unsigned char *FONTX2_get_ascii_font(const unsigned char ascii)
{
	return(&fontx2_ascii_data[		sizeof(FONTX2_HEADER_ASCII_T)
								+	FONTX2_get_ascii_size() * ascii
							]);
}


/**************************************************************************************************
1文字のフォントデータのグリフを得る
**************************************************************************************************/
 unsigned char FONTX2_get_ascii_font_data(const unsigned char ascii, unsigned char x, unsigned char y)
{
	return( fontx2_ascii_data[		sizeof(FONTX2_HEADER_ASCII_T)
								+	FONTX2_get_ascii_size() * ascii
								+	x
								+	FONTX2_get_ascii_width_byte() * y
							]);
}


