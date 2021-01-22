###
# Makefile for cross compiling DOjS for FreeDOS/MS-DOS
# All compilation was done with DJGPP 7.2.0 built from https://github.com/andrewwutw/build-djgpp
###
# enter the path to x-djgpp here
DJGPP=/home/ilu/djgpp/bin
#DJGPP=/home/ilu/djgpp473/bin

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
OPENSSL=openssl-1.1.1i
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

PARTS= \
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
	$(MAKE) -C $(CURL)/lib -f Makefile.dj

$(OPENSSL)/libssl.a: $(WATT32)/lib/libwatt.a
	$(MAKE) -C $(OPENSSL) -f Makefile build_libs
	$(MAKE) -C $(OPENSSL) -f Makefile apps/openssl.exe

$(ALPNG)/libalpng.a:
	$(MAKE) -C $(ALPNG) -f Makefile.zlib

$(ZLIB)/msdos/libz.a:
	$(MAKE) -C $(ZLIB) -f Makefile.dojs

$(PNG)/scripts/libpng.a:
	$(MAKE) -C $(PNG) -f makefile.dojs libpng.a

$(LIBDZCOMM):
	$(MAKE) -C $(DZCOMMDIR) lib/djgpp/libdzcom.a

$(MUJS)/build/release/libmujs.a:
	$(MAKE) -C $(MUJS) build/release/libmujs.a

$(ALLEGRO)/lib/djgpp/liballeg.a:
	cd $(ALLEGRO) && bash ./xmake.sh lib

$(WATT32)/lib/libwatt.a:
	DJ_PREFIX=$(DJGPP) $(MAKE) -C $(WATT32)/src -f DJGPP.MAK

$(EXE): $(PARTS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)
	$(STRIP) $@

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
	rm dxetest.DXE dxetest2.DXE
	curl --remote-name --time-cond cacert.pem https://curl.se/ca/cacert.pem
	cp $(GLIDE)/v1/lib/glide3x.dxe ./GLIDE3X.DXE
	zip -9 -r $(RELZIP) $(EXE) WATTCP.CFG GLIDE3X.DXE CWSDPMI.EXE LICENSE README.md CHANGELOG.md JSBOOT.ZIP examples/ $(DOCDIR) $(GLIDE)/*/lib/glide3x.dxe V_*.BAT TEXUS.EXE cacert.pem *.DXE

devzip: all doc
	rm -f $(RELZIP)
	curl --remote-name --time-cond cacert.pem https://curl.se/ca/cacert.pem
	cp $(GLIDE)/v1/lib/glide3x.dxe ./GLIDE3X.DXE
	cp openssl-1.1.1i/apps/openssl.exe .
	zip -9 -r $(RELZIP) $(EXE) WATTCP.CFG GLIDE3X.DXE CWSDPMI.EXE LICENSE README.md CHANGELOG.md JSBOOT.ZIP examples/*.js tests/*.js $(GLIDE)/*/lib/glide3x.dxe V_*.BAT TEXUS.EXE cacert.pem openssl.exe *.DXE
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
	rm -f $(EXE) DOjS.exe $(ZIP) JSLOG.TXT TEXUS.EXE GLIDE3X.DXE JSBOOT.ZIP cacert.pem
	for dir in $(DXE_DIRS); do \
		$(MAKE) -C $$dir -f Makefile $@; \
	done

distclean: clean alclean jsclean dzclean wattclean zclean apclean sslclean curlclean dxeclean
	rm -rf $(DOCDIR) TEST.TXT JSLOG.TXT synC.txt synJ.txt syn.txt *.DXE *.BMP *.PCX, *.TGA *.PNG TMP1.* TMP2.*

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

.PHONY: clean distclean init doc INT13.EXE $(DXE_DIRS)

DEPS := $(wildcard $(BUILDDIR)/*.d)
ifneq ($(DEPS),)
include $(DEPS)
endif
