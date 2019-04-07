/*	MikMod sound library
	(c) 1998, 1999, 2000 Miodrag Vallat and others - see file AUTHORS for
	complete list.

	This library is free software; you can redistribute it and/or modify
	it under the terms of the GNU Library General Public License as
	published by the Free Software Foundation; either version 2 of
	the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU Library General Public License for more details.

	You should have received a copy of the GNU Library General Public
	License along with this library; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
	02111-1307, USA.
*/

/*==============================================================================

  $Id$

  Portable file I/O routines

==============================================================================*/

/*

	The way this module works:

	- _mm_fopen will call the errorhandler [see mmerror.c] in addition to
	  setting _mm_errno on exit.
	- _mm_iobase is for internal use.  It is used by Player_LoadFP to
	  ensure that it works properly with wad files.
	- _mm_read_I_* and _mm_read_M_* differ : the first is for reading data
	  written by a little endian (intel) machine, and the second is for reading
	  big endian (Mac, RISC, Alpha) machine data.
	- _mm_write functions work the same as the _mm_read functions.
	- _mm_read_string is for reading binary strings.  It is basically the same
	  as an fread of bytes.

*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef HAVE_LIMITS_H
#include <limits.h>
#endif

#include <stdio.h>
#include <string.h>

#include "mikmod_internals.h"

#ifdef SUNOS
extern int fclose(FILE *);
extern int fgetc(FILE *);
extern int fputc(int, FILE *);
extern size_t fread(void *, size_t, size_t, FILE *);
extern int fseek(FILE *, long, int);
extern size_t fwrite(const void *, size_t, size_t, FILE *);
#endif

/* some prototypes */
static BOOL _mm_MemReader_Eof(MREADER* reader);
static BOOL _mm_MemReader_Read(MREADER* reader,void* ptr,size_t size);
static int _mm_MemReader_Get(MREADER* reader);
static int _mm_MemReader_Seek(MREADER* reader,long offset,int whence);
static long _mm_MemReader_Tell(MREADER* reader);

FILE* _mm_fopen(const CHAR* fname, const CHAR* attrib)
{
	FILE *fp;

	if(!(fp=fopen(fname,attrib))) {
		_mm_errno = MMERR_OPENING_FILE;
		if(_mm_errorhandler) _mm_errorhandler();
	}
	return fp;
}

BOOL _mm_FileExists(const CHAR* fname)
{
	FILE *fp;

	if(!(fp=fopen(fname,"r"))) return 0;
	fclose(fp);

	return 1;
}

int _mm_fclose(FILE *fp)
{
	return fclose(fp);
}

/* Sets the current file-position as the new iobase */
void _mm_iobase_setcur(MREADER* reader)
{
	reader->prev_iobase=reader->iobase;  /* store old value in case of revert */
	reader->iobase=reader->Tell(reader);
}

/* Reverts to the last known iobase value. */
void _mm_iobase_revert(MREADER* reader)
{
	reader->iobase=reader->prev_iobase;
}

/*========== File Reader */

typedef struct MFILEREADER {
	MREADER core;
	FILE*   file;
} MFILEREADER;

static BOOL _mm_FileReader_Eof(MREADER* reader)
{
	return feof(((MFILEREADER*)reader)->file);
}

static BOOL _mm_FileReader_Read(MREADER* reader,void* ptr,size_t size)
{
	return !!fread(ptr,size,1,((MFILEREADER*)reader)->file);
}

static int _mm_FileReader_Get(MREADER* reader)
{
	return fgetc(((MFILEREADER*)reader)->file);
}

static int _mm_FileReader_Seek(MREADER* reader,long offset,int whence)
{
	return fseek(((MFILEREADER*)reader)->file,
				 (whence==SEEK_SET)?offset+reader->iobase:offset,whence);
}

static long _mm_FileReader_Tell(MREADER* reader)
{
	return ftell(((MFILEREADER*)reader)->file)-reader->iobase;
}

