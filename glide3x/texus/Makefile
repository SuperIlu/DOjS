###
# TexUS makefile

EXE=texus.exe
CROSS_PLATFORM=i586-pc-msdosdjgpp-
CC=$(CROSS_PLATFORM)gcc
AR=$(CROSS_PLATFORM)ar
LD=$(CROSS_PLATFORM)ld
STRIP=$(CROSS_PLATFORM)strip
RANLIB=$(CROSS_PLATFORM)ranlib

BUILDDIR=build

# local defines, options, includes
LCDEFS= -DGLIDE_DIAGS -DHWC_CSIM -DGLIDE3 -DGLIDE3_ALPHA -DGD_DEBUGGING -DGLIDE_SIMULATOR

CFLAGS += -I. -I../v1/include $(LCDEFS)

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
	$(CC) -s -o $@ $^

$(BUILDDIR)/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILDDIR)/
	rm -f $(EXE)
