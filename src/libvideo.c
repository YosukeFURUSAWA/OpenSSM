/**************************************************************************************************
	Title			: PIC24F Series NTSC Composit VIDEO/Superimpose Driver
	Programmer		: Yosuke FURUSAWA
	Copyright		: Copyright (C) 2010 Yosuke FURUSAWA.
	License			: 4-clause BSD License
	Since			: 2010/04/28

	Filename		: libvideo.c
	Last up date	: 2010/11/21
	Kanji-Code		: Shift-JIS
	TAB Space		: 4

	Note			: 画面両端の 2pxは、使えない
**************************************************************************************************/


/*=================================================================================================
ヘッダファイルをインクロード
=================================================================================================*/
#include <p24FJ64GA002.h>

#include "types.h"
#include "table.h"

#include "librtc.h"
#include "libfontx2.h"
#include "libvideo.h"


/*=================================================================================================
マクロ定義
=================================================================================================*/
#define abs(a)						(((a)>0) ? (a) : -(a))

/* 同期信号出力 */
#define NTSC_SYNC					LATBbits.LATB12
#define NTSC_VIDEO					LATBbits.LATB13

/* NTSC-M 標準設定 */
#define NTSC_LINE					263			/* インターレース 525本の半分 */
#define NTSC_LINE_SYNC				13			/* 垂直帰線区間. 標準11H, ノンインターレースとして +2 */
#define NTSC_LINE_SPACE_TOP			29			/* 上部の空白部分 */

/* パルスの標準時間 @ Fcy = 16MHz */
#define NTSC_DEFAULT_FCY			cpu_fcy[0]
#define NTSC_HORIZON_PULSE			1015
#define NTSC_SERRATION_PULSE		936
#define NTSC_EQUALIZING_PULSE		70
#define NTSC_LEFT_SPACE				180
#define NTSC_VIDEO_WIDTH			256


/*=================================================================================================
構造体
=================================================================================================*/


/*=================================================================================================
グローバル変数
=================================================================================================*/
/* VRAM */
static unsigned int __attribute__((far)) video_memory[ NTSC_VRAM_SIZE / 2 ];

/* ドライバ用 */
NTSC_T ntsc = {
	video_memory,
	0,

	{
		video_memory,
		NTSC_LINE,
		NTSC_LINE_SYNC,
		NTSC_LINE_SYNC + NTSC_LINE_SPACE_TOP,
		NTSC_LINE_SYNC + NTSC_LINE_SPACE_TOP + NTSC_HEIGHT,
		NTSC_HORIZON_PULSE,
		NTSC_SERRATION_PULSE,
		NTSC_EQUALIZING_PULSE,
		NTSC_LEFT_SPACE,
		NTSC_WIDTH,
		0x0000,
		0x0000,
		0x0000,
	},

	{
		video_memory,
		NTSC_LINE,
		NTSC_LINE_SYNC,
		NTSC_LINE_SYNC + NTSC_LINE_SPACE_TOP + 10,
		NTSC_LINE_SYNC + NTSC_LINE_SPACE_TOP + 10 + NTSC_HEIGHT,
		NTSC_HORIZON_PULSE,
		NTSC_SERRATION_PULSE,
		NTSC_EQUALIZING_PULSE,
		NTSC_LEFT_SPACE,
		NTSC_WIDTH,
		0x0000,
		0x0000,
		0x0000,
	},

};

/* API用 */
VIDEO_T video;


/**************************************************************************************************
プロトタイプ宣言
**************************************************************************************************/
 static void VIDEO_init_superimpose(void);


