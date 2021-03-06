/**************************************************************************************************
	Title			: SSM Protocol
	Programer		: Yosuke FURUSAWA.
	Copyright		: Copyright (C) 2007-2010 Yosuke FURUSAWA.
	License			: 4-clause BSD License
	Since			: 2007/11/xx

	Filename		: ssm.h
	Last up date	: 2010/08/22
	Kanji-Code		: Shift-JIS
	TAB Space		: 4
**************************************************************************************************/


#ifndef _SSM_H_
#define _SSM_H_


/*=================================================================================================
�}�N����`
=================================================================================================*/
/* �^�C���̒��a�����߂� */
#define SSM_TIRE_R(x,y,z)	((((x * y) / 50.0) + (z * 25.4)) * 3.1415926)


#define SSM_GEAR_FINAL			(0)
#define SSM_GEAR_1				(1)
#define SSM_GEAR_2				(2)
#define SSM_GEAR_3				(3)
#define SSM_GEAR_4				(4)
#define SSM_GEAR_5				(5)
#define SSM_GEAR_6				(6)
#define SSM_GEAR_7				(7)

#define SSM_MODE_OPENSSM		(0)
#define SSM_MODE_OPENPORT		(1)


/*=================================================================================================
�\����
=================================================================================================*/
typedef struct SSM {
	unsigned int tire_width;
	unsigned char tire_flat;
	unsigned char tire_inch;
	double tire_circle;

	double gear_ratio[8];

	unsigned int price;

	unsigned char mode;

	unsigned int wait;
	unsigned int last;							/* �Ō�Ɏ��s�������� */
	unsigned int cycle;							/* ���s���� */
	unsigned int error;							/* �ʐM�G���[�̉� */
} SSM_T;
typedef SSM_T* pSSM_T;

typedef struct SSM_DATA {
	unsigned int engine;
	double throttle;
	unsigned char speed;
	double boost;
	int coolant;
	int intakeair;
	double battery;
	double maf;
	double afr;
	int ignition;
	int knock;
	double fuel;
	double fuel_rate;
	unsigned char shift;
} SSM_DATA_T;
typedef SSM_DATA_T* pSSM_DATA_T;


/*=================================================================================================
�O���[�o���ϐ�
=================================================================================================*/
extern SSM_T ssm;
extern SSM_DATA_T ssm_data;


/*=================================================================================================
�v���g�^�C�v�錾
=================================================================================================*/
extern void SSM_init(void);
extern BOOL SSM_main(void);


#endif
