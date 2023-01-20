/*
 * Simple test program for DXE loading.
 */

#include <stdio.h>
#include <stdlib.h>
#include <tcp.h>

int main (int argc, char **argv)
{
  int ch;

  while ((ch = getopt(argc, argv, "?vd")) != EOF)
     switch (ch)
     {
       case 'v': puts (wattcpVersion());
                 break;
       case 'd': dbug_init();
                 break;
       default : exit (-1);
     }

  sock_init();
  return (0);
}

