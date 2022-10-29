###
# Makefile for cross compiling DOjS for FreeDOS/MS-DOS
# All compilation was done with DJGPP 7.2.0 built from https://github.com/andrewwutw/build-djgpp
# make sure the DJGPP toolchain is in your path (i586-pc-msdosdjgpp-XXX)!
###

# temp directory for building FreeDOS archive
TMP=/tmp/FDOS

THIRDPARTY	= 3rdparty/
MUJS		= $(THIRDPARTY)/mujs-1.0.5
ALLEGRO		= $(THIRDPARTY)/allegro-4.2.2-xc-master
DZCOMMDIR	= $(THIRDPARTY)/dzcomm
WATT32		= $(THIRDPARTY)/watt32-2.2dev.rel.11/
ZLIB		= $(THIRDPARTY)/zlib-1.2.12
KUBAZIP		= $(THIRDPARTY)/zip-0.2.5
ALPNG		= $(THIRDPARTY)/alpng13
OPENSSL		= $(THIRDPARTY)/openssl-1.1.1q
CURL		= $(THIRDPARTY)/curl-7.84.0
MESA3		= $(THIRDPARTY)/MesaFX-3.4-master
BZIP2		= $(THIRDPARTY)/bzip2-1.0.8
INI			= $(THIRDPARTY)/ini-20220806/src

GLIDE=glide3x
GLIDESDK=$(GLIDE)/v1
TEXUS=$(GLIDE)/texus
FONTCONV=GrxFntConv

LIB_DZCOMM	= $(DZCOMMDIR)/lib/djgpp/libdzcom.a
LIB_MUJS	= $(MUJS)/build/release/libmujs.a
LIB_ALLEGRO	= $(ALLEGRO)/lib/djgpp/liballeg.a
LIB_WATT	= $(WATT32)/lib/libwatt.a
LIB_Z		= $(ZLIB)/msdos/libz.a
LIB_ALPNG	= $(ALPNG)/libalpng.a
LIB_SSL		= $(OPENSSL)/libssl.a
LIB_CURL	= $(CURL)/libcurl.a
LIB_MESA	= $(MESA3)/lib/libgl.a
LIB_BZIP2	= $(BZIP2)/libbzip2.a

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
	-I$(realpath $(OPENSSL))/include \
	-I$(realpath $(CURL))/include \
	-I$(realpath $(INI))/

# linker
LIBS     = -lalleg -lmujs -lm -lemu -lglide3i -ldzcom -lz -lwatt 
LDFLAGS  = -s \
	-L$(MUJS)/build/release \
	-L$(ALLEGRO)/lib/djgpp \
	-L$(GLIDE)/v1/lib \
	-L$(DZCOMMDIR)/lib/djgpp \
	-L$(WATT32)/lib \
	-L$(ZLIB)

# output
EXE      = dojs.exe
RELZIP   = dojs-X.Y.Z.zip
FDZIP    = $(shell pwd)/FreeDOS_dojs.zip

# dirs/files
DOJSPATH		= $(realpath .)
BUILDDIR		= build
DOCDIR			= doc/html
DXE_TEMPLATE	= dxetemplate.txt
DXE_EXPORTS		= src/dexport.c

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

MPARA=-j8

PARTS= \
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
	$(BUILDDIR)/ini/ini.o

