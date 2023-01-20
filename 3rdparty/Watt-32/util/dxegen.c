/* Copyright (C) 1995 Charles Sandmann (sandmann@clio.rice.edu)
   This software may be freely distributed with above copyright, no warranty.
   Based on code by DJ Delorie, it's really his, enhanced, bugs fixed. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* Linux violates POSIX.1 and defines this, but it shouldn't.  We fix it. */
#undef _POSIX_SOURCE
#include <coff.h>
#include <sys/dxe.h>

/* This next function is needed for cross-compiling when the machine
   isn't little-endian like the i386 */

static void
dosswap(void *vdata, char *pattern)
{
  static int endian = 1;
  unsigned char *data, c;
  if (*(unsigned char *)(&endian) == 1)
    return;

  data = (unsigned char *)vdata;
  while (*pattern)
  {
    if (*pattern >= '1' && *pattern <= '9')
    {
      data += (*pattern-'0');
    }
    else if (*pattern == 's')
    {
      c = data[1];
      data[1] = data[0];
      data[0] = c;
      data += 2;
    }
    else if (*pattern == 'l')
    {
      c = data[3];
      data[3] = data[0];
      data[0] = c;
      c = data[1];
      data[1] = data[2];
      data[2] = c;
      data += 4;
    }
    else
      data++;

    pattern++;
  }
}

void exit_cleanup(void)
{
  remove("dxe__tmp.o");
}