/**************************************************************************************************
初期化
**************************************************************************************************/
void VIDEO_init(void)
{
	/* 変数初期化 */
	video.cx = 0;
	video.cy = 0;
	VIDEO_vram_clear(0xaaaa);

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

	/* モニタ出力用の初期化 */
	VIDEO_init_clock();
	RPOR6bits.RP13R = 7;

	if(ntsc.monitor.video_width > NTSC_WIDTH)	ntsc.monitor.video_width = NTSC_WIDTH;
	ntsc.monitor.video_width = ntsc.monitor.video_width >> 4;
	ntsc.superimpose.video_width	= ntsc.monitor.video_width;

	/*           FEDBCA9876543210 */
	SPI1CON1 = 0b0000010000110111;
	SPI1CON2 = 0b0000000000000000;
	SPI1STAT = 0b1000000000000000;
	IPC2bits.SPI1IP = 7;
	IEC0bits.SPI1IE = 1;
	IFS0bits.SPI1IF = 0;
	IPC0bits.OC1IP = 7;
	IEC0bits.OC1IE = 1;
	IFS0bits.OC1IF = 0;
	IPC1bits.OC2IP = 7;
	IEC0bits.OC2IE = 1;
	IFS0bits.OC2IF = 0;

	PR2 = ntsc.monitor.horizon_pulse;
	TMR2 = 0x0000;
	IPC1bits.T2IP = 7;
	IEC0bits.T2IE = 1;
	IFS0bits.T2IF = 0;

	/*        FEDBCA9876543210 */
	T2CON = 0b1000000000000000;

	/* スーパーインポーズ用の初期化 */
	VIDEO_init_superimpose();

	return;
}

 static void VIDEO_init_superimpose(void)
{
	RPOR7bits.RP15R = 10;

	/* VSYNC割り込みの有効化 */
	RPINR0bits.INT1R = 10;			/* RP10 INT1 */
	INTCON2bits.INT1EP = 1;			/* 立ち下がり */
	IPC5bits.INT1IP = 6;
	IEC1bits.INT1IE = 1;
	IFS1bits.INT1IF = 0;

	/* HSYNC割り込みの有効化 */
	RPINR1bits.INT2R = 11;			/* RP11 INT2 */
	INTCON2bits.INT2EP = 0;			/* 立ち上がり */
	IPC7bits.INT2IP = 6;
	IEC1bits.INT2IE = 1;
	IFS1bits.INT2IF = 0;

	/* ビデオ出力用 */
	/*           FEDBCA9876543210 */
	SPI2CON1 = 0b0000010000110111;
	SPI2CON2 = 0b0000000000000000;
	SPI2STAT = 0b1000000000000000;
	IPC8bits.SPI2IP = 6;
	IEC2bits.SPI2IE = 1;
	IFS2bits.SPI2IF = 0;

	IPC6bits.OC3IP = 6;
	IEC1bits.OC3IE = 0;
	IFS1bits.OC3IF = 0;

	PR3  = 0xffff;
	TMR3 = 0x0000;
	T3CON = 0x8000;
	IPC2bits.T3IP   = 6;
	IEC0bits.T3IE   = 1;
	IFS0bits.T3IF   = 0;

	return;
}


/**************************************************************************************************
割り込み周期の初期化
**************************************************************************************************/
void VIDEO_init_clock(void)
{
	ntsc.monitor.horizon_pulse		= (double)NTSC_HORIZON_PULSE	* ((double)cpu_fcy[ OSCTUN ] / (double)NTSC_DEFAULT_FCY);
	ntsc.monitor.serration_pulse	= (double)NTSC_SERRATION_PULSE	* ((double)cpu_fcy[ OSCTUN ] / (double)NTSC_DEFAULT_FCY);
	ntsc.monitor.equalizing_pulse	= (double)NTSC_EQUALIZING_PULSE	* ((double)cpu_fcy[ OSCTUN ] / (double)NTSC_DEFAULT_FCY);
	ntsc.monitor.left_space			= (double)NTSC_LEFT_SPACE		* ((double)cpu_fcy[ OSCTUN ] / (double)NTSC_DEFAULT_FCY);
	ntsc.monitor.video_width		= (double)NTSC_VIDEO_WIDTH		* ((double)cpu_fcy[ OSCTUN ] / (double)NTSC_DEFAULT_FCY);

	ntsc.superimpose.left_space		= (double)60					* ((double)cpu_fcy[ OSCTUN ] / (double)NTSC_DEFAULT_FCY);

	return;
}


/**************************************************************************************************
モニター出力用 NTSC信号処理関数
**************************************************************************************************/
void __attribute__((interrupt, no_auto_psv, shadow)) _T2Interrupt(void)
{
	IFS0bits.T2IF = 0;
	NTSC_SYNC = 0;
	ntsc.monitor.vsync++;

	/* 切り込みパルス */
	if(ntsc.monitor.vsync < ntsc.monitor.line_sync){
		OC1R = ntsc.monitor.serration_pulse;
		OC1CON = 0x0001;
		ntsc.output = 0;

	/* 上部空白 */
	} else if(ntsc.monitor.vsync < ntsc.monitor.line_space_top){
		OC1R = ntsc.monitor.equalizing_pulse;
		OC1CON = 0x0001;

	/* 映像出力 */
	} else if(ntsc.monitor.vsync < ntsc.monitor.line_video){
		OC1R = ntsc.monitor.equalizing_pulse;
		OC1CON = 0x0001;
		OC2R = ntsc.monitor.left_space;
		OC2CON = 0x0001;

	/* 下部空白 */
	} else if(ntsc.monitor.vsync < ntsc.monitor.line - 1){
		OC1R = ntsc.monitor.equalizing_pulse;
		OC1CON = 0x0001;

	} else {
		OC1R = ntsc.monitor.equalizing_pulse;
		OC1CON = 0x0001;
		PR2 = ntsc.monitor.horizon_pulse;
		ntsc.monitor.vsync = 0;
		ntsc.monitor.vram = ntsc.vram;

		/* モニタ出力のときはインポーズ用信号を Highに固定し、LM1881のノイズ耐性を上げる */
		RPOR7bits.RP15R = 0;
		LATBbits.LATB15 = 1;
		ntsc.output = 0;

	}

	return;
}

void __attribute__((interrupt, no_auto_psv, shadow)) _OC1Interrupt(void)
{
	IFS0bits.OC1IF = 0;
	NTSC_SYNC = 1;
	return;
}


void __attribute__((interrupt, no_auto_psv, shadow)) _OC2Interrupt(void)
{
	unsigned int dummy;

	IFS0bits.OC2IF = 0;

	SPI1BUF = *ntsc.monitor.vram++;
	ntsc.monitor.hsync = 2;
	while(SPI1STATbits.SPITBF);
	dummy = SPI1BUF;
	SPI1BUF = *ntsc.monitor.vram++;
	return;
}

