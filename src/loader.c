/* $Id: loader.c,v 1.2 1999/01/03 02:07:16 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved  */
static char *id = "$Id: loader.c,v 1.2 1999/01/03 02:07:16 sybalsky Exp $ Copyright (C) Venue";


/************************************************************************/
/*									*/
/*	(C) Copyright 1989-95 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/*	The contents of this file are proprietary information 		*/
/*	belonging to Venue, and are provided to you under license.	*/
/*	They may not be further distributed or disclosed to third	*/
/*	parties without the specific permission of Venue.		*/
/*									*/
/************************************************************************/

#include "version.h"


#include "sys/exec.h" /* choose one you like */
#include <stdio.h>
#include "lispemul.h"
#include "adr68k.h"
#include "lsptypes.h"
#include "lispmap.h"
#include "lspglob.h"
#include "arith.h"

char *sbrk();
unsigned getpagesize(), getpid();
char * malloc();

#define roundup( a, b) ((((unsigned)(a)+(b)-1)/(b))*(b))

/* A macro to convert a Lisp String to a C string */
#define	LispStringToCString(Lisp, C){	\
	LispPTR	*naddress;				\
	char	*base;					\
	int	length;					\
	int	offset;					\
	naddress = (LispPTR *)(Addr68k_from_LADDR(Lisp));					\
	base = (char *)(Addr68k_from_LADDR(((OneDArray *)naddress)->base));	\
	offset = (int)(((OneDArray *)naddress)->offset);	\
	length = ((OneDArray *)naddress)->totalsize;						\
	strncpy(C, base + offset, length);			\
	C[length] = '\0';				\
	}


int dynamic_load_code(args) 
	LispPTR *args;
/*	args[0]:	LispPTR to file name.
	args[1]:	native addr of where to load the code

	returns: List(load_addr, entry_point, length)
*/
{
char	file_name[512];
int	load_address;
int	entry_point;

	LispStringToCString(args[0], file_name);
	N_GETNUMBER(args[1], load_address, return_error);
	return(load_native_object(load_address, file_name));
return_error:
	printf("error in dynamic load: 0x%x\n",args[1]);
	return(NIL_PTR);
}

load_native_object(load_address, dynamic_file)
	unsigned load_address;
	char *dynamic_file;
{
	int load_length, entry_point;
	typedef char *charstrptr;
	typedef charstrptr charstrlist[2];
	charstrlist p;
	int fd, asize, i;
	struct exec hdr;

	fd = open( dynamic_file, 0 );
	if (fd == -1) return(0);
	if (read( fd, &hdr, sizeof hdr ) == -1) return(0);
	
	load_length = hdr.a_text+hdr.a_data;

#ifdef  DEBUG
	printf("Load Size: %x, Header Rec Size %x\n",load_length,sizeof hdr);
	printf("Data Size: %x, Text Size %x\n",hdr.a_data, hdr.a_text);
#endif

	if (read( fd, load_address, load_length ) == -1) return(0);
	entry_point = (int) ( (CFuncPTR)asmcall(load_address) );
	{register int r0, r1, r2;
	 ARITH_SWITCH((int) load_address, r0);
	 ARITH_SWITCH((int) entry_point, r1);
	 ARITH_SWITCH(load_length, r2);
	 return(N_OP_cons(r0, N_OP_cons(r1, N_OP_cons(r2, NIL_PTR))));
	}
}

/* ****  system call subr, living in sb_OLD_COMPILE_LOAD_NATIVE slot *** */

int do_system_call(arg)
	LispPTR arg;
{
char	cmd_str[512];
register int result;
register int lisp_result;

	LispStringToCString(arg, cmd_str);
#ifdef  DEBUG
	printf(":: %s \n",cmd_str);
#endif
	int_timer_off();
	result = system(cmd_str);
	int_timer_on();
	ARITH_SWITCH(result, lisp_result);
	return(lisp_result);
}


