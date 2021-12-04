/*************************************************************************
 *********** COPYRIGHT (c) 2021 by Joseph Haas (DBA FF Systems)  *********
 *
 *  File name: serial.c
 *
 *  Module:    Control
 *
 *  Summary:   This is the serial I/O module for the F360 MCU
 *
 *******************************************************************/


/********************************************************************
 *  File scope declarations revision history:
 *    03-03-21 jmh:  Removed "percnt" from ISR as the result of the disambigution of the "%" response quoting character.
 *
 *    08-23-20 jmh:  modified to support bluetooth interface
 *    05-12-13 jmh:  creation date
 *
 *******************************************************************/

#include "c8051F520.h"
#include "typedef.h"
#include "init.h"
//#include "stdio.h"
#include "serial.h"

//------------------------------------------------------------------------------
// local defines
//------------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Local Variable Declarations
//-----------------------------------------------------------------------------

#define RXD_ERR 0x01
//#define RXD_CR 0x02					// CR rcvd flag
#define RXD_BS 0x04					// BS rcvd flag
#define RXD_ESC 0x40				// ESC rcvd flag
#define RXD_CHAR 0x80				// CHAR rcvd flag (not used)
#define RXD_BUFF_END 40
idata	S8	rxd_buff[RXD_BUFF_END];		// rx data buffer
		U8	rxd_idx;					// rx buf head ptr = next available buffer input
		U8	rxd_done;					// rx buf tail ptr = next available buffer output
		bit	qTI0B;						// UART TI0 reflection (set by interrupt)
		U8	pfx_idx;
		bit	pfx_det;

#define	PFX_LEN	4
code U8	timark_pfx[] = { 0xb5, 0x62, 0x0d, 0x03 };

//------------------------------------------------------------------------------
// local fn declarations
//------------------------------------------------------------------------------
char eolchr(char c);
char wait_cmd(void);

