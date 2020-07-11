/* 
 *    This a little on the complex side, but I couldn't think of any 
 *    other way to do it. I was getting fed up with having to rewrite 
 *    my asm code every time I altered the layout of a C struct, but I 
 *    couldn't figure out any way to get the asm stuff to read and 
 *    understand the C headers. So I made this program. It includes 
 *    allegro.h so it knows about everything the C code uses, and when 
 *    run it spews out a bunch of #defines containing information about 
 *    structure sizes which the asm code can refer to.
 */


#define USE_CONSOLE

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

/* This #define tells dzcomm.h that this is source file for the library and
 * not to include things the are only for the users code.
 */
#define DZCOMM_LIB_SRC_FILE

#include "dzcomm.h"
#include "dzcomm/dzintern.h"

typedef struct {
   char *name;
   int value;
}offset_entry_t;

offset_entry_t list[] = {
#ifdef DZCOMM_DJGPP
  {"##DZCOMM_DJGPP", 0},
#endif
#ifdef DZCOMM_WATCOM
  {"##DZCOMM_WATCOM", 0},
#endif
#ifdef DZCOMM_DOS
  {"##DZCOMM_DOS", 0},
#endif
#ifdef DZCOMM_WINDOWS
  {"##DZCOMM_WINDOWS", 0},
#endif
#ifdef DZCOMM_LINUX
  {"##DZCOMM_LINUX", 0},
#endif
#ifdef DZCOMM_NO_ASM
  {"##DZCOMM_NO_ASM", 0},
#endif
  {"NEWLINE", 0},
#ifdef DZCOMM_DOS
  {"IRQ_SIZE",    (int)sizeof(_DZ_IRQ_HANDLER)},
  {"IRQ_HANDLER", (int)offsetof(_DZ_IRQ_HANDLER, handler)},
  {"IRQ_NUMBER",  (int)offsetof(_DZ_IRQ_HANDLER, number)},
  {"IRQ_OLDVEC",  (int)offsetof(_DZ_IRQ_HANDLER, old_vector)},
  {"NEWLINE", 0},
  {"DPMI_AX",    (int)offsetof(__dpmi_regs, x.ax)},
  {"DPMI_BX",    (int)offsetof(__dpmi_regs, x.bx)},
  {"DPMI_CX",    (int)offsetof(__dpmi_regs, x.cx)},
  {"DPMI_DX",    (int)offsetof(__dpmi_regs, x.dx)},
  {"DPMI_SP",    (int)offsetof(__dpmi_regs, x.sp)},
  {"DPMI_SS",    (int)offsetof(__dpmi_regs, x.ss)},
  {"DPMI_FLAGS", (int)offsetof(__dpmi_regs, x.flags)},
  {"NEWLINE", 0},
#endif
#ifdef DZCOMM_ASM_USE_FS
  {"#define USE_FS",     0},
  {"#define FSEG %fs:",  0},
#else
  {"#define FSEG",       0},
#endif
  {"NEWLINE", 0},

#ifdef DZCOMM_ASM_PREFIX
#define PREFIX    DZCOMM_ASM_PREFIX "##"
#else
#define PREFIX    ""
#endif
#ifdef DZCOMM_WATCOM
  {"#define FUNC(name)            .globl " PREFIX "name ; nop ; _align_ ; " PREFIX "name:", 0},
#else
  {"#define FUNC(name)            .globl " PREFIX "name ; _align_ ; " PREFIX "name:", 0},
#endif
  {"#define GLOBL(name)           " PREFIX "name", 0},
  {"NEWLINE", 0},
  {NULL, 0}
  };

int main(int argc, char *argv[])
{
   offset_entry_t *p;
   FILE *f;
   int x, y;

   if (argc < 2) {
      fprintf(stderr, "Usage: %s <output file>\n", argv[0]);
      return 1;
   }

   printf("writing structure offsets into %s...\n", argv[1]);

   f = fopen(argv[1], "w");
   if (f == 0) {
      fprintf(stderr, "%s: can not open file %s\n", argv[0], argv[1]);
      return 1;
   }

   fprintf(f, "/* Dzcomm " DZCOMM_VERSION_STR ", " DZCOMM_PLATFORM_STR " */\n");
   fprintf(f, "/* automatically generated structure offsets for use by asm code */\n\n");


   p = list;
   while (p->name != NULL) {
      if (p->name[0] == '#') {
	 if (p->name[1] == '#') {
	    fprintf(f, "#ifndef %s\n#define %s\n#endif\n\n", p->name+2, p->name+2);
	 }
	 else fprintf(f, "%s\n", p->name);
      }
      else {
         fprintf(f, "#define %s %d\n", p->name, p->value);
      }
      p++;
   }

   if (ferror(f)) {
      fprintf(stderr, "%s: cannot write file %s\n", argv[0], argv[1]);
      return 1;
   }

   if (fclose(f)) {
      fprintf(stderr, "%s: cannot close file %s\n", argv[0], argv[1]);
      return 1;
   }

   return 0;
}

#ifdef ALLEGRO_H
END_OF_MAIN();
#endif
