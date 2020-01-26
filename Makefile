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

INCLUDES=-I$(MUJS) -I$(ALLEGRO)/include -I$(GLIDE)/v1/include
LIBS=-lalleg -lmujs -lm -lemu -lglide3i

CFLAGS=-MMD -Wall -std=gnu99 -O2 -march=i386 -mtune=i586 -ffast-math $(INCLUDES) -DPLATFORM_MSDOS -fgnu89-inline -Wmissing-prototypes # -DDEBUG_ENABLED 
LDFLAGS=-L$(MUJS)/build/release -L$(ALLEGRO)/lib/djgpp -L$(GLIDE)/v1/lib

EXE=DOJS.EXE
ZIP=dojs.zip

BUILDDIR=build

DOCDIR=doc/html

CROSS=$(DJGPP)/i586-pc-msdosdjgpp
CROSS_PLATFORM=i586-pc-msdosdjgpp-
CC=$(DJGPP)/$(CROSS_PLATFORM)gcc
AR=$(DJGPP)/$(CROSS_PLATFORM)ar
LD=$(DJGPP)/$(CROSS_PLATFORM)ld
STRIP=$(DJGPP)/$(CROSS_PLATFORM)strip
RANLIB=$(DJGPP)/$(CROSS_PLATFORM)ranlib
export

PARTS= \
	$(BUILDDIR)/dosbuff.o \
	$(BUILDDIR)/ipx.o \
	$(BUILDDIR)/edit.o \
	$(BUILDDIR)/dialog.o \
	$(BUILDDIR)/lines.o \
	$(BUILDDIR)/syntax.o \
	$(BUILDDIR)/file.o \
	$(BUILDDIR)/midiplay.o \
	$(BUILDDIR)/bitmap.o \
	$(BUILDDIR)/color.o \
	$(BUILDDIR)/font.o \
	$(BUILDDIR)/DOjS.o \
	$(BUILDDIR)/funcs.o \
	$(BUILDDIR)/gfx.o \
	$(BUILDDIR)/sound.o \
	$(BUILDDIR)/util.o \
	$(BUILDDIR)/a3d.o \
	$(BUILDDIR)/zbuffer.o \
	$(BUILDDIR)/joystick.o \
	$(BUILDDIR)/dxe.o \
	$(BUILDDIR)/3dfx-texinfo.o \
	$(BUILDDIR)/3dfx-state.o \
	$(BUILDDIR)/3dfx-glide.o

all: init libmujs liballegro TEXUS.EXE $(EXE)

libmujs: $(MUJS)/build/release/libmujs.a

liballegro: $(ALLEGRO)/lib/djgpp/liballeg.a

$(MUJS)/build/release/libmujs.a:
	$(MAKE) -C $(MUJS) build/release/libmujs.a

$(ALLEGRO)/lib/djgpp/liballeg.a:
	cd $(ALLEGRO) && ./xmake.sh lib

$(EXE): $(PARTS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)
	$(STRIP) $@

$(BUILDDIR)/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

TEXUS.EXE:
	$(MAKE) -C $(TEXUS) clean all
	cp $(TEXUS)/TEXUS.EXE .

zip: all doc
	rm -f $(ZIP)
	cp $(GLIDE)/v1/lib/glide3x.dxe .
	zip -9 -v -r $(ZIP) $(EXE) GLIDE3X.DXE CWSDPMI.EXE LICENSE README.md CHANGELOG.md jsboot/ examples/ $(DOCDIR) $(GLIDE)/*/lib/glide3x.dxe V_*.BAT TEXUS.EXE

devzip: all doc
	rm -f $(ZIP)
	cp $(GLIDE)/v1/lib/glide3x.dxe .
	zip -9 -v -r $(ZIP) $(EXE) GLIDE3X.DXE CWSDPMI.EXE LICENSE README.md CHANGELOG.md jsboot/ examples/ $(DOCDIR) $(GLIDE) V_*.BAT TEXUS.EXE glide3x/tests/*.3df glide3x/tests/*.exe tests/ 
	#zip -9 -v -r $(ZIP) $(EXE) $(VOODOO) V_*.BAT TEXUS.EXE tests/ jsboot/*.js glide3x/tests/*.3df glide3x/tests/*.exe
	#zip -9 -v -r $(ZIP) $(EXE) $(VOODOO) V_*.BAT TEXUS.EXE tests/ jsboot/*.js
	scp $(ZIP) smbshare@192.168.2.8:/sata/c64

doc:
	rm -rf $(DOCDIR)
	mkdir -p $(DOCDIR)
	cd doc && jsdoc -c jsdoc.conf.json -d ../$(DOCDIR)

init:
	mkdir -p $(BUILDDIR)

clean:
	rm -rf $(BUILDDIR)/
	rm -f $(EXE) DOjS.exe $(ZIP) JSLOG.TXT TEXUS.EXE

distclean: clean
	cd $(ALLEGRO) && ./xmake.sh clean
	$(MAKE) -C $(MUJS) clean
	rm -rf $(DOCDIR) TEST.TXT JSLOG.TXT GLIDE3X.DXE

glideclean:
	rm -rf dxe.c

dxe.c:
	dxe3res -o dxetmp_v1.c $(GLIDE)/v1/lib/glide3x.dxe
	dxe3res -o dxetmp_v2.c $(GLIDE)/v2/lib/glide3x.dxe
	dxe3res -o dxetmp_v3.c $(GLIDE)/v3/lib/glide3x.dxe
	dxe3res -o dxetmp_v4.c $(GLIDE)/v4/lib/glide3x.dxe
	dxe3res -o dxetmp_vr.c $(GLIDE)/vr/lib/glide3x.dxe
	echo "#include <sys/dxe.h>" >dxe.c
	cat dxetmp_*.c | grep "extern_asm" | sort | uniq >>dxe.c
	echo "DXE_EXPORT_TABLE_AUTO (___dxe_eta___glide3x)" >>dxe.c
	cat dxetmp_*.c | grep "DXE_EXPORT_ASM" | sort | uniq >>dxe.c
	echo "DXE_EXPORT_END" >>dxe.c
	rm dxetmp_*.c

.PHONY: clean distclean init doc

DEPS := $(wildcard $(BUILDDIR)/*.d)
ifneq ($(DEPS),)
include $(DEPS)
endif
