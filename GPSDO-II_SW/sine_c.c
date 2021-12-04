/****************************************************************************************
 ****************** COPYRIGHT (c) 2019 by Joseph Haas (DBA FF Systems)  *****************
 *
 *  File name: sine_c.c
 *
 *  Module:    Control
 *
 *  Summary:   Stored sine array
 *	License and other legal stuff:
 *			   This software, comprised of all files contained in the original distribution archive,
 *				are protected by US Copyright laws.  The files may be used and modified by the person
 *				receiving them under the following terms and conditions:
 *				1) The software, or any protion thereof may not be distributed to any 3rd party by
 *					the recipient or any agent or assign of the recipient.
 *				2) The recipient assumes all risks for the use (or mis-use) of this software
 *
 *
 *  Project scope revision history:
 *    07-24-19 jmh:  Project origin, copied from Orion
 *
 ***************************************************************************************/

/****************************************************************************************
 *  File scope revision history:
 *  07-24-19 jmh:	creation date
 *
 ***************************************************************************************/

#include "typedef.h"
// sine lookup table
// size =   256, min =     5, max =   251 
// MID = 128.000000 SWING = 123.000000
code const U8	SINE[256] = {
	  128,   131,   134,   137,   140,   143,   146,   149,   152,   155,   158,   161,   164,   167,   169,   172,
	  175,   178,   181,   183,   186,   189,   191,   194,   196,   199,   201,   204,   206,   208,   211,   213,
	  215,   217,   219,   221,   223,   225,   227,   229,   230,   232,   234,   235,   236,   238,   239,   240,
	  242,   243,   244,   245,   246,   247,   247,   248,   249,   249,   250,   250,   250,   251,   251,   251,
	  251,   251,   251,   251,   250,   250,   250,   249,   249,   248,   247,   247,   246,   245,   244,   243,
	  242,   240,   239,   238,   236,   235,   234,   232,   230,   229,   227,   225,   223,   221,   219,   217,
	  215,   213,   211,   208,   206,   204,   201,   199,   196,   194,   191,   189,   186,   183,   181,   178,
	  175,   172,   169,   167,   164,   161,   158,   155,   152,   149,   146,   143,   140,   137,   134,   131,
	  128,   125,   122,   119,   116,   113,   110,   107,   104,   101,    98,    95,    92,    89,    87,    84,
	   81,    78,    75,    73,    70,    67,    65,    62,    60,    57,    55,    52,    50,    48,    45,    43,
	   41,    39,    37,    35,    33,    31,    29,    27,    26,    24,    22,    21,    20,    18,    17,    16,
	   14,    13,    12,    11,    10,     9,     9,     8,     7,     7,     6,     6,     6,     5,     5,     5,
	    5,     5,     5,     5,     6,     6,     6,     7,     7,     8,     9,     9,    10,    11,    12,    13,
	   14,    16,    17,    18,    20,    21,    22,    24,    26,    27,    29,    31,    33,    35,    37,    39,
	   41,    43,    45,    48,    50,    52,    55,    57,    60,    62,    65,    67,    70,    73,    75,    78,
	   81,    84,    87,    89,    92,    95,    98,   101,   104,   107,   110,   113,   116,   119,   122,   125
	};
