#include <GL/gl.h>
#include <GL/glu.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <AppKit/psops.h>

extern void set_viewpoint(float, float, float);
extern void draw_scene(int,int,int);
extern void make_matrix(void);
extern void outline_scene(void);
extern void my_init(float width, float height);