void __attribute__((interrupt, no_auto_psv, shadow)) _SPI1Interrupt(void)
{
	unsigned int dummy;

	IFS0bits.SPI1IF = 0;

	if(ntsc.monitor.hsync == 0){
		SPI1BUF = (*ntsc.monitor.vram++) & 0x7fff;					/* 左端を描画しない */
		ntsc.monitor.hsync++;

	} else if(ntsc.monitor.hsync < ntsc.monitor.video_width - 1){
		SPI1BUF = *ntsc.monitor.vram++;
		ntsc.monitor.hsync++;

	} else if(ntsc.monitor.hsync < ntsc.monitor.video_width){
		SPI1BUF = (*ntsc.monitor.vram++) & 0xfffe;					/* 右端を描画しない */
		ntsc.monitor.hsync++;

	} else {
		for(;ntsc.monitor.hsync < (NTSC_WIDTH >> 4); ntsc.monitor.hsync++){
			ntsc.monitor.vram++;
		}
	}

	dummy = SPI1BUF;
	return;
}


/**************************************************************************************************
スーパーインポーズ用 NTSC信号処理関数 (タイミングがシビアすぎて、これ以上の調整不可...)
**************************************************************************************************/
void __attribute__((interrupt, auto_psv)) _INT1Interrupt(void)
{
	unsigned int dummy;

	IFS1bits.INT1IF = 0;

	/* モニタ出力を止める */
	IEC0bits.T2IE = 0;

	/* 原因はよく分からないが、初期化すると面白いほど同期が取れる... */
	VIDEO_init_superimpose();
	ntsc.output = 1;

	dummy = SPI2BUF;
	ntsc.superimpose.vram  = ntsc.vram;
	ntsc.superimpose.vsync = 0;

	return;
}


void __attribute__((interrupt, auto_psv)) _INT2Interrupt(void)
{
	unsigned int dummy;

	IFS1bits.INT2IF = 0;

	ntsc.superimpose.vsync++;

	/* 上部空白 */
	if(ntsc.superimpose.vsync < ntsc.superimpose.line_space_top){
		/* 何もしない */

	/* 映像出力 */
	} else if (ntsc.superimpose.vsync < ntsc.superimpose.line_video){
		TMR3 = 0x0000;
		OC3R = ntsc.superimpose.left_space;
		OC3CON = 0b0000000000001001;
		IEC1bits.OC3IE  = 1;
		dummy = SPI2BUF;

	} else {
		IEC1bits.INT2IE = 0;
		dummy = SPI2BUF;
	}

	return;
}

void __attribute__((interrupt, auto_psv)) _OC3Interrupt(void)
{
	unsigned int dummy;

	IFS1bits.OC3IF = 0;

	SPI2BUF = *ntsc.superimpose.vram++;
	ntsc.superimpose.hsync = 2;
	while(SPI2STATbits.SPITBF);
	dummy = SPI2BUF;
	SPI2BUF = *ntsc.superimpose.vram++;

	IEC1bits.OC3IE = 0;
	return;
}

void __attribute__((interrupt, auto_psv)) _SPI2Interrupt(void)
{
	unsigned int dummy;

	IFS2bits.SPI2IF = 0;

	if(ntsc.superimpose.hsync == 0){
		SPI2BUF = (*ntsc.superimpose.vram++) & 0x7fff;				/* 左端を描画しない */
		ntsc.superimpose.hsync++;

	} else if(ntsc.superimpose.hsync < ntsc.superimpose.video_width - 1){
		SPI2BUF = *ntsc.superimpose.vram++;
		ntsc.superimpose.hsync++;

	} else if(ntsc.superimpose.hsync < ntsc.superimpose.video_width){
		SPI2BUF = (*ntsc.superimpose.vram++) & 0xfffe;				/* 右端を描画しない */
		ntsc.superimpose.hsync++;

	} else {
		for(;ntsc.superimpose.hsync < (NTSC_WIDTH >> 4); ntsc.superimpose.hsync++){
			ntsc.superimpose.vram++;
		}
	}

	dummy = SPI2BUF;
	return;
}

void __attribute__((interrupt, auto_psv)) _T3Interrupt(void)
{
	/* スーパーインポーズ用の外部ビデオ信号がないときに実行される */
	IFS0bits.T3IF = 0;

	IEC0bits.T2IE = 1;
	IEC0bits.T3IE = 0;
	IEC1bits.INT2IE = 0;

	ntsc.output = 0;

	return;
}



/**************************************************************************************************
VRAMの初期化
**************************************************************************************************/
 void VIDEO_vram_clear(unsigned int pattern)
{
	unsigned int i, *vram;

	vram = ntsc.vram;

	for(i = 0; i < (NTSC_VRAM_SIZE >> 1); i++){
		*vram = pattern;
		vram++;
	}

	return;
}


/**************************************************************************************************
出力先を得る
**************************************************************************************************/
 unsigned char VIDEO_get_output(void)
{
	return(ntsc.output);
}


