//------------------------------------------------------------------------------

// flash.c

//------------------------------------------------------------------------------

// Copyright (C) 2019 KE0FF
//

// Date: 08/11/16

// Target: C8051F52x 

//

// Description:

//    This file contains the flash scratchpad r/w routines.

//
#include "typedef.h"
#include "c8051F520.h"
#include "stdio.h"
#define FLASH_INCL
#include "flash.h"
#include "nvmem.h"

//------------------------------------------------------------------------------
// Define Statements
//------------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Variable Declarations
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// flash initialization routine
//-----------------------------------------------------------------------------
void init_flash(void)
{

//	FLSCL = FLRT;
	PSCTL = 0x00;
}

//-----------------------------------------------------------------------------
// find last flash write 
//-----------------------------------------------------------------------------
U16 find_flash(void)
{
U16		addr = 0;
bit		b = 1;

	while(b){
		if(dac_save[addr] == 0xffff){
			b = 0;
		}else{
			addr++;
			if(addr >= MAXARY){
				b = 0;
			}
		}
	}
	if(!addr) addr -= 1;
	return addr;
}

//-----------------------------------------------------------------------------
// read last flash flash word
//-----------------------------------------------------------------------------
U16 read_flast(void)
{
U16		addr;

	addr = find_flash();
	return dac_save[addr];
}

//-----------------------------------------------------------------------------
// write word to last flash
//-----------------------------------------------------------------------------
U8 write_flast(U16 ddata)
{
U16		addr;		// temp addr
char	i = 0;		// temp and return

	if(ddata != 0xffff){
		addr = find_flash() + 1;					// find next empty addr
			if(addr >= MAXARY){						// if past end, erase FLASH
				erase_flash(START_ADDR);
				addr = 0;
			}
		i = (U8)ddata;								// write two data bytes
		wr_flash(i, (U8*)&dac_save[addr]);
		i = (U8)(ddata >> 8);
		wr_flash(i, ((U8*)&dac_save[addr]) + 1);
		i = 1;										// set OK return
	}
	return i;
}

//-----------------------------------------------------------------------------
// flash erase routine
//	erases "scratchpad" sector pointed to by addr
//-----------------------------------------------------------------------------
U8 erase_flash(U8 xdata * addr)
{
	U8	EA_save;
	U8	rtn;

	EA_save = EA;
	EA = 0;							// interrupts = off
	FLKEY = 0xA5;					// unlock FLASH
	FLKEY = 0xF1;
	rtn = FLKEY;
	if(rtn != 0x02){
		rtn |= 0x80;
	}
//	PSCTL = PSWE;					// enable erase
	PSCTL = PSEE|PSWE;				// enable movx
	*addr = 0xff;					// erase sector
	PSCTL = 0x00;					// disbale erase
	EA = EA_save;					// restore intr
	return rtn;
}


//-----------------------------------------------------------------------------
// flash write routine
//	writes byte to scratchpad sector pointed to by addr
//-----------------------------------------------------------------------------
void wr_flash(char byte, U8 xdata * addr)
{
U8	EA_save;

	EA_save = EA;
	EA = 0;							// interrupts = off
	FLKEY = 0xA5;					// unlock FLASH
	FLKEY = 0xF1;
	PSCTL = PSWE;					// enable movx
	*addr = byte;					// write data
	PSCTL = 0x00;					// disable flash wr
	EA = EA_save;					// restore intr
}


