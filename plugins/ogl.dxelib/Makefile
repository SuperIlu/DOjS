## select OpenGL
DXE_CFLAGS	+= -I$(DOJSPATH)/$(MESA3)/include -DUSE_MESA=1 -DUSE_GLUT=1
DXE_LDFLAGS	+= -L$(DOJSPATH)/$(MESA3)/lib -lgl -lGLU -lglut

DXE_NAME	= ogl.DXE
DXE_FILES   = ogl_const.o

include ../Makefile.dxemk