/**************************************************************************************************
点の有無を得る
**************************************************************************************************/
 unsigned int VIDEO_get_point(unsigned int x, unsigned int y)
{
	const unsigned int table[] = {
	/*    FEDCBA9876543210 */
		0b1000000000000000,		/* 0 */
		0b0100000000000000,		/* 1 */
		0b0010000000000000,		/* 2 */
		0b0001000000000000,		/* 3 */
		0b0000100000000000,		/* 4 */
		0b0000010000000000,		/* 5 */
		0b0000001000000000,		/* 6 */
		0b0000000100000000,		/* 7 */
		0b0000000010000000,		/* 8 */
		0b0000000001000000,		/* 9 */
		0b0000000000100000,		/* a */
		0b0000000000010000,		/* b */
		0b0000000000001000,		/* c */
		0b0000000000000100,		/* d */
		0b0000000000000010,		/* e */
		0b0000000000000001,		/* f */
	};

#ifndef VIDEO_FAST
	if(x >= NTSC_WIDTH)		x = NTSC_WIDTH  - 1;
	if(y >= NTSC_HEIGHT)	y = NTSC_HEIGHT - 1;
#endif

#ifdef VIDEO_MIRROR
	x = (NTSC_WIDTH - 1) - x;
#endif

	return(ntsc.vram[ (y << 4) + (x >> 4) ] & table[ (x & 0x0f) ] ? 1 : 0);
}


/**************************************************************************************************
点を描く / 消す
**************************************************************************************************/
 void VIDEO_point(unsigned int x, unsigned int y)
{
#ifndef VIDEO_FAST
	if(x >= NTSC_WIDTH)		x = NTSC_WIDTH  - 1;
	if(y >= NTSC_HEIGHT)	y = NTSC_HEIGHT - 1;
#endif

#ifdef VIDEO_MIRROR
	x = (NTSC_WIDTH - 1) - x;
#endif

	ntsc.vram[ ((NTSC_HEIGHT - 1 - y) << 4) + (x >> 4) ] |=  (0x8000 >> (x & 0x0f));
	return;
}
 void VIDEO_point_(unsigned int x, unsigned int y)
{
#ifndef VIDEO_FAST
	if(x >= NTSC_WIDTH)		x = NTSC_WIDTH  - 1;
	if(y >= NTSC_HEIGHT)	y = NTSC_HEIGHT - 1;
#endif

#ifdef VIDEO_MIRROR
	x = (NTSC_WIDTH - 1) - x;
#endif

	ntsc.vram[ ((NTSC_HEIGHT - 1 - y) << 4) + (x >> 4) ] &= ~(0x8000 >> (x & 0x0f));
	return;
}


/**************************************************************************************************
直線を描く / 消す (Bresenham style)
**************************************************************************************************/
 void VIDEO_line(int x0, int y0, int x1, int y1)
{
	int steep, t;
	int	deltax, deltay, error;
	int x, y;
	int ystep;

#ifndef VIDEO_FAST
	if(x0 < 0)				x0 = 0;
	if(x0 >= NTSC_WIDTH)	x0 = NTSC_WIDTH  - 1;
	if(x1 < 0)				x1 = 0;
	if(x1 >= NTSC_WIDTH)	x1 = NTSC_WIDTH  - 1;
	if(y0 < 0)				y0 = 0;
	if(y0 >= NTSC_HEIGHT)	y0 = NTSC_HEIGHT - 1;
	if(y1 < 0)				y1 = 0;
	if(y1 >= NTSC_HEIGHT)	y1 = NTSC_HEIGHT - 1;
#endif

	steep = (abs(y1 - y0) > abs(x1 - x0));

	if(steep){
		t = x0; x0 = y0; y0 = t;
		t = x1; x1 = y1; y1 = t;
	}
	if(x0 > x1) {
		t = x0; x0 = x1; x1 = t;
		t = y0; y0 = y1; y1 = t;
	}

	deltax = x1 - x0;
	deltay = abs(y1 - y0);
	error = 0;
	y = y0;

	if(y0 < y1)	ystep =  1;
	else		ystep = -1;

	if(steep){
		for(x = x0; x < x1; x++) {
			VIDEO_point(y,x);

			error += deltay;

			if((error << 1) >= deltax){
				y     += ystep;
				error -= deltax;
			}
		}
	} else {
		for(x = x0; x < x1; x++) {
			VIDEO_point(x,y);

			error += deltay;

			if((error << 1) >= deltax){
				y     += ystep;
				error -= deltax;
			}
		}
	}

	return;
}

 void VIDEO_line_(int x0, int y0, int x1, int y1)
{
	int steep, t;
	int	deltax, deltay, error;
	int x, y;
	int ystep;

#ifndef VIDEO_FAST
	if(x0 < 0)				x0 = 0;
	if(x0 >= NTSC_WIDTH)	x0 = NTSC_WIDTH  - 1;
	if(x1 < 0)				x1 = 0;
	if(x1 >= NTSC_WIDTH)	x1 = NTSC_WIDTH  - 1;
	if(y0 < 0)				y0 = 0;
	if(y0 >= NTSC_HEIGHT)	y0 = NTSC_HEIGHT - 1;
	if(y1 < 0)				y1 = 0;
	if(y1 >= NTSC_HEIGHT)	y1 = NTSC_HEIGHT - 1;
#endif

	steep = (abs(y1 - y0) > abs(x1 - x0));

	if(steep){
		t = x0; x0 = y0; y0 = t;
		t = x1; x1 = y1; y1 = t;
	}
	if(x0 > x1) {
		t = x0; x0 = x1; x1 = t;
		t = y0; y0 = y1; y1 = t;
	}

	deltax = x1 - x0;
	deltay = abs(y1 - y0);
	error = 0;
	y = y0;

	if(y0 < y1)	ystep =  1;
	else		ystep = -1;

	if(steep){
		for(x = x0; x < x1; x++) {
			VIDEO_point_(y,x);

			error += deltay;

			if((error << 1) >= deltax){
				y     += ystep;
				error -= deltax;
			}
		}
	} else {
		for(x = x0; x < x1; x++) {
			VIDEO_point_(x,y);

			error += deltay;

			if((error << 1) >= deltax){
				y     += ystep;
				error -= deltax;
			}
		}
	}

	return;
}


