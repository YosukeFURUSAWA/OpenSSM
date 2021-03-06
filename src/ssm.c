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
�w�b�_�t�@�C�����C���N���[�h
=================================================================================================*/
#include <p24FJ64GA002.h>

#include "types.h"
#include "table.h"
#include "ssm.h"

#include "libadc.h"
#include "librtc.h"
#include "libuart.h"


/*=================================================================================================
�}�N����`
=================================================================================================*/
/* �e�M�A��̒��Ԃ������āA�V�t�g�̋��ڂƂ��� */
#define SSM_SHIFT(x,y)		(x - (x - y) / 2.0)

/* �^�C���̊�{�X�y�b�N */
#define TIRE_WIDTH			235			/* �^�C���̕� (mm) */
#define TIRE_FLAT			45			/* �^�C���̝G���� (%) */
#define TIRE_INCH			17			/* �^�C���̒��a (inch) */

/* �M�A�� */
#define GEAR_F				3.900
#define GEAR_1				3.636
#define GEAR_2				2.375
#define GEAR_3				1.761
#define GEAR_4				1.346
#define GEAR_5				1.062
#define GEAR_6				0.842
#define GEAR_7				0.001


/*=================================================================================================
�O���[�o���ϐ�
=================================================================================================*/
SSM_T ssm;
SSM_DATA_T ssm_data;


/*=================================================================================================
�v���g�^�C�v�錾
=================================================================================================*/
static  BOOL SSM_openssm(void);
static  void SSM_log(void);
static  BOOL SSM_openport(void);
static  void SSM_write_packet(void);
static  BOOL SSM_read_packet(pSSM_T ssm, pSSM_DATA_T data);
static  unsigned char SSM_calc_gear(pSSM_T ssm, pSSM_DATA_T data);


/**************************************************************************************************
SSM������
**************************************************************************************************/
void SSM_init(void)
{
	/* ���������� */
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
SSM�ʐM����
**************************************************************************************************/
BOOL SSM_main(void)
{
	static unsigned char mode = 0xff;

	/* �ʐM���[�h�ɂ���ē����ς��� */
	switch(ssm.mode){


	/* OpenSSM���[�h */
	case SSM_MODE_OPENSSM:

		/* �O��̃R�[���ƈقȂ�Ƃ��AUART������������ */
		if(mode != ssm.mode){
			UART1_init(115200);
			UART2_init(  4800);
		}

		mode = ssm.mode;
		return(SSM_openssm());
		break;


	/* OpenPort���ʌ݊����[�h */
	case SSM_MODE_OPENPORT:
	default:

		/* �O��̃R�[���ƈقȂ�Ƃ��AUART������������ */
		if(mode != ssm.mode){
			UART1_init(  4800);
			UART2_init(  4800);
		}

		mode = ssm.mode;
		return(SSM_openport());
		break;
	}

	/* �����ɂ�����o�O */
	return(FALSE);
}


/*-------------------------------------------------------------------------------------------------
OpenSSM���[�h
-------------------------------------------------------------------------------------------------*/
static  BOOL SSM_openssm(void)
{
	static unsigned int tick = 0;
	static unsigned char flag = 0;

	/* ���s���� */
	if(RTC_get_ticks(tick, rtc.tick) < ssm.wait) return(FALSE);
	tick = rtc.tick;

	if(!SSM_read_packet(&ssm, &ssm_data)){
		ssm.error++;

		/* �G���[�����������Ƃ��́A1�T�C�N���̃E�F�C�g�����Ă���p�P�b�g�𑗂� */
		if(ssm.error >> 1){
			SSM_write_packet();
			ssm.cycle = 0xffff;
		}
	} else {
		ssm.cycle = RTC_get_ticks(ssm.last, rtc.tick);
		ssm.last = rtc.tick;
		SSM_write_packet();
	}

	/* �X�y�[�X�L�[�̓��͂��������Ȃ烍�O���o��/�~�߂� */
	if(UART1_getch() == ' ')	flag++;
	if(flag % 2)				SSM_log();

	return(TRUE);
}


/*-------------------------------------------------------------------------------------------------
UART1�փ��O���o��
-------------------------------------------------------------------------------------------------*/
static  void SSM_log(void)
{
	double sec;

	/* �o�b�t�@�����ꂻ���ȂƂ��́A��΂�... */
	if(UART1_get_sendbuf() != UART1_TX_BUFFER_SIZE) return;
	UART1_buf_clear();

	/* ���� */
	sec = rtc.sec + (double)rtc.msec / 1000.0;
	UART1_putint(rtc.hour);
	UART1_putch(':');
	UART1_putint(rtc.min);
	UART1_putch(':');
	UART1_putdouble(sec, 3);
	UART1_putch(',');

	/* SSM�f�[�^ */
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

	/* �����x */
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
UART1 �� UART2 ���N���X�ڑ�
OpenPort���ʌ݊��̒ʐM���[�h
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
SSM�u���b�N�ǂݍ���
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
		UART2_putch(packet[i]);								/* ���M�G���[�͖�������... */
	}

	return;
}

static  BOOL SSM_read_packet(pSSM_T ssm, pSSM_DATA_T data)
{
	int buf;
	unsigned char i;
	double fuel;

	static unsigned char read_packet[ 18 ];

	/* ��U�A�o�b�t�@�ɓ���� */
	i = 0;
	while((buf = UART2_getch()) != -1 && i < 45){
		/* ��ǂ� */
		i++;
	}
	i = 0;
	while((buf = UART2_getch()) != -1 && i < 18){
		read_packet[i] = (unsigned char)buf;
		i++;
	}

	/* �p�P�b�g�̊m�F */
	if(read_packet[ 0] != 0x80) return(FALSE);				/* �R�}���h�w�b�_ */
	if(read_packet[ 1] != 0xf0) return(FALSE);				/* �ʐM���� MSB */
	if(read_packet[ 2] != 0x10) return(FALSE);				/* �ʐM���� LSB */
	if(read_packet[ 3] != 0x0e) return(FALSE);				/* �R�}���h + �f�[�^�̃T�C�Y */
	if(read_packet[ 4] != 0xe8) return(FALSE);				/* �R�}���h */

	/* �ϊ� */
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
�M�A�𐄑�����
-------------------------------------------------------------------------------------------------*/
static  unsigned char SSM_calc_gear(pSSM_T ssm, pSSM_DATA_T data)
{
	double ratio;
	unsigned char shift;

	/* ���s���łȂ��Ƃ��́A�j���[�g�����Ƃ��� */
	if(data->engine  <  1000) return(0);
	if(data->speed   ==    0) return(0);

	/* �^�C���T�C�Y�A�ԑ��A�G���W����]������M�A������߂� */
	ratio  = (data->engine / (data->speed * ssm->gear_ratio[ SSM_GEAR_FINAL ]) * ssm->tire_circle * 60.0);
	ratio /= 1000000.0;

	/* �V�t�g�����߂� */
	if(SSM_SHIFT(ssm->gear_ratio[SSM_GEAR_1], ssm->gear_ratio[SSM_GEAR_2]) <= ratio) shift = SSM_GEAR_1;
	if(SSM_SHIFT(ssm->gear_ratio[SSM_GEAR_1], ssm->gear_ratio[SSM_GEAR_2]) >  ratio) shift = SSM_GEAR_2;
	if(SSM_SHIFT(ssm->gear_ratio[SSM_GEAR_2], ssm->gear_ratio[SSM_GEAR_3]) >  ratio) shift = SSM_GEAR_3;
	if(SSM_SHIFT(ssm->gear_ratio[SSM_GEAR_3], ssm->gear_ratio[SSM_GEAR_4]) >  ratio) shift = SSM_GEAR_4;
	if(SSM_SHIFT(ssm->gear_ratio[SSM_GEAR_4], ssm->gear_ratio[SSM_GEAR_5]) >  ratio) shift = SSM_GEAR_5;
	if(SSM_SHIFT(ssm->gear_ratio[SSM_GEAR_5], ssm->gear_ratio[SSM_GEAR_6]) >  ratio) shift = SSM_GEAR_6;
	if(SSM_SHIFT(ssm->gear_ratio[SSM_GEAR_6], ssm->gear_ratio[SSM_GEAR_7]) >  ratio) shift = SSM_GEAR_7;

	return(shift);
}
