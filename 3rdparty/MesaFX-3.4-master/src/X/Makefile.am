## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = no-dependencies

INCLUDES = -I$(top_srcdir)/include -I$(top_srcdir)/src $(X_CFLAGS) $(FX_CFLAGS)

if HAVE_X11
# Build a libtool convenience library.
noinst_LTLIBRARIES = libMesaX11.la
endif

libMesaX11_la_SOURCES = \
	glxapi.c \
	glxapi.h \
	fakeglx.c \
	xfonts.c \
	xmesa1.c \
	xmesa2.c \
	xmesa3.c \
	xmesa4.c \
	xmesaP.h

libMesaX11_la_LIBADD = $(X_LIBS) $(X_PRE_LIBS) $(X_LIBADD)
