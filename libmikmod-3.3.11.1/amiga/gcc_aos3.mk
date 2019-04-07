# makefile fragment for m68k-amigaos / gcc

LDFLAGS+= -noixemul
LDLIBS += -lm
CFLAGS += -noixemul
CPPFLAGS+= -DWORDS_BIGENDIAN=1