MREADER *_mm_new_file_reader(FILE* fp)
{
	MFILEREADER* reader=(MFILEREADER*)MikMod_calloc(1,sizeof(MFILEREADER));
	if (reader) {
		reader->core.Eof =&_mm_FileReader_Eof;
		reader->core.Read=&_mm_FileReader_Read;
		reader->core.Get =&_mm_FileReader_Get;
		reader->core.Seek=&_mm_FileReader_Seek;
		reader->core.Tell=&_mm_FileReader_Tell;
		reader->file=fp;
	}
	return (MREADER*)reader;
}

void _mm_delete_file_reader (MREADER* reader)
{
	MikMod_free(reader);
}

/*========== File Writer */

typedef struct MFILEWRITER {
	MWRITER core;
	FILE*   file;
} MFILEWRITER;

static int _mm_FileWriter_Seek(MWRITER* writer,long offset,int whence)
{
	return fseek(((MFILEWRITER*)writer)->file,offset,whence);
}

static long _mm_FileWriter_Tell(MWRITER* writer)
{
	return ftell(((MFILEWRITER*)writer)->file);
}

static BOOL _mm_FileWriter_Write(MWRITER* writer, const void* ptr, size_t size)
{
	return (fwrite(ptr,size,1,((MFILEWRITER*)writer)->file)==size);
}

static int _mm_FileWriter_Put(MWRITER* writer,int value)
{
	return fputc(value,((MFILEWRITER*)writer)->file);
}

MWRITER *_mm_new_file_writer(FILE* fp)
{
	MFILEWRITER* writer=(MFILEWRITER*)MikMod_calloc(1,sizeof(MFILEWRITER));
	if (writer) {
		writer->core.Seek =&_mm_FileWriter_Seek;
		writer->core.Tell =&_mm_FileWriter_Tell;
		writer->core.Write=&_mm_FileWriter_Write;
		writer->core.Put  =&_mm_FileWriter_Put;
		writer->file=fp;
	}
	return (MWRITER*) writer;
}

void _mm_delete_file_writer (MWRITER* writer)
{
	MikMod_free (writer);
}

/*========== Memory Reader */

typedef struct MMEMREADER {
	MREADER core;
	const void *buffer;
	long len;
	long pos;
} MMEMREADER;

void _mm_delete_mem_reader(MREADER* reader)
{
	MikMod_free(reader);
}

MREADER *_mm_new_mem_reader(const void *buffer, long len)
{
	MMEMREADER* reader=(MMEMREADER*)MikMod_calloc(1,sizeof(MMEMREADER));
	if (reader)
	{
		reader->core.Eof =&_mm_MemReader_Eof;
		reader->core.Read=&_mm_MemReader_Read;
		reader->core.Get =&_mm_MemReader_Get;
		reader->core.Seek=&_mm_MemReader_Seek;
		reader->core.Tell=&_mm_MemReader_Tell;
		reader->buffer = buffer;
		reader->len = len;
		reader->pos = 0;
	}
	return (MREADER*)reader;
}

static BOOL _mm_MemReader_Eof(MREADER* reader)
{
	MMEMREADER* mr = (MMEMREADER*) reader;
	if (!mr) return 1;
	if (mr->pos >= mr->len) return 1;
	return 0;
}

static BOOL _mm_MemReader_Read(MREADER* reader,void* ptr,size_t size)
{
	unsigned char *d;
	const unsigned char *s;
	MMEMREADER* mr;
	long siz;
	BOOL ret;

	if (!reader || !size || (size > (size_t) LONG_MAX))
		return 0;

	mr = (MMEMREADER*) reader;
	siz = (long) size;
	if (mr->pos >= mr->len) return 0;	/* @ eof */
	if (mr->pos + siz > mr->len) {
		siz = mr->len - mr->pos;
		ret = 0; /* not enough remaining bytes */
	}
	else {
		ret = 1;
	}

	s = (const unsigned char *) mr->buffer;
	s += mr->pos;
	mr->pos += siz;
	d = (unsigned char *) ptr;

	while (siz) {
		*d++ = *s++;
		siz--;
	}

	return ret;
}

