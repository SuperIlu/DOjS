###
# Makefile for cross compiling DOjS for FreeDOS/MS-DOS
# All compilation was done with DJGPP 12.2.0 built from https://github.com/jwt27/build-gcc
# make sure the DJGPP toolchain is in your path (i586-pc-msdosdjgpp-XXX)!
###

# temp directory for building FreeDOS archive
TMP=/tmp/FDOS

THIRDPARTY	= 3rdparty
MUJS		= $(THIRDPARTY)/mujs-1.0.5
ALLEGRO		= $(THIRDPARTY)/allegro-4.2.2-xc-master
DZCOMMDIR	= $(THIRDPARTY)/dzcomm
WATT32		= $(THIRDPARTY)/Watt-32
ZLIB		= $(THIRDPARTY)/zlib-1.3.1
KUBAZIP		= $(THIRDPARTY)/zip-0.3.2
ALPNG		= $(THIRDPARTY)/alpng13
CURL		= $(THIRDPARTY)/curl-8.11.0
MESA3		= $(THIRDPARTY)/MesaFX-3.4-master
BZIP2		= $(THIRDPARTY)/bzip2-1.0.8
INI			= $(THIRDPARTY)/ini-20220806/src
MBEDTLS		= $(THIRDPARTY)/mbedtls-3.6.2
WEBP		= $(THIRDPARTY)/libwebp-1.3.2
TIFF		= $(THIRDPARTY)/tiff-4.6.0
JASPER_SRC	= $(THIRDPARTY)/jasper-version-4.2.0
JASPER_BIN	= $(THIRDPARTY)/jasper-djgpp
JPEG		= $(THIRDPARTY)/jpeg-9f

GLIDE		= glide3x
GLIDESDK	= $(GLIDE)/v1
TEXUS		= $(GLIDE)/texus

JSDOC_TEMPLATES ?= $(shell npm root)/better-docs $(shell npm root -g)/better-docs

LIB_DZCOMM	= $(DZCOMMDIR)/lib/djgpp/libdzcom.a
LIB_MUJS	= $(MUJS)/build/release/libmujs.a
LIB_ALLEGRO	= $(ALLEGRO)/lib/djgpp/liballeg.a
LIB_WATT	= $(WATT32)/lib/libwatt.a
LIB_Z		= $(ZLIB)/msdos/libz.a
LIB_ALPNG	= $(ALPNG)/libalpng.a
LIB_CURL	= $(CURL)/lib/libcurl.a
LIB_MESA	= $(MESA3)/lib/libgl.a
LIB_BZIP2	= $(BZIP2)/libbzip2.a
LIB_MBEDTLS = $(MBEDTLS)/library/libmbedtls.a
LIB_WEBP 	= $(WEBP)/src/libwebp.a
LIB_TIFF 	= $(TIFF)/libtiff/.libs/libtiff.a
LIB_JASPER 	= $(JASPER_BIN)/src/libjasper/libjasper.a
LIB_JPEG 	= $(JPEG)/libjpeg.a

# compiler
CDEF     = -DGC_BEFORE_MALLOC -DLFB_3DFX -DEDI_FAST #-DDEBUG_ENABLED # -DMEMDEBUG 
CFLAGS   = -MMD -Wall -std=gnu99 -O2 -march=i386 -mtune=i586 -ffast-math -fomit-frame-pointer $(INCLUDES) -fgnu89-inline -Wmissing-prototypes $(CDEF)
INCLUDES = \
	-I$(realpath ./src) \
	-I$(realpath $(MUJS)) \
	-I$(realpath $(ALLEGRO))/include \
	-I$(realpath $(GLIDESDK))/include \
	-I$(realpath $(DZCOMMDIR))/include \
	-I$(realpath $(WATT32))/inc \
	-I$(realpath $(ZLIB)) \
	-I$(realpath $(KUBAZIP))/src \
	-I$(realpath $(ALPNG))/src \
	-I$(realpath $(MBEDTLS))/include \
	-I$(realpath $(MBEDTLS))/library \
	-I$(realpath $(CURL))/include \
	-I$(realpath $(WEBP))/src \
	-I$(realpath $(TIFF))/libtiff \
	-I$(realpath $(JASPER_SRC))/src/libjasper/include \
	-I$(realpath $(JASPER_BIN))/src/libjasper/include \
	-I$(realpath $(JPEG)) \
	-I$(realpath $(INI))/

