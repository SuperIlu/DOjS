CC=gcc
LD=$(CC)

LIBMIKMOD_CONFIG=libmikmod-config
LIBMIKMOD_CFLAGS=$(shell $(LIBMIKMOD_CONFIG) --cflags)
LIBMIKMOD_LIBS  =$(shell $(LIBMIKMOD_CONFIG) --libs)

CFLAGS=-Wall -g $(LIBMIKMOD_CFLAGS)
LDFLAGS=$(LIBMIKMOD_LIBS)

PROG=splayFP

OBJS=splayFP.o

.PHONY : clean install

all: $(OBJS)
	$(LD) $(OBJS) $(LDFLAGS) -o $(PROG)

%.o: %.c %.h
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f *.o $(PROG)

