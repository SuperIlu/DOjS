# makefile fragment for m68k-amigaos / vbcc

LDLIBS += -lm881
CFLAGS += -cpu=68020 -fpu=68881
CPPFLAGS += -D__AMIGA__
CPPFLAGS += -DWORDS_BIGENDIAN=1
