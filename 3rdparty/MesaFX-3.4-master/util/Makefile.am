## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = no-dependencies

INCLUDES = -I$(top_srcdir)/include $(GLUT_CFLAGS)

EXTRA_PROGRAMS = glstate

LDADD = $(GLUT_LIBS) $(top_builddir)/src-glu/libGLU.la \
	$(top_builddir)/src/libGL.la -lm

EXTRA_DIST = errcheck.c glstate.h glutskel.c idproj.c mwmborder.c \
	readtex.c showbuffer.c showbuffer.h winpos.c
