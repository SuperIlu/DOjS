#include <stdio.h>
#include <windows.h>

static BOOL CALLBACK callback (LPSTR cp)
{
  printf ("\t callback(): cp: \"%s\", valid: %d\n",
          cp, IsValidCodePage(atoi(cp)));
  return (TRUE);
}

int main (void)
{
  puts ("\nEnumerating codepages:");
  EnumSystemCodePages (callback, CP_INSTALLED);
  return 0;
}