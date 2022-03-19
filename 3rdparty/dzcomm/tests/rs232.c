/*
 * DZcomm : a serial port API.
 * file : serial.c
 *
 * Library tester
 * 
 * v1.0 By Neil Townsend
 *
 * See readme.txt for copyright information.
 */

#include <stdio.h>
#ifdef DZCOMM_ALLEGRO_AVAIL_PROGS
#include <allegro.h>
#endif
#include <dzcomm.h>

inline void dz_print_comm_err() {printf("%s",szDZCommErr);fflush(stdout);}

comm_port *port1 = NULL;
comm_port *port2 = NULL;
comm_port *cur_port;

void give_usage(FILE *fp)
{
fprintf(fp, "serial\n");
fprintf(fp, "------\n");
fprintf(fp, "From the dzcomm package.\n");
fprintf(fp, "\n");
fprintf(fp, "For testing the dzcomm serial port library. Will run through\n");
fprintf(fp, "every speed/mode and verify communication. Either run between\n");
fprintf(fp, "two ports on one machine or single ports on separate machines.\n");
fprintf(fp, "\n");
fprintf(fp, "Usage\n");
fprintf(fp, "serial -h [<file 1> [<file 2>]]\n");
fprintf(fp, "\n");
fprintf(fp, "Where <file 1> is an optional .ini file describing the first\n");
fprintf(fp, "serial port to use, <file 2> the second. In the absence of\n");
fprintf(fp, "either serial will run on one port only which will be a guess\n");
fprintf(fp, "at COM 1. The '-h' argument gives this message\n");
fprintf(fp, "\n");
}


void parse_command_line(int argc, char **argv)
{
   comm_port *port;
   comm       comn = _com1;
   int        i;

   if (argc > 0) {
      for (i=1; i<argc; i++) {
         if (!strcmp(argv[i], "-h")) {
	    give_usage(stderr);
            exit(0);
	 }
	 else {
	    port = comm_port_init(comn++);
	    if (port == NULL) {
	       dz_print_comm_err();
	       exit(1);
	    }
	    if (comm_port_load_settings(port, argv[i]) != 1) {
	       dz_print_comm_err();
	       exit(1);
	    }
	    if (!port1) port1 = port;
	    else port2 = port;
	 }
      }
   }


   /* Default condition: One port, use COM A */
   if (port1 == NULL) {
      port1 = comm_port_init(comn++);
      if (port1 == NULL) {
	 dz_print_comm_err();
	 exit(1);
      }
   }

   if (port1) {
      if (comm_port_install_handler(port1) != 1) {
	  dz_print_comm_err();
	 exit(1);
      }
   }
   if (port2) {
      if (comm_port_install_handler(port2) != 1) {
	  dz_print_comm_err();
	 exit(1);
      }
   }
}

void show_status(char *desc, int o1, int i1, int o2, int i2)
{
   static int lo1 = -1;
   static int li1 = -1;
   static int lo2 = -1;
   static int li2 = -1;

   if ((lo1 == o1) && (li1 == i1) && (lo2 == o2) && (li2 == i2)) return;
   fprintf(stderr, desc);

   if (o1 == -2) {
      lo1 = lo2 = li1 = li2 = -1;
      fprintf(stderr, " - PASSED\n");
      return;
   }

   lo1 = o1;
   li1 = i1;
   lo2 = o2;
   li2 = i2;

   fprintf(stderr, "[%3d %3d]", o1, i1);
   if (port2 != NULL) fprintf(stderr, " [%3d %3d]", o2, i2);
   fprintf(stderr, "\r");
}