# linker
LIBS     = -lalleg -lmujs -lm -lemu -lglide3i -ldzcom -lz -lwatt 
LDFLAGS  = -s \
	-L$(MUJS)/build/release \
	-L$(ALLEGRO)/lib/djgpp \
	-L$(GLIDE)/v1/lib \
	-L$(DZCOMMDIR)/lib/djgpp \
	-L$(WATT32)/lib \
	-L$(MBEDTLS)/library/ \
	-L$(ZLIB)

# output
EXE      = dojs.exe
RELZIP   = dojs-X.Y.Z.zip
FDZIP    = $(shell pwd)/FreeDOS_dojs-X.Y.Z.zip

# dirs/files
DOJSPATH		= $(realpath .)
BUILDDIR		= build
DOCDIR			= doc/html
DXE_TEMPLATE	= dxetemplate.txt
DXE_EXPORTS		= src/dexport.c

## compiler and binutils
CROSS=i586-pc-msdosdjgpp
CROSS_PLATFORM=i586-pc-msdosdjgpp-
CC=$(CROSS_PLATFORM)gcc
CXX=$(CROSS_PLATFORM)g++
AR=$(CROSS_PLATFORM)ar
LD=$(CROSS_PLATFORM)ld
STRIP=$(CROSS_PLATFORM)strip
RANLIB=$(CROSS_PLATFORM)ranlib
DXE3GEN = dxe3gen
DXE3RES = dxe3res
export

## other tools used in Makefile
AWKPRG		= awk
CATPRG		= cat
CPPRG		= cp
CUTPRG		= cut
CURLPRG		= curl
ECHOPRG		= echo
EGREPPRG	= egrep
FINDPRG		= find
GREPPRG		= grep
JSDOCPRG	= jsdoc
MKDIRPRG	= mkdir
PYTHONPRG	= python3
RMPRG		= rm
SEDPRG		= sed
SHPRG		= bash
SORTPRG		= sort
UNIQPRG		= uniq
ZIPPRG		= zip
FONTCONV	= GrxFntConv
NPM_INSTALL = npm install -g

MPARA=-j8

PARTS= \
	$(BUILDDIR)/vgm.o \
	$(BUILDDIR)/blurhash.o \
	$(BUILDDIR)/blender.o \
	$(BUILDDIR)/bytearray.o \
	$(BUILDDIR)/intarray.o \
	$(BUILDDIR)/3dfx-glide.o \
	$(BUILDDIR)/3dfx-state.o \
	$(BUILDDIR)/3dfx-texinfo.o \
	$(BUILDDIR)/bitmap.o \
	$(BUILDDIR)/color.o \
	$(BUILDDIR)/dialog.o \
	$(BUILDDIR)/DOjS.o \
	$(BUILDDIR)/glidedxe.o \
	$(BUILDDIR)/edi_render.o \
	$(BUILDDIR)/edit.o \
	$(BUILDDIR)/file.o \
	$(BUILDDIR)/font.o \
	$(BUILDDIR)/flic.o \
	$(BUILDDIR)/funcs.o \
	$(BUILDDIR)/lowlevel.o \
	$(BUILDDIR)/gfx.o \
	$(BUILDDIR)/inifile.o \
	$(BUILDDIR)/joystick.o \
	$(BUILDDIR)/lines.o \
	$(BUILDDIR)/midiplay.o \
	$(BUILDDIR)/socket.o \
	$(BUILDDIR)/sound.o \
	$(BUILDDIR)/syntax.o \
	$(BUILDDIR)/util.o \
	$(BUILDDIR)/watt.o \
	$(BUILDDIR)/zip/src/zip.o \
	$(BUILDDIR)/zipfile.o \
	$(BUILDDIR)/dexport.o \
	$(BUILDDIR)/systime.o \
	$(BUILDDIR)/ini/ini.o