/**************************************************************************************************
円または孤を描く / 消す
**************************************************************************************************/
 void VIDEO_arc (unsigned int x, unsigned int y, unsigned int r, unsigned int start, unsigned int end)
{
	unsigned int deg;

#ifndef VIDEO_FAST
	if(start > end){
		deg   = end;
		end   = start;
		start = deg;
	}
#endif

	for(deg = start; deg < end; deg++){
		VIDEO_point (x + (double)r * tcos[deg], y + (double)r * tsin[deg]);
	}

	return;
}

 void VIDEO_arc_(unsigned int x, unsigned int y, unsigned int r, unsigned int start, unsigned int end)
{
	unsigned int deg;

#ifndef VIDEO_FAST
	if(start > end){
		deg   = end;
		end   = start;
		start = deg;
	}
#endif

	for(deg = start; deg < end; deg++){
		VIDEO_point_(x + (double)r * tcos[deg], y + (double)r * tsin[deg]);
	}

	return;
}




/**************************************************************************************************
*****                                                                                         *****
*****                        FONTX2 Driver 依存関数群                                         *****
*****                                                                                         *****
**************************************************************************************************/


#ifdef VIDEO_MIRROR
 static unsigned char reverse (unsigned char buf)
{
	return(
			((buf & 0b00000001) << 7)
		+	((buf & 0b00000010) << 5)
		+	((buf & 0b00000100) << 3)
		+	((buf & 0b00001000) << 1)
		+	((buf & 0b00010000) >> 1)
		+	((buf & 0b00100000) >> 3)
		+	((buf & 0b01000000) >> 5)
		+	((buf & 0b10000000) >> 7)
	);

}
#endif


/**************************************************************************************************
キャラクタ側の座標をセット
**************************************************************************************************/
 void VIDEO_locate(unsigned int x, unsigned int y)
{
#ifdef VIDEO_MIRROR
	video.cx = NTSC_WIDTH / FONTX2_get_ascii_width() - x - 1;
#else
	video.cx = x;
#endif
	video.cy = y;
	return;
}


/**************************************************************************************************
文字を描く / VRAMへフォントを転送する
**************************************************************************************************/
 void VIDEO_putch(unsigned char c)
{
	/* 文字列の連続出力のための、折り返し処理 */
#ifdef VIDEO_MIRROR
	if(video.cx == 0){
		video.cx = NTSC_WIDTH / FONTX2_get_ascii_width();
		//video.cy++;
	}
#else
	if(video.cx >= (NTSC_WIDTH / FONTX2_get_ascii_width())){
		video.cx = 0;
		video.cy++;
	}
#endif

	if(video.cy >= (NTSC_HEIGHT / FONTX2_get_ascii_height())){
		video.cy = 0;
	}

#if defined FONT_FAST8
{
	/* フォントをバイト単位で VRAMへ転送する */
	unsigned char i;
	unsigned int *vram;
	unsigned char *font;

	vram = &ntsc.vram[ video.cy * FONTX2_get_ascii_height() * (NTSC_WIDTH >> 4) + (video.cx >> 1)];
	font = FONTX2_get_ascii_font(c);

	if(video.cx & 0x01){
		for(i = 0; i < FONTX2_get_ascii_height(); i++){
			*vram &= 0xff00;
#ifdef VIDEO_MIRROR
			*vram |= (int)reverse(*font++);
#else
			*vram |= (int)(*font++);
#endif
			 vram += (NTSC_WIDTH >> 4);
		}
	} else {
		for(i = 0; i < FONTX2_get_ascii_height(); i++){
			*vram &= 0x00ff;
#ifdef VIDEO_MIRROR
			*vram |= (int)reverse(*font++) << 8;
#else
			*vram |= (int)(*font++) << 8;
#endif
			 vram += (NTSC_WIDTH >> 4);
		}	
	}
}
#else
{
	/* フォントを VRAMへ転送する。遅いけれど、byte単位以外の変態フォントサイズに対応するため */

	unsigned int x, y, i, j, k;
	unsigned char grif;

	for(y = 0; y < FONTX2_get_ascii_height(); y++){

		i = FONTX2_get_ascii_height() * video.cy + y;

		for(j = 0; j < FONTX2_get_ascii_width_byte(); j++){

#ifdef VIDEO_MIRROR
			grif = reverse(FONTX2_get_ascii_font_data(c, j, y));
#else
			grif = FONTX2_get_ascii_font_data(c, j, y);
#endif

			for(x = 0; x < 8 && (j * 8 + x) < FONTX2_get_ascii_width(); x++){

				k = x + video.cx * FONTX2_get_ascii_width();

				if(grif & 0x80)	ntsc.vram[(i << 4) + (k >> 4)] |=  (0x8000 >> (k & 0x0F));
				else			ntsc.vram[(i << 4) + (k >> 4)] &= ~(0x8000 >> (k & 0x0F));

				grif = grif << 1;
			}
		}
	}
}
#endif

#ifdef VIDEO_MIRROR
	video.cx--;
#else
	video.cx++;
#endif
	return;
}


