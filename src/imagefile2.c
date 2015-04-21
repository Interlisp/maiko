/* $Id: imagefile2.c,v 1.2 1999/01/03 02:07:07 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved  */
static char *id = "$Id: imagefile2.c,v 1.2 1999/01/03 02:07:07 sybalsky Exp $ Copyright (C) Venue";



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


#include <stdio.h>
#include <pixrect/pixrect_hs.h>

#define FALSE 0
#define TRUE  !FALSE

typedef struct {
	FILE *file;
	struct rasterfile rh;
	colormap_t colormap;
} RASTERFILE_INFO;

unsigned int
open_rasterfile( filename )
char *filename;
{
	FILE *file;
	RASTERFILE_INFO *fileinfo;
	int status;

	if( (fileinfo = (RASTERFILE_INFO*)malloc( sizeof( RASTERFILE_INFO ) )) == 0 ) {
		return( 0 );
	}  /* end if( fileinfo ) */

	if( (fileinfo->file = fopen( filename, "r" )) != 0 ) {
		if( (status = pr_load_header( fileinfo->file
						, &(fileinfo->rh) )) == 0 ) {
			if( fileinfo->rh.ras_magic == RAS_MAGIC ) { 
							/* RasterFile */
				pr_load_colormap( fileinfo->file
						, &(fileinfo->rh)
						, &(fileinfo->colormap) );
					/* normal case */
				return( (unsigned int)fileinfo ); 
			} /* end if( fileinfo->rh.ras_magic ) */ 
			fclose( fileinfo->file );
		} /* end if( status ) */
	} /* end if( file ) */

	free( fileinfo );
	return( 0 ); /* error return */

} /* end open_rasterfile */

unsigned int
create_rasterfile( filename )
char *filename;
{
	RASTERFILE_INFO *fileinfo;

	if( !(fileinfo = (RASTERFILE_INFO*)malloc( sizeof( RASTERFILE_INFO ) )) ) {
		return( 0 );
	} /* end if( fileinfo ) */
	fileinfo->file = fopen( filename, "w" );

	fileinfo->rh.ras_magic = 0;

	return( (unsigned int)fileinfo );
	
} /* end create_rasterfile */

close_rasterfile( fileinfo )
RASTERFILE_INFO *fileinfo;
{
	fclose( fileinfo->file );
	free( (char*)fileinfo );
} /* end close_rasterfile */

unsigned int
read_rasterfile( fileinfo )
RASTERFILE_INFO *fileinfo;
{
	Pixrect *pict;
	pict = pr_load_std_image( fileinfo->file
				, &(fileinfo->rh)
				, &(fileinfo->colormap) );
	return( (unsigned int)pict );

} /* end read_rasterfile */

int 
write_rasterfile( fileinfo, pix  )
RASTERFILE_INFO *fileinfo;
Pixrect *pix;
{
	int status;

	if( fileinfo->rh.ras_magic == RAS_MAGIC ) {
		if( ( fileinfo->rh.ras_width  != pix->pr_size.x )
		 || ( fileinfo->rh.ras_height != pix->pr_size.y )
		 || ( fileinfo->rh.ras_depth  != pix->pr_depth  ) ) {
			return( FALSE );
		} /* end if( .. ) */
	} else {
		if( !(status = init_rasterfile_header( fileinfo, pix )) ) {
			return( FALSE );
		} /* end if( !status ) */
	} /* end if( fileinfo->rh.ras_magic ) */

	if( (status = pr_dump_image( pix, fileinfo->file, &(fileinfo->rh) )) == NULL ) {
		return( TRUE ); /* normal return */
	} 
	return( FALSE );
} /* end write_rasterfile */

int 
init_rasterfile_header( fileinfo, pix )
RASTERFILE_INFO *fileinfo;
Pixrect *pix;
{
	Pixrect *ret_pix;
	int status;

	switch( pix->pr_depth ) {
	case 32: 
		if( !(ret_pix = pr_dump_init( pix, &(fileinfo->rh)
						, NULL, 0 )) ) {
			return( FALSE );
		} /* end if( ret_pix ) */

		fileinfo->rh.ras_length = fileinfo->rh.ras_width * fileinfo->rh.ras_height * 4;
		fileinfo->rh.ras_type = RT_STANDARD;

		if( (status = pr_dump_header( fileinfo->file
					, &(fileinfo->rh), NULL )) ) {
			return( FALSE );
		} /* end if( status ) */
		break;
	case 1:
	case 8:
	default:
		return( FALSE );
		break;
	} /* end switch( pix->pr_depth ) */

	return( TRUE );
} /* end init_rasterfile_header */

int
position_rasterfile( fileinfo, n )
RASTERFILE_INFO *fileinfo;
int n;
{
	long position;
	int status;

	if( n > 0 ) {
		position = sizeof( struct rasterfile ) 
				+ (n - 1)*fileinfo->rh.ras_length;
		if( (status = fseek( fileinfo->file
					, position, (int)0 )) == 0 ) {
				/* normal return */
			return( TRUE );
		} /* end if( status ) */
	} /* end if( n ) */

	/* error return */
	return( FALSE );
	
} /* end position_rasterfile */
