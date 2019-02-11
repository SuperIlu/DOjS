/**
 ** dbgprint.c ---- GRX debug support
 **
 ** Copyright (c) 1998 Hartmut Schirmer
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

#ifdef  DEBUG
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "grxdebug.h"

#ifndef DBG_LOGFILE
#define DBG_LOGFILE "grxdebug.log"
#endif

char *_GR_debug_file;
int   _GR_debug_line;
#ifdef __GNUC__
char *_GR_debug_function;
# endif
int   _GR_debug_flags = DEBUG-0;

void _GR_debug_printf(char *fmt,...)
{
	FILE *dfp = NULL;
	va_list ap;
	dfp = fopen(DBG_LOGFILE,"at");
	if(!dfp) return;
#ifdef __GNUC__
	fprintf(dfp,"%s|%s|%d: ",
		_GR_debug_file, _GR_debug_function, _GR_debug_line);
#else
	fprintf(dfp,"%s/%d: ", _GR_debug_file, _GR_debug_line);
#endif
	va_start(ap,fmt);
	vfprintf(dfp,fmt,ap);
	va_end(ap);
	fclose(dfp);
}

#endif
