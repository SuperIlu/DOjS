ACLOCAL_AMFLAGS = -I m4

lib_LTLIBRARIES = libsmacker.la
libsmacker_la_SOURCES = smacker.c \
			smk_bitstream.c \
			smk_hufftree.c
libsmacker_la_LDFLAGS = -version-info 1:1:1

noinst_PROGRAMS = driver smk2avi

driver_SOURCES = driver.c
driver_LDADD = $(lib_LTLIBRARIES)
driver_DEPENDENCIES = $(lib_LTLIBRARIES)

smk2avi_SOURCES = smk2avi.c
smk2avi_LDADD = $(lib_LTLIBRARIES)
smk2avi_DEPENDENCIES = $(lib_LTLIBRARIES)
