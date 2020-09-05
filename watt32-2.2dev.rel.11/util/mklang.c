#include <stdio.h>
#include <stdlib.h>

#include "sysdep.h"

/* Supported language translations: Norwegian, Swedish, German
 */
static const char *languages[] = { "no", "sv", "de" };

static void Usage (void)
{
  fprintf (stderr, "syntax: mklang C-files ... | @resp\n");
  exit (1);
}

int main (int argc, char **argv)
{
  if (argc < 2)
     Usage();

  while (argc > 1)
  {
    int   i;
    long  line = 0;
    char  buf[512];
    const char *ext;
    const char *fname = argv[argc-1];
    FILE       *fin   = fopen (fname, "r");

    if (!fin)
    {
      fprintf (stderr, "Cannot read `%s'\n", fname);
      return (0);
    }
    ext = fname + strlen(fname) - 2;
    if (strnicmp(ext,".c",2))
    {
      fprintf (stderr, "`%s' is not a C-file\n", fname);
      return (0);
    }

    while (fgets(buf,sizeof(buf),fin))
    {
      char *cp, *cp1;

      if (line++ == 0)
         printf ("#\n# File %s\n#\n", fname);

      cp = strstr (buf, "_LANG(\"");
      if (!cp)
         continue;

      cp += 6;
      cp1 = strstr (cp, "\")");
      if (!cp1)
         continue;

      *(++cp1) = '\0';

      printf ("# line %ld\n%s\n", line, cp);
      for (i = 0; i < sizeof(languages)/sizeof(languages[0]); i++)
          printf ("%.2s:\n", languages[i]);
      puts ("");
    }
    fclose (fin);
    argc--;
  }
  return (0);
}

