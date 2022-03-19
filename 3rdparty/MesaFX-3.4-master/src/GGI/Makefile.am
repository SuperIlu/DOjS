## Process this file with automake to produce Makefile.in

SUBDIRS = default display include

INCLUDES = -I$(top_srcdir)/include -I$(top_srcdir)/src/GGI/include \
	-I$(top_srcdir)/src $(GGI_CFLAGS)
	
# Build a libtool convenience library.
noinst_LTLIBRARIES = libMesaGGI.la

libMesaGGI_la_SOURCES = ggimesa.c
libMesaGGI_la_LIBADD = $(GGI_LIBS)

ggimesaconfdatadir = ${sysconfdir}/ggi
ggimesaconfdata_DATA = ggimesa.conf

DISTCLEANFILES = ggimesa.conf
