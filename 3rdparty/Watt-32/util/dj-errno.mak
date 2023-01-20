#
# Makefile for djgpp dj_err.exe and win32/dj_err.exe utilities.
#
# Targets:
#   dj_err.exe:
#     A plain DOS version built using good old djgpp under DOS.
#     Or the Windows hosted djgpp cross compiler
#     (see the github.com link below).
#
#   win32/dj_err.exe:
#     A MinGW compiled version that can run under Windows.
#

default all:
	@echo 'Usage: "make -f dj-errno.mak dj_err.exe"'
	@echo '    or "make -f dj-errno.mak win32/dj_err.exe" (requires a MinGW gcc)'

ifeq ($(OS),Windows_NT)
  ifneq ($(DJ_PREFIX),)
    #
    # Windows hosted djgpp cross compiler. Get it from:
    #   https://github.com/andrewwutw/build-djgpp/releases
    #
    # Define an env-var 'DJ_PREFIX=c:/develop/djgpp/bin/i586-pc-msdosdjgpp-'
    # Thus the full path to 'gcc' becomes:
    #   $(DJ_PREFIX)gcc.exe
    #
    # If not building on Windows, the '$(BIN_PREFIX)gcc' should simply
    # become 'gcc' and GNU-make should find that on %PATH.
    #
    BIN_PREFIX = $(DJ_PREFIX)

    ifeq ($(wildcard $(BIN_PREFIX)gcc.exe),)
      $(error Failed to find 'i586-pc-msdosdjgpp-gcc.exe'.)
    endif
  endif
endif

dj_err.exe: errnos.c
	$(BIN_PREFIX)gcc -s -I../inc -o dj_err.exe errnos.c

ifeq ($(BIN_PREFIX),)
win32/dj_err.exe:
	$(error Something wrong; cannot build $@ without 'i586-pc-msdosdjgpp-gcc.exe' and a MinGW gcc.)
else

#
# The folllowing 'win32/dj_err.exe' rule uses the <errno.h> from the
# above Windows hosted djgpp cross-compiler.
#
# But the gcc itself must be a windows hosted gcc (MinGW). With some
# added hacks to make it look like a djgpp-compiled version.
#
DJ_ROOT = $(subst /bin/i586-pc-msdosdjgpp-,,$(DJ_PREFIX))

#
# Make an env-var to suite your djgpp minor version.
# Otherwise the latest (?) minor version is assumed.
#
__DJGPP_MINOR__ ?= 6

DJ_ERR_CFLAGS = -m32 -s -DWATT32_DJGPP_MINGW  -D__DJGPP__=2 -D__DJGPP_MINOR__=$(__DJGPP_MINOR__)

#
# Force including djgpp's <errno.h> and NOT MinGW's <errno.h>
#
DJ_ERR_CFLAGS += --include $(DJ_ROOT)/i586-pc-msdosdjgpp/sys-include/errno.h -D_ERRNO_H_ -D_INC_ERRNO

win32/dj_err.exe: errnos.c errnos.mak
	gcc $(DJ_ERR_CFLAGS) -I../inc -o win32/dj_err.exe errnos.c

endif