#
# Errno Cross make:
#
# Makefile for the Errno utility used for Watt-32 developement.
#
# Contributed by Ozkan Sezer <sezeroz@gmail.com>
# for cross-compiling Watt-32 on Linux.
#
#  NOTE: Some make programs may not be able to generate all
#        targets due to DPMI/DOS-extender conflicts etc.
#        Depends on generated "generrno.h" to be present in cwd.
#        "generrno.h" must be created by "./src/configur.sh" first.
#

all: dj_err mw_err mw64_err wc_err

dj_err: errnos.c generrno.h
	$(CC) -s -include generrno.h -o dj_err errnos.c

mw64_err: errnos.c generrno.h
	$(CC) -s -include generrno.h -o mw64_err errnos.c

mw_err: errnos.c generrno.h
	$(CC) -s -include generrno.h -o mw_err errnos.c

wc_err: errnos.c generrno.h
	$(CC) -s -include generrno.h -o wc_err errnos.c

clean:
	rm -f dj_err mw_err mw64_err wc_err generrno.h