/**************************************************************************************************
文字列を描く
**************************************************************************************************/
 void VIDEO_putstr(const char *s)
{
	while(*s){
		VIDEO_putch(*s++);
	}

	return;
}

/**************************************************************************************************
16進を描く
**************************************************************************************************/
 void VIDEO_puthex(unsigned char a)
{
	VIDEO_putch(hex[(a >> 4) & 0x0f]);
	VIDEO_putch(hex[ a		 & 0x0f]);
	return;
}


/**************************************************************************************************
2進を描く
**************************************************************************************************/
 void VIDEO_putbin(unsigned char a)
{
	VIDEO_putch('0' + ((a >> 7) & 0x01));
	VIDEO_putch('0' + ((a >> 6) & 0x01));
	VIDEO_putch('0' + ((a >> 5) & 0x01));
	VIDEO_putch('0' + ((a >> 4) & 0x01));
	VIDEO_putch('0' + ((a >> 3) & 0x01));
	VIDEO_putch('0' + ((a >> 2) & 0x01));
	VIDEO_putch('0' + ((a >> 1) & 0x01));
	VIDEO_putch('0' + ( a       & 0x01));

	return;
}


/**************************************************************************************************
10進を描く (printfの代わり)
**************************************************************************************************/
 void VIDEO_putuint(unsigned int digit, unsigned char size)
{

	if(size > 4){ if(digit > 9999){ VIDEO_putch('0' + digit / 10000 % 10); } else { VIDEO_putch(' '); }; };
	if(size > 3){ if(digit >  999){ VIDEO_putch('0' + digit /  1000 % 10); } else { VIDEO_putch(' '); }; };
	if(size > 2){ if(digit >   99){ VIDEO_putch('0' + digit /   100 % 10); } else { VIDEO_putch(' '); }; };
	if(size > 1){ if(digit >    9){ VIDEO_putch('0' + digit /    10 % 10); } else { VIDEO_putch(' '); }; };

	VIDEO_putch('0' + digit % 10);
	return;
}


 void VIDEO_putint(int digit, unsigned char size)
{
	if(digit < 0)	VIDEO_putch('-');
	else			VIDEO_putch(' ');

	digit = abs(digit);
	if(size > 4){ if(digit > 9999){ VIDEO_putch('0' + digit / 10000 % 10); } else { VIDEO_putch(' '); }; };
	if(size > 3){ if(digit >  999){ VIDEO_putch('0' + digit /  1000 % 10); } else { VIDEO_putch(' '); }; };
	if(size > 2){ if(digit >   99){ VIDEO_putch('0' + digit /   100 % 10); } else { VIDEO_putch(' '); }; };
	if(size > 1){ if(digit >    9){ VIDEO_putch('0' + digit /    10 % 10); } else { VIDEO_putch(' '); }; };
	VIDEO_putch('0' + digit % 10);
	return;
}


 void VIDEO_putdouble(double digit, unsigned char size, unsigned char size2)
{
	if(digit < 0)	VIDEO_putch('-');
	else			VIDEO_putch(' ');

	digit = abs(digit);

	if(size > 4){ if((unsigned int)digit > 9999){ VIDEO_putch('0' + (unsigned char)((unsigned int)(digit / 10000) % 10)); } else { VIDEO_putch(' '); }; };
	if(size > 3){ if((unsigned int)digit >  999){ VIDEO_putch('0' + (unsigned char)((unsigned int)(digit /  1000) % 10)); } else { VIDEO_putch(' '); }; };
	if(size > 2){ if((unsigned int)digit >   99){ VIDEO_putch('0' + (unsigned char)((unsigned int)(digit /   100) % 10)); } else { VIDEO_putch(' '); }; };
	if(size > 1){ if((unsigned int)digit >    9){ VIDEO_putch('0' + (unsigned char)((unsigned int)(digit /    10) % 10)); } else { VIDEO_putch(' '); }; };
	VIDEO_putch('0' + (unsigned char)((unsigned int)digit % 10));
	VIDEO_putch('.');
	VIDEO_putch('0' + (unsigned int)((unsigned int)(digit * 10.0) % 10));
	if(size2 > 1)	VIDEO_putch('0' + (unsigned int)((unsigned int)(digit *   100.0) % 10));
	if(size2 > 2)	VIDEO_putch('0' + (unsigned int)((unsigned int)(digit *  1000.0) % 10));
	if(size2 > 3)	VIDEO_putch('0' + (unsigned int)((unsigned int)(digit * 10000.0) % 10));

	return;
}


