###
# TexUS makefile

EXE=TEXUS.EXE
CROSS_PLATFORM=i586-pc-msdosdjgpp-
CC=$(DJGPP)/$(CROSS_PLATFORM)gcc
AR=$(DJGPP)/$(CROSS_PLATFORM)ar
LD=$(DJGPP)/$(CROSS_PLATFORM)ld
STRIP=$(DJGPP)/$(CROSS_PLATFORM)strip
RANLIB=$(DJGPP)/$(CROSS_PLATFORM)ranlib

BUILDDIR=build

# local defines, options, includes
LCDEFS= -DGLIDE_DIAGS -DHWC_CSIM -DGLIDE3 -DGLIDE3_ALPHA -DGD_DEBUGGING -DGLIDE_SIMULATOR

CFLAGS=-I. -I../v1/include $(LCDEFS)

PARTS  = \
    $(BUILDDIR)/codec.o \
    $(BUILDDIR)/eigen.o \
    $(BUILDDIR)/bitcoder.o \
    $(BUILDDIR)/texuslib.o \
    $(BUILDDIR)/clamp.o \
    $(BUILDDIR)/read.o \
    $(BUILDDIR)/resample.o \
    $(BUILDDIR)/mipmap.o \
    $(BUILDDIR)/quantize.o \
    $(BUILDDIR)/ncc.o \
    $(BUILDDIR)/nccnnet.o \
    $(BUILDDIR)/pal256.o \
    $(BUILDDIR)/pal6666.o \
    $(BUILDDIR)/dequant.o \
    $(BUILDDIR)/view.o \
    $(BUILDDIR)/util.o \
    $(BUILDDIR)/diffuse.o \
    $(BUILDDIR)/write.o \
    $(BUILDDIR)/tga.o \
    $(BUILDDIR)/3df.o \
    $(BUILDDIR)/ppm.o \
    $(BUILDDIR)/rgt.o \
    $(BUILDDIR)/txs.o \
    $(BUILDDIR)/cmd.o

all: init $(EXE)

init:
	mkdir -p $(BUILDDIR)

$(EXE): $(PARTS)
	$(CC) -o $@ $^
	$(STRIP) $@

$(BUILDDIR)/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILDDIR)/
	rm -f $(EXE) TEXUS.exe
