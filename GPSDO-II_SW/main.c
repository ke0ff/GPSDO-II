/****************************************************************************************
 ****************** COPYRIGHT (c) 2021 by Joseph Haas (DBA FF Systems)  *****************
 *
 *  File name: main.c
 *
 *  Module:    Control
 *
 *  Summary:   This is the main code file for the GPSDO, MK-II application
 *	License and other legal stuff:
 *			   This software, comprised of all files contained in the original distribution archive,
 *				are protected by US Copyright laws.  The files may be used and modified by the person
 *				receiving them under the following terms and conditions:
 *				1) The software, or any protion thereof may not be distributed to any 3rd party by
 *					the recipient or any agent or assign of the recipient.
 *				2) The recipient assumes all risks for the use (or mis-use) of this software.
 *
 *
 *  Project scope revision history:
 *    11-24-21 jmh:  Initial project functionality coded.  Temperature, DAC, UART, and LEDs all functional.
 *					 Coded the begninings of control loops for the VCO and TEC.
 *    11-22-21 jmh:  Project origin, copied from HM133
 *
 ***************************************************************************************/

//--------------------------------------------------------------------------------------
// main.c
//
//      The following resources of the F531 are used:
//      24.500 MHz internal osc
//
//      UART: GPS command I/O
//
//      Timer0: bbSPI clock driver
//      Timer1: UART baud clock (38400 baud)
//      Timer2: Application timer (10ms/tic)
//
//      ADC: n/u
//
//      PCA: 
//			 CEX0 = GPS time pulse
//			 CEX1 = fan pwm out (pwm mode also used for on-off control)
//			 CEX2 = vco divider time pulse (deprecated)
//
//      SYSTEM NOTES:
//
//		This project drives a VCO and TEC stack to both temperature and frequency stabilize
//		a 10 MHz VCO.  The VCO output is buffered and provided to up to three 50 ohm outputs.
//		LEDs annunciate the GPS time-pulse outputs, the TEC fan control, and VCO stabilization
//		status.
//
//		Two LEDs are provided for VCO stab status: "ALIVE" and "ERROR".  Alive (GRN) toggles whenever
//		there is a valid time pulse capture.  In normal operation the toggle period is as long
//		as 5 sec, or as short as 1 sec.  Error (YEL) blinks at a 1 sec rate with a variable duty
//		cycle to denote the level of error.  Steady on is the maximum error state (no GPS pulses),
//		while steady off is the minimum error state (VCO stability within a set tolerance).
//
//		Using a divider toggle period of 5 sec, the conversion ratio calculates to be about 100/175 DACLSBs/ns
//		to get an approximate solution to close the loop.  Reading this time directly from the GPS time-mark
//		value gives a way to directly calculate a solution:
//
//			deldac = delta(tt) * 100 / 175
//
//--------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
// compile defines
#include "init.h"
#include "typedef.h"
#include "c8051F520.h"
#include "serial.h"
#include "flash.h"
#include "nvmem.h"

//-----------------------------------------------------------------------------
// Definitions
//-----------------------------------------------------------------------------



//#define	IS_SIM				// enable this define if using simulator

//  see init.h for #defines


//-----------------------------------------------------------------------------
// External Variables
//-----------------------------------------------------------------------------

//extern code const U8	SINE[];
//idata volatile	U16		hmd_timer;						// HM151 data idle timeout timer

//-----------------------------------------------------------------------------
// Main Variables
//-----------------------------------------------------------------------------

// port assignments

// PCB version 2 bbSPI defines
sbit SPCK       = P0^0;                         // (o) [spi]	SPI CLK
sbit MISO       = P0^1;                         // (i) [spi]	SPI MISO
sbit MOSI       = P0^2;                         // (o) [spi]	SPI MOSI
sbit GPSTP      = P0^3;                         // (i) [pca]	
sbit TXD_MCU    = P0^4;                         // (o) [uart]	
sbit RXD_MCU    = P0^5;                         // (i) [uart]	
sbit TEC_FANON_N = P0^6;                        // (o) [pca]	
sbit VCOTP      = P0^7;                         // (i) [pca]	
sbit CS_DAC_N   = P1^0;                         // (o) [gpio]	
                                                //
