## Toplevel Makefile

CC := gcc
CFLAGS := -W -Wall -O3 -I.

VERSION := 1.5
# (note to self: remember to edit loadpng.h as well)

LIB := libldpng.a

all: lib examples


ifdef DJDIR
include Makefile.dj
else
ifdef MINGDIR
include Makefile.mgw
else
include Makefile.unx
endif
endif

#--------------------------------------------------
.PHONY: lib

lib: $(LIB)

$(LIB): loadpng.o savepng.o regpng.o
	ar rs $@ $^

#--------------------------------------------------
.PHONY: examples

EXAMPLES := examples/example$(EXE) examples/exalpha$(EXE) \
		examples/exdata$(EXE) examples/browse$(EXE)

examples: $(EXAMPLES)

examples/example$(EXE): examples/example.c $(LIB)
	$(CC) $(CFLAGS) $(EXAMPLECFLAGS) -o $@ $^ $(LIB) $(EXAMPLELIBS)

examples/exalpha$(EXE): examples/exalpha.c $(LIB)
	$(CC) $(CFLAGS) $(EXAMPLECFLAGS) -o $@ $^ $(LIB) $(EXAMPLELIBS)

examples/exdata$(EXE): examples/exdata.c $(LIB)
	$(CC) $(CFLAGS) $(EXAMPLECFLAGS) -o $@ $^ $(LIB) $(EXAMPLELIBS)

examples/browse$(EXE): examples/browse.c $(LIB)
	$(CC) $(CFLAGS) $(EXAMPLECFLAGS) -o $@ $^ $(LIB) $(EXAMPLELIBS)

#--------------------------------------------------
# Note: this requires rm
.PHONY: clean

clean:
	rm -f $(EXAMPLES) examples/saved.png
	rm -f loadpng.o savepng.o regpng.o $(LIB) $(SHAREDLIB)
