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
�w�b�_�t�@�C�����C���N���[�h
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
�}�N����`
=================================================================================================*/


/*================================================================================================
�\����
=================================================================================================*/
typedef struct CONFIG {
	void *ptr;
	unsigned char size;
} CONFIG_T;
typedef CONFIG_T* pCONFIG_T;


/*================================================================================================
�O���[�o���ϐ�
=================================================================================================*/
/* �ۑ��Ώۂ�ݒ肷�� */
static const CONFIG_T config[] = {
	{ &extmeter,			sizeof(EXTMETER_T),		},
	{ &screen,				sizeof(SCREEN_T),		},
	{ &ssm,					sizeof(SSM_T),			},
//	{ &ntsc,				sizeof(NTSC_T),			},		/* ��肪�������Ƃ��\������Ȃ��Ȃ� */
	{ NULL,					0x00,					},
};


/*================================================================================================
�v���g�^�C�v�錾
=================================================================================================*/
static  BOOL CONFIG_write_and_verify(const unsigned int ptr, const unsigned char data);
static  unsigned int CONFIG_make_checksum(void);
static  unsigned int CONFIG_read_checksum(void);


/**************************************************************************************************
������
**************************************************************************************************/
BOOL CONFIG_init(void)
{
	return(TRUE);
}


/**************************************************************************************************
�ۑ�����
---------------------------------------------------------------------------------------------------
���A�������ʓ|�Ȃ̂ŁA������������ WDT���Z�b�g������
**************************************************************************************************/
BOOL CONFIG_save(void)
{
	unsigned char *target_ptr;
	unsigned int i, j, eeprom_ptr, checksum;

	eeprom_ptr = 0;

	/* CONFIG���X�g�̏��� */
	for(i = 0; config[i].size != 0; i++){
		ClrWdt();
		target_ptr = config[i].ptr;

		/* �ۑ��Ώۂ̍\���̂��������� */
		for(j = 0; j < config[i].size; j++){
			while(!CONFIG_write_and_verify(eeprom_ptr, *target_ptr));
			target_ptr++;
			eeprom_ptr++;
		}
	}

	/* �`�F�b�N�T���̋L�^ */
	checksum = CONFIG_make_checksum();
	while(!CONFIG_write_and_verify(eeprom_ptr, (checksum >> 8)));
	eeprom_ptr++;
	while(!CONFIG_write_and_verify(eeprom_ptr, (checksum     )));

	return(TRUE);
}


/**************************************************************************************************
�ǂݍ���
---------------------------------------------------------------------------------------------------
���A�������ʓ|�Ȃ̂ŁA������������ WDT���Z�b�g������
**************************************************************************************************/
BOOL CONFIG_load(void)
{
	unsigned char *target_ptr;
	unsigned int i, j, eeprom_ptr, checksum, verify, buf;

	eeprom_ptr = 0;

	/* �`�F�b�N�T�����m�F���� */
	checksum = CONFIG_make_checksum();
	verify   = CONFIG_read_checksum();

	/* �`�F�b�N�T�����قȂ�Ƃ��̓G���[��Ԃ� */
	if(checksum != verify) return(FALSE);


	/* �������ɏ������� */
	for(i = 0; config[i].size != 0; i++){
		ClrWdt();
		target_ptr = config[i].ptr;

		/* �ۑ��Ώۂ̍\���̂�Ǎ� */
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
�f�[�^��`��
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
�x���t�@�C�t����
---------------------------------------------------------------------------------------------------
���A�������ʓ|�Ȃ̂ŁA������������ WDT���Z�b�g������
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
EEPROM�̃f�[�^����`�F�b�N�T���𐶐�����
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
EEPROM�̃`�F�b�N�T����ǂݍ���
-------------------------------------------------------------------------------------------------*/
static  unsigned int CONFIG_read_checksum(void)
{
	int buf;
	unsigned int i, eeprom_ptr, checksum;

	/* �`�F�b�N�T���̃A�h���X�����߂� */
	eeprom_ptr = 0;
	for(i = 0; config[i].size != 0; i++){
		eeprom_ptr += config[i].size;
	}

	/* �`�F�b�N�T����ǂݍ��� */
	while((buf = EEPROM_read(eeprom_ptr)) == -1);
	checksum = buf << 8;
	eeprom_ptr++;
	while((buf = EEPROM_read(eeprom_ptr)) == -1);
	checksum = checksum + buf;

	return(checksum);
}
