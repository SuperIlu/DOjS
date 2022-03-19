## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = 1.4 foreign

if HAVE_DEMOS
DEMO_DIRS = book demos samples xdemos

exec:
	@for subdir in $(DEMO_DIRS); do \
	  echo "Making exec in $$subdir"; \
	  (cd $$subdir && $(MAKE) $(AM_MAKEFLAGS) exec); \
	done;
else
exec:
endif

if NEED_GLUT
SUB_GLUT = src-glut
endif

SUBDIRS = include src src-glu $(SUB_GLUT) $(DEMO_DIRS) util

EXTRA_DIST = docs/INSTALL docs/INSTALL.GNU docs/CONFIG docs/IAFA-PACKAGE \
	docs/RELNOTES docs/VERSIONS docs/CONFORM  docs/COPYING \
	docs/COPYRIGHT docs/README docs/README.3DFX docs/README.BEOS \
	docs/README.GGI docs/README.MINGW32 docs/README.MITS \
	docs/README.QUAKE docs/README.THREADS docs/README.X11 \
	images/girl.rgb images/reflect.rgb images/tile.rgb

strip:
	@for subdir in src src-glu $(SUB_GLUT); do \
	  echo "Making strip in $$subdir"; \
	  (cd $$subdir && $(MAKE) $(AM_MAKEFLAGS) strip); \
	done;
