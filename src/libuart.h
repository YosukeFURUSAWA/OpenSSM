/**************************************************************************************************
	Title			: PIC24F Series UART Driver
	Programmer		: Ryuz
	Programmer		: Yosuke FURUSAWA
	Copyright		: Copyright (C) 1998-2000 Ryuz.
	Copyright		: Copyright (C) 2000-2010 Yosuke FURUSAWA.
	License			: 4-clause BSD License
	Since			: 1998/xx/xx

	Filename		: libuart.h
	Last up date	: 2010/08/11
	Kanji-Code		: Shift-JIS
	TAB Space		: 4
**************************************************************************************************/


#ifndef _LIBUART_H_
#define _LIBUART_H_


/*=================================================================================================
マクロ定義
================================================================================================*/
/* バッファサイズ */
#define UART1_TX_BUFFER_SIZE			60
#define UART1_RX_BUFFER_SIZE			8
#define UART2_TX_BUFFER_SIZE			50
#define UART2_RX_BUFFER_SIZE			68


/*=================================================================================================
プロトタイプ宣言
================================================================================================*/
extern void UART1_init(unsigned long baud);
extern  void UART1_buf_clear(void);
extern int UART1_getch(void);
extern int UART1_putch(unsigned char buf);
extern  void UART1_putstr(char *buf);
extern void UART1_putint(int digit);
extern void UART1_putdouble(double digit, unsigned char size2);
extern  void UART1_puthex(unsigned char a);
extern  unsigned char UART1_get_sendbuf(void);
extern  unsigned char UART1_get_recvbuf(void);

extern void UART2_init(unsigned long baud);
extern  void UART2_buf_clear(void);
extern int UART2_getch(void);
extern int UART2_putch(unsigned char buf);
extern  void UART2_putstr(char *buf);
extern void UART2_putint(int digit);
extern void UART2_putdouble(double digit, unsigned char size2);
extern  void UART2_puthex(unsigned char a);
extern  unsigned char UART2_get_sendbuf(void);
extern  unsigned char UART2_get_recvbuf(void);

extern  unsigned int UART_get_brg(unsigned long baud);
extern  unsigned long UART1_get_baud(void);
extern  unsigned long UART2_get_baud(void);

#endif
