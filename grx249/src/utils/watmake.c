/**
 ** watmake.c ---- generates makefile.wat from depend.dj2
 **
 ** Copyright (c) 1998 Hartmut Schirmer & Gary Sands
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

/*
** NOTE: The src/depend.dj2 file in the GRX v2.3 archive is
**       a dummy file -- If you want to use this utility, run
**
**            ...\SRC> make -f makefile.dj2 dep
**
**       to create a real dependency file for DJGPP v2
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define DJGPP_DEP "depend.dj2"
#define WATC_MAKE "makefile.wat"

#define DBGPRINT "dbgprint.obj"

typedef struct _objs {
  char *name;
  struct _objs *next;
} OBJLIST;

char *WatcomFiles[] = {
   "utils\\watcom32",
   NULL
};

char *IgnoredFiles[] = {
  NULL
};

OBJLIST *objs = NULL;

char *strgsave(char *s) {
  int len = strlen(s);
  char *ns = malloc(sizeof(char)*(len+1));
  assert(ns != NULL);
  strcpy(ns,s);
  return ns;
}

void double_entry(char *name) {
  fprintf(stderr, "object file %s multiple defined !\n", name);
  exit(1);
}

void add2objlist(OBJLIST **root, char *ns, char **ignore) {
  OBJLIST *p, *q, *neu = NULL;
  int scmp;

  if (ignore) {
    while (*ignore) {
      if (strcmp(*ignore, ns) == 0) return;
      ++ignore;
    }
  }
  neu = malloc(sizeof(OBJLIST));
  assert(neu != NULL);
  neu->name = strgsave(ns);
  neu->next = NULL;
  if (*root == NULL) {
    *root = neu;
    return;
  } else
  scmp = strcmp(ns, (*root)->name);
  if (scmp < 0) {
    neu ->next = *root;
    *root = neu;
    return;
  }
  if (scmp == 0) double_entry(ns);
  p = *root;
  q = p->next;
  while (q) {
    scmp = strcmp(ns, q->name);
    if (scmp < 0) {
      neu->next = q;
      p->next = neu;
      return;
    }
    if (scmp == 0) double_entry(ns);
    p = q;
    q = q->next;
  }
  p->next = neu;
}

void print_rule(FILE *makefile, char *n) {
     char *cp;

     cp = strrchr(n, '\\');
     if (!cp) cp = n; else ++cp;
     fprintf(makefile, ".\\%s.obj : .\\",cp);
     cp = n;
     if (strncmp("..\\",cp,3) == 0) {
       cp += 3;
     } else {
       fprintf(makefile, "src\\");
     }
     fprintf(makefile, "%s.c .AUTODEPEND\n", cp);
     fprintf(makefile, "\t$(CC) $[@ $(CC_OPTS)\n\n");
}

int main(void) {
  FILE *makefile, *depend;
  OBJLIST *olp;
  char *cp, **cpp;
  int len, first, nlen;

  cpp = WatcomFiles;
  while (*cpp) {
    add2objlist(&objs, *cpp, NULL);
    cpp++;
  }

  depend = fopen(DJGPP_DEP, "rt");
  assert(depend != NULL);
  while (!feof(depend)) {
    char line[150];
    if (fgets(line, 150-4, depend) != line) break;
    cp = strchr(line, ':');
    if (cp) {
      *cp = '\0';
      cp = strrchr(line, '.');
      if (cp)
	*cp = '\0';
      cp = strchr(line, '/');
      while (cp) {
	*cp = '\\';
	cp = strchr(cp, '/');
      }
      add2objlist(&objs, line, IgnoredFiles);
    }
  }
  fclose(depend);
  makefile = fopen(WATC_MAKE, "wt");
  assert(makefile != NULL);
  fprintf(makefile, "!define BLANK \"\"\n\n"
		    "## Object Files\n");
  len = fprintf(makefile, "OBJS =");
  olp = objs;
  while (olp != NULL) {
     cp = strrchr(olp->name, '\\');
     if (!cp) cp = olp->name; else ++cp;
     nlen = 3 + strlen(cp) + 4;
     if (nlen+len > 75) {
       fprintf(makefile, " &\n");
       len = 0;
     }
     len += fprintf(makefile, " .\\%s.obj", cp);
     olp = olp->next;
  }
  fprintf(makefile, "\n");
  fprintf(makefile, "!ifdef DEBUG\n"
		    "OBJS += .\\" DBGPRINT "\n"
		    "!endif\n"
		    "OBJS += .AUTODEPEND\n\n");
  fprintf(makefile, "## implicit rules do not seem to work with wmake - it complains about\n");
  fprintf(makefile, "## no default action???\n");
  fprintf(makefile, ".c.obj:\n");
  fprintf(makefile, "\t$(CC) $[@ $(CC_OPTS)\n\n");
  fprintf(makefile, "## Rules\n");
  olp = objs;
  while (olp != NULL) {
     print_rule(makefile, olp->name);
     olp = olp->next;
  }
  print_rule(makefile, DBGPRINT);

  fprintf(makefile, "## wat32mak.lb1 is a text file with the names of all the object files\n");
  fprintf(makefile, "## this gets around DOS command line length limit\n\n");
  fprintf(makefile, "$(GRXLIB) : $(OBJS)\n"
		    "  %%create wat32mak.lb1\n");
  len = fprintf(makefile, "!ifneq BLANK \"");
  olp = objs;
  first = 1;
  while (olp != NULL) {
     cp = strrchr(olp->name, '\\');
     if (!cp) cp = olp->name; else ++cp;
     if (!first) {
       fprintf(makefile, " ");
       ++len;
     }
     first = 0;
     nlen = strlen(cp) + 4;
     if (len+nlen > 75) {
       fprintf(makefile, "&\n");
       len = 0;
     }
     len += fprintf(makefile, "%s.obj", cp);
     olp = olp->next;
  }
  fprintf(makefile, "\"\n");
  len = fprintf(makefile, " @for %%i in (");
  olp = objs;
  first = 1;
  while (olp != NULL) {
     cp = strrchr(olp->name, '\\');
     if (!cp) cp = olp->name; else ++cp;
     if (!first) {
       fprintf(makefile, " ");
       ++len;
     }
     first = 0;
     nlen = strlen(cp) + 4;
     if (len+nlen > 75) {
       fprintf(makefile, "&\n");
       len = 0;
     }
     len += fprintf(makefile, "%s.obj", cp);
     olp = olp->next;
  }
  if (len > 40) fprintf(makefile, "&\n");
  fprintf(makefile, ") do @%%append wat32mak.lb1 +'%%i'\n");
  fprintf(makefile, "!endif\n");
  fprintf(makefile, "!ifdef DEBUG\n");
  fprintf(makefile, "@for %%i in (" DBGPRINT ") do @%%append wat32mak.lb1 +'%%i'\n");
  fprintf(makefile, "!endif\n");
  fprintf(makefile, "!ifneq BLANK \"\"\n");
  fprintf(makefile, " @for %%i in () do @%%append wat32mak.lb1 +'%%i'\n");
  fprintf(makefile, "!endif\n");
  fprintf(makefile, " $(LIB) $(LIB_OPTS) $(GRXLIB) @wat32mak.lb1\n");

  fclose(makefile);
  return 0;
}
