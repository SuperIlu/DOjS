#include <stdio.h>
#include <stdlib.h>
#include "GL/osmesa.h"

int gl_width=480;
int gl_height=480;

void init(void)
{
   GLfloat light_ambient[] = { 0.0, 0.0, 0.0, 1.0 };
   GLfloat light_diffuse[] = { 1.0, 1.0, 1.0, 1.0 };
   GLfloat light_specular[] = { 1.0, 1.0, 1.0, 1.0 };
   GLfloat light_position[] = { 1.0, 1.0, 1.0, 0.0 };

   glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
   glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
   glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
   glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    
   glEnable(GL_LIGHTING);
   glEnable(GL_LIGHT0);
   glEnable(GL_DEPTH_TEST);
}

/*
 * reshape simply creates an orthographic projection
 * within the viewport.
 */
void reshape(int w, int h)
{
   glViewport(0,0,w,h);
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   glOrtho(-2.5, 2.5, -2.5, 2.5, -5.0, 5.0);

   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
}

void display(void)
{
   GLfloat red_mat[]   = { 1.0, 0.0, 0.0, 0.5 };
   GLfloat green_mat[] = { 0.0, 1.0, 0.0, 0.5 };
   GLfloat blue_mat[]  = { 0.0, 0.0, 1.0, 0.5};

   glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

   glPushMatrix();
   glRotatef(20.0, 1.0, 0.0, 0.0);

   glPushMatrix();
   glTranslatef(-0.75, 0.5, 0.0); 
   glRotatef(90.0, 1.0, 0.0, 0.0);
   glMaterialfv( GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, red_mat );
   auxSolidTorus(0.275, 0.85);
   glPopMatrix();

   glPushMatrix();
   glTranslatef(-0.75, -0.5, 0.0); 
   glRotatef(270.0, 1.0, 0.0, 0.0);
   glMaterialfv( GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, green_mat );
   auxSolidCone(1.0, 2.0);
   glPopMatrix();

   glPushMatrix();
   glTranslatef(0.75, 0.0, -1.0);
   glMaterialfv( GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, blue_mat );
   auxSolidSphere(1.0);
   glPopMatrix();

   glPopMatrix();
}

void render_image(void)
{
    init();
    reshape(gl_width,gl_height);
    display();
}

