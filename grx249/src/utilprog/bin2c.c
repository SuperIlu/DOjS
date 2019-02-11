/**
 ** BCC2GRX  -  Interfacing Borland based graphics programs to LIBGRX
 ** Copyright (C) 1993-97 by Hartmut Schirmer
 **
 **
 ** Contact :                Hartmut Schirmer
 **                          Feldstrasse 118
 **                  D-24105 Kiel
 **                          Germany
 **
 ** e-mail : hsc@techfak.uni-kiel.de
 **
 ** This file is part of the GRX graphics library.
 **
 ** The GRX graphics library is free software; you can redistribute it
 ** and/or modify it under some conditions; see the "copying.grx" file
 ** for details.
 **
 ** This library is distributed in the hope that it will be useful,
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 **
 **/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <malloc.h>

#define TRUE (1==1)
#define FALSE (!TRUE)

void usage(char *err)
{
  puts("bin2c -- convert binary files into C source\n");
  puts("Usage:\n");
  puts("  bin2c <input> <global name> [<output>]");
  if (err != NULL)
    printf("\n\nError: %s\n", err);
  exit(1);
}

char *timestr(void)
{
  struct tm *lt;
  time_t    t;

  t = time(NULL);
  lt = localtime(&t);
  return asctime(lt);
}

long filesize(FILE *f)
{
  long posi, res;

  posi = ftell(f);
  fseek(f, 0, SEEK_END);
  res = ftell(f);
  fseek(f, posi, SEEK_SET);
  return res;
}

int main(int argc, char *argv[])
{
  FILE *inp, *outp;
  char name[1000];
  long length, count;
  int linec;
  unsigned char *buffer, *komma;

  if (argc < 3 || argc > 4) usage("Incorrect command line");

  strcpy(name,"binary_data_field");
  inp = fopen( argv[1], "rb");
  if (inp == NULL) usage("Couldn't opem input file");

  strcpy( name, argv[2]);
  if (argc > 3) {
    outp = fopen(argv[3], "w");
    if (outp == NULL) usage("Couldn't open output file");
  } else
    outp = stdout;

  length = filesize(inp);
  buffer = (unsigned char *)malloc( length);
  if (buffer == NULL) usage("Out of memory");
  if (fread( buffer, length, 1, inp) != 1) usage("read error");
  fclose(inp);

  fprintf(outp, "unsigned char %s[] = {\n", name);
  count = 0;
  linec = 0;
  komma = " ";
  while (count != length) {
    linec += fprintf(outp, "%s", komma);
    if (linec > 75) {
      fprintf(outp, "\n ");
      linec = 1;
    }
    linec += fprintf(outp, "%d", *buffer);
    komma = ",";
    ++count;
    ++buffer;
  }
  fprintf(outp, "\n};\n");
  fclose( outp);
  return 0;
}
