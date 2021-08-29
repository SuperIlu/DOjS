###
# Makefile for cross compiling DOjS for FreeDOS/MS-DOS
# All compilation was done with DJGPP 7.2.0 built from https://github.com/andrewwutw/build-djgpp
###
# enter the path to x-djgpp here
DJGPP=/home/ilu/djgpp/bin
#DJGPP=/home/ilu/djgpp473/bin

# temp directory for building FreeDOS archive
TMP=/tmp/FDOS

MUJS=mujs-1.0.5
ALLEGRO=allegro-4.2.2-xc-master
GLIDE=glide3x
TEXUS=$(GLIDE)/texus
DZCOMMDIR=dzcomm
LIBDZCOMM=$(DZCOMMDIR)/lib/djgpp/libdzcom.a
WATT32=watt32-2.2dev.rel.11/
ZLIB=zlib-1.2.11
KUBAZIP=zip
ALPNG=alpng13
OPENSSL=openssl-1.1.1k
CURL=curl-7.74.0

# compiler
CDEF     = -DGC_BEFORE_MALLOC -DLFB_3DFX -DEDI_FAST #-DDEBUG_ENABLED # -DMEMDEBUG 
CFLAGS   = -MMD -Wall -std=gnu99 -O2 -march=i386 -mtune=i586 -ffast-math -fomit-frame-pointer $(INCLUDES) -fgnu89-inline -Wmissing-prototypes $(CDEF)
INCLUDES = \
	-I$(realpath .) \
	-I$(realpath $(MUJS)) \
	-I$(realpath $(ALLEGRO))/include \
	-I$(realpath $(GLIDE))/v1/include \
	-I$(realpath $(DZCOMMDIR))/include \
	-I$(realpath $(WATT32))/inc \
	-I$(realpath $(ZLIB)) \
	-I$(realpath $(KUBAZIP))/src \
	-I$(realpath $(ALPNG))/src \
	-I$(realpath $(CURL))/include

# linker
LIBS     = -lalleg -lmujs -lm -lemu -lglide3i -ldzcom -lz -lwatt 
LDFLAGS  = \
	-L$(MUJS)/build/release \
	-L$(ALLEGRO)/lib/djgpp \
	-L$(GLIDE)/v1/lib \
	-L$(DZCOMMDIR)/lib/djgpp \
	-L$(WATT32)/lib \
	-L$(ZLIB)

# output
EXE      = DOJS.EXE
RELZIP   = dojs.zip
FDZIP    = $(shell pwd)/FreeDOS_dojs.zip

# dirs/files
BUILDDIR		= build
DOCDIR			= doc/html
DXE_TEMPLATE	= dxetemplate.txt
DXE_EXPORTS		= dexport.c

CROSS=$(DJGPP)/i586-pc-msdosdjgpp
CROSS_PLATFORM=i586-pc-msdosdjgpp-
CC=$(DJGPP)/$(CROSS_PLATFORM)gcc
AR=$(DJGPP)/$(CROSS_PLATFORM)ar
LD=$(DJGPP)/$(CROSS_PLATFORM)ld
STRIP=$(DJGPP)/$(CROSS_PLATFORM)strip
RANLIB=$(DJGPP)/$(CROSS_PLATFORM)ranlib
DXE3GEN = dxe3gen
DXE3RES = dxe3res
export

MPARA=-j8

PARTS= \
	$(BUILDDIR)/blender.o \
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
	$(BUILDDIR)/funcs.o \
	$(BUILDDIR)/lowlevel.o \
	$(BUILDDIR)/gfx.o \
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
	$(BUILDDIR)/dexport.o

DXE_DIRS := $(wildcard *.dxelib)

all: init libmujs liballegro dzcomm libwatt32 libz alpng libcurl TEXUS.EXE $(EXE) $(DXE_DIRS) JSBOOT.ZIP

libmujs: $(MUJS)/build/release/libmujs.a

liballegro: $(ALLEGRO)/lib/djgpp/liballeg.a

dzcomm: $(LIBDZCOMM)