sbit CS_TS      = P1^1;                         // (o) [gpio]	PPM Error LED
sbit RST_GPS_N  = P1^2;                         // (o) [gpio]	PPM Fair LED
sbit TEC_HOT_N  = P1^3;                         // (o) [gpio]	PPM Good LED
sbit TEC_COOL_N = P1^4;                         // (o) [gpio]	PLL Divider reset (0 = reset)
sbit DIV_RST    = P1^5;                         // (o) [gpio]	PEA reset (1 = reset)
sbit ALIVE	    = P1^6;                         // (o) [gpio]	Status LED (0 = on, 1 = off)
sbit ERROR      = P1^7;                         // (o) [gpio]	PEA register select

//-----------------------------------------------------------------------------
// Local variables
//-----------------------------------------------------------------------------
	// app timers
	volatile	U8	 	waittimer;		    // wait() function timer
	volatile	U8	 	ttimer;		        // temperature loop timer
	volatile	U16	 	gpstimer;		    // gps valid timer
	volatile	U16	 	fantimer;		    // fan on timer
	volatile	U8	 	blinkpwm;			// blink regs for ERROR LED
	volatile	U8	 	blinktimer;
	volatile	U8	 	blinkpwm2;			// blink regs for ALIVE LED
	volatile	U8	 	blinktimer2;
				bit		blink_alive;		// blink enable for ALIVE LED
				bit		thold;				// 16-bit timer hold flag
	volatile	U8	 	ovrflo_count;		// pca overflow counter
	
	// bbSPI registers
	volatile	U8		spdr_r;				// bbSPI MISO register
	volatile	U8		spdr;				// bbSPI MOSI register
	volatile	U8		spmask;				// bbSPI shift mask register

	// PCA capture registers
	volatile	U16		gcap;				// gps time pulse capture
	volatile	U16		dcap;				// vco divider time pulse capture
	volatile	U8		cflag;				// capture flags


//-----------------------------------------------------------------------------
// Local Prototypes
//-----------------------------------------------------------------------------

void send8(U8 sdata);
U16 read_1722(U8 cdata);
void rw_5761(U8 cdata, U16 ddata);

