#cRSID makefile

CC=gcc
AR=ar rcs
CP=cp
RM=rm -f

APPNAME = crsid
APPSOURCES = cRSID.c
LIBHEADER = libcRSID.h
LIBSOURCE = libcRSID.c
LIBSOURCES = $(LIBHEADER) $(LIBSOURCE)
SUBSOURCES = C64/*.c host/*.c
LIBSHARED = libcRSID.so
LIBSTATIC = libcRSID.a
APPDIR = /usr/bin
LIBDIR = /usr/lib
INCDIR = /usr/include

CRSID_PLATFORM_PC = 1
CRSID_PLATFORM = $(CRSID_PLATFORM_PC)
BACKEND_IDS = -DCRSID_PLATFORM=$(CRSID_PLATFORM_PC)

ifeq ($(CRSID_PLATFORM),$(CRSID_PLATFORM_PC))
DEFINES = -DCRSID_PLATFORM_PC
CCFLAGS = `sdl-config --cflags --libs`  -lSDL  -O2  -s
endif


all: $(APPNAME) $(LIBSHARED) $(LIBSTATIC)


$(APPNAME): $(APPSOURCES) $(LIBSOURCES) $(SUBSOURCES)
	$(CC)  $(APPSOURCES)  $(LIBSOURCES)  -o crsid  $(DEFINES)  $(CCFLAGS)


$(LIBSHARED): $(LIBSOURCES) $(SUBSOURCES)
	$(CC)  -c $(LIBSOURCE)  -fpic  -o $(LIBSHARED).o  $(DEFINES)  $(CCFLAGS)
	$(CC)  $(LIBSHARED).o  -shared  -o $(LIBSHARED)
	$(RM) $(LIBSHARED).o


$(LIBSTATIC): $(LIBSOURCES) $(SUBSOURCES)
	$(CC)  -c $(LIBSOURCE)  -o $(LIBSTATIC).o  $(DEFINES)  $(CCFLAGS)
	$(AR) $(LIBSTATIC) $(LIBSTATIC).o
	$(RM) $(LIBSTATIC).o


test: $(APPNAME)
	./$(APPNAME) resources/music/Flash_It_Back.sid info


curvegen:
	$(CC)  resources/curvegen.c  -o resources/curvegen  -lm  -s
	./resources/curvegen


install: $(APPNAME) $(LIBSHARED) $(LIBSTATIC)
	$(CP)  $(LIBSHARED) $(LIBSTATIC)  $(LIBDIR)
	$(CP)  $(LIBHEADER)  $(INCDIR)
	$(CP)  $(APPNAME)  $(APPDIR)


uninstall:
	$(RM) $(APPDIR)/$(APPNAME) $(LIBDIR)/$(LIBSHARED) $(LIBDIR)/$(LIBSTATIC) $(INCDIR)/$(LIBHEADER)


clean:
	$(RM) $(APPNAME) *.o *.a *.so resources/curvegen


purge: clean uninstall


$(APPNAME).exe: $(APPSOURCES) $(LIBSOURCES) $(SUBSOURCES)
	i686-w64-mingw32-gcc  $(APPSOURCES)  $(LIBSOURCES)  -o crsid.exe  $(DEFINES) \
    -s -Wl,-subsystem,console -lmingw32 -lmingwex -mwindows -lSDLmain -lSDL \
    -I/usr/local/sdlmingw/include  -L/usr/local/sdlmingw/lib
