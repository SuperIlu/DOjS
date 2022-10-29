LFLAGS=-fprofile-arcs -ftest-coverage
CLANG=clang
CLANG_FORMAT=clang-format
XXD=xxd

ifeq ($(OS),Windows_NT)
CFLAGS=-Wall
CFLAGS_OBJECT=/Fo:
CFLAGS_EXE=/Fe:
O_SUFFIX=.obj
EXE_SUFFIX=.exe
else
CFLAGS=-g -Wall -pipe --std=c1x -O3 -pedantic -Wsuggest-attribute=const -Wsuggest-attribute=format -Wclobbered -Wempty-body -Wignored-qualifiers -Wmissing-field-initializers -Wold-style-declaration -Wmissing-parameter-type -Woverride-init -Wtype-limits -Wuninitialized -Wunused-but-set-parameter -fprofile-arcs -ftest-coverage
CFLAGS_OBJECT=-o
CFLAGS_EXE=-o
O_SUFFIX=.o
EXE_SUFFIX=
endif

TESTPROG=testprog$(EXE_SUFFIX)

default: $(TESTPROG)

$(TESTPROG): pdfgen$(O_SUFFIX) tests/main$(O_SUFFIX) tests/penguin$(O_SUFFIX) tests/rgb$(O_SUFFIX)
	$(CC) $(CFLAGS_EXE) $@ pdfgen$(O_SUFFIX) tests/main$(O_SUFFIX) tests/penguin$(O_SUFFIX) tests/rgb$(O_SUFFIX) $(LFLAGS)

tests/fuzz-dstr: tests/fuzz-dstr.c pdfgen.c
	$(CLANG) -I. -g -o $@ $< -fsanitize=fuzzer,address,undefined,integer

tests/fuzz-%: tests/fuzz-%.c pdfgen.c
	$(CLANG) -I. -g -o $@ $< pdfgen.c -fsanitize=fuzzer,address,undefined,integer

tests/penguin.c: data/penguin.jpg
	# Convert data/penguin.jpg to a C source file with binary data in a variable
	$(XXD) -i $< > $@ || ( rm -f $@ ; false )

%$(O_SUFFIX): %.c
	$(CC) -I. -c $< $(CFLAGS_OBJECT) $@ $(CFLAGS)

check: $(TESTPROG) pdfgen.c pdfgen.h example-check
	cppcheck --std=c99 --enable=style,warning,performance,portability,unusedFunction --quiet pdfgen.c pdfgen.h tests/main.c
	$(CXX) -c pdfgen.c $(CFLAGS_OBJECT) /dev/null -Werror -Wall -Wextra
	./tests.sh
	./tests.sh acroread
	$(CLANG_FORMAT) pdfgen.c | colordiff -u pdfgen.c -
	$(CLANG_FORMAT) pdfgen.h | colordiff -u pdfgen.h -
	$(CLANG_FORMAT) tests/main.c | colordiff -u tests/main.c -
	gcov -r pdfgen.c

example-check: FORCE
	# Extract the code block from the README & make sure it compiles
	sed -n '/^```/,/^```/ p' < README.md | sed '/^```/ d' > example-check.c
	$(CC) $(CFLAGS) -o example-check example-check.c pdfgen.c $(LFLAGS)
	rm example-check example-check.c

check-fuzz-%: tests/fuzz-% FORCE
	mkdir -p fuzz-artifacts
	./$< -verbosity=0 -max_total_time=240 -max_len=8192 -rss_limit_mb=1024 -artifact_prefix="./fuzz-artifacts/"

fuzz-check: check-fuzz-image-data check-fuzz-image-file check-fuzz-header check-fuzz-text check-fuzz-dstr

format: FORCE
	$(CLANG_FORMAT) -i pdfgen.c pdfgen.h tests/main.c tests/fuzz-*.c

docs: FORCE
	doxygen pdfgen.dox 2>&1 | tee doxygen.log
	cat doxygen.log | test `wc -c` -le 0

FORCE:

clean:
	rm -f *$(O_SUFFIX) tests/*$(O_SUFFIX) $(TESTPROG) *.gcda *.gcno *.gcov tests/*.gcda tests/*.gcno output.pdf output.txt tests/fuzz-header tests/fuzz-text tests/fuzz-image-data tests/fuzz-image-file output.pdftk fuzz-image-file.pdf fuzz-image-data.pdf fuzz-image.dat doxygen.log tests/penguin.c fuzz.pdf output.ps
	rm -rf docs fuzz-artifacts infer-out