//******************************************************************************
// main()
//  The main function inits I/O and process I/O.
//
//******************************************************************************
void main(void) //using 0
{
idata	volatile U8		i;				// temp uchar
idata	volatile U8		vco_state;		// vco state machine reg
//idata	volatile U8		dacupdate;		// tracking loop -- 1 = incremented last
				bit		run;			// warm restart trigger
				bit		vipl;			// vco IPL flag
				bit		o;				// timark polarity temp flags
				bit		oo;
//idata volatile	U8		tempf;		// temp cflag
	  volatile	U16		tt_temp;
idata volatile	U16		ecount;			// temp
idata volatile	U16		ii;				// temp
//idata volatile	U16		dacmax;			// dac min/max values
//idata volatile	U16		dacmin;
data volatile	U16		dac;			// dac value
data volatile	U32		deldac;			// delta DAC value
data volatile	U32		tt;				// time-mark 
data volatile	U32		aa;				// time-mark accuracy
data volatile	U32		tto;			// previous cycle time-mark
data volatile	U32		avett;			// ave time-mark accum 


	// start of main (outer loop)
	while(1){									// outer loop is for soft-restart capability
		PCA0MD = 0x00;							// disable watchdog
		EA = 0;
		// init MCU system
		Init_Device();							// init MCU
		CS_TS = 0;
		CS_DAC_N = 1;
		RXD_MCU = 1;                        	// (i) [uart]	
		SPCK = 0;								// (o) [spi]	SPI CLK
		MISO = 1;								// (i) [spi]	SPI MISO
		MOSI = 0;								// (o) [spi]	SPI MOSI
		GPSTP = 1;								// (i) [pca]	
//		TEC_FANON_N = 0;						// (o) [pca]	
		PCA0CPM1  = FAN_OFF;
//		PCA0CPL1 = 0x0;
//		PCA0CPH1 = 0x0;							// fan off (PWM = 100%)
		VCOTP = 1;								// (i) [pca]	
		RST_GPS_N = 1;							// (o) [gpio]	PPM Fair LED
		TEC_HOT_N = 1;							// (o) [gpio]	PPM Good LED
		TEC_COOL_N = 1;							// (o) [gpio]	PLL Divider reset (0 = reset)
		DIV_RST = 1;							// (o) [gpio]	PEA reset (1 = reset)
		ALIVE = 0;								// (o) [gpio]	Status LED (0 = on, 1 = off)
		ERROR = 0;								// (o) [gpio]	PEA register select
		blinkpwm = BLINK_0;
		init_flash();							// init FLASH registers
		init_serial();
		TEC_HOT_N  = 1;                         // init TEC H-bridge
		TEC_COOL_N = 1;
		EA = 1;
		wait(10);
		DIV_RST = 0;
		cflag = 0;
		read_1722(1);							// init temperature sensor
		rw_5761(DAC_WCNTL, DAC_CONFIG);			// init DAC
		run = 1;								// enable run
		thold = 1;
		gpstimer = 0;
		thold = 0;
		getm(&tt, &aa, 1);							// init get time-mark function
		vco_state = VCO_DR;						// init VCO state machine
		ecount = AVE_COUNT;
		avett = 0;
//		dacupdate = DAC_HOLD_COUNT;
//		read_flast();
//		write_flast(0);

/*		rw_5761(DAC_WRDAC, 33864);			// set DAC output
		rw_5761(DAC_WRDAC, 33863);				// set DAC output
		rw_5761(DAC_WRDAC, 33862);			// set DAC output
		rw_5761(DAC_WRDAC, 33861);			// set DAC output*/

		vipl = 1;
		// main loop (run)
		while(run){											// inner-loop runs the main application
			PCON = 1;										// set idle mode (WAI)
			switch(vco_state){
			//
			// ********** VCO acquisition loop ************ //
			//
			case VCO_AQS:
				//  VCO AQS loop
				i = getm(&tt, &aa, 0);
				if(i == 0){
					ALIVE = ~ALIVE;
					if(vipl){
						tto = tt;
						vipl = 0;
						deldac = 16384L;					// start at 1/2 DAC_FS
					}else{
						if(tt > tto){						// VCO too fast
							dac -= (U16)deldac;
						}else{								// VCO too slow
							dac += (U16)deldac;
						}
						if(deldac > 1){
							deldac >>= 1;
						}else{
							vco_state = VCO_TRACK;
							blinkpwm = BLINK_10;			// ERROR LED = 10%
						}
						tto = tt;
						rw_5761(DAC_WRDAC, dac);			// set DAC output
					}
				}
				if((i == 0) || (i == 4)){					// reset gps timeout if GPS is active and time valid
					thold = 1;
					gpstimer = GPS_TIMEOUT;
					thold = 0;
				}
				if(!gpstimer) vco_state = VCO_DR;			// GPS lost, switch to DR mode
				break;
			//
			// ************ VCO tracking loop ************* //
			//
#define	KP	1264L
#define	KPD	1000L
			case VCO_TRACK:
				i = getm(&tt, &aa, 0);						// update current time mark (tt)
				if(i == 0){
					if((tt > DEADLOCK_L) && (tt < DEADLOCK_U)){
						i = 10;
						DIV_RST = 1;						// re-sync the GPS and DIV time-pulses
						while(DIV_RST);
						thold = 1;							// start GPS time-out
						gpstimer = GPS_TIMEOUT;
						thold = 0;
					}						
				}
				if(i == 0){									// tt is valid..
//					if(!dacupdate){
						if(tt != tto){
							if(tt > MID_MARK) o = 1;
							else o = 0;
							if(tto > MID_MARK) oo = 1;
							else oo = 0;
							if(o == oo){						// values are in same hemisphere
								if(tt > tto){					// VCO too slow
									deldac = tt - tto;
									tt_temp = (U16)deldac;
									avett += deldac;
//									if(deldac > aa) dac += ((U16)deldac * KP) / KPD;
//									else dac += 1;
								}else{							// VCO too fast
									deldac = tto - tt;
									tt_temp = (U16)deldac;
									avett -= deldac;
//									if(deldac > aa) dac -= ((U16)deldac * KP) / KPD;
//									else dac -= 1;
								}
							}else{								// values have crossed hemispheres, must handle decimal rollover...
								if(o == 0){						// VCO too slow
									deldac = tt + (MAX_MARK - tto);
									tt_temp = (U16)deldac;
									avett += deldac;
//									if(deldac > aa) dac += ((U16)deldac * KP) / KPD;
//									else dac += 1;
								}else{							// VCO too fast
									deldac = tto + (MAX_MARK - tt);
									tt_temp = (U16)deldac;
									avett -= deldac;
//									if(deldac > aa) dac -= ((U16)deldac * KP) / KPD;
//									else dac -= 1;
								}
							}
							tto = tt;
//							if(dac < dacmin) dacmin = dac;
//							if(dac > dacmax) dacmax = dac;
//							dac = ((dacmax - dacmin) >> 1) + dacmin;
//							dacupdate = DAC_HOLD_COUNT;
						}
//					}else{
//						dacupdate--;						// decrement hold count (dac is not updated
//					}
					if(ecount){
						if(--ecount == 0){
							ecount = AVE_COUNT;
//							dacmin = dac;
//							dacmax = dac;
							ALIVE = ~ALIVE;
							if(avett > 0x7fffffffL){		// if ave deltaT is negative...
								avett = (~avett) + 1;
								avett /= AVE_COUNT;
								dac -= (U16)((avett * KP) / KPD);
							}else{
								avett /= AVE_COUNT;
								dac += (U16)((avett * KP) / KPD);
							}
							avett = 0;
							rw_5761(DAC_WRDAC, dac);		// set DAC output
						}
					}
				}
				if((i == 0) || (i == 4)){					// reset gps timeout if GPS is active and time valid
					thold = 1;
					gpstimer = GPS_TIMEOUT;
					thold = 0;
				}
				if(!gpstimer) vco_state = VCO_DR2;			// GPS lost, switch to DR mode
				break;

			case VCO_TRACK1:
				i = getm(&tt, &aa, 0);
				if((cflag & GPS_TP) || (i == 0)){
					DIV_RST = 1;							// re-sync the GPS and DIV time-pulses
					while(DIV_RST);
					thold = 1;								// start GPS time-out
					gpstimer = GPS_TIMEOUT;
					thold = 0;
					cflag &= ~(GPSTPS | GPSFINE | GPS_TP | DIV_TP);
					vco_state = VCO_TRACK2;					// set acquisition state
					ALIVE = 0;
//					tto = tt;
				}
				if((i == 0) || (i == 4)){					// reset gps timeout if GPS is active and time valid
					thold = 1;
					gpstimer = GPS_TIMEOUT;
					thold = 0;
				}
				if(!gpstimer) vco_state = VCO_DR2;			// GPS lost, switch to DR mode
				break;

			case VCO_TRACK2:
				i = getm(&tt, &aa, 0);
				if(i == 0){
					thold = 1;								// start GPS time-out
					gpstimer = GPS_TIMEOUT;
					thold = 0;
					cflag &= ~(GPSTPS | GPSFINE | GPS_TP | DIV_TP);
					vco_state = VCO_TRACK;					// set acquisition state
					blinkpwm = BLINK_10;					// set error 2 indication
					tto = tt;
				}
				if((i == 0) || (i == 4)){					// reset gps timeout if GPS is active and time valid
					thold = 1;
					gpstimer = GPS_TIMEOUT;
					thold = 0;
				}
				if(!gpstimer) vco_state = VCO_DR2;			// GPS lost, switch to DR mode
				break;
			//
			// ********* VCO dead-reconning loop ********** //
			//
			default:
			case VCO_DR:
				// set DAC to stored value if there is no GPS acquired
				dac = read_flast();							// IPL DR entry, recall saved DAC or use code default
				if(dac == 0xffff){
					dac = 34150; //33933; //34039; //33966;					// flash empty, use emperical value
//					dac = 50000;
//					deldac = 1;
					blink_alive = 0;
					vco_state = VCO_TRACK1;
					thold = 1;								// fake the GPS for now (it will drop out of the tracking loop if absent)
					gpstimer = GPS_TIMEOUT;
					thold = 0;
				}else{
					deldac = 1;
					blinkpwm = BLINK_100;					// set error 3 indication
					ERROR = 1;
					vco_state = VCO_DR1;					// init VCO state machine
					blink_alive = 1;
				}
//				dacmin = dac;								// init min/max
//				dacmax = dac;
				rw_5761(DAC_WRDAC, dac);					// set DAC output
				wait(10);									// let the VCO settle a bit
				break;

			case VCO_DR2:									// DR entry (keep current DAC setting)
				blinkpwm = BLINK_100;						// set error 3 indication
				ERROR = 1;
				vco_state = VCO_DR1;						// init VCO state machine
				blink_alive = 1;
				break;

			case VCO_DR1:
				// look for GPS activity
				if((cflag & GPS_TP) || (getm(&tt, &aa, 0) == 0)){
					vipl = 1;								// re-IPL the VCO
					DIV_RST = 1;							// re-sync the GPS and DIV time-pulses
					while(DIV_RST);
					thold = 1;								// start GPS time-out
					gpstimer = GPS_TIMEOUT;
					thold = 0;
					cflag &= ~(GPSTPS | GPSFINE | GPS_TP | DIV_TP);
					vco_state = VCO_AQS;					// set acquisition state
					blinkpwm = BLINK_50;					// set error 2 indication
					blink_alive = 0;
					ALIVE = 0;
				}
				break;
			}
			// TEC control loop
			if(ttimer == 0){
				ttimer = TEMP_TIMER;
				ii = read_1722(0);
				if(ii > TEMP_27){							// apply cool
					TEC_HOT_N = 1;
					TEC_COOL_N = 0;
					thold = 1;
					fantimer = FAN_ON_TIME;
					thold = 0;
//					TEC_FANON_N = 1;
//					PCA0CPL1 = 0xff;
//					PCA0CPH1 = 0xff;						// fan on (PWM = 0.0015%)
					PCA0CPM1  = FAN_ON;						// fan on (PWM enabled at 100%)
				}else{
					if(ii < TEMP_23){						// apply heat
						TEC_COOL_N = 1;
						TEC_HOT_N = 0;
						thold = 1;
						fantimer = FAN_ON_TIME;
						thold = 0;
//						TEC_FANON_N = 1;
//						PCA0CPL1 = 0xff;
//						PCA0CPH1 = 0xff;					// fan on (PWM = 0.0015%)
						PCA0CPM1  = FAN_ON;					// fan on (PWM enabled at 100%)
					}else{
						TEC_HOT_N = 1;						// TEC off
						TEC_COOL_N = 1;
						if(fantimer == 0){
//							PCA0CPL1 = 0x0;
//							PCA0CPH1 = 0x0;					// fan off (PWM = 100%)
							PCA0CPM1  = FAN_OFF;			// fan on (PWM disabled)
						}
					}
				}
			}
		} // end while(run)
	} // end outer while()
} // end main()