DXE_DIRS := $(wildcard plugins/*.dxelib)

all: dojs $(DXE_DIRS) JSBOOT.ZIP

dojs: init libmujs liballegro dzcomm libwatt32 libz alpng libcurl mesa3 libwebp libtiff libjasper libjpeg texus.exe fntconv.exe $(EXE)

mesa3: $(LIB_MESA)
$(LIB_MESA):
	$(MAKE) $(MPARA) -C $(MESA3) -f Makefile.dja

libcurl: $(LIB_CURL)
$(LIB_CURL): $(LIB_MBEDTLS) $(LIB_Z)
	$(MAKE) $(MPARA) -C $(CURL)/lib -f Makefile.mk CFG=-zlib-mbedtls-watt TRIPLET=i586-pc-msdosdjgpp WATT_ROOT=$(WATT32)

libmbedtls: $(LIB_MBEDTLS)
$(LIB_MBEDTLS):
	$(MAKE) $(MPARA) -C $(MBEDTLS) -f Makefile lib

alpng: $(LIB_ALPNG)
$(LIB_ALPNG):
	$(MAKE) -C $(ALPNG) -f Makefile.zlib

libz: $(LIB_Z)
$(LIB_Z):
	$(MAKE) $(MPARA) -C $(ZLIB) -f Makefile.dojs

libwebp: $(LIB_WEBP)
$(LIB_WEBP):
	$(MAKE) $(MPARA) -C $(WEBP) -f makefile.djgpp src/libwebp.a sharpyuv/libsharpyuv.a

libbzip2: $(LIB_BZIP2)
$(LIB_BZIP2):
	$(MAKE) $(MPARA) -C $(BZIP2) -f Makefile libbz2.a

dzcomm: $(LIB_DZCOMM)
$(LIB_DZCOMM):
	$(MAKE) -C $(DZCOMMDIR) lib/djgpp/libdzcom.a

libmujs: $(LIB_MUJS)
$(LIB_MUJS):
	$(MAKE) $(MPARA) -C $(MUJS) build/release/libmujs.a

liballegro: $(LIB_ALLEGRO)
$(LIB_ALLEGRO):
	cd $(ALLEGRO) && $(SHPRG) ./xmake.sh lib

libwatt32: $(LIB_WATT)
$(LIB_WATT):
	DJ_PREFIX=$(dir $(shell which $(CC))) $(MAKE) $(MPARA) -C $(WATT32)/src -f djgpp.mak

configure_tiff: $(TIFF)/Makefile
$(TIFF)/Makefile:
	(cd $(TIFF) && HOST=$(CROSS) CFLAGS="$(CFLAGS)" LDFLAGS="" LIBS="" $(SHPRG) ./djgpp-config.sh)

libtiff: $(LIB_TIFF)
$(LIB_TIFF): $(TIFF)/Makefile
	$(MAKE) $(MPARA) -C $(TIFF)

libjasper: $(LIB_JASPER)
$(LIB_JASPER):
	(cd $(JASPER_SRC) && $(SHPRG) ./cmake-djgpp.sh)

libjpeg: $(LIB_JPEG)
$(LIB_JPEG):
	$(MAKE) $(MPARA) -C $(JPEG) -f makefile.dj libjpeg.a

$(EXE): $(PARTS) init libmujs liballegro dzcomm libwatt32 libz alpng libcurl mesa3 libtiff libjasper libjpeg
	$(CC) $(LDFLAGS) -o $@ $(PARTS) $(LIBS)

$(BUILDDIR)/%.o: src/%.c Makefile
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILDDIR)/loadpng/%.o: $(LOADPNG)/%.c Makefile
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILDDIR)/zip/src/%.o: $(KUBAZIP)/src/%.c Makefile
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILDDIR)/ini/%.o: $(INI)/%.c Makefile
	$(CC) $(CFLAGS) -c $< -o $@

$(DXE_DIRS): init libmujs liballegro dzcomm libwatt32 libz alpng libcurl libtiff libjasper libjpeg mesa3
	$(MAKE) -C $@

$(DXE_EXPORTS): dxetemplate.txt $(MUJS)/mujs.h
	$(PYTHONPRG) ./extract_functions.py $(DXE_TEMPLATE) $(MUJS)/mujs.h $@

JSBOOT.ZIP: $(shell $(FINDPRG) jsboot/ -type f)
	$(RMPRG) -f $@
	$(ZIPPRG) -9 -r $@ jsboot/

texus.exe:
	$(MAKE) -C $(TEXUS) clean all
	$(CPPRG) $(TEXUS)/texus.exe .

fntconv.exe:
	$(MAKE) -C $(FONTCONV) clean all
	$(CPPRG) $(FONTCONV)/fntconv.exe .

cacert.pem:
	$(CURLPRG) --remote-name --time-cond cacert.pem https://curl.se/ca/cacert.pem

zip: all cacert.pem doc
	$(RMPRG) -f $(RELZIP)
	$(RMPRG) -f dxetest.DXE dxetest2.DXE
	$(CPPRG) $(GLIDE)/v1/lib/glide3x.dxe ./GLIDE3X.DXE
	$(ZIPPRG) -9 -r $(RELZIP) $(EXE) dojs.ini WATTCP.CFG GLIDE3X.DXE CWSDPMI.EXE LICENSE *.md JSBOOT.ZIP examples/ $(DOCDIR) $(GLIDE)/*/lib/glide3x.dxe DPM.BAT V_*.BAT texus.exe fntconv.exe cacert.pem *.DXE

devzip: all cacert.pem doc
	$(RMPRG) -f $(RELZIP)
	$(CPPRG) $(GLIDE)/v1/lib/glide3x.dxe ./GLIDE3X.DXE
	$(ZIPPRG) -9 -r $(RELZIP) $(EXE) dojs.ini WATTCP.CFG GLIDE3X.DXE CWSDPMI.EXE LICENSE *.md JSBOOT.ZIP examples/ tests/*.js tests/*.svg $(GLIDE)/*/lib/glide3x.dxe *.BAT texus.exe fntconv.exe cacert.pem *.DXE
	$(CPPRG)p $(RELZIP) smbshare@192.168.2.8:/sata/c64

dostodon: zip
	$(CPPRG) png.DXE jpeg.DXE sqlite.DXE webp.DXE curl.DXE dojs.exe cacert.pem JSBOOT.ZIP ../GitHub/DOStodon/

doc:
	$(RMPRG) -rf $(DOCDIR)
	$(MKDIRPRG) -p $(DOCDIR)
	# if this fails add JSDOC_TEMPLATES='<location(s) to look for templates>' to your make invocation
	for i in $(JSDOC_TEMPLATES); do [ -d $$i ] && cd doc && $(JSDOCPRG) --verbose -t $$i -c jsdoc.conf.json -d ../$(DOCDIR) && break; done

init: configure_tiff
	$(MKDIRPRG) -p $(BUILDDIR) $(BUILDDIR)/loadpng $(BUILDDIR)/zip/src $(BUILDDIR)/ini
	# make sure compile time is always updated
	$(RMPRG) -f $(BUILDDIR)/DOjS.o

clean:
	$(RMPRG) -rf $(BUILDDIR)/
	$(RMPRG) -f $(EXE) $(ZIP) JSLOG.TXT texus.exe fntconv.exe GLIDE3X.DXE JSBOOT.ZIP cacert.pem W32DHCP.TMP
	for dir in $(DXE_DIRS); do \
		$(MAKE) -C $$dir -f Makefile $@; \
	done

distclean: clean alclean jsclean dzclean wattclean zclean apclean mbedtlsclean curlclean dxeclean mesa3clean webpclean bzip2clean distclean_tiff jasperclean jpegclean
	$(MAKE) -C $(TEXUS) clean
	$(MAKE) -C $(FONTCONV) clean
	$(RMPRG) -rf $(DOCDIR) TEST.TXT JSLOG.TXT synC.txt synJ.txt syn.txt *.DXE *.BMP *.PCX, *.TGA *.PNG TMP1.* TMP2.*
	$(RMPRG) -rf $(GLIDE)/*/test $(GLIDE)/texus/*.exe $(GLIDE)/texus/*.EXE $(GLIDE)/texus/build

dzclean:
	$(MAKE) -C $(DZCOMMDIR) clean

jsclean:
	$(MAKE) -C $(MUJS) clean

alclean:
	cd $(ALLEGRO) && $(SHPRG) ./xmake.sh clean

wattclean:
	$(MAKE) -C $(WATT32)/src -f djgpp.mak clean

zclean:
	$(MAKE) -C $(ZLIB) -f Makefile.dojs clean

webpclean:
	$(MAKE) -C $(WEBP) -f makefile.djgpp clean

bzip2clean:
	$(MAKE) -C $(BZIP2) -f Makefile clean

dxeclean:
	$(RMPRG) -f $(DXE_EXPORTS)

apclean:
	$(MAKE) -C $(ALPNG) -f Makefile.zlib clean

mbedtlsclean:
	$(MAKE) -C $(MBEDTLS) -f Makefile clean
	$(FINDPRG) $(MBEDTLS) -name \*.d -exec rm {} +

mesa3clean:
	$(MAKE) -C $(MESA3) -f Makefile.dja realclean

curlclean:
	$(MAKE) $(MPARA) -C $(CURL)/lib -f Makefile.mk CFG=-zlib-mbedtls-watt TRIPLET=i586-pc-msdosdjgpp WATT_ROOT=$(WATT32) clean
	$(RMPRG) -f $(LIB_CURL)

muclean:
	$(RMPRG) -f $(LIB_MUJS)

glideclean:
	$(RMPRG) -rf glidedxe.c

tiffclean:
	$(MAKE) -C $(TIFF) clean

distclean_tiff:
	-(cd $(TIFF) && $(MAKE) distclean)

jasperclean:
	$(RMPRG) -rf $(JASPER_BIN)

jpegclean:
	$(MAKE) -C $(JPEG) -f makefile.dj clean

fixnewlines:
	$(FINDPRG) . -iname "*.sh" -exec dos2unix -v \{\} \;

glidedxe.c:
	$(DXE3RES) -o dxetmp_v1.c $(GLIDE)/v1/lib/glide3x.dxe
	$(DXE3RES) -o dxetmp_v2.c $(GLIDE)/v2/lib/glide3x.dxe
	$(DXE3RES) -o dxetmp_v3.c $(GLIDE)/v3/lib/glide3x.dxe
	$(DXE3RES) -o dxetmp_v4.c $(GLIDE)/v4/lib/glide3x.dxe
	$(DXE3RES) -o dxetmp_vr.c $(GLIDE)/vr/lib/glide3x.dxe
	$(ECHOPRG) "#include <sys/dxe.h>" >$@
	$(CATPRG) dxetmp_*.c | $(GREPPRG) "extern_asm" | $(SORTPRG) | $(UNIQPRG) >>$@
	$(ECHOPRG) "DXE_EXPORT_TABLE_AUTO (___dxe_eta___glide3x)" >>$@
	$(CATPRG) dxetmp_*.c | $(GREPPRG) "DXE_EXPORT_ASM" | $(SORTPRG) | $(UNIQPRG) >>$@
	$(ECHOPRG) "DXE_EXPORT_END" >>$@
	$(RMPRG) dxetmp_*.c

syntaxF:
	$(RMPRG) -f synC.txt synJ.txt syn.txt
	$(GREPPRG) NFUNCDEF  src/*.c plugins/*.dxelib/*.c | $(CUTPRG) -d "," -f 2 | tr -d "_ " | $(AWKPRG) '{ print "\"" $$0 "\"" }' >synC.txt
	$(EGREPPRG) "^function " jsboot/3dfx.js  jsboot/a3d.js  jsboot/color.js  jsboot/file.js  jsboot/func.js  jsboot/ipx.js | $(CUTPRG) -d " " -f 2 | $(CUTPRG) -d "(" -f 1 | tr -d "_ " | awk '{ print "\"" $$0 "\"" }' > synJ.txt
	$(CATPRG) synC.txt synJ.txt | $(AWKPRG) '{ print length, "EDI_SYNTAX(LIGHTRED," $$0 "), //" }' | $(SORTPRG) -nr | $(UNIQPRG) | $(CUTPRG) -d' ' -f2- >syn.txt

syntaxM:
	$(RMPRG) -f synC.txt syn.txt
	$(GREPPRG) NPROTDEF  src/*.c plugins/*.dxelib/*.c | $(CUTPRG) -d "," -f 3 | $(SEDPRG) s/^\ \"/\"\./ | tr -d " " | $(AWKPRG) '{ print "\"" $$0 "\"" }' >synC.txt
	$(CATPRG) synC.txt | $(AWKPRG) '{ print length, "EDI_SYNTAX(RED," $$0 "), //" }' | $(SORTPRG) -nr | $(UNIQPRG) | $(CUTPRG) -d' ' -f2- >syn.txt

syntaxP:
	$(RMPRG) -f synC.txt syn.txt
	$(GREPPRG) js_defproperty  src/*.c plugins/*.dxelib/*.c | $(CUTPRG) -d "," -f 3 | $(SEDPRG) s/^\ \"/\"\./ >synC.txt
	$(CATPRG) synC.txt | $(AWKPRG) '{ print length, "EDI_SYNTAX(YELLOW," $$0 "), //" }' | $(SORTPRG) -nr | $(UNIQPRG) | $(CUTPRG) -d' ' -f2- >syn.txt

fdos: zip
	# clean and re-create  working directories
	$(RMPRG) -rf $(TMP) $(FDZIP)
	$(MKDIRPRG) -p $(TMP)/APPINFO
	$(MKDIRPRG) -p $(TMP)/DEVEL/DOJS
	$(MKDIRPRG) -p $(TMP)/SOURCE/DOJS
	$(MKDIRPRG) -p $(TMP)/tmp
	
	# copy LSMs
	$(CPPRG) FDOS/* $(TMP)/APPINFO

	# copy glide DXEs
	$(CPPRG) --parents $(GLIDE)/*/lib/glide3x.dxe $(TMP)/DEVEL/DOJS

	# copy html-docs
	$(CPPRG) -r --parents $(DOCDIR)/ $(TMP)/DEVEL/DOJS

	# copy distribution files
	$(CPPRG) -R \
		$(EXE) \
		WATTCP.CFG \
		CWSDPMI.EXE \
		LICENSE \
		*.md \
		JSBOOT.ZIP \
		texus.exe \
		fntconv.exe \
		cacert.pem \
		examples/ \
		V_*.BAT \
		DPM.BAT \
		*.DXE \
		$(TMP)/DEVEL/DOJS

	# ZIP up files with long file names into LFNFILES.ZIP
	(cd $(TMP)/DEVEL/DOJS && $(ZIPPRG) -9 -r LFNFILES.ZIP CHANGELOG.md doc/html && $(RMPRG) -rf CHANGELOG.md doc/)

	# make clean and copy source files
	$(MAKE) distclean
	$(CPPRG) -R \
		*.py \
		*.md \
		src/ \
		plugins/ \
		3rdparty/ \
		WATTCP.CFG \
		CWSDPMI.EXE \
		dojs.ini \
		LICENSE \
		Makefile \
		dxetemplate.txt \
		V_*.BAT \
		doc/ \
		examples/ \
		glide3x/ \
		jsboot/ \
		tests/ \
		GrxFntConv/ \
		$(TMP)/tmp
	# to be sure remove testdata!
	$(RMPRG) -rf $(TMP)/tmp/tests/testdata
	# zip up sources and remove tmp
	(cd $(TMP)/tmp && $(ZIPPRG) -9 -r ../SOURCE/DOJS/SOURCES.ZIP * && $(RMPRG) -rf $(TMP)/tmp)

	# ZIP up everything as DOS ZIP and clean afterwards
	(cd $(TMP) && $(ZIPPRG) -k -9 -r $(FDZIP) *)
	$(RMPRG) -rf $(TMP)

distribution:
	$(MAKE) -f Makefile.linux distclean && $(MAKE) -f Makefile.linux zip && $(MAKE) -f Makefile.linux distclean && $(MAKE) distclean && $(MAKE) fdos && $(MAKE) distclean

node_install:
	$(NPM_INSTALL) jsdoc
	$(NPM_INSTALL) better-docs
	$(NPM_INSTALL) @babel/core @babel/cli
	$(NPM_INSTALL) @babel/preset-env
	$(NPM_INSTALL) @babel/plugin-transform-exponentiation-operator


.PHONY: clean distclean init doc zip fdos $(DXE_DIRS)

DEPS := $(wildcard $(BUILDDIR)/*.d)
ifneq ($(DEPS),)
include $(DEPS)
endif
