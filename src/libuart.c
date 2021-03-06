/**************************************************************************************************
	Title			: PIC24F Series UART Driver
	Programmer		: Ryuz
	Programmer		: Yosuke FURUSAWA
	Copyright		: Copyright (C) 1998-2000 Ryuz. (Imported HOS-v3)
	Copyright		: Copyright (C) 2000-2010 Yosuke FURUSAWA.
	License			: 4-clause BSD License
	Since			: 1998/xx/xx

	Filename		: libuart.h
	Last up date	: 2010/08/11
	Kanji-Code		: Shift-JIS
	TAB Space		: 4
**************************************************************************************************/


/*=================================================================================================
ヘッダファイルをインクロード
=================================================================================================*/
#include <p24FJ64GA002.h>

#include "table.h"
#include "libuart.h"


/*=================================================================================================
マクロ定義
================================================================================================*/
#define abs(a)					(((a)>0) ? (a) : -(a))


/*=================================================================================================
グローバル変数
================================================================================================*/
/* 送信バッファ */
char uart1_tx_buf[UART1_TX_BUFFER_SIZE];
unsigned char uart1_tx_stptr;
unsigned char uart1_tx_enptr;

char uart2_tx_buf[UART2_TX_BUFFER_SIZE];
unsigned char uart2_tx_stptr;
unsigned char uart2_tx_enptr;

/* 受信バッファ */
char uart1_rx_buf[UART1_RX_BUFFER_SIZE];
unsigned char uart1_rx_stptr;
unsigned char uart1_rx_enptr;

char uart2_rx_buf[UART2_RX_BUFFER_SIZE];
unsigned char uart2_rx_stptr;
unsigned char uart2_rx_enptr;


/**************************************************************************************************
UART1 送信割込
**************************************************************************************************/
void __attribute__((interrupt, auto_psv)) _U1TXInterrupt(void)
{
	unsigned char nxptr;

	IFS0bits.U1TXIF = 0;

	/* 送信バッファが空 */
	if ( uart1_tx_stptr == uart1_tx_enptr ) {
		//IFS0bits.U1TXIF = 0;
		return;
	}

	/* 送信バッファから 1文字取り出して送信 */
	while(!U1STAbits.TRMT);
	U1TXREG = uart1_tx_buf[uart1_tx_stptr];

	/* ポインタ更新 */
	nxptr = uart1_tx_stptr + 1;
	if (nxptr >= UART1_TX_BUFFER_SIZE) nxptr = 0;
	uart1_tx_stptr = nxptr;

	return;
}


/**************************************************************************************************
UART2 送信割込
**************************************************************************************************/
void __attribute__((interrupt, auto_psv)) _U2TXInterrupt(void)
{
	unsigned char nxptr;

	IFS1bits.U2TXIF = 0;

	/* 送信バッファが空 */
	if ( uart2_tx_stptr == uart2_tx_enptr ) {
		//IFS1bits.U2TXIF = 0;
		return;
	}

	/* 送信バッファから 1文字取り出して送信 */
	while(!U2STAbits.TRMT);
	U2TXREG = uart2_tx_buf[uart2_tx_stptr];

	/* ポインタ更新 */
	nxptr = uart2_tx_stptr + 1;
	if (nxptr >= UART2_TX_BUFFER_SIZE) nxptr = 0;
	uart2_tx_stptr = nxptr;

	return;
}


/**************************************************************************************************
UART1 受信割込
**************************************************************************************************/
void __attribute__((interrupt, auto_psv)) _U1RXInterrupt(void)
{
	unsigned char nxptr;

	IFS0bits.U1RXIF = 0;

	if(U1STAbits.URXDA){
		/* ポインタ更新 */
		nxptr = uart1_rx_enptr + 1;
		if(nxptr >= UART1_RX_BUFFER_SIZE) nxptr = 0;
		if(uart1_rx_stptr != nxptr){
			uart1_rx_buf[ uart1_rx_enptr ] = U1RXREG;
			uart1_rx_enptr = nxptr;
		} else {
			/* バッファが満杯 */
			/* 取りこぼすことにする */
		}
	}

	return;
}