/* ****** stuff below is old & should be removed when the subr is *** */

int dynamic_load (host_file, dynamic_file, load_address, needs_compile, needs_link, do_load)
	char *host_file, *dynamic_file;
	unsigned load_address, needs_compile, needs_link, do_load;

{
	int result = 0;
	char fnamec[80], fnameo[80], fnameil[80], cc_str[255], ld_str[255];
	unsigned pagsiz = getpagesize();
	
	host_file = "lisp";	/* TEMPORARY ********** */

	printf("loading: %s into %s \n",dynamic_file,host_file);
	if (load_address == 0) load_address = roundup(sbrk(0),pagsiz);
	sprintf(fnamec, "%s.c", dynamic_file);
	sprintf(fnameo, "%s.o", dynamic_file);
	sprintf(fnameil, "%s.il", dynamic_file);
	sprintf(cc_str,"/bin/cc  -pipe -c %s -o %s  %s -O -I/users/krivacic/maiko/inc /users/krivacic/maiko/src/disp68K.il",fnamec,fnameo,fnameil);
	sprintf(ld_str,"/bin/ld -N -s -Ttext %x -A /users/krivacic/maiko/bin/%s -o %s %s -lc",
			load_address,host_file,dynamic_file,fnameo);


	if (needs_compile) 
		{result = exec_command(cc_str);
		 if (result) return(0);
		}
	if (needs_link) 
		{result = exec_command(ld_str);
		 if (result) return(0);
		}
	if (do_load) {char rm_str[200];
		result = load_object(load_address, dynamic_file);	
		sprintf(rm_str,"/bin/rm %s %s %s %s",fnamec, fnameil, fnameo, dynamic_file);
/* ***		exec_command(rm_str); *** */
		}
	TopOfStack = result;
	return(result);

}  /*dynamic_load */

exec_command(cmd_str)
	char *cmd_str;
{	typedef char *charstrptr;
	typedef charstrptr charstrlist[21];
	charstrlist p;
	char ws[21][80];
	int i, ii, cmd_length;

	int_timer_off();
	i = system(cmd_str);
	int_timer_on();
	return(i);

} /*exec_command */

load_object(load_address, dynamic_file)
	unsigned load_address;
	char *dynamic_file;
{
	int load_length, entry_point;
	typedef char *charstrptr;
	typedef charstrptr charstrlist[2];
	charstrlist p;
	int fd, asize, i;
	struct exec hdr;
	char *addr2;
	unsigned pagsiz = getpagesize();

	fd = open( dynamic_file, 0 );
	if (fd == -1) return(0);
	if (read( fd, &hdr, sizeof hdr ) == -1) return(0);
	
	asize = roundup(hdr.a_text+hdr.a_data+hdr.a_bss, pagsiz)+pagsiz;
#ifdef  DEBUG
	printf("Load Size: %x, Header Rec Size %x\n",asize,sizeof hdr);
	printf("Data Size: %x, Text Size %x\n",hdr.a_data, hdr.a_text);
#endif
	addr2 = sbrk(asize);
	addr2 = (char *)roundup(addr2,pagsiz);

	if ( load_address != (unsigned)addr2 ){ return(NIL_PTR); }

	load_length = hdr.a_text+hdr.a_data;
	read( fd, addr2, load_length );
	entry_point = (int) ( (CFuncPTR)asmcall(addr2) );
	{register int r0, r1, r2;
#ifdef  DEBUG
	 printf("Values 0x%x & 0x%x\n",entry_point,load_length);
#endif
	 ARITH_SWITCH((int) addr2, r0);
	 ARITH_SWITCH((int) entry_point, r1);
	 ARITH_SWITCH(load_length, r2);
#ifdef  DEBUG
	 printf("Returning 0x%x & 0x%x\n",r1,r2);
#endif
	 return(N_OP_cons(r0, N_OP_cons(r1, N_OP_cons(r2))));
	}
}
