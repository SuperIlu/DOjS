INCDIR = ../../include
GL_LIBS = generic.o -L../../lib -lMesaGLU -lMesaGL
PROGS = chess nurbs surface\
	checker checker2 texturesurf font model disk

all: generic.o $(PROGS)

generic.o:
	$(CC) -c -I$(INCDIR) $(CFLAGS) $*.m -o $@

.c: $(LIB_DEP) generic.o
	$(CC) -I$(INCDIR) $(CFLAGS) $*.c $(GL_LIBS) -framework AppKit -o $@

clean:
	rm -f $(PROGS) *.o
