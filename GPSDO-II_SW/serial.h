/*************************************************************************
 *********** COPYRIGHT (c) 2020 by Joseph Haas (DBA FF Systems)  *********
 *
 *  File name: serial.h
 *
 *  Module:    Control
 *
 *  Summary:   This is the header file for serial I/O.
 *
 *******************************************************************/


/********************************************************************
 *  File scope declarations revision history:
 *    08-23-20 jmh:  modified to support bluetooth interface
 *    09-09-12 jmh:  creation date
 *
 *******************************************************************/

//------------------------------------------------------------------------------
// extern defines
#define	MAX_CTR 4			// max# response chrs to get

//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// public Function Prototypes
//------------------------------------------------------------------------------

void init_serial(void);
void init_buff(void);
char putch(const char c);
//void cleanline(void);
//char anych00(void);
//char getch00(void);
//char gotch00(void);
//char gotcr(void);
//void putss (char *string);
//char* getss (char* string, U8 maxlen);
//char iseol(char c);
//void cpy_str (char* src, char* dest);
//char hiasc (U8 num);
//char lowasc (U8 num);
U8 getm (U32* rslt, U32* accuracy, U8 cmd);

//------------------------------------------------------------------------------
// global defines
//------------------------------------------------------------------------------

#define NOTBUF 0
#define TBUF 1
#define	ESC	27