/**************************************************************************************************
UART2 受信割込
**************************************************************************************************/
void __attribute__((interrupt, auto_psv)) _U2RXInterrupt(void)
{
	unsigned char nxptr;

	IFS1bits.U2RXIF = 0;

	if(U2STAbits.URXDA){
		/* ポインタ更新 */
		nxptr = uart2_rx_enptr + 1;
		if(nxptr >= UART2_RX_BUFFER_SIZE) nxptr = 0;
		if(uart2_rx_stptr != nxptr){
			uart2_rx_buf[ uart2_rx_enptr ] = U2RXREG;
			uart2_rx_enptr = nxptr;
		} else {
			/* バッファが満杯 */
			/* 取りこぼすことにする */
		}
	}

	return;
}


/**************************************************************************************************
初期化
**************************************************************************************************/
void UART1_init(unsigned long baud)
{
	RPINR18bits.U1RXR = 5;				/* RXD : RP5 */
	RPOR2bits.RP4R = 3;					/* TXD : RP4 */

	U1MODE = 0b1000100000001000;
	U1STA  = 0b0010010000000000;
	U1BRG  = UART_get_brg(baud);

	/* 送信割込 */
	IPC3bits.U1TXIP = 3;
	IEC0bits.U1TXIE = 1;
	IFS0bits.U1TXIF = 0;

	/* 受信割込 */
	IPC2bits.U1RXIP = 4;
	IEC0bits.U1RXIE = 1;
	IFS0bits.U1RXIF = 0;

	UART1_buf_clear();
	return;
}


void UART2_init(unsigned long baud)
{
	RPINR19bits.U2RXR = 6;				/* RXD : RP6 */
	RPOR3bits.RP7R = 5;					/* TXD : RP7 */

	U2MODE = 0b1000100000001000;
	U2STA  = 0b0010010000000000;
	U2BRG  = UART_get_brg(baud);

	/* 送信割込 */
	IPC7bits.U2TXIP = 3;
	IEC1bits.U2TXIE = 1;
	IFS1bits.U2TXIF = 0;

	/* 受信割込 */
	IPC7bits.U2RXIP = 4;
	IEC1bits.U2RXIE = 1;
	IFS1bits.U2RXIF = 0;

	UART2_buf_clear();
	return;
}


/**************************************************************************************************
バッファクリア
**************************************************************************************************/
 void UART1_buf_clear(void)
{
	uart1_tx_stptr = 0;
	uart1_tx_enptr = 0;

	uart1_rx_stptr = 0;
	uart1_rx_enptr = 0;

	return;
}

 void UART2_buf_clear(void)
{
	uart2_tx_stptr = 0;
	uart2_tx_enptr = 0;

	uart2_rx_stptr = 0;
	uart2_rx_enptr = 0;

	return;
}


/**************************************************************************************************
1byte受信
**************************************************************************************************/
int UART1_getch(void)
{
	unsigned char buf;
	unsigned char nxptr;
	unsigned char ipl;

	ipl = SRbits.IPL;
	SRbits.IPL = 5;
	Nop();

	/* バッファが空 */
	if(uart1_rx_stptr == uart1_rx_enptr){
		SRbits.IPL = ipl;
		return(-1);
	}

	/* バッファから1byte取り出す */
	buf = uart1_rx_buf[ uart1_rx_stptr ];

	/* ポインタ更新 */
	nxptr = uart1_rx_stptr + 1;
	if(nxptr >= UART1_RX_BUFFER_SIZE) nxptr = 0;
	uart1_rx_stptr = nxptr;

	SRbits.IPL = ipl;
	return(buf);
}

int UART2_getch(void)
{
	unsigned char buf;
	unsigned char nxptr;
	unsigned char ipl;

	ipl = SRbits.IPL;
	SRbits.IPL = 5;
	Nop();

	/* バッファが空 */
	if(uart2_rx_stptr == uart2_rx_enptr){
		SRbits.IPL = ipl;
		return(-1);
	}

	/* バッファから1byte取り出す */
	buf = uart2_rx_buf[ uart2_rx_stptr ];

	/* ポインタ更新 */
	nxptr = uart2_rx_stptr + 1;
	if(nxptr >= UART2_RX_BUFFER_SIZE) nxptr = 0;
	uart2_rx_stptr = nxptr;

	SRbits.IPL = ipl;
	return(buf);
}