DXE_DIRS := $(wildcard plugins/*.dxelib)

all: init libmujs liballegro dzcomm libwatt32 libz alpng libcurl mesa3 texus.exe fntconv.exe $(EXE) $(DXE_DIRS) JSBOOT.ZIP

mesa3: $(LIB_MESA)
$(LIB_MESA):
	$(MAKE) $(MPARA) -C $(MESA3) -f Makefile.dja

libcurl: $(LIB_CURL)
$(LIB_CURL): $(LIB_SSL) $(LIB_Z)
	$(MAKE) $(MPARA) -C $(CURL)/lib -f Makefile.dj

libopenssl: $(LIB_SSL)
$(LIB_SSL): $(LIB_WATT)
	$(MAKE) $(MPARA) -C $(OPENSSL) -f Makefile build_libs
	$(MAKE) $(MPARA) -C $(OPENSSL) -f Makefile apps/openssl.exe

alpng: $(LIB_ALPNG)
$(LIB_ALPNG):
	$(MAKE) -C $(ALPNG) -f Makefile.zlib

libz: $(LIB_Z)
$(LIB_Z):
	$(MAKE) $(MPARA) -C $(ZLIB) -f Makefile.dojs

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
	cd $(ALLEGRO) && bash ./xmake.sh lib

libwatt32: $(LIB_WATT)
$(LIB_WATT):
	DJ_PREFIX=$(dir $(shell which $(CC))) $(MAKE) $(MPARA) -C $(WATT32)/src -f DJGPP.MAK

$(EXE): $(PARTS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

$(BUILDDIR)/%.o: src/%.c Makefile
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILDDIR)/loadpng/%.o: $(LOADPNG)/%.c Makefile
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILDDIR)/zip/src/%.o: $(KUBAZIP)/src/%.c Makefile
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILDDIR)/ini/%.o: $(INI)/%.c Makefile
	$(CC) $(CFLAGS) -c $< -o $@

$(DXE_DIRS):
	$(MAKE) -C $@

$(DXE_EXPORTS): dxetemplate.txt $(MUJS)/mujs.h
	python3 ./extract_functions.py $(DXE_TEMPLATE) $(MUJS)/mujs.h $@

JSBOOT.ZIP: $(shell find jsboot/ -type f)
	rm -f $@
	zip -9 -r $@ jsboot/

texus.exe:
	$(MAKE) -C $(TEXUS) clean all
	cp $(TEXUS)/texus.exe .

fntconv.exe:
	$(MAKE) -C $(FONTCONV) clean all
	cp $(FONTCONV)/fntconv.exe .

zip: all doc
	rm -f $(RELZIP)
	rm -f dxetest.DXE dxetest2.DXE
	curl --remote-name --time-cond cacert.pem https://curl.se/ca/cacert.pem
	cp $(GLIDE)/v1/lib/glide3x.dxe ./GLIDE3X.DXE
	zip -9 -r $(RELZIP) $(EXE) dojs.ini WATTCP.CFG GLIDE3X.DXE CWSDPMI.EXE LICENSE *.md JSBOOT.ZIP examples/ $(DOCDIR) $(GLIDE)/*/lib/glide3x.dxe DPM.BAT V_*.BAT texus.exe fntconv.exe cacert.pem *.DXE

devzip: all doc
	rm -f $(RELZIP)
	curl --remote-name --time-cond cacert.pem https://curl.se/ca/cacert.pem
	cp $(GLIDE)/v1/lib/glide3x.dxe ./GLIDE3X.DXE
	cp $(OPENSSL)/apps/openssl.exe .
	zip -9 -r $(RELZIP) $(EXE) dojs.ini WATTCP.CFG GLIDE3X.DXE CWSDPMI.EXE LICENSE *.md JSBOOT.ZIP examples/ tests/*.js tests/*.svg $(GLIDE)/*/lib/glide3x.dxe *.BAT texus.exe fntconv.exe cacert.pem openssl.exe *.DXE
	scp $(RELZIP) smbshare@192.168.2.8:/sata/c64

doc:
	rm -rf $(DOCDIR)
	mkdir -p $(DOCDIR)
	cd doc && jsdoc --verbose -c jsdoc.conf.json -d ../$(DOCDIR)

init:
	mkdir -p $(BUILDDIR) $(BUILDDIR)/loadpng $(BUILDDIR)/zip/src $(BUILDDIR)/ini
	# make sure compile time is always updated
	rm -f $(BUILDDIR)/DOjS.o

clean:
	rm -rf $(BUILDDIR)/
	rm -f $(EXE) $(ZIP) JSLOG.TXT texus.exe fntconv.exe GLIDE3X.DXE JSBOOT.ZIP cacert.pem W32DHCP.TMP
	for dir in $(DXE_DIRS); do \
		$(MAKE) -C $$dir -f Makefile $@; \
	done

distclean: clean alclean jsclean dzclean wattclean zclean apclean sslclean curlclean dxeclean mesa3clean
	$(MAKE) -C $(TEXUS) clean
	$(MAKE) -C $(FONTCONV) clean
	rm -rf $(DOCDIR) TEST.TXT JSLOG.TXT synC.txt synJ.txt syn.txt *.DXE *.BMP *.PCX, *.TGA *.PNG TMP1.* TMP2.* openssl.exe
	rm -rf $(GLIDE)/*/test $(GLIDE)/texus/*.exe $(GLIDE)/texus/*.EXE $(GLIDE)/texus/build

dzclean:
	$(MAKE) -C $(DZCOMMDIR) clean

jsclean:
	$(MAKE) -C $(MUJS) clean

alclean:
	cd $(ALLEGRO) && bash ./xmake.sh clean

wattclean:
	$(MAKE) -C $(WATT32)/src -f DJGPP.MAK clean

zclean:
	$(MAKE) -C $(ZLIB) -f Makefile.dojs clean

bzip2clean:
	$(MAKE) -C $(BZIP2) -f Makefile clean

dxeclean:
	rm -f $(DXE_EXPORTS)

apclean:
	$(MAKE) -C $(ALPNG) -f Makefile.zlib clean

sslclean:
	$(MAKE) -C $(OPENSSL) -f Makefile clean

mesa3clean:
	$(MAKE) -C $(MESA3) -f Makefile.dja realclean

curlclean:
	$(MAKE) -C $(CURL)/lib -f Makefile.dj clean
	rm -f $(CURL)/lib/libcurl.a

muclean:
	rm -f $(MUJS)/build/release/libmujs.a

glideclean:
	rm -rf glidedxe.c

fixnewlines:
	find . -iname "*.sh" -exec dos2unix -v \{\} \;

glidedxe.c:
	dxe3res -o dxetmp_v1.c $(GLIDE)/v1/lib/glide3x.dxe
	dxe3res -o dxetmp_v2.c $(GLIDE)/v2/lib/glide3x.dxe
	dxe3res -o dxetmp_v3.c $(GLIDE)/v3/lib/glide3x.dxe
	dxe3res -o dxetmp_v4.c $(GLIDE)/v4/lib/glide3x.dxe
	dxe3res -o dxetmp_vr.c $(GLIDE)/vr/lib/glide3x.dxe
	echo "#include <sys/dxe.h>" >$@
	cat dxetmp_*.c | grep "extern_asm" | sort | uniq >>$@
	echo "DXE_EXPORT_TABLE_AUTO (___dxe_eta___glide3x)" >>$@
	cat dxetmp_*.c | grep "DXE_EXPORT_ASM" | sort | uniq >>$@
	echo "DXE_EXPORT_END" >>$@
	rm dxetmp_*.c

syntaxF:
	rm -f synC.txt synJ.txt syn.txt
	grep NFUNCDEF  src/*.c plugins/*.dxelib/*.c | cut -d "," -f 2 | tr -d "_ " | awk '{ print "\"" $$0 "\"" }' >synC.txt
	egrep "^function " jsboot/3dfx.js  jsboot/a3d.js  jsboot/color.js  jsboot/file.js  jsboot/func.js  jsboot/ipx.js | cut -d " " -f 2 | cut -d "(" -f 1 | tr -d "_ " | awk '{ print "\"" $$0 "\"" }' > synJ.txt
	cat synC.txt synJ.txt | awk '{ print length, "EDI_SYNTAX(LIGHTRED," $$0 "), //" }' | sort -nr | uniq | cut -d' ' -f2- >syn.txt

syntaxM:
	rm -f synC.txt syn.txt
	grep NPROTDEF  src/*.c plugins/*.dxelib/*.c | cut -d "," -f 3 | sed s/^\ \"/\"\./ | tr -d " " | awk '{ print "\"" $$0 "\"" }' >synC.txt
	cat synC.txt | awk '{ print length, "EDI_SYNTAX(RED," $$0 "), //" }' | sort -nr | uniq | cut -d' ' -f2- >syn.txt

syntaxP:
	rm -f synC.txt syn.txt
	grep js_defproperty  src/*.c plugins/*.dxelib/*.c | cut -d "," -f 3 | sed s/^\ \"/\"\./ >synC.txt
	cat synC.txt | awk '{ print length, "EDI_SYNTAX(YELLOW," $$0 "), //" }' | sort -nr | uniq | cut -d' ' -f2- >syn.txt

fdos: zip
	# clean and re-create  working directories
	rm -rf $(TMP) $(FDZIP)
	mkdir -p $(TMP)/APPINFO
	mkdir -p $(TMP)/DEVEL/DOJS
	mkdir -p $(TMP)/SOURCE/DOJS
	mkdir -p $(TMP)/tmp
	
	# copy LSMs
	cp FDOS/* $(TMP)/APPINFO

	# copy glide DXEs
	cp --parents $(GLIDE)/*/lib/glide3x.dxe $(TMP)/DEVEL/DOJS

	# copy html-docs
	cp -r --parents $(DOCDIR)/ $(TMP)/DEVEL/DOJS

	# copy distribution files
	cp -R \
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
	(cd $(TMP)/DEVEL/DOJS && zip -9 -r LFNFILES.ZIP CHANGELOG.md doc/html && rm -rf CHANGELOG.md doc/)

	# make clean and copy source files
	make distclean
	cp -R \
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
	# zip up sources and remove tmp
	(cd $(TMP)/tmp && zip -9 -r ../SOURCE/DOJS/SOURCES.ZIP * && rm -rf $(TMP)/tmp)

	# ZIP up everything as DOS ZIP and clean afterwards
	(cd $(TMP) && zip -k -9 -r $(FDZIP) *)
	rm -rf $(TMP)

.PHONY: clean distclean init doc zip fdos $(DXE_DIRS)

DEPS := $(wildcard $(BUILDDIR)/*.d)
ifneq ($(DEPS),)
include $(DEPS)
endif