int main_loop(char *desc, int max_val)
{
   static int nsy = 1;

   int   r;
   int   got_i1;
   int   sent_i1;
   int   got_i2;
   int   sent_i2;

   got_i1 = -1;
   sent_i1 = 0;
   if (port2) {
       got_i2 = -1;
       sent_i2 = 0;
   }
   else {
       got_i2 = max_val;
       sent_i2 = max_val;
   }
   comm_port_reinstall(port1);
   if (port2 != NULL) comm_port_reinstall(port2);

   if (nsy) {
      fprintf(stderr, "Press return to start\n");
#ifdef ALLEGRO_H
      while (!keypressed());
      readkey();
#else
      getc(stdin);
#endif
      nsy = 0;
   }
   /* We are now listening at the new configuration. Wait for 0.1 s
    * before starting to transmit to allow the slower system to also
    * be listening before we tranmit. Don't need to do this on the
    * first time thorugh (nsy condition above) as the user has provides
    * the synchronisation then.
    */
   else if (port2 == NULL) {
#ifdef ALLEGRO_H
      rest(100);
#else
      usleep(100000);
#endif
      }

   do {
       if (sent_i1 < max_val) {
           comm_port_out(port1, (unsigned char) sent_i1++);
       }
       if ((r = comm_port_test(port1)) >= 0) {
           if (r == (got_i1+1)) got_i1++;
	   else fprintf(stderr, "ERROR on 1 [expected %d got %d]\n", got_i1+1, r);
       }

       if (port2) {
	   if (sent_i2 < max_val) {
	       comm_port_out(port2, (unsigned char) sent_i2++);
	   }
	   if ((r = comm_port_test(port2)) >= 0) {
	       if (r == (got_i2+1)) got_i2++;
	       else fprintf(stderr, "ERROR on 2 [expected %d got %d]\n", got_i2+1, r);
	   }
       }


       /* Any key will result in a stop if compiled with allegro. Otherwise,
        * use ctrl-c.
	*/
#ifdef ALLEGRO_H
       if (keypressed()) {
	  comm_port_flush_output(port1);
	  if (port2 != NULL) comm_port_flush_output(port2);
	  fprintf(stderr, "\nSTOPPED BY USER\n");
          return -1;
       }
#endif

       show_status(desc, sent_i1-1, got_i1, sent_i2-1, got_i2);
   } while ((got_i1 < (max_val-1)) || (sent_i1 < (max_val-1)) || (got_i2 < (max_val-1)) || (sent_i2 < (max_val-1)));

   show_status(desc, -2, 0, 0, 0);

   return 1;
}

int main(int argc, char **argv)
{
   int               max_val = 50;
   char              desc[255];
   data_bits         bits;
   baud_bits         baud;
   stop_bits         stop;
   parity_bits       parity;
   flow_control_type fc;
   
#ifdef ALLEGRO_H
   allegro_init();
   install_keyboard();
   install_timer();
   set_gfx_mode(GFX_TEXT, 0, 0, 0, 0);
#endif
   dzcomm_init();

   parse_command_line(argc, argv); // Will exit in this if command line requires it

   for (fc=DZ_MIN_CONTROL; fc<=DZ_MAX_CONTROL; fc++) {
      comm_port_set_flow_control(port1, fc);
      if (port2 != NULL) comm_port_set_flow_control(port2, fc);
      for (parity=DZ_MIN_PARITY; parity<=DZ_MAX_PARITY; parity++) {
         comm_port_set_parity(port1, parity);
	 if (port2 != NULL) comm_port_set_parity(port2, parity);
	 for (bits=DZ_MIN_DATA; bits<=DZ_MAX_DATA; bits++) {
	    comm_port_set_data_bits(port1, bits);
	    if (port2 != NULL) comm_port_set_data_bits(port2, bits);
	    for (baud=DZ_MIN_BAUD; baud<=DZ_MAX_BAUD; baud++) {
	       comm_port_set_baud_rate(port1, baud);
	       if (port2 != NULL) comm_port_set_baud_rate(port2, baud);
	       max_val = 1 << num_data_bits[bits];
	       for (stop=DZ_MIN_STOP; stop<=DZ_MAX_STOP; stop++) {
		  comm_port_set_stop_bits(port1, stop);
		  if (port2 != NULL) comm_port_set_stop_bits(port2, stop);
		  sprintf(desc, "(%s parity, %s control. bits:%d baud:%6d stop:%d):",
			  parity_desc[parity], flow_control_desc[fc], num_data_bits[bits],
			  baud_from_baud_bits[baud], num_stop_bits[stop]);
		  if (main_loop(desc, max_val) == -1) exit(0);
	       }
	    }
	 }
      }
   }

   return 0;
}

#ifdef ALLEGRO_H
END_OF_MAIN();
#endif


