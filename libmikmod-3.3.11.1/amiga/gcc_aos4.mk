# makefile fragment for ppc-amigaos4 / gcc

LDFLAGS+= -noixemul
LDLIBS += -lm
CFLAGS += -noixemul
CPPFLAGS+= -DWORDS_BIGENDIAN=1

#CPPFLAGS+= -D__USE_INLINE__
#CPPFLAGS+= -D__USE_OLD_TIMEVAL__