// *********************************************
//  *************** SUBROUTINES ***************
// *********************************************

//************************************************************************
// send8() does a bit-bang SPI into a CD4094 to produce a port expansion
//	output.  Uses T0 to clock data
//************************************************************************
void send8(U8 sdata){
	
	spmask = 0x80;									// init mask
	spdr = sdata;									// store to bbSPI data reg
	TR0 = 1;
	while(TR0);										// loop until xfr complete
	return;
}

//************************************************************************
// read_1722() does a bit-bang SPI into DS1722 temp sensor IC
//	CDATA == 0, do temp read, else send config data to sensor
//************************************************************************
U16 read_1722(U8 cdata){
	U16	i = 0;

	CS_TS = 1;
	if(cdata){
		send8(0x80);								// set config register
		send8(0xee);
	}else{
		send8(0x01);								// read data register
		send8(0x00);								// read data register
		i = (U16)spdr_r;
		send8(0x00);								// read data register
		i |= ((U16)spdr_r) << 8;
	}
	CS_TS = 0;
	return i;
}

//************************************************************************
// rw_5761() does a bit-bang SPI r/w for the AD5761 DAC
//	cdata is register addr (lower 4 bits are active)
//	ddata is DAC data
//************************************************************************
void rw_5761(U8 cdata, U16 ddata){

	CS_DAC_N = 0;
	send8(cdata);									// write config (addr) register
	send8((U8)(ddata >> 8));						// write data
	send8((U8)ddata);
	CS_DAC_N = 1;
	return;
}
//************************************************************************
// wait() waits the U8 value then returns.
//************************************************************************
void wait(U8 wvalue){

	waittimer = wvalue;								// set the timer
	while(waittimer);								// wait for it to expire
	return;
}