static int _mm_MemReader_Get(MREADER* reader)
{
	MMEMREADER* mr;
	int c;

	mr = (MMEMREADER*) reader;
	if (mr->pos >= mr->len) return EOF;
	c = ((const unsigned char*) mr->buffer)[mr->pos];
	mr->pos++;

	return c;
}

static int _mm_MemReader_Seek(MREADER* reader,long offset,int whence)
{
	MMEMREADER* mr;

	if (!reader) return -1;
	mr = (MMEMREADER*) reader;
	switch(whence)
	{
	case SEEK_CUR:
		mr->pos += offset;
		break;
	case SEEK_SET:
		mr->pos = reader->iobase + offset;
		break;
	case SEEK_END:
		mr->pos = mr->len + offset;
		break;
	default: /* invalid */
		return -1;
	}
	if (mr->pos < reader->iobase) {
		mr->pos = mr->core.iobase;
		return -1;
	}
	if (mr->pos > mr->len) {
		mr->pos = mr->len;
	}
	return 0;
}

static long _mm_MemReader_Tell(MREADER* reader)
{
	if (reader) {
		return ((MMEMREADER*)reader)->pos - reader->iobase;
	}
	return 0;
}

/*========== Write functions */

void _mm_write_string(const CHAR* data,MWRITER* writer)
{
	if(data)
		_mm_write_UBYTES(data,strlen(data),writer);
}

void _mm_write_M_UWORD(UWORD data,MWRITER* writer)
{
	_mm_write_UBYTE(data>>8,writer);
	_mm_write_UBYTE(data&0xff,writer);
}

void _mm_write_I_UWORD(UWORD data,MWRITER* writer)
{
	_mm_write_UBYTE(data&0xff,writer);
	_mm_write_UBYTE(data>>8,writer);
}

void _mm_write_M_ULONG(ULONG data,MWRITER* writer)
{
	_mm_write_M_UWORD(data>>16,writer);
	_mm_write_M_UWORD(data&0xffff,writer);
}

void _mm_write_I_ULONG(ULONG data,MWRITER* writer)
{
	_mm_write_I_UWORD(data&0xffff,writer);
	_mm_write_I_UWORD(data>>16,writer);
}

void _mm_write_M_SWORD(SWORD data,MWRITER* writer)
{
	_mm_write_M_UWORD((UWORD)data,writer);
}

void _mm_write_I_SWORD(SWORD data,MWRITER* writer)
{
	_mm_write_I_UWORD((UWORD)data,writer);
}

void _mm_write_M_SLONG(SLONG data,MWRITER* writer)
{
	_mm_write_M_ULONG((ULONG)data,writer);
}

void _mm_write_I_SLONG(SLONG data,MWRITER* writer)
{
	_mm_write_I_ULONG((ULONG)data,writer);
}

void _mm_write_M_SWORDS(SWORD *buffer,int cnt,MWRITER* writer)
{
	while(cnt-- > 0) _mm_write_M_SWORD(*(buffer++),writer);
}

void _mm_write_M_UWORDS(UWORD *buffer,int cnt,MWRITER* writer)
{
	while(cnt-- > 0) _mm_write_M_UWORD(*(buffer++),writer);
}

void _mm_write_I_SWORDS(SWORD *buffer,int cnt,MWRITER* writer)
{
	while(cnt-- > 0) _mm_write_I_SWORD(*(buffer++),writer);
}

void _mm_write_I_UWORDS(UWORD *buffer,int cnt,MWRITER* writer)
{
	while(cnt-- > 0) _mm_write_I_UWORD(*(buffer++),writer);
}

void _mm_write_M_SLONGS(SLONG *buffer,int cnt,MWRITER* writer)
{
	while(cnt-- > 0) _mm_write_M_SLONG(*(buffer++),writer);
}

void _mm_write_M_ULONGS(ULONG *buffer,int cnt,MWRITER* writer)
{
	while(cnt-- > 0) _mm_write_M_ULONG(*(buffer++),writer);
}