libwatt32: $(WATT32)/lib/libwatt.a

libz: $(ZLIB)/msdos/libz.a

alpng: $(ALPNG)/libalpng.a

libopenssl: $(OPENSSL)/libssl.a

libcurl: $(OPENSSL)/libssl.a $(ZLIB)/msdos/libz.a $(CURL)/libcurl.a

$(CURL)/libcurl.a:
	$(MAKE) $(MPARA) -C $(CURL)/lib -f Makefile.dj

$(OPENSSL)/libssl.a: $(WATT32)/lib/libwatt.a
	$(MAKE) $(MPARA) -C $(OPENSSL) -f Makefile build_libs
	$(MAKE) $(MPARA) -C $(OPENSSL) -f Makefile apps/openssl.exe

$(ALPNG)/libalpng.a:
	$(MAKE) -C $(ALPNG) -f Makefile.zlib

$(ZLIB)/msdos/libz.a:
	$(MAKE) $(MPARA) -C $(ZLIB) -f Makefile.dojs

$(PNG)/scripts/libpng.a:
	$(MAKE) $(MPARA) -C $(PNG) -f makefile.dojs libpng.a

$(LIBDZCOMM):
	$(MAKE) -C $(DZCOMMDIR) lib/djgpp/libdzcom.a

$(MUJS)/build/release/libmujs.a:
	$(MAKE) $(MPARA) -C $(MUJS) build/release/libmujs.a

$(ALLEGRO)/lib/djgpp/liballeg.a:
	cd $(ALLEGRO) && bash ./xmake.sh lib

$(WATT32)/lib/libwatt.a:
	DJ_PREFIX=$(DJGPP) $(MAKE) $(MPARA) -C $(WATT32)/src -f DJGPP.MAK

$(EXE): $(PARTS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)
	$(STRIP) $@
	#rm -f DOJS.exe

$(BUILDDIR)/%.o: %.c Makefile
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILDDIR)/loadpng/%.o: $(LOADPNG)/%.c Makefile
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILDDIR)/zip/src/%.o: $(KUBAZIP)/src/%.c Makefile
	$(CC) $(CFLAGS) -c $< -o $@

$(DXE_DIRS):
	$(MAKE) -C $@

$(DXE_EXPORTS): dxetemplate.txt $(MUJS)/mujs.h
	python3 ./extract_functions.py $(DXE_TEMPLATE) $(MUJS)/mujs.h $@

JSBOOT.ZIP: $(shell find jsboot/ -type f)
	rm -f $@
	zip -9 -r $@ jsboot/

TEXUS.EXE:
	$(MAKE) -C $(TEXUS) clean all
	cp $(TEXUS)/TEXUS.EXE .

zip: all doc
	rm -f $(RELZIP)
	rm -f dxetest.DXE dxetest2.DXE
	curl --remote-name --time-cond cacert.pem https://curl.se/ca/cacert.pem
	cp $(GLIDE)/v1/lib/glide3x.dxe ./GLIDE3X.DXE
	zip -9 -r $(RELZIP) $(EXE) WATTCP.CFG GLIDE3X.DXE CWSDPMI.EXE LICENSE *.md JSBOOT.ZIP examples/ $(DOCDIR) $(GLIDE)/*/lib/glide3x.dxe V_*.BAT TEXUS.EXE cacert.pem *.DXE

devzip: all doc $(RELZIP)
	rm -f $(RELZIP)
	curl --remote-name --time-cond cacert.pem https://curl.se/ca/cacert.pem
	cp $(GLIDE)/v1/lib/glide3x.dxe ./GLIDE3X.DXE
	cp $(OPENSSL)/apps/openssl.exe .
	zip -9 -r $(RELZIP) $(EXE) WATTCP.CFG GLIDE3X.DXE CWSDPMI.EXE LICENSE *.md JSBOOT.ZIP examples/*.js tests/*.js tests/*.svg $(GLIDE)/*/lib/glide3x.dxe V_*.BAT TEXUS.EXE cacert.pem openssl.exe *.DXE
	scp $(RELZIP) smbshare@192.168.2.8:/sata/c64