//-----------------------------------------------------------------------------
// pca_intr
//-----------------------------------------------------------------------------
//
// <add description>
//-----------------------------------------------------------------------------

void pca_intr(void) interrupt 9 using 2{
	U8	i;		// temps
	U8	j;

    // process GPS TimePulse
    if(CCF0 == 1){
		DIV_RST = 0;								// enable divider
        i = PCA0L;									// get capture time
		j = PCA0H;
		gcap = (((U16)i) << 8) + ((U16)j);
		if(cflag & DIV_TP) cflag |= TP_RDY;
		else cflag |= GPS_TP;
        CCF0 = 0;                       			// clr intr flag
    }
    // process fan PWM -- we have to process the ISR flag to get the PWM to work
	//	(even though we don't need to do anything other than clear the flag)
    if(CCF1 == 1){
        CCF1 = 0;                       			// clr intr flag
    }
    // process divider-chain TimePulse
    if(CCF2 == 1){
        i = PCA0L;									// get capture time
		j = PCA0H;
		dcap = (((U16)i) << 8) + ((U16)j);
		if(cflag & GPS_TP) cflag |= TP_RDY;
		else cflag |= DIV_TP;
        CCF2 = 0;                      				// clr intr flag
    }
    // process PCA overflow
    if(CF == 1){
		ovrflo_count++;
        CF = 0;                       				// clr intr flag
    }
    return;
}

