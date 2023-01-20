#
# GNU Makefile for Metaware High-C sample applications
#

CC     = hc386
CFLAGS = -g -w3 -Hnocopyr -Hpragma=Offwarn(491) -Hpragma=Offwarn(553) \
         -Hpragma=Offwarn(572) -Hon=relax_func_ptr_rules -D__MSDOS__ \
         -Dsnprintf=_bprintf -I../inc

C_ARGS  = $(TEMP)/hc386.arg

LD      = 386link
LD_ARGS = $(TEMP)/386link.arg

WATT_LIB = ../lib/wattcphf.lib

#
# Uncomment to enable profiling for Metaware High-C
# Copy this file from your Metaware installation and
# make it read-only.
#
# MW_PROF = mwprof.obj

#
# 386link flags:
#
LDFLAGS  = -lib hc386,hc387,hcna       # MetaWare High-C libraries
LDFLAGS += -lib dosx32                 # PharLap DOSX API library
LDFLAGS += -lib $(WATT_LIB)            # Watt-32 lib for MetaWare/PharLap
LDFLAGS += -lib exc_hc                 # Exception handler library
LDFLAGS += -libpath $(LIBPATH)         # lib search path
LDFLAGS += -offset 1000h               # start at 4kB (trap 0-pointer access)
LDFLAGS += -stack 50000                # allocate stack
LDFLAGS += -386                        # 386 (or later) processor target
LDFLAGS += -twocase                    # case sensitive link
LDFLAGS += -nostub                     # don't prepend a stub
LDFLAGS += -unprivileged               # run at ring 3
LDFLAGS += -fullwarn                   # give all warnings
LDFLAGS += -maxdata 0                  # limit data to what we use (no heap)
LDFLAGS += -fullseg                    # Make segment listing
LDFLAGS += -publist both               # map-list by name and address
LDFLAGS += -purge none *
LDFLAGS += -mapnames 30
LDFLAGS += -mapwidth 132
LDFLAGS += -pack                       # pack BSS segment
LDFLAGS += -nobanner                   # supress startup banner

ifdef MW_PROF
  LDFLAGS += -realbreak end_real       # profiler uses real-code
  LDFLAGS += -cvsymbols                # CodeView symbol format
else
  LDFLAGS += -symbols                  # -symbols (386debug) or -cvsymbols (Mdb)
endif

PROGS = ping.exp    popdump.exp rexec.exp   tcpinfo.exp cookie.exp \
        daytime.exp dayserv.exp finger.exp  host.exp    lpq.exp    \
        lpr.exp     ntime.exp   ph.exp      stat.exp    htget.exp  \
        revip.exp   tracert.exp tcptalk.exp uname.exp   vlsm.exp   \
        whois.exp   blather.exp lister.exp  ident.exp

all: $(PROGS)
	@echo 'High-C/PharLap binaries done.'

$(PROGS): $(C_ARGS) $(LD_ARGS) $(WATT_LIB)

$(C_ARGS): highc.mak
	@echo "$(CFLAGS)" > $@

$(LD_ARGS): highc.mak
	@echo "$(strip $(LDFLAGS))" > $@

tcptalk.exp:
	$(CC) -c @$(C_ARGS) -o $*.ho $*.c
	$(LD) $(MW_PROF) $*.ho -exe $*.exp -lib conio @$(LD_ARGS)

tracert.exp: tracert.c geoip.c
	$(CC) -c @$(C_ARGS) -DUSE_GEOIP tracert.c geoip.c
	$(LD) @$(LD_ARGS) $(MW_PROF) tracert.obj geoip.obj -exe tracert.exp
	rm -f tracert.obj geoip.obj

%.exp: %.c
	$(CC) -c @$(C_ARGS) -o $*.ho $*.c
	$(LD) $(MW_PROF) $*.ho -exe $*.exp @$(LD_ARGS)


clean:
	rm -f *.ho $(C_ARGS) $(LD_ARGS)

vclean: clean
	rm -f $(PROGS)
