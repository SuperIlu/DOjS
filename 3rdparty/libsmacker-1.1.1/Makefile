# modified for DOjS by Andre Seidelt

# To compile "driver"
#
#   make 
#

INCLUDE_PATH=.

CFLAGS=-MMD -Os -Wall -Wwrite-strings -Wpointer-arith -Wconversion -Wstrict-prototypes -Wmissing-prototypes

LIBS=libsmacker.a
OBJS = smacker.o smk_bitstream.o smk_hufftree.o

all: $(LIBS) driver smk2avi

%.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(LIBS): $(OBJS) $(OBJA)
	$(AR) cr $@ $(OBJS) $(OBJA)

driver: driver.o
	$(CC) -o $@ $< -L. -lsmacker

smk2avi: smk2avi.o
	$(CC) -o $@ $< -L. -lsmacker

# INCLUDE_PATH and LIBRARY_PATH were set for [make] in djgpp.env .

.PHONY : uninstall clean

clean:
	$(RM) *.d
	$(RM) *.o
	$(RM) $(LIBS)

DEPS := $(wildcard *.d)
ifneq ($(DEPS),)
include $(DEPS)
endif