//-----------------------------------------------------------------------------
// init_serial() initializes serial port vars
//-----------------------------------------------------------------------------
//
void init_serial(void){
	
	init_buff();
/*	getch00();					// init the fns
	cleanline();
	gotch00();
	anych00();
	gotcr();*/
	qTI0B = 1;					// UART TI0 reflection (set by interrupt)
}
//
//-----------------------------------------------------------------------------
// init_buff() initializes serial port buffer
//-----------------------------------------------------------------------------
//
void init_buff(void){

	pfx_idx = 0;
	pfx_det = 0;
	rxd_idx = 0;
	rxd_done = 0;
}
//
//-----------------------------------------------------------------------------
// putch, UART0
//-----------------------------------------------------------------------------
//
// SFR Paged version of putch, no CRLF translation
/*
char putch (char c)  {

	// output character
	while(!qTI0B){				// wait for tx buffer to clear
		continue;
	}
	qTI0B = 0;
	SBUF0 = c;
	return (c);
}
*/
//
//-----------------------------------------------------------------------------
// getch00 checks for input @ RX0.  If no chr, return '\0'.
//-----------------------------------------------------------------------------
//
// SFR Paged, waits for a chr and returns
// Processor spends most of its idle time waiting in this fn
/*
char getch00(void)
{
	char c = '\0';		// default to null return

	if(rxd_tptr != rxd_hptr){
		c = rxd_buff[rxd_tptr];				// pull chr and update pointer
		rxd_buff[rxd_tptr++] = '~';			// backfill for debug
		if(rxd_tptr == RXD_BUFF_END){
			rxd_tptr = 0;
		}
	}
	return c;
}
*/
//
//-----------------------------------------------------------------------------
// cleanline() cleans buffer up to and including 1st CR.  
//	Pull CHRS from buffer and discard until first CR is encountered.  Then, exit.
//-----------------------------------------------------------------------------
/*
void cleanline(void)
{
data	char c;	// temp char

	if(gotch00()){							// skip if buffer empty
		do{
			c = getch00();					// pull chr and update pointer
		}while(gotch00());					// repeat until CR or buffer empty
	}
	rxd_crcnt = 0;
	return;
}
*/
//
//-----------------------------------------------------------------------------
// gotch00 checks for input @ RX0.  If no chr, return '\0'.
//-----------------------------------------------------------------------------
//
// returns 0 if no chr in buffer or if current chr is '\r'
/*
char gotch00(void)
{
	char c = 0;

	if((rxd_tptr != rxd_hptr) && (rxd_buff[rxd_tptr] != '\r')){
		c = 1;						// set buffer has data
	}
	if(rxd_stat & RXD_BS){				// process backspace
		rxd_stat &= ~RXD_BS;			// clear flag
		putss("\b \b");					// echo clearing BS to terminal
	}
	return c;
}
*/
//
//-----------------------------------------------------------------------------
// anych00 checks for any input @ RX0.  If no chr, return '\0'.
//-----------------------------------------------------------------------------
//
// returns 0 if no chr in buffer or if current chr is '\r'
/*
char anych00(void)
{
	char c = 0;

	if((rxd_tptr != rxd_hptr) && (rxd_buff[rxd_tptr] != '\0')){
		c = 1;						// set buffer has data
	}
	return c;
}
*/
//
//-----------------------------------------------------------------------------
// gotcr checks for '\r' @ RX0.  If no chr, return '\0'.
//-----------------------------------------------------------------------------
//
// returns 0 if no cr rcvd
/*
char gotcr(void)
{
	char c = 0;

	if(rxd_crcnt){
		c = 1;							// set buffer has data
	}
	return c;
}
*/
//-----------------------------------------------------------------------------
// cpy_str() copies src to dest
//-----------------------------------------------------------------------------
/*
void cpy_str (char* src, char* dest)
{
	char c;

	do{
		c = *src;
		*dest = c;
		src += 1;
		dest += 1;
	}while(c);
	return;
}
*/
//-----------------------------------------------------------------------------
// char hiasc() returns hi nybble as ASCII
//-----------------------------------------------------------------------------
/*
char hiasc (U8 num)
{
	char c;

	c = ((num>>4)& 0x0f) + '0';
	if(c > '9'){
		c += 'A' - '9' - 1;
	}
	return c;
}
*/
//-----------------------------------------------------------------------------
// char lowasc() returns low nybble as ASCII
//-----------------------------------------------------------------------------
/*
char lowasc (U8 num)
{
	char c;

	c = (num & 0x0f) + '0';
	if(c > '9'){
		c += 'A' - '9' - 1;
	}
	return c;
}
*/
//-----------------------------------------------------------------------------
// putss() does puts w/o newline
//-----------------------------------------------------------------------------
/*
void putss (char* string)
{
	while(*string){
		putch(*string++);
	}
	return;
}
*/
//-----------------------------------------------------------------------------
// getss() copies rxd capture to string.  Returns ptr to next empty chr of string
//-----------------------------------------------------------------------------
/*
char* getss (char* string, U8 maxlen)
{
	char* pstr = string;
	char  c;
	U8	  ctr = 0;
	bit	EA_save;

	do{
		c = getch00();
		if(ctr < maxlen){
			*pstr = c;						// only capture the first "maxlen" chrs
			pstr += 1;
			ctr += 1;
		}
	}while(c != '\0');
	if(rxd_crcnt){
		EA_save = EA;					// prohibit intrpts
		EA = 0;
		rxd_crcnt -= 1;					// clear flag
		EA = EA_save;					// re-set intrpt enable
	}
	return *pstr;
}
*/