//-----------------------------------------------------------------------------
// Timer0_ISR
//-----------------------------------------------------------------------------
//
// Called when timer 0 overflows (NORM mode):
//      drives bbSPI by shifting 8 bits out MOSI at the timer rate/2 (the timer
//			sets the half clock period).
//		If T0 enabled with spmask = 0, nothing happens except T0 is disabled when
//			the interrupt happens.  This is used to time the strobe pulse (external
//			code handles the strobe set/clear).
//
//-----------------------------------------------------------------------------

void Timer0_ISR(void) interrupt 1
{
	if(!spmask){									// used to time strobe pulse
		TR0 = 0;									// turn off T0 intrpt
	}else{
		if(SPCK){									// toggle clock & set MOSI or shift data, depending on edge
			if(MISO == 1) spdr_r |= spmask;			// shift in data
			else spdr_r &= ~spmask;
			SPCK = 0;
			spmask >>= 1;							// FE: shift data
			if(!spmask) TR0 = 0;					// xfr done, disable intrpt
		}else{
			SPCK = 1;
			if(spdr & spmask) MOSI = 1;				// RE: transfer sdata bit to output
			else MOSI = 0;
		}
	}
    TF0 = 0;                           				// Clear Timer0 intrpt flag

	return;
}

//-----------------------------------------------------------------------------
// Timer1_ISR
//-----------------------------------------------------------------------------
//
// Called when timer 1 overflows (NORM mode):
//      drives DDS
//
//-----------------------------------------------------------------------------
/*
void Timer1_ISR(void) interrupt 3 using 2
{
	// DDS vars
	U8		pac;				// temp regs
	U16		pdac;
	static U16	phaccum1;		// tone 1 phacc
	static U16	phaccum2;		// tone 2 phacc

    TF1 = 0;                           				// Clear Timer2 interrupt flag
	
	if(ipldds){
		phaccum1 = 0;
		phaccum2 = 0;
		ipldds = 0;
	}
	// process phase accumulator 1
	phaccum1 += delF1;								// add delta for tone 1
	pac = (U8)(phaccum1 >> 8);
	pdac = (U16)SINE[pac];

	// process phase accumulator 2
	phaccum2 += delF2;								// add delta for tone 2
	pac = (U8)(phaccum2 >> 8);
	pdac += (U16)SINE[pac];							// add tone 2 DAC to holding reg
	pdac >>= 1;										// div by 2 to get 8 bit combined tone DAC value
	// store pdac to pwm
	PCA0CPH1 = (U8)pdac;							// storing here to sync update
}*/

//-----------------------------------------------------------------------------
// Timer2_ISR
//-----------------------------------------------------------------------------
//
// Called when timer 2 overflows (NORM mode):
//      updates app timers @ 10ms rate
//		rate = (sysclk/12) / (65536 - TH:L)
//
//-----------------------------------------------------------------------------

void Timer2_ISR(void) interrupt 5 using 2
{

    TF2H = 0;                           			// Clear Timer2 interrupt flag
	if(!thold){
		if(fantimer) fantimer--;					// wrap 16 bit timers in a hold space to prevent register contention
		if(gpstimer) gpstimer--;
	}
	if(waittimer) waittimer--;						// process app timers 
	if(ttimer) ttimer--;
	blinktimer--;									// ERROR LED blink timer (pwm period is 1 sec)
	if(blinkpwm == blinktimer){
		if(blinkpwm) ERROR = 1;
	}
	if(blinktimer == 0){
		if(blinkpwm < BLINK_100) ERROR = 0;
		blinktimer = BLINK_RATE;
	}
	if(blink_alive){								// ALIVE LED blink timer (pwm period is 0.5 sec)
		blinktimer2--;
		if(blinkpwm2 == blinktimer2){
			if(blinkpwm2) ALIVE = 1;
		}
		if(blinktimer2 == 0){
			if(blinkpwm2 < BLINK2_100) ALIVE = 0;
			blinktimer2 = BLINK2_RATE;
		}
	}
	return;
}

//#undef IS_MAINC
//**************
// End Of File
//**************
