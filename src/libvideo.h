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

	Note			: ��ʗ��[�� 2px�́A�g���Ȃ�
**************************************************************************************************/


#ifndef _LIBVIDEO_H_
#define _LIBVIDEO_H_


/*=================================================================================================
�}�N����`
=================================================================================================*/
/* �������}�N��. �����`�F�b�N���̊m�F���������s���Ȃ��Ȃ� */
//#define VIDEO_FAST

/* ���E���]�}�N��
��ʕ`�悪���E���]�ɕ`�����B�o�b�N���j�^�p�J�����Ȃǂ𗬗p����ƁA�J�����o�͂�
���E���]���Ă���̂ŁA�����ɑΉ����邽��
*/
//#define VIDEO_MIRROR

/* �𑜓x. 288x224px�܂œ���m�F�ς�. �������e�ʂɒ��� */
#define NTSC_WIDTH					256									/* 16�̔{���Ƃ��邱�� */
#define NTSC_HEIGHT					192									/* 262 - 13 �ȉ��Ƃ��邱�� */
#define NTSC_VRAM_SIZE				((NTSC_WIDTH >> 3) * NTSC_HEIGHT)	/* �o�C�g�v�Z */

#define GRAPH_SIZE					(100)
#define TRACK_SIZE					(5)


/*=================================================================================================
�\���̐錾
=================================================================================================*/
/* �r�f�I�o�͂̊e��p�����[�^ */
typedef struct NTSC_STATUS {
	unsigned int *vram;									/* �o�͗p VRAM�|�C���^ */

	unsigned int line;									/* ���������̖{�� */
	unsigned int line_sync;								/* �����A����� */
	unsigned int line_space_top;						/* �㕔�̋󔒕��� */
	unsigned int line_video;							/* �f������ */

	unsigned int horizon_pulse;							/* ���������p���X */
	unsigned int serration_pulse;						/* �؂荞�݃p���X */
	unsigned int equalizing_pulse;						/* �����p���X */
	unsigned int left_space;							/* �����̋󔒕��� */
	unsigned int video_width;							/* ���ۂɏo�͂���鉡�̉𑜓x */

	unsigned int status;								/* �X�e�[�g�}�V�� */
	unsigned int vsync;									/* ���������J�E���^ */
	unsigned int hsync;									/* ���������J�E���^ */
} NTSC_STATUS_T;
typedef NTSC_STATUS_T* pNTSC_STATUS_T;

/* �h���C�o�p�̍\���� */
typedef struct NTSC {
	unsigned int *vram;									/* �r�f�I�������̐擪�|�C���^ */
	unsigned char output;								/* �f���o�͐� */

	NTSC_STATUS_T monitor;								/* ���j�^�o�͗p (SPI1) */
	NTSC_STATUS_T superimpose;							/* �X�[�p�[�C���|�[�Y�p (SPI2) */
} NTSC_T;
typedef NTSC_T* pNTSC_T;

/* �O���Q��(API)�p�̍\���� */
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
�O���[�o���ϐ�
=================================================================================================*/
extern NTSC_T ntsc;
extern VIDEO_T video;


/*=================================================================================================
�v���g�^�C�v�錾
=================================================================================================*/
extern void VIDEO_init(void);
extern void VIDEO_init_clock(void);
extern  void VIDEO_vram_clear(unsigned int pattern);
extern  unsigned char VIDEO_get_output(void);
extern  unsigned int VIDEO_get_point(unsigned int x, unsigned int y);


/* �`��֐� */
extern  void VIDEO_point (unsigned int x, unsigned int y);
extern  void VIDEO_point_(unsigned int x, unsigned int y);

extern  void VIDEO_line (int x0, int y0, int x1, int y1);
extern  void VIDEO_line_(int x0, int y0, int x1, int y1);

#define VIDEO_circle(x,y,r)		VIDEO_arc(x, y, r, 0, 360)
#define VIDEO_circle_(x,y,r)	VIDEO_arc(x, y, r, 0, 360)
extern  void VIDEO_arc (unsigned int x, unsigned int y, unsigned int r, unsigned int start, unsigned int end);
extern  void VIDEO_arc_(unsigned int x, unsigned int y, unsigned int r, unsigned int start, unsigned int end);


/* FONTX2 Driver�ˑ��֐� */
extern  void VIDEO_locate(unsigned int x, unsigned int y);
extern  void VIDEO_putch(unsigned char c);
extern  void VIDEO_putstr(const char *s);
extern  void VIDEO_puthex(unsigned char a);
extern  void VIDEO_putbin(unsigned char a);
extern  void VIDEO_putuint(unsigned int digit, unsigned char size);
extern  void VIDEO_putint(int digit, unsigned char size);
extern  void VIDEO_putdouble(double digit, unsigned char size, unsigned char size2);


/* �O���t�\�� */
extern  void GRAPH_init(pGRAPH_T graph, unsigned int x, unsigned int y, unsigned int width, unsigned int height);
extern  void GRAPH_putdata(pGRAPH_T graph, unsigned int data);
extern  void GRAPH_draw_point(pGRAPH_T graph);
extern  void GRAPH_draw_line(pGRAPH_T graph);
extern  void GRAPH_draw_bar(pGRAPH_T graph);


/* ���[�^�\�� */
extern  void METER_init(pMETER_T meter, unsigned int x, unsigned int y, unsigned char r);
extern  void METER_draw(pMETER_T meter, unsigned int value);


/* �O�Օ\�� */
extern  void TRACK_init(pTRACK_T track, unsigned int x, unsigned int y, unsigned int width, unsigned int height);
extern  void TRACK_putdata(pTRACK_T track, unsigned int x, unsigned int y);
extern  void TRACK_draw_point(pTRACK_T track);


#endif