doc:
	rm -rf $(DOCDIR)
	mkdir -p $(DOCDIR)
	cd doc && jsdoc --verbose -c jsdoc.conf.json -d ../$(DOCDIR)

init:
	mkdir -p $(BUILDDIR) $(BUILDDIR)/loadpng $(BUILDDIR)/zip/src
	# make sure compile time is always updated
	rm -f $(BUILDDIR)/DOjS.o

clean:
	rm -rf $(BUILDDIR)/
	rm -f $(EXE) DOJS.exe $(ZIP) JSLOG.TXT TEXUS.EXE GLIDE3X.DXE JSBOOT.ZIP cacert.pem
	for dir in $(DXE_DIRS); do \
		$(MAKE) -C $$dir -f Makefile $@; \
	done

distclean: clean alclean jsclean dzclean wattclean zclean apclean sslclean curlclean dxeclean
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

dxeclean:
	rm -f $(DXE_EXPORTS)

apclean:
	$(MAKE) -C $(ALPNG) -f Makefile.zlib clean

sslclean:
	$(MAKE) -C $(OPENSSL) -f Makefile clean

curlclean:
	$(MAKE) -C $(CURL)/lib -f Makefile.dj clean
	rm -f $(CURL)/lib/libcurl.a

glideclean:
	rm -rf glidedxe.c

fixnewlines:
	find . -iname *.sh -exec dos2unix -v \{\} \;

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
	grep NFUNCDEF  *.c *.dxelib/*.c | cut -d "," -f 2 | tr -d "_ " | awk '{ print "\"" $$0 "\"" }' >synC.txt
	egrep "^function " jsboot/3dfx.js  jsboot/a3d.js  jsboot/color.js  jsboot/file.js  jsboot/func.js  jsboot/ipx.js | cut -d " " -f 2 | cut -d "(" -f 1 | tr -d "_ " | awk '{ print "\"" $$0 "\"" }' > synJ.txt
	cat synC.txt synJ.txt | awk '{ print length, "EDI_SYNTAX(LIGHTRED," $$0 "), //" }' | sort -nr | uniq | cut -d' ' -f2- >syn.txt

syntaxM:
	rm -f synC.txt syn.txt
	grep NPROTDEF  *.c *.dxelib/*.c | cut -d "," -f 3 | sed s/^\ \"/\"\./ | tr -d " " | awk '{ print "\"" $$0 "\"" }' >synC.txt
	cat synC.txt | awk '{ print length, "EDI_SYNTAX(RED," $$0 "), //" }' | sort -nr | uniq | cut -d' ' -f2- >syn.txt

syntaxP:
	rm -f synC.txt syn.txt
	grep js_defproperty  *.c *.dxelib/*.c | cut -d "," -f 3 | sed s/^\ \"/\"\./ >synC.txt
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
		TEXUS.EXE \
		cacert.pem \
		examples/ \
		V_*.BAT \
		*.DXE \
		$(TMP)/DEVEL/DOJS

	# ZIP up files with long file names into LFNFILES.ZIP
	(cd $(TMP)/DEVEL/DOJS && zip -9 -r LFNFILES.ZIP CHANGELOG.md doc/html && rm -rf CHANGELOG.md doc/)

	# make clean and copy source files
	make distclean
	cp -R \
		*.dxelib *.c *.h *.py *.md \
		WATTCP.CFG \
		CWSDPMI.EXE \
		LICENSE \
		Makefile \
		Makefile.dxemk \
		dxetemplate.txt \
		V_*.BAT \
		allegro-4.2.2-xc-master/ \
		alpng13/ \
		curl-7.74.0/ \
		doc/ \
		dzcomm/ \
		examples/ \
		glide3x/ \
		jsboot/ \
		mujs-1.0.5/ \
		openssl-1.1.1k/ \
		tests/ \
		watt32-2.2dev.rel.11/ \
		zip/ \
		zlib-1.2.11/ \
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
