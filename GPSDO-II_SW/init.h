/*************************************************************************
 *********** COPYRIGHT (c) 2021 by Joseph Haas (DBA FF Systems)  *********
 *
 *  File name: init.h
 *
 *  Module:    Control
 *
 *  Summary:   This is the header file for main.
 *
 *******************************************************************/


/********************************************************************
 *  File scope declarations revision history:
 *    11-22-21 jmh:  creation date (modified to support GPSDO, MK-II)
 *
 *******************************************************************/

#include "typedef.h"

#ifndef INIT_H
#define INIT_H
#endif

//-----------------------------------------------------------------------------
// Global Constants
//-----------------------------------------------------------------------------

// timer definitions.  Uses EXTXTAL #def to select between ext crystal and int osc
//  for normal mode.
// SYSCLK value in Hz
#define SYSCLKL 10000L
#ifdef EXTXTAL
#define SYSCLKF 20000000L
#else
#define SYSCLKF (12000000L) // / 8)
#endif
#define	SYSCLK	24500000L
// timer2 register value
#define TMR2RLL_VAL (U8)((65536 -(SYSCLK/(12L * 1000L))) & 0xffL)
#define TMR2RLH_VAL (U8)((65536 -(SYSCLK/(12L * 1000L))) >> 8L)
#define	TMRSECHACK	2
#define TMRIPL		1
#define TMRRUN		0
#define	T2_MS1PS	25

#define MS_PER_TIC  10
// General timer constants
#define MS25        	(25/MS_PER_TIC)
#define MS35        	(35/MS_PER_TIC)
#define MS40        	(40/MS_PER_TIC)
#define MS45        	(45/MS_PER_TIC)
#define MS50        	(50/MS_PER_TIC)
#define MS75        	(75/MS_PER_TIC)
#define MS80        	(80/MS_PER_TIC)
#define MS100        	(100/MS_PER_TIC)
#define MS125       	(125/MS_PER_TIC)
#define MS250       	(250/MS_PER_TIC)
#define MS400       	(400/MS_PER_TIC)
#define MS450       	(450/MS_PER_TIC)
#define MS500       	(500/MS_PER_TIC)
#define MS750       	(750/MS_PER_TIC)
#define MS800       	(800/MS_PER_TIC)
#define MS1000      	(1000/MS_PER_TIC)
#define MS1500      	(1500/MS_PER_TIC)
#define MS1650      	(1650/MS_PER_TIC)
#define MS2000      	(2000/MS_PER_TIC)
#define MS2500      	(2500/MS_PER_TIC)
#define MS5000      	(5000/MS_PER_TIC)
#define MS10000     	(10000/MS_PER_TIC)
#define MS10000     	(10000/MS_PER_TIC)
#define MS12500     	(12500/MS_PER_TIC)
#define MINPERHOUR		60
#define SECPERMIN		60
#define MINPER6HOUR		(6 * MINPERHOUR)
#define MINPER12HOUR	(12 * MINPERHOUR)
#define MINPER18HOUR	(18 * MINPERHOUR)
#define MINPER24HOUR	(24 * MINPERHOUR)

#define	FAN_ON			0x42
#define	FAN_OFF			0x02
#define	FAN_ON_TIME		500	//10MS10000
#define	TEMP_TIMER		MS1000
#define	GPS_TIMEOUT		(MS12500)
// ERROR LED defines
#define	BLINK_RATE		MS1000
#define	BLINK_100		(BLINK_RATE)
#define	BLINK_99		(BLINK_RATE - 1)
#define	BLINK_90		(BLINK_RATE - MS100)
#define	BLINK_50		(BLINK_RATE/2)
#define	BLINK_10		(BLINK_RATE/10)
#define	BLINK_0			0
#define	BLINK_OFF		BLINK_RATE
// ALIVE LED defines
#define	BLINK2_RATE		MS500
#define	BLINK2_100		(BLINK2_RATE)
#define	BLINK2_99		(BLINK2_RATE - 1)
#define	BLINK2_90		(BLINK2_RATE - MS50)
#define	BLINK2_50		(BLINK2_RATE/2)
#define	BLINK2_10		(BLINK2_RATE/10)
#define	BLINK2_0		0
#define	BLINK2_OFF		BLINK2_RATE

// cflag bitmap
#define	GPS_TP			0x01
#define	DIV_TP			0x02
#define	TP_RDY			0x04
#define	MASK_TP			(GPS_TP | DIV_TP | TP_RDY)
#define	GPSTPS			0x10			// GPS IPL activities executed
#define	GPSFINE			0x20			// GPS fine mode active
#define	PPMFAIR			0x40
#define	PPMGOOD			0x80			// VCO loop health flags