//-----------------------------------------------------------------------------
// getm() validates the serial buffer.  If valid, passes the (U32)time mark
//	via pointer reference.  Return value is true if error, false if data valid
//-----------------------------------------------------------------------------
U8 getm (U32* rslt, U32* accuracy, U8 cmd){
	U8	chks_a = 0x0d;		// chksum temps
	U8	chks_b = 0x0d;
	U8	i;					// temp
	U8	rtrn = 1;			// return val
static	data U32	xxo;
static	data U16	yyo;
data	U32	xx;
data 	U16	yy;
data 	U32	zz;

	if(cmd){
		yyo = 0xffff;
		xxo = 0xffffffffL;
	}
	if(rxd_done == 1){						// look for data ready signal
		rtrn = 2;							// rtrn value traps the fail point
		chks_a += 0x03;						// init the checksum with the class/id (these are not buffered)
		chks_b += chks_a;
		for(i=0; i<30; i++){				// calc checksum per Ublox spec
			chks_a += rxd_buff[i];
			chks_b += chks_a;
		}
		// validate checksum
		if((chks_a == rxd_buff[i]) && (chks_b == rxd_buff[i+1])){
			rtrn = 3;
			i = rxd_buff[3] & (TMK_TVALID | TMK_RE);
			if(i & TMK_TVALID) rtrn = 4;		// set valid GPS timing return
			// validate time valid, rising edge, & ch = 0
			if((i == (TMK_TVALID | TMK_RE)) && (rxd_buff[2] == 0)){
				yy = (U16)rxd_buff[6] & 0x00ff;
				yy |= (((U16)rxd_buff[7]) << 8) & 0xff00;
				xx = ((U32)rxd_buff[10]) & 0x000000ffL;
				xx |= (((U32)rxd_buff[11]) << 8) & 0x0000ff00L;
				xx |= (((U32)rxd_buff[12]) << 16) & 0x00ff0000L;
				xx |= (((U32)rxd_buff[13]) << 24) & 0xff000000L;
				if((xx == xxo) && (yyo == yy)){
					// extract ns portion of time mark
					zz = ((U32)rxd_buff[14]) & 0x000000ffL;
					zz |= (((U32)rxd_buff[15]) << 8) & 0x0000ff00L;
					zz |= (((U32)rxd_buff[16]) << 16) & 0x00ff0000L;
					zz |= (((U32)rxd_buff[17]) << 24) & 0xff000000L;
					*rslt = zz;					// pass back the value
					// extract accuracy
					zz = ((U32)rxd_buff[26]) & 0x000000ffL;
					zz |= (((U32)rxd_buff[27]) << 8) & 0x0000ff00L;
					zz |= (((U32)rxd_buff[28]) << 16) & 0x00ff0000L;
					zz |= (((U32)rxd_buff[29]) << 24) & 0xff000000L;
					*accuracy = zz;				// pass back the value
					rtrn = 0;					// set "no error" return
				}else{
					chks_a++;
				}
				xxo = xx + 5000L;
				yyo = yy;
			}
		}
		rxd_done = 0;						// clear signal
	}
	return rtrn;
}
/*
                0  1  2  3  4  5  6  7  8  9  10 11
B5 62 0D 03     1C 00 00 4D 41 1C 89 08 89 08 13 A1  µb.....MA......¡

                12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27
          0010  C6 1F F1 1D 03 00 FB A4 C6 1F 06 1E 03 00 1C 00  Æ.ñ...û¤Æ.......

                28 29 30 31
          0020  00 00 69 36 
*/
//
//-----------------------------------------------------------------------------
// rxd_intr
//-----------------------------------------------------------------------------
//
// UART1 rx intr.  Captures RX data and places into fixed buffer
//	Uses "trap sentinel" to itentify when to start storing data to the buffer
//	(traps on sync1/sync2/class/id = B5 62 0d 03(.
//
//	rxd_done is signal register to real-time function getm() that the buffer is
//	ready for processing.
//
//	ISR echoes TIO to qTIOB to allow polled TX of UART data.
//

void rxd_intr(void) interrupt 4
{
	char	c;
	
	if(TI0){
		qTI0B = 1;										// set TX reflection flag
		TI0 = 0;
	}
	if(RI0){
		c = SBUF0;										// get inbound chr
		if(!rxd_done){
			if(pfx_det){								// if buffer armed, fill it
				rxd_buff[rxd_idx++] = c;
				if(rxd_idx >= rxd_buff[0]+6){			// check for end of data
					rxd_done = 1;						// signal data ready
					pfx_det = 0;						// reset prefix scan index
				}
				if(rxd_idx >= RXD_BUFF_END){
					pfx_det = 0;						// buffer overflow, abort
				}
			}else{
				if(timark_pfx[pfx_idx] == c){			// compare inbound stream to time-mark prefix
					pfx_idx++;							// if a char match, advance to next chr
					if(pfx_idx == PFX_LEN){				// if end of prefix, arm buffer to receive data
						pfx_det = 1;
						rxd_idx = 0;
					}
				}else{
					pfx_idx = 0;						// no match, reset prefix index
				}
			}
		}
		RI0 = 0;										// clear intr flag
	}
	return;
}
