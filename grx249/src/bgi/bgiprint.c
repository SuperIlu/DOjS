/************************************************************************
 *                                                                      *
 * Printing from BCC2GRX                                                *
 *                                                                      *
 * Author: Andris Pavenis    (pavenis@latnet.lv)                        *
 *                                                                      *
 ************************************************************************/


#include <stdio.h>
#include <string.h>
#include <grx20.h>
#include <grxprint.h>
#include <libbcc.h>
#include "bccgrx00.h"



int     __gr_print_mode = -1;
char  * __gr_print_dest = NULL;


static  int     bgiprint_init (void);
static  int     bgiprint_close (void);
                              

int     set_BGI_print_mode ( int mode , char * dest )
  {
      if (dest)
        if (*dest==0)
           dest = 0;
      __gr_print_mode = mode;
      if (__gr_print_dest) free(__gr_print_dest);
      __gr_print_dest = dest ? strdup(dest) : NULL;
      __gr_set_libbcc_init_hooks (bgiprint_init,bgiprint_close);
      return 0;
  }



static  int     bgiprint_init (void)
  {
      int  ret;
      ret = GrPrintSetMode (__gr_print_mode);
      if (ret!=TRUE) { ERR=grInvalidMode; return -1; }
      __gr_Mode = 1;
      __gr_INIT = 1;
      graphdefaults();
      return 0;
  }


static  int     bgiprint_close (void)
  {
      if (__gr_print_dest)
        {
            GrPrintToFile (__gr_print_dest);
            free (__gr_print_dest);
            __gr_print_dest = 0;
        }
      else
        {
            GrDoPrinting ();
        }
      return 0;
  }



