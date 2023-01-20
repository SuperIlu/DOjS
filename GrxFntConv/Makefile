OBJS = \
	bdf.o \
	fntconv.o \
	buildfnt.o \
	dumpfna.o \
	dumpfont.o \
	dumpgrx.o
EXE      = fntconv.exe
CFLAGS  += -std=gnu99 -Wall -I. -O2 #-O0 -g3 -DDEBUG
LDFLAGS  = -s
LIBS     = -lm

all: $(EXE)

%o: %o Makefile
	$(CC) $(CFLAGS) -c $< -o $@

$(EXE): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

clean:
	rm -f $(EXE) $(OBJS)
