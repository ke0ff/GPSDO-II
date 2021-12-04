/*************************************************************************
 *********** COPYRIGHT (c) 2020 by Joseph Haas (DBA FF Systems)  *********
 *
 *  File name: bluetooth.h
 *
 *  Module:    Control
 *
 *  Summary:   This is the header file for bluetooth I/O.
 *
 *******************************************************************/


/********************************************************************
 *  File scope declarations revision history:
 *    08-16-20 jmh:  creation date
 *
 *******************************************************************/

//------------------------------------------------------------------------------
// extern defines
//------------------------------------------------------------------------------

				//       0         1           2            3        4       5           6            7        8
	enum resp_id{zero_rspn,conn_rspn,secure_rspn,secure2_rspn,rmt_rspn,ok_rspn,stream_rspn,stream2_rspn,err_rspn,

				//        9        a         b          c            d         e       f
				reboot_rspn,cmd_rspn,err2_rspn,retry_rspn,reboot2_rspn,disc_rspn,no_rspn};

// bluetooth state machine defines
#define	STATE_INIT		0xff
#define	STATE_QUERY		0xfe

#define	STATE_0			0
#define	STATE_1			1
#define	STATE_2			2
#define	STATE_3			3
#define	STATE_4			4
#define	STATE_5			5
#define	STATE_6			6
#define	STATE_7			7
#define	STATE_8			8
#define	STATE_9			9
#define	STATE_IDLE		10
#define	STATE_P1_ADDR	11
#define	STATE_P1_SET	12
#define	STATE_P1_DLY	13
#define	STATE_P1_CLR	14
#define	STATE_P1_EXIT	15
#define	STATE_P2_ADDR	16
#define	STATE_P2_SET	17
#define	STATE_P2_DLY	18
#define	STATE_P2_CHECK	19
#define	STATE_P2_CLR	20
#define	STATE_P2_EXIT	21
#define	PULSE_MASK		0x10
#define	PULSE2_MASK		0x20
#define	KEY_DLY			((U16)150)					// remote CH button press delay time (ms)
#define	BT_TIMEOUT		1500
#define	BT_TIMEOUTS		300
#define	BT_TIMEOUTC		4000

// IC901A Control Head Key Address defines
#define	V_M				0x10
#define	CALL			0x11
#define	BAND			0x12
#define	MODE			0x13
#define	MHZ				0x14
#define	H_L				0x15
#define	SQUP			0x16
#define	SQDN			0x17

#define	SUB				0x1e
#define	M_S				0x1d
#define	MW				0x1c
#define	SET				0x1b
#define	TS				0x1a
#define	TSQ				0x19
#define	VOLUP			0x18
#define	VOLDN			0x1f

#define	SMUTE			0x20
#define	LOCK			0x22
#define	CHECK			0x23

//------------------------------------------------------------------------------
// public Function Prototypes
//------------------------------------------------------------------------------
U8 process_BT(U8 cmd);
U8 respscan (char* string);

//------------------------------------------------------------------------------
// global defines
//------------------------------------------------------------------------------