void _mm_write_I_SLONGS(SLONG *buffer,int cnt,MWRITER* writer)
{
	while(cnt-- > 0) _mm_write_I_SLONG(*(buffer++),writer);
}

void _mm_write_I_ULONGS(ULONG *buffer,int cnt,MWRITER* writer)
{
	while(cnt-- > 0) _mm_write_I_ULONG(*(buffer++),writer);
}

/*========== Read functions */

BOOL _mm_read_string(CHAR* buffer,int cnt,MREADER* reader)
{
	return reader->Read(reader,buffer,cnt);
}

UWORD _mm_read_M_UWORD(MREADER* reader)
{
	UWORD result=((UWORD)_mm_read_UBYTE(reader))<<8;
	result|=_mm_read_UBYTE(reader);
	return result;
}

UWORD _mm_read_I_UWORD(MREADER* reader)
{
	UWORD result=_mm_read_UBYTE(reader);
	result|=((UWORD)_mm_read_UBYTE(reader))<<8;
	return result;
}

ULONG _mm_read_M_ULONG(MREADER* reader)
{
	ULONG result=((ULONG)_mm_read_M_UWORD(reader))<<16;
	result|=_mm_read_M_UWORD(reader);
	return result;
}

ULONG _mm_read_I_ULONG(MREADER* reader)
{
	ULONG result=_mm_read_I_UWORD(reader);
	result|=((ULONG)_mm_read_I_UWORD(reader))<<16;
	return result;
}

SWORD _mm_read_M_SWORD(MREADER* reader)
{
	return((SWORD)_mm_read_M_UWORD(reader));
}

SWORD _mm_read_I_SWORD(MREADER* reader)
{
	return((SWORD)_mm_read_I_UWORD(reader));
}

SLONG _mm_read_M_SLONG(MREADER* reader)
{
	return((SLONG)_mm_read_M_ULONG(reader));
}

SLONG _mm_read_I_SLONG(MREADER* reader)
{
	return((SLONG)_mm_read_I_ULONG(reader));
}

BOOL _mm_read_M_SWORDS(SWORD *buffer,int cnt,MREADER* reader)
{
	while(cnt-- > 0) *(buffer++)=_mm_read_M_SWORD(reader);
	return !reader->Eof(reader);
}

BOOL _mm_read_M_UWORDS(UWORD *buffer,int cnt,MREADER* reader)
{
	while(cnt-- > 0) *(buffer++)=_mm_read_M_UWORD(reader);
	return !reader->Eof(reader);
}

BOOL _mm_read_I_SWORDS(SWORD *buffer,int cnt,MREADER* reader)
{
	while(cnt-- > 0) *(buffer++)=_mm_read_I_SWORD(reader);
	return !reader->Eof(reader);
}

BOOL _mm_read_I_UWORDS(UWORD *buffer,int cnt,MREADER* reader)
{
	while(cnt-- > 0) *(buffer++)=_mm_read_I_UWORD(reader);
	return !reader->Eof(reader);
}

BOOL _mm_read_M_SLONGS(SLONG *buffer,int cnt,MREADER* reader)
{
	while(cnt-- > 0) *(buffer++)=_mm_read_M_SLONG(reader);
	return !reader->Eof(reader);
}

BOOL _mm_read_M_ULONGS(ULONG *buffer,int cnt,MREADER* reader)
{
	while(cnt-- > 0) *(buffer++)=_mm_read_M_ULONG(reader);
	return !reader->Eof(reader);
}

BOOL _mm_read_I_SLONGS(SLONG *buffer,int cnt,MREADER* reader)
{
	while(cnt-- > 0) *(buffer++)=_mm_read_I_SLONG(reader);
	return !reader->Eof(reader);
}

BOOL _mm_read_I_ULONGS(ULONG *buffer,int cnt,MREADER* reader)
{
	while(cnt-- > 0) *(buffer++)=_mm_read_I_ULONG(reader);
	return !reader->Eof(reader);
}

/* ex:set ts=4: */
