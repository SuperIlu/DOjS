# AllegroPNG
# Linux without dependencies

SOURCES = src/alpng_save.c src/alpng_interlacing.c src/alpng_filereader.c src/alpng_drawer.c src/alpng_common.c src/alpng_filters.c src/quantization/octree.c src/inflate/huffman.c src/inflate/inflate.c src/inflate/input.c

OBJECTS = objects/alpng_save.o objects/alpng_interlacing.o objects/alpng_filereader.o objects/alpng_drawer.o objects/alpng_common.o objects/alpng_filters.o objects/octree.o objects/huffman.o objects/inflate.o objects/input.o

.PHONY: all clean

all: libalpng.a ex01

libalpng.a: $(OBJECTS)
	ar crs libalpng.a $?

objects/%.o: %.c
	gcc $(CFLAGS) -O3 -Wall -Wextra -ansi -pedantic -Isrc -c $< -o $@ 

objects/%.o: src/%.c
	gcc $(CFLAGS) -O3 -Wall -Wextra -ansi -pedantic -Isrc -c $< -o $@ 

objects/%.o: src/inflate/%.c
	gcc $(CFLAGS) -O3 -Wall -Wextra -ansi -pedantic -Isrc -c $< -o $@ 

objects/%.o: src/quantization/%.c
	gcc $(CFLAGS) -O3 -Wall -Wextra -ansi -pedantic -Isrc -c $< -o $@ 

ex01: objects/ex01.o libalpng.a
	gcc $(LDFLAGS)  -s -O3 -Wall -Wextra -ansi -pedantic $^ -o $@ -Isrc -L. `allegro-config --libs`  


clean: 
	rm libalpng.a
	rm ex01
	rm objects/*.o