/**************************************************************************************************
グラフ初期化
**************************************************************************************************/
 void GRAPH_init(pGRAPH_T graph, unsigned int x, unsigned int y, unsigned int width, unsigned int height)
{
	unsigned int i;

#ifndef VIDEO_FAST							/* このブロックは未検証 */
	if(width >= GRAPH_SIZE){
		width = GRAPH_SIZE;
	}
	if(x + width > NTSC_WIDTH){
		width = NTSC_WIDTH - x;
	}
	if(y + height > NTSC_HEIGHT){
		height = NTSC_HEIGHT - y;
	}
#endif

	graph->x = x;
	graph->y = y;
	graph->width = width;
	graph->height = height;
	graph->ptr = 0;

	for(i = 0; i < GRAPH_SIZE; i++){
		graph->data[i] = 0;
	}

	return;
}


/**************************************************************************************************
グラフにデータを入れる
**************************************************************************************************/
 void GRAPH_putdata(pGRAPH_T graph, unsigned int data)
{
	if(data > 100) data = 100;

	graph->data[graph->ptr] = data;
	graph->ptr = (graph->ptr + 1) % graph->width;

	return;
}


/**************************************************************************************************
グラフを描く
**************************************************************************************************/
 void GRAPH_draw_point(pGRAPH_T graph)
{
	unsigned int i, w, x, y;

	for(i = graph->y; i < graph->y + graph->height; i++){
		VIDEO_point_(graph->x, i);
	}

	w = graph->width * 2;
	for(i = graph->width; i < w; i++){
		x = graph->x + i - graph->width;

		y = graph->y + (((unsigned int)graph->data[(graph->ptr + i - 1) % graph->width] *  graph->height) / 100);
		VIDEO_point_(x, y);

		y = graph->y + (((unsigned int)graph->data[(graph->ptr + i    ) % graph->width] *  graph->height) / 100);
		VIDEO_point(x, y);
	}

	return;
}

 void GRAPH_draw_line(pGRAPH_T graph)
{
	unsigned int i, w, x, y1, y2, y3;

	w = graph->width * 2 - 2;
	for(i = graph->width + 1; i < w; i++){
		x = graph->x + i - graph->width;
		y1 = graph->y + (((unsigned int)graph->data[(graph->ptr + i - 1) % graph->width] * graph->height) / 100);
		y2 = graph->y + (((unsigned int)graph->data[(graph->ptr + i    ) % graph->width] * graph->height) / 100);
		y3 = graph->y + (((unsigned int)graph->data[(graph->ptr + i + 1) % graph->width] * graph->height) / 100);
		VIDEO_line_(x, y1, x + 1, y2);
		VIDEO_line (x, y2, x + 1, y3);
	}

	for(i = graph->y; i < graph->y + graph->height; i++){
		VIDEO_point_(graph->x, i);
		VIDEO_point_(graph->x + graph->width, i);
	}

	return;
}

 void GRAPH_draw_bar(pGRAPH_T graph)
{
	unsigned int i, x, y;

	for(i = 0; i < graph->width; i++){
		x = graph->x + i;
		y = graph->y + (((unsigned int)graph->data[(graph->ptr + i) % graph->width] *  graph->height) / 100);
		VIDEO_line_(x, graph->y, x, y);
		VIDEO_line (x, y       , x, graph->y + graph->height);
	}

	return;
}


/**************************************************************************************************
メータ初期化
**************************************************************************************************/
 void METER_init(pMETER_T meter, unsigned int x, unsigned int y, unsigned char r)
{
	meter->x = x;
	meter->y = y;
	meter->r = r;
	meter->old = 0;

	VIDEO_arc(x, y, r, 270, 360);

	return;
}


/**************************************************************************************************
メータを描く
**************************************************************************************************/
 void METER_draw(pMETER_T meter, unsigned int value)
{
	double rc, rs;

	if(value > 100) value = 100;
	rc = (double)meter->r * tcos[0];
	rs = (double)meter->r * tsin[0];
	VIDEO_line(meter->x + rc, meter->y + rs, meter->x + 0.7 * rc, meter->y + 0.7 * rs);

	rc = (double)meter->r * tcos[270];
	rs = (double)meter->r * tsin[270];
	VIDEO_line(meter->x + rc, meter->y + rs, meter->x + 0.7 * rc, meter->y + 0.7 * rs);

	rc = (double)meter->r * tcos[meter->old];
	rs = (double)meter->r * tsin[meter->old];
	VIDEO_line_(meter->x + rc, meter->y + rs, meter->x + 0.7 * rc, meter->y + 0.7 * rs);

	meter->old = ((100 - value) * 27) / 10;
	rc = (double)meter->r * tcos[meter->old];
	rs = (double)meter->r * tsin[meter->old];
	VIDEO_line (meter->x + rc, meter->y + rs, meter->x + 0.7 * rc, meter->y + 0.7 * rs);

	return;
}


