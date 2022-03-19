## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = no-dependencies

INCLUDES = -I$(top_srcdir)/include -I$(top_srcdir)/src

if HAVE_OSMESA
# Build a libtool convenience library.
noinst_LTLIBRARIES = libMesaOS.la
endif

libMesaOS_la_SOURCES = osmesa.c