/**************************************************************************************************
1byte送信
**************************************************************************************************/
int UART1_putch(unsigned char buf)
{
	unsigned char nxptr;
	unsigned char ipl;

	ipl = SRbits.IPL;
	SRbits.IPL = 4;
	Nop();

	if(uart1_tx_stptr == uart1_tx_enptr){
		while(!U1STAbits.TRMT);
		U1TXREG = buf;
		SRbits.IPL = ipl;
		return(buf);
	}

	nxptr = uart1_tx_enptr + 1;
	if(nxptr >= UART1_TX_BUFFER_SIZE) nxptr = 0;
	if(uart1_tx_stptr == nxptr){
		SRbits.IPL = ipl;
		return(-1);
	}

	/* 送信バッファに1文字入れる */
	uart1_tx_buf[uart1_tx_enptr] = buf;
	uart1_tx_enptr = nxptr;

	SRbits.IPL = ipl;
	return(buf);
}

int UART2_putch(unsigned char buf)
{
	unsigned char nxptr;
	unsigned char ipl;

	ipl = SRbits.IPL;
	SRbits.IPL = 4;
	Nop();

	if(uart2_tx_stptr == uart2_tx_enptr){
		while(!U2STAbits.TRMT);
		U2TXREG = buf;
		SRbits.IPL = ipl;
		return(buf);
	}

	nxptr = uart2_tx_enptr + 1;
	if(nxptr >= UART2_TX_BUFFER_SIZE) nxptr = 0;
	if(uart2_tx_stptr == nxptr){
		SRbits.IPL = ipl;
		return(-1);
	}

	/* 送信バッファに1文字入れる */
	uart2_tx_buf[uart2_tx_enptr] = buf;
	uart2_tx_enptr = nxptr;

	SRbits.IPL = ipl;
	return(buf);
}


/**************************************************************************************************
Nbyte送信
**************************************************************************************************/
 void UART1_putstr(char *buf)
{
	while(*buf){
		while(UART1_putch(*buf) < 0);
		*buf++;
	}

	return;
}

 void UART2_putstr(char *buf)
{
	while(*buf){
		while(UART2_putch(*buf) < 0);
		*buf++;
	}

	return;
}


/**************************************************************************************************
10進送信
**************************************************************************************************/
void UART1_putint(int digit)
{
	if(digit < 0) UART1_putch('-');

	digit = abs(digit);

	if(digit > 9999) UART1_putch('0' + digit / 10000 % 10);
	if(digit >  999) UART1_putch('0' + digit /  1000 % 10);
	if(digit >   99) UART1_putch('0' + digit /   100 % 10);
	if(digit >    9) UART1_putch('0' + digit /    10 % 10);

	UART1_putch('0' + digit % 10);
	return;
}

void UART2_putint(int digit)
{
	if(digit < 0) UART2_putch('-');

	digit = abs(digit);

	if(digit > 9999) UART2_putch('0' + digit / 10000 % 10);
	if(digit >  999) UART2_putch('0' + digit /  1000 % 10);
	if(digit >   99) UART2_putch('0' + digit /   100 % 10);
	if(digit >    9) UART2_putch('0' + digit /    10 % 10);

	UART2_putch('0' + digit % 10);
	return;
}

void UART1_putdouble(double digit, unsigned char size2)
{
	if(digit < 0) UART1_putch('-');

	digit = abs(digit);

	if(digit > 9999) UART1_putch('0' + (unsigned char)(digit / 10000) % 10);
	if(digit >  999) UART1_putch('0' + (unsigned char)(digit /  1000) % 10);
	if(digit >   99) UART1_putch('0' + (unsigned char)(digit /   100) % 10);
	if(digit >    9) UART1_putch('0' + (unsigned char)(digit /    10) % 10);
	UART1_putch('0' + (unsigned char)digit % 10);
	UART1_putch('.');
	UART1_putch('0' + (unsigned int)(digit * 10.0) % 10);
	if(size2 > 1)	UART1_putch('0' + (unsigned int)(digit *   100.0) % 10);
	if(size2 > 2)	UART1_putch('0' + (unsigned int)(digit *  1000.0) % 10);
	if(size2 > 3)	UART1_putch('0' + (unsigned int)(digit * 10000.0) % 10);

	return;
}

