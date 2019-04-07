AUTOMAKE_OPTIONS = foreign

if BUILD_DOCS
   MAYBE_DOCS = docs
endif
SUBDIRS = drivers include loaders mmio depackers playercode \
	  posix dlapi $(MAYBE_DOCS)

bin_SCRIPTS = libmikmod-config

EXTRA_DIST = libmikmod-config.in libmikmod.m4 libmikmod.pc.in \
	     libmikmod.spec.in libmikmod.spec COPYING.LESSER \
	     amiga android macosx win32 \
	     dos dc gp32 macintosh os2 psp \
	     examples \
	     config.h.cmake CMakeLists.txt cmake

m4datadir = $(datadir)/aclocal
m4data_DATA = libmikmod.m4

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libmikmod.pc

ACLOCAL_AMFLAGS = -I m4

# always define MIKMOD_BUILD when building libmikmod
# regardless of static build or shared library build..
AM_CFLAGS = -DMIKMOD_BUILD=1

AM_CPPFLAGS = -I@top_srcdir@/include

lib_LTLIBRARIES = libmikmod.la

libmikmod_la_SOURCES = \
		dlapi/dl_hpux.c   \
		drivers/drv_AF.c \
		drivers/drv_aiff.c \
		drivers/drv_ahi.c \
		drivers/drv_aix.c \
		drivers/drv_alsa.c \
		drivers/drv_dart.c \
		drivers/drv_dc.c \
		drivers/drv_ds.c \
		drivers/drv_esd.c \
		drivers/drv_gp32.c \
		drivers/drv_hp.c \
		drivers/drv_mac.c \
		drivers/drv_nas.c \
		drivers/drv_nos.c \
		drivers/drv_openal.c \
		drivers/drv_os2.c \
		drivers/drv_osles.c \
		drivers/drv_oss.c \
		drivers/drv_osx.c \
		drivers/drv_pulseaudio.c \
		drivers/drv_pipe.c \
		drivers/drv_psp.c \
		drivers/drv_raw.c \
		drivers/drv_sam9407.c \
		drivers/drv_sb.c \
		drivers/drv_sdl.c \
		drivers/drv_sgi.c \
		drivers/drv_sndio.c \
		drivers/drv_stdout.c \
		drivers/drv_sun.c \
		drivers/drv_ultra.c \
		drivers/drv_wav.c \
		drivers/drv_win.c \
		drivers/drv_wss.c \
		drivers/drv_xaudio2.c \
		loaders/load_669.c \
		loaders/load_amf.c \
		loaders/load_asy.c \
		loaders/load_dsm.c \
		loaders/load_far.c \
		loaders/load_gdm.c \
		loaders/load_gt2.c \
		loaders/load_imf.c \
		loaders/load_it.c  \
		loaders/load_m15.c \
		loaders/load_med.c \
		loaders/load_mod.c \
		loaders/load_mtm.c \
		loaders/load_okt.c \
		loaders/load_s3m.c \
		loaders/load_stm.c \
		loaders/load_stx.c \
		loaders/load_ult.c \
		loaders/load_umx.c \
		loaders/load_uni.c \
		loaders/load_xm.c \
		mmio/mmalloc.c \
		mmio/mmerror.c \
		mmio/mmio.c \
		depackers/mmcmp.c \
		depackers/pp20.c \
		depackers/s404.c \
		depackers/xpk.c \
		posix/strcasecmp.c \
		playercode/mdreg.c \
		playercode/mdriver.c \
		playercode/mdulaw.c \
		playercode/mloader.c \
		playercode/mlreg.c \
		playercode/mlutil.c \
		playercode/mplayer.c \
		playercode/munitrk.c \
		playercode/mwav.c \
		playercode/npertab.c \
		playercode/sloader.c \
		playercode/virtch.c \
		playercode/virtch2.c \
		playercode/virtch_common.c

libmikmod_la_LDFLAGS = -no-undefined -version-info @MIKMOD_LT_CURRENT@:@MIKMOD_LT_REVISION@:@MIKMOD_LT_AGE@
libmikmod_la_LIBADD = @LTLIBOBJS@ @LIBRARY_LIB@

