/*
 * Queue Demo program for dzcomm (dzcomm.sourceforge.net)
 *
 * Copyright 1999-2002 Neil Townsend, Oxford, England
 * neilt@users.sourceforge.net
 * file : exqueue.c
 * version : 1.0
 */

#include <stdio.h>
#ifdef DZCOMM_ALLEGRO_AVAIL_PROGS
#include <allegro.h>
#endif
#include "dzcomm.h"

int main() {
   int         i;
   char        op_string[50];
   fifo_queue *q;

#ifdef ALLEGRO_H
   /* Set up allegro and the screen - see exhello.c for comments */
   allegro_init();
   install_keyboard();
   set_gfx_mode(GFX_SAFE, 320, 200, 0, 0);
   set_palette(desktop_palette);
#endif

   /* 1. Set up the queue, 100 ints*/
   q = queue_new(100);

   /* 2. Put data on queue */
   for (i=0; i<40; i++) queue_put(q, i+32);

   /* 3. Take data off the queue */
   for (i=0; i<40; i++) op_string[i] = (char)queue_get(q);
   op_string[i] = '\0';

   /* 4. Show it on the screen */
#ifdef ALLEGRO_H
   acquire_screen();
   textout_centre(screen, font, op_string, SCREEN_W/2, SCREEN_H/2, 255);
   release_screen();
#else
   printf(op_string);
   printf("\n");
#endif

   /* 5. Delete the queue */
   queue_delete(q);

#ifdef ALLEGRO_H
   readkey();
#endif

   return 0;
}

#ifdef ALLEGRO_H
END_OF_MAIN();
#endif
