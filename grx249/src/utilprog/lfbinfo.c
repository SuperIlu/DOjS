/**
 ** lfbinfo.c ---- print linux framebuffer information
 **
 ** Copyright (c) 2001 Mariano Alvarez Fernandez
 ** [e-mail: malfer@teleline.es]
 **
 ** This file is part of the GRX graphics library.
 **
 ** The GRX graphics library is free software; you can redistribute it
 ** and/or modify it under some conditions; see the "copying.grx" file
 ** for details.
 **
 ** This library is distributed in the hope that it will be useful,
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 **
 **/

#include <stdio.h>
#include <stdlib.h>
#include <linux/fb.h>
#include <fcntl.h>
#include <sys/ioctl.h>

int main()
{
	struct fb_fix_screeninfo refix;
	struct fb_var_screeninfo resul;
	int fd;
	char *fbname;
	char *default_fbname = "/dev/fb0";
	
	fbname = getenv( "FRAMEBUFFER" );
	if( fbname == NULL ) fbname = default_fbname;
	fd = open( fbname,O_RDWR );
	if( fd == -1 ){
		printf( "can't open %s\n",fbname );
		return 1;
	}

	printf( "Device file: %s\n",fbname );

	printf( "Fixed data\n" );
	printf( "----------\n" );
	ioctl( fd,FBIOGET_FSCREENINFO,&refix );
	printf( "smem_start: %d\n",(int)refix.smem_start );
	printf( "smem_len: %d\n",(int)refix.smem_len );
	printf( "type: %d\n",(int)refix.type );
	printf( "type_aux: %d\n",(int)refix.type_aux );
	printf( "visual: %d\n",(int)refix.visual );
	printf( "xpanstep: %d  ypanstep: %d  ywrapstep: %d \n",
		refix.xpanstep,refix.ypanstep,refix.ywrapstep );
	printf( "line_length: %d\n",(int)refix.line_length );
	printf( "mmio_start: %d\n",(int)refix.mmio_start );
	printf( "mmio_len: %d\n",(int)refix.mmio_len );
	printf( "accel: %d\n",(int)refix.accel );
	ioctl( fd,FBIOGET_VSCREENINFO,&resul );
	printf( "Variable data\n" );
	printf( "-------------\n" );
	printf( "xres: %d\n",(int)resul.xres );
	printf( "yres: %d\n",(int)resul.yres );
	printf( "xres_virtual: %d\n",(int)resul.xres_virtual );
	printf( "yres_virtual: %d\n",(int)resul.yres_virtual );
	printf( "xoffset: %d\n",(int)resul.xoffset );
	printf( "yoffset: %d\n",(int)resul.yoffset );
	printf( "bpp: %d\n",(int)resul.bits_per_pixel );
	printf( "red offset: %d  length: %d  msb_right: %d\n",
		resul.red.offset,resul.red.length,resul.red.msb_right );
	printf( "green offset: %d  length: %d  msb_right: %d\n",
		resul.green.offset,resul.green.length,resul.green.msb_right );
	printf( "blue offset: %d  length: %d  msb_right: %d\n",
		resul.blue.offset,resul.blue.length,resul.blue.msb_right );
	printf( "transp offset: %d  length: %d  msb_right: %d\n",
		resul.transp.offset,resul.transp.length,resul.transp.msb_right );
	return 0;
}