//AD5761 DAC defines
#define	DAC_NOP			0x0				// no-operation
#define	DAC_WRINP		0x1				// write to input register
#define	DAC_UPDD		0x2				// xfr input reg to DAC
#define	DAC_WRDAC		0x3				// write to input reg and DAC
#define	DAC_WCNTL		0x4				// write cntl reg
#define	DAC_DRST		0x7				// data reset
#define	DAC_DCDIS		0x9				// disable daisy-chain
#define	DAC_RINP		0xa				// read input reg
#define	DAC_RDAC		0xb				// read DAC reg
#define	DAC_RCNTL		0xc				// read cntl reg
#define	DAC_FRST		0xf				// full reset
//AD5761 control reg bitmap defines
#define	DAC_CNTL_CV_MASK	0x0600		// clear voltage mask
#define	DAC_CNTL_CV_0		0x0000		// clear voltage = 0 scale
#define	DAC_CNTL_CV_MID		0x0200		// clear voltage = mid scale
#define	DAC_CNTL_CV_MAX		0x0400		// clear voltage = max scale
#define	DAC_CNTL_OVR		0x0100		// over-range enable
#define	DAC_CNTL_B2C		0x0080		// 2's compliment data
#define	DAC_CNTL_ETS		0x0040		// over-temp power-down enable
#define	DAC_CNTL_IRO		0x0020		// int reeference enable
#define	DAC_CNTL_PV_MASK	0x0018		// power-up voltage mask
#define	DAC_CNTL_PV_0		0x0000		// power-up 0 scale
#define	DAC_CNTL_PV_MID		0x0008		// power-up = mid scale
#define	DAC_CNTL_PV_MAX		0x0010		// power-up = max scale
#define	DAC_CNTL_RA_MASK	0x0007		// output range mask
#define	DAC_CNTL_RA_0		0x0000		//  -10V to +10V
#define	DAC_CNTL_RA_1		0x0001		//    0V to +10V
#define	DAC_CNTL_RA_2		0x0002		//   -5V to +5V
#define	DAC_CNTL_RA_3		0x0003		//    0V to +5V
#define	DAC_CNTL_RA_4		0x0004		// -2.5V to +7.5V
#define	DAC_CNTL_RA_5		0x0005		//   -3V to +3V
#define	DAC_CNTL_RA_6		0x0006		//    0V to +16V
#define	DAC_CNTL_RA_7		0x0007		//    0V to +20V
#define	DAC_CONFIG			(DAC_CNTL_CV_MID | DAC_CNTL_ETS | DAC_CNTL_IRO | DAC_CNTL_PV_MID | DAC_CNTL_RA_3)

// VCO state machine defines
#define	VCO_AQS		0x01
#define	VCO_AQS1	0x02
#define	VCO_TRACK	0x10
#define	VCO_TRACK1	0x11
#define	VCO_TRACK2	0x12
#define	VCO_DR		0x20
#define	VCO_DR1		0x21
#define	VCO_DR2		0x22

#define	AVE_COUNT	5
#define	MIN_MAX_COUNT	10
#define	DAC_HOLD_COUNT	1
#define	MAX_MARK	(1000000L)
#define	MID_MARK	(MAX_MARK / 2L)
#define	DEADLOCK_U	(600000L)
#define	DEADLOCK_L	(400000L)

// DS1722 defines
#define	TEMP_27		0x1b00				// TEC control dead-band
#define	TEMP_23		0x1720

// Ublox defines
// time mark flags
#define	TMK_MODE	0x01			// 1 = running
#define	TMK_RUN		0x02			// 1 = stopped
#define	TMK_FE		0x04			// 1 = new falling edge
#define	TMK_TB		0x18			// 0 = rx time, 1 = GNSS time, 2 = UTC
#define	TMK_UTC		0x20			// 1 = UTC available
#define	TMK_TVALID	0x40			// 1 = time valid
#define	TMK_RE		0x80			// 1 = new rising edge
//-----------------------------------------------------------------------------
// Global variables
//-----------------------------------------------------------------------------

#ifndef IS_MAINC
//extern U8 spi_tmr;
#endif

//-----------------------------------------------------------------------------
// Prototypes
//-----------------------------------------------------------------------------

void Init_Device(void);
void wait(U8 wvalue);

//-----------------------------------------------------------------------------
// End Of File
//-----------------------------------------------------------------------------
