#
# Generate a pkg-config file watt-32.pc for MinGW-[32|64] or CygWin.
# Requires an up-to-date GNU make (4.x?), sed, grep, cp and cut.
#
THIS_FILE = ../util/pkg-conf.mak

MAKE_VERSION   = $(shell $(MAKE) --version)
WATT32_VERSION = $(shell grep WATTCP_VER_STRING ../inc/tcp.h | cut -d' ' -f4 | sed -e 's/\"//g')

#
# Since I got the error:
#  /usr/bin/sh: -c: line 0: unexpected EOF while looking for matching `''
#
# In the below 'WATT32_PACKAGE', I was forced to hard-code this version.
#
WATT32_VERSION_2 = 2.2.11
WATT32_ROOT      = $(realpath $(WATT_ROOT))

#
# Needs these env-vars to be set (required only if you build for these off-course):
#  %CYGWIN_ROOT  - CygWin install root-dir. E.g. "c:\cygwin".
#  %MINGW32      - Ditto for MinGW.         E.g. "c:\MinGW32".
#  %MINGW64      - Ditto for MinGW64-w64.   E.g. "c:\MinGW64".
#
# These pkgconfig directories are assumed to exist. Otherwise create them.
# If you don't use 'pkgconfig', it doesn't matter.
#
CYGWIN_DIR   = $(realpath $(CYGWIN_ROOT))/usr/lib/pkgconfig
CYGWIN_DIR   = /usr/lib/pkgconfig
CYGWIN64_DIR = /usr/lib/pkgconfig
MINGW32_DIR  = $(realpath $(MINGW32))/lib/pkgconfig
MINGW64_DIR  = $(realpath $(MINGW64))/lib/pkgconfig

#
# Note: this makefile ignores the $PKG_CONFIG_PATH variable and
# just assumes pkg-config is installed in 'lib/pkgconfig' for
# each of the above tool-sets. Change $(X_DIR) to suite.
#
all:
	@echo 'Check: $$(WATT32_VERSION): $(WATT32_VERSION), $$(WATT32_ROOT): $(WATT32_ROOT)'
	@echo 'Generate pkg-config file (watt-32.pc) for MinGW32, MinGW64-w64 or CygWin.'
	@echo 'Usage: $(MAKE) -f $(THIS_FILE) mingw32_pkg | mingw64_pkg | cygwin_pkg'

mingw32_pkg:
	@TARGET=$(MINGW32_DIR); WATT_LIB=libwatt32.dll.a; VENDOR="MinGW32"; \
	export TARGET WATT_LIB VENDOR; \
	$(MAKE) -f $(THIS_FILE) write_pkg copy_pkg_32

mingw64_pkg:
	@TARGET=$(MINGW64_DIR); WATT_LIB=libwatt32.dll.a; VENDOR="MinGW64-w64"; \
	export TARGET WATT_LIB VENDOR; \
	$(MAKE) -f $(THIS_FILE) write_pkg_32 copy_pkg_32 write_pkg_64 copy_pkg_64

cygwin_pkg:
	@TARGET=$(CYGWIN_DIR); WATT_LIB=libwatt32-cygwin.dll.a; VENDOR="CygWin/Win32"; \
	export TARGET WATT_LIB VENDOR; \
	$(MAKE) -f $(THIS_FILE) write_pkg copy_pkg_32

cygwin64_pkg:
	@TARGET=$(CYGWIN64_DIR); WATT_LIB=libwatt32-cygwin64.dll.a; VENDOR="CygWin64/Win64"; \
	export TARGET WATT_LIB VENDOR; \
	$(MAKE) -f $(THIS_FILE) write_pkg_64 copy_pkg_64

#
# Optional arg1:
#   $(1): either '/x86' or '/x64' for a MinGW64/CygWin32/CygWin64 .pc-file.
#
define WATT32_PACKAGE
  prefix=$(WATT32_ROOT)
  exec_prefix=$(WATT32_ROOT)
  libdir=$${exec_prefix}/lib$(1)
  includedir=$${exec_prefix}/inc
  watt_module_version=$(WATT32_VERSION_2)
  URL: http://www.watt-32.net
  Name: Watt-32
  Description: Watt-32 tcp/ip stack for $(VENDOR)
  Version: $(WATT32_VERSION_2)
  Cflags: -I$${includedir} -DWATT32
  Libs: $${libdir}/$(WATT_LIB)
endef

.POSIX: write_pkg
write_pkg: check_watt_root
	$(file > ./watt-32.pc,$(call WATT32_PACKAGE,))
	@echo

.POSIX: write_pkg_32
write_pkg_32: check_watt_root
	$(file > ./watt-32.pc,$(call WATT32_PACKAGE,/x86))
	@echo

write_pkg_64: check_watt_root
	$(file > ./watt-32_64.pc,$(call WATT32_PACKAGE,/x64))
	@echo

copy_pkg_32:
	cp --update ./watt-32.pc $(TARGET)

copy_pkg_64:
	cp --update ./watt-32_64.pc $(TARGET)

check_watt_root: FORCE
ifeq ($(WATT_ROOT),)
	$(error "%WATT_ROOT% not set. Read INSTALL step 1.")
endif

FORCE:

#
# Not used.
#
check_make:
	@echo 'Make version: $(MAKE_VERSION).'
ifeq ($(findstring pc-mingw32,$(MAKE_VERSION)),pc-mingw32)
	@echo 'MingW32.'
endif
ifeq ($(findstring pc-msys,$(MAKE_VERSION)),pc-msys)
	@echo 'MSys.'
endif
ifeq ($(findstring MSVC,$(MAKE_VERSION)),MSVC)
	@echo 'MSVC.'
endif




