#ifndef RS232C_H
#define RS232C_H 1

/* $Id: rs232c.h,v 1.2 1999/01/03 02:06:22 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved  */



/************************************************************************/
/*									*/
/*	(C) Copyright 1989-96 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/

#include "lispemul.h" /* for LispPTR, DLword, DLbyte */

/* 
 RS232C Buffer Status
*/

#define INACTIVE                   0
#define ACTIVE                     1
#define TERMINATED                 2

/*
 DLRS232C.IOP.COMMANDS
*/

#define ON			   0
#define OFF 			   1
#define BREAK_ON 		   2
#define BREAK_OFF 		   3
#define ABORT_INPUT 		   4
#define ABORT_OUTPUT 		   5
#define SET_RS366_STATUS 	   6
#define GET_STATUS 		   7
#define MAJOR_SET_PARAMETERS 	   8
#define MINOR_SET_PARAMETERS      14
#define SET_CHANNEL_RESET_FLAG    15

/*
 Device Status
*/

#define IOP_DATA_LINE_OCCUPIED 	4096
#define PRESENT_NEXT_DIGIT 	2048
#define CALL_ORIGINATION_STATUS 1024
#define ABANDON_CALL_AND_RETRY 	 512
#define POWER_INDICATION 	 256
#define REAK_DETECTED 		 128
#define DATA_LOST 		  64
#define CLEAR_TO_SEND 		  32
#define NOT_DEFINED 		  16
#define CARRIER_DETECT 		   8
#define RING_HEARD 		   4
#define DATA_SET_READY 		   2
#define RING_INDICATOR 		   1

/*
 RS232C Data Structure
*/

typedef struct {
	DLword   frame_timeout;
	DLbyte   correspondent;
	DLbyte   synch_char;
	unsigned reset_ring_heard        :  1;
	unsigned reset_break_detected    :  1;
	unsigned reset_data_lost         :  1;
	unsigned request_to_send         :  1;
	unsigned data_terminal_ready     :  1;
	unsigned stop_bits               :  1;
	unsigned line_type               :  2; 
	unsigned parity                  :  3;
	unsigned char_length             :  2;
	unsigned synch_count             :  3;
	unsigned nil1                    :  3;
	unsigned line_speed              :  5;
	DLbyte   nil2; 
	DLword   interrupt_mask;
	DLword   flowcontrol_on;
	DLword   flowcontrol_xon_char;
	DLword   flowcontrol_xoff_char;
} DLRS232C_PARAMETER_CSB;

typedef struct {
	DLword   block_pointer_lo;
	DLword   block_pointer_hi;
	DLword   byte_count;
	DLword   returned_byte_count;
	DLword   transfer_status;
	DLword   nil1;
	unsigned completed               :  1;
	unsigned put                     :  1;
	unsigned nil2                    :  6;
	LispPTR  synch_event;
	LispPTR  next;
} DLRS232C_IOCB;

typedef struct {
	unsigned success 	         :  1;
	unsigned nil2 		         :  6;
	unsigned data_lost	         :  1; 
	unsigned device_error 	         :  1; 
 	unsigned frame_timeout           :  1; 
	unsigned checksum_error          :  1;
	unsigned parity_error            :  1;  
	unsigned asynch_frame_error      :  1;
	unsigned invalid_character       :  1;
	unsigned aborted 	         :  1;
	unsigned disaster  	         :  1;
} DLRS232C_IOCB_TRANSFER_STATUS;

typedef struct {
	unsigned rs232c_absent           :  1;
	unsigned nil	                 : 15;
} DLRS232C_HDW_CONF;

typedef struct {
	unsigned busy                    :  1;
	unsigned nil                     : 15;
} DLRS232C_IOP_GET_FLAG;

typedef struct {
	unsigned busy                    :  1;
	unsigned nil                     : 15;
} DLRS232C_IOP_PUT_FLAG;

typedef struct {
	unsigned busy	                 :  1;
	unsigned nil 	                 : 11;
	unsigned command                 :  4;
} DLRS232C_IOP_MISC_CMD;

typedef struct {
	unsigned success  	         :  1;
	unsigned nil 		         : 14;
	unsigned unimplemented 	         :  1;
} DLRS232C_PARAMETER_OUTCOME;

typedef struct {
	unsigned nil1			 :  3;
	unsigned data_line_occupied 	 :  1;
	unsigned present_next_digit	 :  1;
	unsigned call_origination_status :  1;
	unsigned abandon_call_and_retry  :  1;
	unsigned power_indication   	 :  1;
	unsigned break_detected 	 :  1;
	unsigned data_lost 		 :  1;
	unsigned clear_to_send 		 :  1;
	unsigned nil2 			 :  1;
	unsigned carrier_detect 	 :  1;
	unsigned ring_heard 		 :  1;
	unsigned data_set_ready 	 :  1;
	unsigned ring_indicator 	 :  1;
} DLRS232C_DEVICE_STATUS;

typedef struct {
	DLword nil[22];
	DLword rs232c_length;
	char   rs232c_data;
} RS232C_ENCAPSULATION;

extern DLword *Lisp_world;

#endif
