/*************************************************************************
 *********** COPYRIGHT (c) 2021 by Joseph Haas (DBA FF Systems)  *********
 *
 *  File name: f300_init.c
 *
 *  Module:    Control
 *
 *  Summary:   This is the init code for the F530.
 *
 *******************************************************************/


/********************************************************************
 *  File scope declarations revision history:
 *    11-22-21 jmh:  creation date (modified to support GPSDO, MK-II)
 *
 *******************************************************************/

#include "compiler_defs.h"
#include "c8051F520.h"

/////////////////////////////////////
//  Generated Initialization File  //
/////////////////////////////////////

// Peripheral specific initialization functions,
// Called from the Init_Device() function
void PCA_Init()
{
    PCA0CN    = 0x40;
    PCA0MD    &= ~0x40;
    PCA0MD    = 0x08;
    PCA0CPM0  = 0x21;
    PCA0CPM1  = 0x43;
    PCA0CPM2  = 0x21;
}

void Timer_Init()
{
    TCON      = 0x40;
    TMOD      = 0x22;
    CKCON     = 0x01;
    TL0       = 0xC2;
    TH0       = 0xC2;
    TH1       = 0xB0;
    TMR2CN    = 0x04;
    TMR2RLL   = 0x3F;
    TMR2RLH   = 0xB0;
    TMR2L     = 0x3F;
    TMR2H     = 0xB0;
}

void UART_Init()
{
    SCON0     = 0x10;
}

void SPI_Init()
{
    SPI0CFG   = 0x40;
    SPI0CN    = 0x00;
    SPI0CKR   = 0x79;
}

void Port_IO_Init()
{
    // P0.0  -  Skipped,     Push-Pull,  Digital
    // P0.1  -  Skipped,     Open-Drain, Digital
    // P0.2  -  Skipped,     Push-Pull,  Digital
    // P0.3  -  CEX0  (PCA), Open-Drain, Digital
    // P0.4  -  TX   (UART), Push-Pull,  Digital
    // P0.5  -  RX   (UART), Open-Drain, Digital
    // P0.6  -  CEX1  (PCA), Push-Pull,  Digital
    // P0.7  -  CEX2  (PCA), Open-Drain, Digital

    // P1.0  -  Unassigned,  Push-Pull,  Digital
    // P1.1  -  Unassigned,  Push-Pull,  Digital
    // P1.2  -  Unassigned,  Push-Pull,  Digital
    // P1.3  -  Unassigned,  Push-Pull,  Digital
    // P1.4  -  Unassigned,  Push-Pull,  Digital
    // P1.5  -  Unassigned,  Push-Pull,  Digital
    // P1.6  -  Unassigned,  Push-Pull,  Digital
    // P1.7  -  Unassigned,  Push-Pull,  Digital

    P0MDOUT   = 0x55;
    P1MDOUT   = 0xFF;
    P0SKIP    = 0x07;
    P0SKIP    |= 0x30;
    XBR0      = 0x01;
    XBR1      = 0x43;
}

void Oscillator_Init()
{
    OSCICN    = 0xC7;
}

void Interrupts_Init()
{
    EIE1      = 0x04;
    IE        = 0x32;
}

// Initialization function for device,
// Call Init_Device() from your main program
void Init_Device(void)
{
    PCA_Init();
    Timer_Init();
    UART_Init();
    SPI_Init();
    Port_IO_Init();
    Oscillator_Init();
    Interrupts_Init();
}
