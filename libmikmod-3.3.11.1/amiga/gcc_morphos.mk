# makefile fragment for ppc-morphos / gcc

LDFLAGS+= -noixemul
CFLAGS += -noixemul
CPPFLAGS += -DWORDS_BIGENDIAN=1
