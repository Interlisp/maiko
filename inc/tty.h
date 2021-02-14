#ifndef TTY_H
#define TTY_H 1
/* $Id: tty.h,v 1.2 1999/01/03 02:06:29 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved  */

/************************************************************************/
/*									*/
/*	(C) Copyright 1989-96 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/
#include "lispemul.h" /* for DLword */

/*
 TTY Command
*/

#define TTY_GET_STATUS     33280>>8
#define TTY_ON             33536>>8
#define TTY_OFF            33792>>8
#define TTY_BREAK_ON       34304>>8
#define TTY_BREAK_OFF      34560>>8
#define PUT_CHAR             128
#define ABORT_PUT            133
#define SET_PARAM            129
#define SET_DSR            33025
#define SET_CTS            33026
#define SET_CHAR_LENGTH    33028
#define SET_PARITY         33032
#define SET_STOP_BITS      33040
#define SET_BAUD_RATE      33056
#define SET_ALL_PARAMETERS 33087

typedef struct {
	unsigned command             : 8;
	unsigned outdata             : 8;
} DLTTY_OUT_COMMAND;

typedef struct {
	unsigned on_off              : 4;
	unsigned line_speed          : 4;
	unsigned stop_bits           : 2;
	unsigned parity              : 2;
	unsigned char_length         : 2;
	unsigned clear_to_send       : 1;
	unsigned data_set_ready      : 1;
	DLword  notify_mask;
} DLTTY_OUT_CSB;

typedef struct {
	unsigned state               : 1;
	unsigned nil1                : 7;
	unsigned success             : 1;
	unsigned break_detected      : 1;
	unsigned framing_error       : 1;
	unsigned data_lost           : 1;
	unsigned parity_error        : 1;
	unsigned nil2                : 2;
	unsigned not_ready           : 1;
	char     in_data;
	unsigned data_terminal_ready : 1;
	unsigned nil3                : 4;
	unsigned request_to_send     : 1;
	unsigned rx_ready            : 1;
	unsigned tx_ready            : 1;
} DLTTY_IN_CSB;

#endif /* TTY_H */