/**************************************************************************************************
トラッカー初期化
**************************************************************************************************/
 void TRACK_init(pTRACK_T track, unsigned int x, unsigned int y, unsigned int width, unsigned int height)
{
	unsigned int i;

	track->x = x;
	track->y = y;
	track->width = width;
	track->height = height;
	track->ptr = 0;

	for(i = 0; i < TRACK_SIZE; i++){
		track->data[i][0] = 0;
		track->data[i][1] = 0;
	}

	return;
}


/**************************************************************************************************
トラッカーにデータを入れる
**************************************************************************************************/
 void TRACK_putdata(pTRACK_T track, unsigned int x, unsigned int y)
{
	if(x > 100) x = 100;
	if(y > 100) y = 100;

	track->ptr = (track->ptr + 1) % TRACK_SIZE;
	track->data[ track->ptr ][0] = x;
	track->data[ track->ptr ][1] = y;

	return;
}


/**************************************************************************************************
トラッカーを描く
**************************************************************************************************/
 void TRACK_draw_point(pTRACK_T track)
{
	unsigned int tmp;

	/* 消す */
	tmp = (track->ptr + 1) % TRACK_SIZE;
	VIDEO_point_(track->x + (track->width  * track->data[ tmp ][0]) / 100,
				 track->y + (track->height * track->data[ tmp ][1]) / 100);

	VIDEO_point_(track->x + (track->width  * track->data[ tmp ][0]) / 100 + 1,
				 track->y + (track->height * track->data[ tmp ][1]) / 100);

	VIDEO_point_(track->x + (track->width  * track->data[ tmp ][0]) / 100,
				 track->y + (track->height * track->data[ tmp ][1]) / 100 + 1);

	VIDEO_point_(track->x + (track->width  * track->data[ tmp ][0]) / 100 + 1,
				 track->y + (track->height * track->data[ tmp ][1]) / 100 + 1);

	VIDEO_point_(track->x + (track->width  * track->data[ tmp ][0]) / 100 + 2,
				 track->y + (track->height * track->data[ tmp ][1]) / 100);

	VIDEO_point_(track->x + (track->width  * track->data[ tmp ][0]) / 100,
				 track->y + (track->height * track->data[ tmp ][1]) / 100 + 2);

	VIDEO_point_(track->x + (track->width  * track->data[ tmp ][0]) / 100 + 1,
				 track->y + (track->height * track->data[ tmp ][1]) / 100 + 2);

	VIDEO_point_(track->x + (track->width  * track->data[ tmp ][0]) / 100 + 2,
				 track->y + (track->height * track->data[ tmp ][1]) / 100 + 1);

	VIDEO_point_(track->x + (track->width  * track->data[ tmp ][0]) / 100 + 2,
				 track->y + (track->height * track->data[ tmp ][1]) / 100 + 2);


	/* 描く */
	VIDEO_point(track->x + (track->width  * track->data[ track->ptr ][0]) / 100,
				track->y + (track->height * track->data[ track->ptr ][1]) / 100);

	VIDEO_point(track->x + (track->width  * track->data[ track->ptr ][0]) / 100 + 1,
				track->y + (track->height * track->data[ track->ptr ][1]) / 100);

	VIDEO_point(track->x + (track->width  * track->data[ track->ptr ][0]) / 100,
				track->y + (track->height * track->data[ track->ptr ][1]) / 100 + 1);

	VIDEO_point(track->x + (track->width  * track->data[ track->ptr ][0]) / 100 + 1,
				track->y + (track->height * track->data[ track->ptr ][1]) / 100 + 1);

	VIDEO_point(track->x + (track->width  * track->data[ track->ptr ][0]) / 100 + 2,
				track->y + (track->height * track->data[ track->ptr ][1]) / 100);

	VIDEO_point(track->x + (track->width  * track->data[ track->ptr ][0]) / 100,
				track->y + (track->height * track->data[ track->ptr ][1]) / 100 + 2);

	VIDEO_point(track->x + (track->width  * track->data[ track->ptr ][0]) / 100 + 1,
				track->y + (track->height * track->data[ track->ptr ][1]) / 100 + 2);

	VIDEO_point(track->x + (track->width  * track->data[ track->ptr ][0]) / 100 + 2,
				track->y + (track->height * track->data[ track->ptr ][1]) / 100 + 1);

	VIDEO_point(track->x + (track->width  * track->data[ track->ptr ][0]) / 100 + 2,
				track->y + (track->height * track->data[ track->ptr ][1]) / 100 + 2);


	/* 十字を描く */
	tmp = track->y + (track->height >> 1);
	VIDEO_line(track->x, tmp, track->x + track->width, tmp);

	tmp = track->x + (track->width >> 1);
	VIDEO_line(tmp, track->y, tmp, track->y + track->height);

	return;
}
