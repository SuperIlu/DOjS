::
:: Only used from 'src/configur.sh' on Linux to start DOSemu to generate these files.
::
util\wc_err -s > src\build\watcom\syserr.c
util\wc_err -e > inc\sys\watcom.err