void UART2_putdouble(double digit, unsigned char size2)
{
	if(digit < 0) UART2_putch('-');

	digit = abs(digit);

	if(digit > 9999) UART2_putch('0' + (unsigned char)(digit / 10000) % 10);
	if(digit >  999) UART2_putch('0' + (unsigned char)(digit /  1000) % 10);
	if(digit >   99) UART2_putch('0' + (unsigned char)(digit /   100) % 10);
	if(digit >    9) UART2_putch('0' + (unsigned char)(digit /    10) % 10);
	UART2_putch('0' + (unsigned char)digit % 10);
	UART2_putch('.');
	UART2_putch('0' + (unsigned int)(digit * 10.0) % 10);
	if(size2 > 1)	UART2_putch('0' + (unsigned int)(digit *   100.0) % 10);
	if(size2 > 2)	UART2_putch('0' + (unsigned int)(digit *  1000.0) % 10);
	if(size2 > 3)	UART2_putch('0' + (unsigned int)(digit * 10000.0) % 10);

	return;
}


/**************************************************************************************************
16進送信
**************************************************************************************************/
 void UART1_puthex(unsigned char a)
{
	UART1_putch(hex[(a >> 4)	& 0x0f]);
	UART1_putch(hex[ a			& 0x0f]);
	return;
}

 void UART2_puthex(unsigned char a)
{
	UART2_putch(hex[(a >> 4)	& 0x0f]);
	UART2_putch(hex[ a			& 0x0f]);
	return;
}


/**************************************************************************************************
空き送信バッファサイズ
**************************************************************************************************/
 unsigned char UART1_get_sendbuf(void)
{
	if(uart1_tx_stptr >= uart1_tx_enptr)	return(UART1_TX_BUFFER_SIZE - (uart1_tx_stptr - uart1_tx_enptr));
	else									return(UART1_TX_BUFFER_SIZE -  uart1_tx_enptr + uart1_tx_stptr);
}

 unsigned char UART2_get_sendbuf(void)
{
	if(uart2_tx_stptr >= uart2_tx_enptr)	return(UART2_TX_BUFFER_SIZE - (uart2_tx_stptr - uart2_tx_enptr));
	else									return(UART2_TX_BUFFER_SIZE -  uart2_tx_enptr + uart2_tx_stptr);
}


/**************************************************************************************************
空き受信バッファサイズ
**************************************************************************************************/
 unsigned char UART1_get_recvbuf(void)
{
	if(uart1_rx_stptr >= uart1_rx_enptr)	return(UART1_RX_BUFFER_SIZE - (uart1_rx_stptr - uart1_rx_enptr));
	else									return(UART1_RX_BUFFER_SIZE -  uart1_rx_enptr + uart1_rx_stptr);
}

 unsigned char UART2_get_recvbuf(void)
{
	if(uart2_rx_stptr >= uart2_rx_enptr)	return(UART2_RX_BUFFER_SIZE - (uart2_rx_stptr - uart2_rx_enptr));
	else									return(UART2_RX_BUFFER_SIZE -  uart2_rx_enptr + uart2_rx_stptr);
}


/**************************************************************************************************
BRGを得る / ボーレートを得る
**************************************************************************************************/
 unsigned int UART_get_brg(unsigned long baud)
{
	return( ((cpu_fcy[ OSCTUN ] >> 2) / baud) - 1 );
}

 unsigned long UART1_get_baud(void)
{
	return( (unsigned long)(cpu_fcy[ OSCTUN ] >> 2) / (unsigned long)(U1BRG + 1));
}

 unsigned long UART2_get_baud(void)
{
	return( (unsigned long)(cpu_fcy[ OSCTUN ] >> 2) / (unsigned long)(U2BRG + 1));
}
