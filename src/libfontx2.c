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
�w�b�_�t�@�C�����C���N���[�h
=================================================================================================*/
#include "types.h"

#include "libfontx2.h"


/*=================================================================================================
�\����
=================================================================================================*/
/* ASCII�R�[�h�p���p�����t�H���g�̃w�b�_ */
typedef struct FONTX2_HEADER_ASCII {
	char ident[6];
	char fontname[8];

	unsigned char width;
	unsigned char height;
	unsigned char codetype;

} FONTX2_HEADER_ASCII_T;
typedef FONTX2_HEADER_ASCII_T* pFONTX2_HEADER_ASCII_T;


/*=================================================================================================
�O���[�o���ϐ�
=================================================================================================*/
const unsigned char fontx2_ident[] = "FONTX2";


/**************************************************************************************************
*****                                                                                         *****
*****           ���i�g�ݍ��ݎ��ɂ́A�t�H���g�̃��C�Z���X���\���Ɋm�F���邱��!                 *****
*****                                                                                         *****
**************************************************************************************************/

/* 1�����C���N���[�h���邱�� */
const unsigned char fontx2_ascii_data[] = {
//#include "font/4x8.txt"			/* ����t�H���g */
//#include "font/akagi11a.txt"		/* �ԏ�t�H���g */
//#include "font/gonhn12x.txt"		/* �׃S�V�b�N�́u���`�n��12�v�t�H���g */
//#include "font/gothn12x.txt"		/* �׃S�V�b�N�́u���`�n��12�v�t�H���g2 */
//#include "font/jpnhn8x.txt"		/* NEC PC-98  8dot �t�H���g (PC-9821As2���甲���܂���...) */
//#include "font/jpnhn16x.txt"		/* NEC PC-98 16dot �t�H���g (PC-9821As2���甲���܂���...) */
//#include "font/k6x10.txt"			/* k12x10 (k6x10) �t�H���g */
//#include "font/kyohn16x.txt"		/* ���ȏ��́u�l�`��16�v�t�H���g */
//#include "font/noho12a.txt"		/* �̂� 12dot �t�H���g */
//#include "font/noho16a.txt"		/* �̂� 16dot �t�H���g */
//#include "font/mgohn16x.txt"		/* �ۃS�V�b�N�́u�H�t��16�v�t�H���g */
//#include "font/minhn12x.txt"		/* �������́u������ 12�v�t�H���g */
//#include "font/minhn14x.txt"		/* �������́u������ 14�v�t�H���g */
//#include "font/minhn16x.txt"		/* �������́u������ 16�v�t�H���g */
//#include "font/mplhn10.txt"		/* M+ BITMAP 10dot �t�H���g */
//#include "font/mplhn11.txt"		/* M+ BITMAP 11dot �t�H���g */
//#include "font/mplhn12.txt"		/* M+ BITMAP 12dot �t�H���g */
//#include "font/mplhn13.txt"		/* M+ BITMAP 13dot �t�H���g */
//#include "font/paw16a.txt"		/* �ς��t�H���g */
//#include "font/reihn16x.txt"		/* �ꏑ�́u�����x 16�v�t�H���g */
//#include "font/xbghn16x.txt"		/* �t�@���e�[���u���� 16�v�t�H���g */
#include "font/shnhn16.txt"			/* ���_�t�H���g */
//#include "font/8x16rk.txt"		/* X11�t�H���g */
};

const pFONTX2_HEADER_ASCII_T fontx2_ascii = (pFONTX2_HEADER_ASCII_T)&fontx2_ascii_data;


/**************************************************************************************************
������
**************************************************************************************************/
BOOL FONTX2_init(void)
{
	/* �����t�H���g�f�[�^���m�F����. strcmp()���� */
	if(fontx2_ident[0] != fontx2_ascii->ident[0])	return(FALSE);
	if(fontx2_ident[1] != fontx2_ascii->ident[1])	return(FALSE);
	if(fontx2_ident[2] != fontx2_ascii->ident[2])	return(FALSE);
	if(fontx2_ident[3] != fontx2_ascii->ident[3])	return(FALSE);
	if(fontx2_ident[4] != fontx2_ascii->ident[4])	return(FALSE);
	if(fontx2_ident[5] != fontx2_ascii->ident[5])	return(FALSE);


	/* �t�H���g�T�C�Y���������}�N���̏����ƍ����Ă��邩�m�F���� */
#ifdef FONT_FAST8
	if(FONTX2_get_ascii_width() != 8)				return(FALSE);
#endif


	return(TRUE);
}


/**************************************************************************************************
1�����̉���px���擾����
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
1�����̏c��px���擾����
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
1�����̃t�H���g�T�C�Y(byte)���擾����
**************************************************************************************************/
 unsigned char FONTX2_get_ascii_size(void)
{
	return( FONTX2_get_ascii_width_byte() * FONTX2_get_ascii_height_byte() );
}


/**************************************************************************************************
1�����̃t�H���g�f�[�^�̃|�C���^���擾����
**************************************************************************************************/
 unsigned char *FONTX2_get_ascii_font(const unsigned char ascii)
{
	return(&fontx2_ascii_data[		sizeof(FONTX2_HEADER_ASCII_T)
								+	FONTX2_get_ascii_size() * ascii
							]);
}


/**************************************************************************************************
1�����̃t�H���g�f�[�^�̃O���t�𓾂�
**************************************************************************************************/
 unsigned char FONTX2_get_ascii_font_data(const unsigned char ascii, unsigned char x, unsigned char y)
{
	return( fontx2_ascii_data[		sizeof(FONTX2_HEADER_ASCII_T)
								+	FONTX2_get_ascii_size() * ascii
								+	x
								+	FONTX2_get_ascii_width_byte() * y
							]);
}


