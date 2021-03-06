/**************************************************************************************************
	Title			: PIC24F Series NTSC Composit VIDEO/Superimpose Driver
	Programmer		: Yosuke FURUSAWA
	Copyright		: Copyright (C) 2010 Yosuke FURUSAWA.
	License			: 4-clause BSD License
	Since			: 2010/04/28

	Filename		: libvideo.h
	Last up date	: 2010/11/21
	Kanji-Code		: Shift-JIS
	TAB Space		: 4

	Note			: 画面両端の 2pxは、使えない
**************************************************************************************************/


#ifndef _LIBVIDEO_H_
#define _LIBVIDEO_H_


/*=================================================================================================
マクロ定義
=================================================================================================*/
/* 高速化マクロ. 引数チェック等の確認処理を実行しなくなる */
//#define VIDEO_FAST

/* 左右反転マクロ
画面描画が左右反転に描かれる。バックモニタ用カメラなどを流用すると、カメラ出力が
左右反転しているので、それらに対応するため
*/
//#define VIDEO_MIRROR

/* 解像度. 288x224pxまで動作確認済み. メモリ容量に注意 */
#define NTSC_WIDTH					256									/* 16の倍数とすること */
#define NTSC_HEIGHT					192									/* 262 - 13 以下とすること */
#define NTSC_VRAM_SIZE				((NTSC_WIDTH >> 3) * NTSC_HEIGHT)	/* バイト計算 */

#define GRAPH_SIZE					(100)
#define TRACK_SIZE					(5)


/*=================================================================================================
構造体宣言
=================================================================================================*/
/* ビデオ出力の各種パラメータ */
typedef struct NTSC_STATUS {
	unsigned int *vram;									/* 出力用 VRAMポインタ */

	unsigned int line;									/* 水平同期の本数 */
	unsigned int line_sync;								/* 垂直帰線区間 */
	unsigned int line_space_top;						/* 上部の空白部分 */
	unsigned int line_video;							/* 映像部分 */

	unsigned int horizon_pulse;							/* 水平同期パルス */
	unsigned int serration_pulse;						/* 切り込みパルス */
	unsigned int equalizing_pulse;						/* 等価パルス */
	unsigned int left_space;							/* 左側の空白部分 */
	unsigned int video_width;							/* 実際に出力される横の解像度 */

	unsigned int status;								/* ステートマシン */
	unsigned int vsync;									/* 垂直同期カウンタ */
	unsigned int hsync;									/* 水平同期カウンタ */
} NTSC_STATUS_T;
typedef NTSC_STATUS_T* pNTSC_STATUS_T;

/* ドライバ用の構造体 */
typedef struct NTSC {
	unsigned int *vram;									/* ビデオメモリの先頭ポインタ */
	unsigned char output;								/* 映像出力先 */

	NTSC_STATUS_T monitor;								/* モニタ出力用 (SPI1) */
	NTSC_STATUS_T superimpose;							/* スーパーインポーズ用 (SPI2) */
} NTSC_T;
typedef NTSC_T* pNTSC_T;

/* 外部参照(API)用の構造体 */
typedef struct VIDEO {
	char cx, cy;
} VIDEO_T;
typedef VIDEO_T* pVIDEO_T;

typedef struct GRAPH {
	unsigned int x;
	unsigned int y;
	unsigned int width;
	unsigned int height;
	unsigned int ptr;
	unsigned char data[ GRAPH_SIZE ];
} GRAPH_T;
typedef GRAPH_T* pGRAPH_T;

typedef struct METER {
	unsigned int x;
	unsigned int y;
	unsigned char r;
	unsigned int old;
} METER_T;
typedef METER_T* pMETER_T;

typedef struct TRACK {
	unsigned int x;
	unsigned int y;
	unsigned int width;
	unsigned int height;
	unsigned int ptr;
	unsigned char data[ TRACK_SIZE ][2];
} TRACK_T;
typedef TRACK_T* pTRACK_T;


/*=================================================================================================
グローバル変数
=================================================================================================*/
extern NTSC_T ntsc;
extern VIDEO_T video;


/*=================================================================================================
プロトタイプ宣言
=================================================================================================*/
extern void VIDEO_init(void);
extern void VIDEO_init_clock(void);
extern  void VIDEO_vram_clear(unsigned int pattern);
extern  unsigned char VIDEO_get_output(void);
extern  unsigned int VIDEO_get_point(unsigned int x, unsigned int y);


/* 描画関数 */
extern  void VIDEO_point (unsigned int x, unsigned int y);
extern  void VIDEO_point_(unsigned int x, unsigned int y);

extern  void VIDEO_line (int x0, int y0, int x1, int y1);
extern  void VIDEO_line_(int x0, int y0, int x1, int y1);

#define VIDEO_circle(x,y,r)		VIDEO_arc(x, y, r, 0, 360)
#define VIDEO_circle_(x,y,r)	VIDEO_arc(x, y, r, 0, 360)
extern  void VIDEO_arc (unsigned int x, unsigned int y, unsigned int r, unsigned int start, unsigned int end);
extern  void VIDEO_arc_(unsigned int x, unsigned int y, unsigned int r, unsigned int start, unsigned int end);


/* FONTX2 Driver依存関数 */
extern  void VIDEO_locate(unsigned int x, unsigned int y);
extern  void VIDEO_putch(unsigned char c);
extern  void VIDEO_putstr(const char *s);
extern  void VIDEO_puthex(unsigned char a);
extern  void VIDEO_putbin(unsigned char a);
extern  void VIDEO_putuint(unsigned int digit, unsigned char size);
extern  void VIDEO_putint(int digit, unsigned char size);
extern  void VIDEO_putdouble(double digit, unsigned char size, unsigned char size2);


/* グラフ表示 */
extern  void GRAPH_init(pGRAPH_T graph, unsigned int x, unsigned int y, unsigned int width, unsigned int height);
extern  void GRAPH_putdata(pGRAPH_T graph, unsigned int data);
extern  void GRAPH_draw_point(pGRAPH_T graph);
extern  void GRAPH_draw_line(pGRAPH_T graph);
extern  void GRAPH_draw_bar(pGRAPH_T graph);


/* メータ表示 */
extern  void METER_init(pMETER_T meter, unsigned int x, unsigned int y, unsigned char r);
extern  void METER_draw(pMETER_T meter, unsigned int value);


/* 軌跡表示 */
extern  void TRACK_init(pTRACK_T track, unsigned int x, unsigned int y, unsigned int width, unsigned int height);
extern  void TRACK_putdata(pTRACK_T track, unsigned int x, unsigned int y);
extern  void TRACK_draw_point(pTRACK_T track);


#endif
