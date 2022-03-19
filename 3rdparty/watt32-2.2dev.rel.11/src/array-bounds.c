#include <stdlib.h>
#define MAX_PROTO_ALIASES 0

struct _protoent {
  char             *p_name;
  char             *p_aliases [MAX_PROTO_ALIASES+1];
  int               p_proto;
  struct _protoent *p_next;
};

extern struct _protoent *proto0;

void endprotoent (void)
{
  struct _protoent *p, *next;

  for (p = proto0; p; p = next)
  {
    int i;
    for (i = 0; p->p_aliases[i]; i++)
        free (p->p_aliases[i]);
    next = p->p_next;
    free (p->p_name);
    free (p);
  }
}


