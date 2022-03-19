## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = no-dependencies

INCLUDES = -I$(top_srcdir)/include -I$(top_srcdir)/src $(FX_CFLAGS)

if HAVE_FX
# Build a libtool convenience library.
noinst_LTLIBRARIES = libMesaFX.la
if HAVE_X86
SUBDIRS = X86
endif
endif

libMesaFX_la_SOURCES = \
	fxapi.c \
	fxcva.c \
	fxcva.h \
	fxcvatmp.h \
	fxclip.c \
	fxcliptmp.h \
	fxdd.c \
	fxddspan.c \
	fxddtex.c \
	fxdrv.h \
	fxglidew.c \
        fxglidew.h \
	fxfastpath.c \
	fxfasttmp.h \
	fxpipeline.c \
	fxrender.c \
	fxrender.h \
	fxsanity.c \
	fxsetup.c \
	fxtexman.c \
	fxtrifuncs.c \
	fxtritmp.h \
	fxvs_tmp.h \
	fxvsetup.c \
	fxvsetup.h \
	fxwgl.c


libMesaFX_la_LIBADD = $(FX_LIBS)

EXTRA_DIST = fxwgl.c

