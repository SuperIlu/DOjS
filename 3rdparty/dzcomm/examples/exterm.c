/* dzcomm - serial comms library
 * Copyright 1997-8    Dim Zegebart, Moscow, Russia
 * Copyright 1999-2002 Neil Townsend, Oxford, England
 * neilt@users.sourceforge.net
 * http://dzcomm.sourcefroge.net
 * file: exterm.c
 * version: 1.0
 */

/*
 * Note:
 *
 * If compiled without allegro, this code loses most of its
 * functionality it simply displays on the screen the data
 * that has come in on the first specified comm port. This
 * is because, without some library or other to facilitate
 * non blocking keyboard reading it is very messy to do and
 * this is meant to be a simple demo!
 */


#include <stdio.h>
#ifdef DZCOMM_ALLEGRO_AVAIL_PROGS
#include <allegro.h>
#endif
#include <dzcomm.h>

inline void dz_print_comm_err() {printf("%s\n",szDZCommErr);fflush(stdout);}

comm_port *port1;
comm_port *port2;
comm_port *cur_port;

#ifdef ALLEGRO_H

/* On some systems it's better not to bring up a window if you just want the keyboard */
#ifdef ALLEGRO_DOS
#define have_allegro_window 0
#else
#define have_allegro_window 1
#endif

void initialise_screen() {

   if (have_allegro_window) {
      set_gfx_mode(GFX_SAFE, 320, 200, 0, 0);
      set_palette(desktop_palette);

      acquire_screen();
      textout_centre(screen, font, "Type Here!!",
		     SCREEN_W/2, SCREEN_H/2-60, 255);
      textout(screen, font, "Ctrl-M: switch between comm ports",
	      0, SCREEN_H/2-32, 255);
      textout(screen, font, "Ctrl-B: send a half second break signal",
	      0, SCREEN_H/2-16, 255);
      textout(screen, font, "Ctrl-C: quit", 0, SCREEN_H/2, 255);
      textout(screen, font, "Other key presses sent to current port.",
	      0, SCREEN_H/2+16, 255);
      textout(screen, font, "Current port:", 0, SCREEN_H/2+40, 255);
      textout(screen, font, cur_port->szName,
	      SCREEN_W/2, SCREEN_H/2+40, 255);
      release_screen();
   }
   else {
      printf("Ctrl-M: switch between comm ports\n");
      printf("Ctrl-B: send a half second break signal\n");
      printf("Ctrl-C: quit\n");
      printf("Other key presses sent to current port.\n");
      printf("\nCurrent port: %s\n\n", cur_port->szName);
   }
}

void inform_port_change() {
   if (have_allegro_window) {
      acquire_screen();
      textout(screen, font, "                     ",
	      SCREEN_W/2, SCREEN_H/2+40, 255);
      textout(screen, font, cur_port->szName,
	      SCREEN_W/2, SCREEN_H/2+40, 255);
      printf("\n\n");
      release_screen();
   }
   else {
     printf("\n\nChanging to port: %s\n\n", cur_port->szName);
   }
}
#endif

void show_received_character(char c) {
   printf("%c", c);fflush(stdout);
}

int main(void)
{
   int           c;
#ifdef ALLEGRO_H
   unsigned char ch;

   allegro_init();
   install_keyboard();
#endif
   dzcomm_init();


   /* Set up comm1 */

   if ((port1 = comm_port_init(_com1)) == NULL) {
      dz_print_comm_err();
      exit(1);
   }

   if (comm_port_load_settings(port1, "exterm1.ini") == 0) {
       dz_print_comm_err();
       exit(1);
   }

   if (!comm_port_install_handler(port1)) {
      dz_print_comm_err();
      exit(1);
   }

   cur_port = port1;

#ifdef ALLEGRO_H
   /* Set up comm2 */

   if ((port2 = comm_port_init(_com2)) == NULL) {
       dz_print_comm_err();
       exit(1);
   }

   if (comm_port_load_settings(port2, "exterm2.ini") == 0) {
       dz_print_comm_err();
       exit(1);
   }

   if (!comm_port_install_handler(port2)) {
      dz_print_comm_err();
      exit(1);
   }
#else
   port2 = port1;
#endif

#ifdef ALLEGRO_H
   initialise_screen();
#else
   printf("Press Ctrl-C for a messy quit.\n");
   printf("\nCurrent port is: %s.\n\n", cur_port->szName);
#endif

   while(1) {

#ifdef ALLEGRO_H
      if (keypressed()) {
         c  = readkey();
         ch = ascii_(c);
         if (ctrl_(c,'C'))  {
            return (0);
         }
	 else if (ctrl_(c,'B')) {
	    comm_port_send_break(cur_port, 500);
	 }
         else if (ctrl_(c,'M')) {
            if (cur_port == port2) cur_port = port1;
            else                   cur_port = port2;
	    inform_port_change();
         }
         else comm_port_out(cur_port, ch);
      }
#endif

      if ((c = comm_port_test(cur_port)) != -1) {
	 show_received_character(data_(c));
      }
   }

}

#ifdef ALLEGRO_H
END_OF_MAIN();
#endif