int main(int argc, char **argv)
{
  int errors = 0;
  int debug = 0;
  unsigned bss_start = 0;
  FILHDR fh;
  FILE *input_f, *output_f;
  SCNHDR sc;
  char *data, *strings;
  SYMENT *sym;
  RELOC *relocs;
  int strsz, i;
  dxe_header dh;

  if (argc < 4)
  {
    printf("Usage: dxegen [-d] output.dxe symbol input.o [input2.o ... -lgcc -lc]\n");
    return 1;
  }
  if (!strcmp(argv[1],"-d"))
  {
    debug = 1;
    argc--;
    argv++;
  }

  input_f = fopen(argv[3], "rb");
  if (!input_f)
  {
    perror(argv[3]);
    return 1;
  }

  fread(&fh, 1, FILHSZ, input_f);
  dosswap(&fh, "sslllss");
  if (fh.f_nscns != 1 || argc > 4)
  {
    char command[10*1024], *libdir;
    fclose(input_f);

#ifdef DXE_LD
    strcpy(command, DXE_LD);
#else
    strcpy(command, "ld");
#endif
    strcat(command, " -X -S -r -o dxe__tmp.o -L");
    libdir = getenv("DXE_LD_LIBRARY_PATH");
    if (libdir)
      strcat(command, libdir);
    else
    {
      libdir = getenv("DJDIR");
      if (!libdir)
      {
	fprintf(stderr, "Error: neither DXE_LD_LIBRARY_PATH nor DJDIR are set in environment\n");
	exit(1);
      }
      strcat(command, libdir);
      strcat(command, "/lib");
    }
    strcat(command, " ");
    for(i=3; argv[i]; i++)
    {
      strcat(command, argv[i]);
      strcat(command, " ");
    }
    strcat(command," -T dxe.ld ");

    if (debug)
       printf ("%s\n",command);

    i = system(command);
    if(i)
      return i;

    input_f = fopen("dxe__tmp.o", "rb");
    if (!input_f)
    {
      perror(argv[3]);
      return 1;
    }
    else
      atexit(exit_cleanup);

    fread(&fh, 1, FILHSZ, input_f);
    dosswap(&fh, "sslllss");
    if (fh.f_nscns != 1)
    {
      printf("Error: input file has more than one section; use -M for map\n");
      return 1;
    }
  }

  fseek(input_f, fh.f_opthdr, 1);
  fread(&sc, 1, SCNHSZ, input_f);
  dosswap(&sc, "8llllllssl");

  dh.magic = DXE_MAGIC;
  dh.symbol_offset = -1;
  dh.element_size = sc.s_size;
  dh.nrelocs = sc.s_nreloc;

  data = malloc(sc.s_size);
  fseek(input_f, sc.s_scnptr, 0);
  fread(data, 1, sc.s_size, input_f);

  sym = malloc(sizeof(SYMENT)*fh.f_nsyms);
  fseek(input_f, fh.f_symptr, 0);
  fread(sym, fh.f_nsyms, SYMESZ, input_f);
  fread(&strsz, 1, 4, input_f);
  dosswap(&strsz, "l");
  strings = malloc(strsz);
  fread(strings+4, 1, strsz-4, input_f);
  strings[0] = 0;
  for (i=0; i<fh.f_nsyms; i++)
  {
    char tmp[9], *name;
    if (sym[i].e.e.e_zeroes)
    {
      dosswap(sym+i, "8lscc");
      memcpy(tmp, sym[i].e.e_name, 8);
      tmp[8] = 0;
      name = tmp;
    }
    else
    {
      dosswap(sym+i, "lllscc");
      name = strings + sym[i].e.e.e_offset;
    }
#if 0
    printf("[%3d] 0x%08x 0x%04x 0x%04x %d %s\n",
	   i,
	   sym[i].e_value,
	   sym[i].e_scnum & 0xffff,
	   sym[i].e_sclass,
	   sym[i].e_numaux,
	   name
	   );
#endif
    if (sym[i].e_scnum == 0)
    {
      printf("Error: object contains unresolved external symbols (%s)\n", name);
      errors ++;
    }
    if (strncmp(name, argv[2], strlen(argv[2])) == 0)
    {
      if (dh.symbol_offset != -1)
      {
	printf("Error: multiple symbols that start with %s (%s)!\n", argv[2], name);
	errors++;
      }
      dh.symbol_offset = sym[i].e_value;
    }
    else if (strcmp(name, ".bss") == 0 && !bss_start)
    {
      bss_start = sym[i].e_value;
/*      printf("bss_start 0x%x\n",bss_start); */
      memset(data+bss_start, 0, sc.s_size - bss_start);
    }
    i += sym[i].e_numaux;
  }

  if (dh.symbol_offset == -1)
  {
    printf("Error: symbol %s not found!\n", argv[2]);
    errors++;
  }

  relocs = malloc(sizeof(RELOC)*sc.s_nreloc);
  fseek(input_f, sc.s_relptr, 0);
  fread(relocs, sc.s_nreloc, RELSZ, input_f);

#if 0
  /* Don't swap - it's in i386 order already */
  for (i=0; i<sc.s_nreloc; i++)
    dosswap(relocs+i, "lls");
#endif

#if 0
  /* Thus, this won't work except on PCs */
  for (i=0; i<sc.s_nreloc; i++)
    printf("0x%08x %3d 0x%04x - 0x%08x\n",
	   relocs[i].r_vaddr,
	   relocs[i].r_symndx,
	   relocs[i].r_type,
	   *(long *)(data + relocs[i].r_vaddr)
	   );
#endif

  fclose(input_f);
  if (errors)
    return errors;

  output_f = fopen(argv[1], "wb");
  if (!output_f)
  {
    perror(argv[1]);
    return 1;
  }

  for (i=0; i<sc.s_nreloc; i++)
    if(*(char *)(&relocs[i].r_type) == 0x14)	/* Don't do these, they are relative */
      dh.nrelocs--;

  dosswap(&dh, "llll");
  fwrite(&dh, 1, sizeof(dh), output_f);
  fwrite(data, 1, sc.s_size, output_f);
  for (i=0; i<sc.s_nreloc; i++)
    if(*(char *)(&relocs[i].r_type) != 0x14)	/* Don't do these, they are relative */
      fwrite(&(relocs[i].r_vaddr), 1, sizeof(long), output_f);

  fclose(output_f);
  return 0;
}
