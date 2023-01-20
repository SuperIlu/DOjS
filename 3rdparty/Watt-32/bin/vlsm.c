
/*
** Charley Kline, University of Illinois Computing Services.
** cvk@uiuc.edu
*/

/*
** commands:
**
**   init net mask
**   alloc name size [specific-request]
**   blockalloc prefix size n
**   grow net new-size
**   free net
**   stats
**   load filename
**   write
**   list
**   quit
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <tcp.h>

#include "tree.h"

#if defined(__GNUC__)
#pragma GCC diagnostic ignored "-Wchar-subscripts"
#endif

static bool  debugflag = FALSE;
static char *gerror;
static char  gcurfile[256];
static bool  gloaded = FALSE;
static bool  gmodified = FALSE;
static char  progPath[256];

static TREE  groot;

/*
 * Fix trouble with multiple defined symbols,
 */
#define perror(str)   perror_s(str)
#define strerror(err) strerror_s(err)


/*
 * Command line parsing globals.
 */
char  cmdbuf[512];
int   symc;
char *symv[24];


/*
 * Prototypes
 */
void cmd_init  (void);
void cmd_alloc (void);
void cmd_block (void);
void cmd_grow  (void);
void cmd_free  (void);
void cmd_load  (void);
void cmd_write (void);
void cmd_help  (void);
void cmd_quit  (void);
void cmd_list  (void);
void cmd_stats (void);
int  init_tree (char *addr, int netbits);
int  parmscan  (char **p);

struct cmdentry {
       char  *c_name;         /* ASCII command name */
       void (*c_func)(void);  /* Pointer to handler, NULL if unimplemented */
     };

struct cmdentry * wordsym (char *s);

static struct cmdentry cmdtab[] = {
       { "INIT",          cmd_init  },
       { "ALLOCATE",      cmd_alloc },
       { "BLOCKALLOCATE", NULL      },
       { "GROW",          cmd_grow  },
       { "FREE",          cmd_free  },
       { "LOAD",          cmd_load  },
       { "STATS",         NULL      },
       { "WRITE",         cmd_write },
       { "LIST",          cmd_list  },
       { "HELP",          cmd_help  },
       { "QUIT",          cmd_quit  },
       {  NULL,           NULL      },
     };


/*
 * Cheat sheet for number of hosts for a given number of netbits.
 */
u_long netsize[] = {
       0,0,0,0,0,0,0,0,  /* 0-7 make no sense unless class A's are aggregated! */
       16777214,   /* 8  */
       8388606,    /* 9  */
       4194302,    /* 10 */
       2097150,    /* 11 */
       1048574,    /* 12 */
       524286,     /* 13 */
       262142,     /* 14 */
       131070,     /* 15 */
       65534,      /* 16 */
       32766,      /* 17 */
       16382,      /* 18 */
       8190,       /* 19 */
       4094,       /* 20 */
       2046,       /* 21 */
       1022,       /* 22 */
       510,        /* 23 */
       254,        /* the beloved 24 */
       126,        /* 25 */
       62,         /* 26 */
       30,         /* 27 */
       14,         /* 28 */
       6,          /* 29 */
       2,          /* 30 */
       0, 0        /* 31 and 30 make no sense */
     };



int MS_CDECL main (int argc, char **argv)
{
  struct cmdentry *cmdptr;
  char  *p;

  strcpy (progPath, argv[0]);
#if defined(__DJGPP__) || defined(__CYGWIN__)
  p = strrchr (progPath, '/');
#else
  p = strrchr (progPath, '\\');
#endif
  *p = '\0';


  puts ("\n>>> Variable-length Subnet Management Program.");
  puts (">>> Charley Kline, University of Illinois at Urbana-Champaign.");

  if (argc > 1 && !strcmp(argv[1],"-d"))
     debugflag = 1;

  for (;;)
  {
    if (get_syms() == EOF)
    {
      cmd_quit();
      break;
    }
    if ((cmdptr = wordsym(symv[0])) == NULL)
       continue;
    if (cmdptr->c_func)
         (*cmdptr->c_func)();
    else printf(">>> %s Command unimplemented.\n", symv[0]);
  }
  return (0);
}


void cmd_quit (void)
{
  if (gmodified)
  {
    fprintf (stderr, "!!! WARNING Modified tree has not been saved.\n");
    fprintf (stderr, "!!! Quit again to exit without saving.\n");
    gmodified = FALSE;
  }
  else
    exit (0);
}

void cmd_help (void)
{
  char buf [20 + sizeof(progPath)];
  char *pager = getenv ("PAGER");

  if (pager)
       sprintf(buf, "%s vlms.hlp", pager);
  else sprintf (buf, "more < %s\\vlsm.hlp", progPath);
  system (buf);
}

int bits_in_net (char *net)
{
  u_long network = inet_network (net);

  if (IN_CLASSA(network))
     return (IN_CLASSA_NSHIFT);

  if (IN_CLASSB(network))
     return (IN_CLASSB_NSHIFT);

  if (IN_CLASSC(network))
     return (IN_CLASSC_NSHIFT);

  if (IN_CLASSD(network))
     return (IN_CLASSD_NSHIFT);

  return (0);
}

void cmd_load (void)
{
  FILE *fd;
  char  name[32], net[32];
  int   bits, nlines = 0;
  TREE *p;

  if (symc != 2)
  {
    fprintf (stderr, "!!! Usage: %s filename\n", symv[0]);
    return;
  }

  if (gmodified)
  {
    fprintf (stderr, "!!! WARNING Modified tree has not been saved.\n");
    fprintf (stderr, "!!! Issue command again to overwrite existing tree.\n");
    gmodified = FALSE;
    return;
  }

  /* Read in the list of current subnets and force them
   * into the tree at their proper positions.
   */
  if ((fd = fopen(symv[1], "r")) == NULL)
  {
    fprintf(stderr, "!!! Cannot open %s\n", symv[1]);
    return;
  }

  fscanf (fd, "%s %s #%d\n", name, net, &bits);

  if (!init_tree(net, bits))
  {
    puts (gerror);
    exit (16);
  }

  while (fscanf(fd, "%s %s #%d\n", name, net, &bits) == 3)
  {
    DWORD x;
    x = ddtolong (net);
    p = alloc_net (bits, &groot, &x);
    if (p == NULL)
    {
      printf ("Allocation error, %s %s %s\n", errormsg, name, net);
      exit (16);
    }
    p->nt_name = strdup (name);
    nlines++;
  }
  fclose (fd);
  gloaded = TRUE;
  strcpy (gcurfile, symv[1]);
  printf (">>> %s loaded. %d allocations, largest hole %d bits.\n",
          symv[1], nlines, MAX(groot.nt_zerohole, groot.nt_onehole));
}

void cmd_init (void)
{
  int bits;

  if (symc != 3)
  {
    fprintf (stderr, "!!! Usage: %s net_ip_address netbits\n", symv[0]);
    return;
  }

  if (gloaded)
  {
    fprintf(stderr, "!!! Can't overwrite existing tree.\n");
    return;
  }

  bits = atoi (symv[2]);
  if (bits < 8 || bits > 31)
  {
    fprintf (stderr, "!!! netbits of %s makes no sense.\n", symv[2]);
    return;
  }

  if (!init_tree(symv[1], bits))
  {
    puts (gerror);
    exit (16);
  }

  gloaded = TRUE;
  gmodified = TRUE;
  gcurfile[0] = '\0';
}


void cmd_write (void)
{
  FILE *fd;
  char *filename = NULL;

  if (symc > 2)
  {
    fprintf (stderr, "!!! Usage: %s [new-filename]\n", symv[0]);
    return;
  }

  if (!gloaded)
  {
    fprintf (stderr, "!!! No tree is available to write.\n");
    return;
  }

  if (symc == 2)
       filename = symv[1];
  else if (gcurfile[0])
       filename = gcurfile;

  if (filename == NULL)
  {
    fprintf (stderr, "!!! Usage: %s filename\n", symv[0]);
    return;
  }

  /* Attempt to open the file.
   */
  if ((fd = fopen(filename, "wt")) == NULL)
  {
    perror (filename);
    return;
  }

  /* Print the root information
   */
  fprintf (fd, "ROOT-OF-TREE    %-24s #%d\n",
  longtodd (groot.nt_addr), groot.nt_netbits);

  /* Print the tree itself.
   */
  print_tree (&groot, fd);

  fclose (fd);
  gmodified = FALSE;
  printf (">>> %s Written.\n", filename);
}


void cmd_list (void)
{
  if (symc > 1)
  {
    fprintf (stderr, "!!! Usage: %s\n", symv[0]);
    return;
  }

  /* Print the root information
   */
  printf ("ROOT-OF-TREE    %-24s #%d\n",
          longtodd (groot.nt_addr), groot.nt_netbits);

  /* Print the tree itself.
   */
  print_tree (&groot, stdout);
}


void cmd_alloc (void)
{
  int   nhosts, nbits;
  DWORD x;
  TREE *p;

  if (symc < 3 || symc > 4)
  {
    fprintf (stderr, "!!! Usage: %s net-name #-of-hosts [specific-IP]\n", symv[0]);
    return;
  }

  if (!gloaded)
  {
    fprintf (stderr, "!!! No tree is available to modify.\n");
    return;
  }

  nhosts = atoi(symv[2]);
  if (nhosts < 1)
  {
    fprintf (stderr, "!!! Number of hosts desired makes no sense\n");
    fprintf (stderr, "!!! Usage: %s net-name #-of-hosts [specific-IP]\n",symv[0]);
    return;
  }
  nbits = 32;
  nhosts += 2;   /* net address plus broadcast address */
  while (nhosts)
  {
    nhosts >>= 1;
    nbits--;
  }

  if (symc == 4)      /* specific request */
  {
    x = ddtolong (symv[3]);
    if (x == (DWORD)-1)
    {
      fprintf (stderr, "!!! Address %s makes no sense.\n", symv[3]);
      return;
    }
    p = alloc_net (nbits, &groot, &x);
  }
  else
    p = alloc_net (nbits, &groot, NULL);


  if (p == NULL)
  {
    fprintf (stderr, "!!! Could not allocate: %s\n", errormsg);
    return;
  }

  p->nt_name = strdup (symv[1]);
  gmodified = TRUE;

  printf (">>> %s/%d has been allocated.\n", longtodd(p->nt_addr), p->nt_netbits);

  x = 0xffffffff << (32 - p->nt_netbits);
  printf (">>> Subnet mask for this net is %s.\n", longtodd(x));
  x = ~x | p->nt_addr;
  printf (">>> Broadcast address for this net is %s.\n", longtodd(x));
}


void cmd_grow (void)
{
  DWORD x;
  TREE *p;

  if (symc != 2)
  {
    fprintf (stderr, "!!! Usage: %s network\n", symv[0]);
    return;
  }

  if (!gloaded)
  {
    fprintf (stderr, "!!! No tree is available to modify.\n");
    return;
  }

  if ((x = ddtolong(symv[1])) == (DWORD)-1)
  {
    fprintf (stderr, "!!! Address %s makes no sense.\n", symv[1]);
    return;
  }

  if ((p = find_net(&groot, x)) == NULL)
  {
    fprintf (stderr, "!!! Couldn't find that net, %s\n", symv[1]);
    return;
  }

  if ((p = grow_net(p)) == NULL)
  {
    fprintf (stderr, "!!! Couldn't grow the net.\n");
    return;
  }

  printf (">>> %s has been grown to %d bits (room for %lu hosts).\n",
          longtodd(p->nt_addr), p->nt_netbits, netsize[p->nt_netbits]);

  x = 0xffffffff << (32 - p->nt_netbits);
  printf (">>> The new subnet mask is %s.\n", longtodd(x));
  x = ~x | p->nt_addr;
  printf (">>> The new broadcast address is %s.\n", longtodd(x));
  gmodified = TRUE;
}


void cmd_free (void)
{
  DWORD x;
  TREE *p;

  if (symc != 2)
  {
    fprintf (stderr, "!!! Usage: %s network\n", symv[0]);
    return;
  }

  if (!gloaded)
  {
    fprintf (stderr, "!!! No tree is available to modify.\n");
    return;
  }

  if ((x = ddtolong(symv[1])) == (DWORD)-1)
  {
    fprintf (stderr, "!!! Address %s makes no sense.\n", symv[1]);
    return;
  }

  if ((p = find_net(&groot, x)) == NULL)
  {
    fprintf (stderr, "!!! Couldn't find that net, %s\n", symv[1]);
    return;
  }

  del_net (p);

  printf (">>> %s has been returned to the free pool.\n", symv[1]);
  gmodified = TRUE;
}


int init_tree (char *addr, int netbits)
{
  DWORD x;
  TREE *p;

  /* Initialize tree.
   */
  groot.nt_addr = ddtolong (addr);
  if (groot.nt_addr == (DWORD)-1)
  {
    gerror = "Invalid IP address";
    return (FALSE);
  }
  if (netbits < 8 || netbits > 28)
  {
    gerror = "Invalid number of netbits";
    return (FALSE);
  }
  groot.nt_netbits   = netbits;
  groot.nt_name      = NULL;
  groot.nt_hostbits  = 0;
  groot.nt_zeroptr   = groot.nt_oneptr = NULL;
  groot.nt_zerohole  = groot.nt_onehole = groot.nt_netbits + 1;
  groot.nt_parentptr = NULL;

  /*
   * Preallocate "naughty" subnets.
   * Subnets may not have all zero nor all one bits to disambiguate
   * broadcast and net addresses from the un-subnetted equivalents.
   * These placeholders get NULL names so they are not written out
   * when the tree is saved. They get recreated in-core each time
   * the file is loaded.
   */
  x = ddtolong (addr);
  p = alloc_net (32, &groot, &x);
  p->nt_name = NULL;
  x |= (1L << (32-netbits)) - 1L;
  p = alloc_net (32, &groot, &x);
  p->nt_name = NULL;
  return (TRUE);
}


/*
 * Prompt for a command, read it in, and break it into white-space-separated
 * tokens in the symv array.
 */
int get_syms (void)
{
  char *p;
  int l;

top:
  printf ("\nCmd? ");
  fflush (stdout);

  if (fgets(cmdbuf, sizeof(cmdbuf), stdin) == NULL)
     return (EOF);

  l = strlen (cmdbuf);
  while (strchr("\r\n ", cmdbuf[--l]))
        ;
  cmdbuf[++l] = '\0';

  if (cmdbuf[0] == '#')
     goto top;
  if (cmdbuf[0] == '!')
  {
    system (cmdbuf+1);
    goto top;
  }

  for (p = cmdbuf; *p && isspace(*p); p++)
      ;
  symc = 0;
  symv[symc] = p;
  while (parmscan(&p))
        symv[++symc] = p;
  symv[++symc] = 0;
  return (symc);
}


int parmscan (char **p)
{
  char *q = *p;

  while (*q && !isspace(*q))
        q++;
  if (*q == '\0')
     return (0);

  *q++ = '\0';
  while (*q && isspace(*q))
        q++;
  if (*q == '\0')
     return (0);
  *p = q;
  return (1);
}

/*
 * Find the command table entry whose name is _s_ and return a pointer
 * to it. The command in _s_ is modified by converting it to lower case
 * first. Unambiguous abbreviations of valid commands are recognized.
 * wordsym() prints its own error messages.
 */
struct cmdentry * wordsym (char *s)
{
  struct cmdentry *p;
  struct cmdentry *q = NULL;
  int    matches = 0;
  int    slen = strlen(s);
  char  *r;

  for (r = s; *r; r++)
      if (islower(*r))
         *r = toupper (*r);

  for (p = cmdtab; p->c_name; p++)
      if (strncmp(p->c_name, s, slen) == 0)
      {
        q = p;
        matches++;
      }

  switch (matches)
  {
    case 0:  printf ("%s Command unrecognized.\r\n", s);
             return (NULL);
    case 1:  return (q);
    default: printf ("%s: Ambiguous.\r\n", s);
             return (NULL);
  }
}

DWORD ddtolong (char *s)
{
  u_long c1, c2, c3, c4;
  int    rc = sscanf (s, "%ld.%ld.%ld.%ld", &c1, &c2, &c3, &c4);

  if (rc != 4)
     return (DWORD)(-1);
  return (c1 << 24) + (c2 << 16) + (c3 << 8) + c4;
}


char * longtodd (DWORD x)
{
  int c1, c2, c3, c4;
  static char buf[18];

  c1 = x >> 24;
  c2 = (x >> 16) & 0xff;
  c3 = (x >> 8) & 0xff;
  c4 = x & 0xff;
  sprintf (buf, "%d.%d.%d.%d", c1, c2, c3, c4);
  return (buf);
}


/*============================= tree.c ==========================*/

char *errormsg;

/*
 * This is the binary tree variable length subnet allocation
 * algorithm.
 *
 * Charley Kline, University of Illinois Computing Services.
 * cvk@uiuc.edu
 */


/*
 * Allocate the left child node of the given parent. Their IP net
 * address is assigned automatically.
 */
static void alloc_left_kid (TREE *parent)
{
  TREE *p;

  if (parent->nt_zeroptr)
  {
    fprintf (stderr, "*** ALK already has kids\n");
    exit (16);
  }

  p = malloc (sizeof(TREE));
  p->nt_addr      = parent->nt_addr;
  p->nt_netbits   = parent->nt_netbits + 1;
  p->nt_name      = NULL;
  p->nt_hostbits  = 0;
  p->nt_zeroptr   = p->nt_oneptr = NULL;
  p->nt_parentptr = parent;
  p->nt_zerohole  = p->nt_onehole = parent->nt_zerohole + 1;
  parent->nt_zeroptr = p;
  if (debugflag)
     printf ("@@@ ALK created %s/%d\n", longtodd(p->nt_addr), p->nt_netbits);
}

/*
 * Allocate the right child node of the given parent. Their IP net
 * address is assigned automatically.
 */
static void alloc_right_kid (TREE *parent)
{
  TREE *p;

  if (parent->nt_oneptr)
  {
    fprintf (stderr, "*** ARK already has kids\n");
    exit (16);
  }

  p = malloc (sizeof(TREE));
  p->nt_addr      = parent->nt_addr;
  p->nt_netbits   = parent->nt_netbits + 1;
  p->nt_addr     |= 1L << (32 - p->nt_netbits);
  p->nt_name      = NULL;
  p->nt_hostbits  = 0;
  p->nt_zeroptr   = p->nt_oneptr = NULL;
  p->nt_parentptr = parent;
  p->nt_zerohole  = p->nt_onehole = parent->nt_onehole + 1;
  parent->nt_oneptr = p;
  if (debugflag)
     printf ("@@@ ARK created %s/%d\n", longtodd(p->nt_addr), p->nt_netbits);
}

/*
 * Print a tree in /etc/networks format. Recursive.
 */
void print_tree (TREE *p, FILE *fd)
{
  if (p->nt_name)
     fprintf (fd, "%-24s %-15s      #%d\n", p->nt_name,
              longtodd(p->nt_addr), p->nt_netbits);
  else
  {
    if (debugflag)
        printf ("[node] %s/%d %d %d\n", longtodd(p->nt_addr),
                p->nt_netbits, p->nt_zerohole, p->nt_onehole);
    if (p->nt_zeroptr)
       print_tree (p->nt_zeroptr, fd);
    if (p->nt_oneptr)
       print_tree(p->nt_oneptr, fd);
  }
}

/*
 * Allocate a new net with _netbits_ net bits, searching recursively
 * in the tree for the biggest possible place to put it.
 */
TREE * alloc_net (int netbits, TREE *p, DWORD *specific_request)
{
  int  whichway;
  int  holesize;
  TREE *r;

  if (debugflag)
  {
    printf (">>> Request alloc %d-bit net, ", netbits);
    if (specific_request)
         printf ("%s, ", longtodd(*specific_request));
    else printf ("no specific request,");
    printf (" visiting %s/%d\n", longtodd(p->nt_addr), p->nt_netbits);
  }

  /*
   * If this node is free and of the right size, our search is finished.
   * The "hole size" is set to something impossibly large so the tree
   * walk doesn't follow down this way any more.
   */
  if (p->nt_netbits == netbits && !p->nt_zeroptr && !p->nt_oneptr)
  {
    p->nt_zerohole = p->nt_onehole = IMPOSSIBLE_HOLE;
    return (p);
  }

  /*
   * Else figure out in which direction the bigger hole is. Allocate
   * to the left if there are equal size holes in each direction.
   * If we have a specific request, figure out which way in the tree
   * to go in order to go towards that entry, by looking at the next
   * bit in the IP address.
   */
  if (specific_request)
  {
    if (*specific_request & (1L << (31-p->nt_netbits)))
    {
      whichway = 1;
      holesize = p->nt_onehole;
    }
    else
    {
      whichway = 0;
      holesize = p->nt_zerohole;
    }
  }
  else
  {
    if (p->nt_onehole < p->nt_zerohole)
    {
      whichway = 1;
      holesize = p->nt_onehole;
    }
    else
    {
      whichway = 0;
      holesize = p->nt_zerohole;
    }
  }

  if (netbits < holesize)
  {
    errormsg = "No hole that big";
    return (NULL);
  }

  /* Now we know which way to go. Allocate a child node there if
   * necessary, and call recursively to continue the search.
   */
  if (whichway == 0)
  {
    if (p->nt_zeroptr == NULL)
       alloc_left_kid (p);
    r = alloc_net (netbits, p->nt_zeroptr, specific_request);
    p->nt_zerohole = MIN (p->nt_zeroptr->nt_zerohole, p->nt_zeroptr->nt_onehole);
  }
  else
  {
    if (p->nt_oneptr == NULL)
       alloc_right_kid (p);
    r = alloc_net (netbits, p->nt_oneptr, specific_request);
    p->nt_onehole = MIN (p->nt_oneptr->nt_zerohole, p->nt_oneptr->nt_onehole);
  }
  return (r);
}


TREE *find_net (TREE *p, DWORD x)
{
  if (p == NULL)
     return (NULL);

  if (debugflag)
  {
    printf (">>> find_net %s examining ", longtodd(x));
    printf ("%s\n", longtodd(p->nt_addr));
  }

  if (x == p->nt_addr && p->nt_name)
     return (p);

  if (x & (1L << (31-p->nt_netbits)))
       return find_net (p->nt_oneptr, x);
  else return find_net (p->nt_zeroptr, x);
}

TREE * grow_net (TREE *p)
{
  TREE *parent = p->nt_parentptr;

  if (parent == NULL)
  {
    printf ("!!! Can't grow out of top of tree.\n");
    return (NULL);
  }

  /* Can only grow from left-hand branch into right-hand branch.
   */
  if (parent->nt_zeroptr != p)
  {
    printf ("!!! No address space to grow into from here.\n");
    return (NULL);
  }

  /* Can only grow if the adjacent branch of the tree is unallocated.
   */
  if (parent->nt_oneptr)
  {
    printf ("!!! Adjacent address space is already full.\n");
    return (NULL);
  }

  /* Okay, we're going to join with the adjacent hole. Copy our
   * information into the parent and adjust the subnet size.
   */
  parent->nt_addr     = p->nt_addr;
  parent->nt_netbits  = p->nt_netbits - 1;
  parent->nt_name     = p->nt_name;
  parent->nt_hostbits = p->nt_hostbits;
  parent->nt_oneptr   = parent->nt_zeroptr = NULL;
  parent->nt_zerohole = parent->nt_onehole = IMPOSSIBLE_HOLE;
  free (p);

  /* Iterate up the tree, adjusting hole sizes toward the root
   */
  for (p = parent->nt_parentptr; p; p = p->nt_parentptr)
  {
    p->nt_onehole  = MIN (p->nt_oneptr->nt_zerohole, p->nt_oneptr->nt_onehole);
    p->nt_zerohole = MIN (p->nt_zeroptr->nt_zerohole, p->nt_zeroptr->nt_onehole);
  }
  return (parent);
}


void del_net (TREE *p)
{
  TREE *parent;

  if ((parent = p->nt_parentptr) == NULL)
  {
    printf ("!!! Can't delete the root!\n");
    return;
  }

  /* Effect the deletion by updating the parent's hole sizes
   * to reflect a hole where this net was.
   */
  if (parent->nt_zeroptr == p)
  {
    parent->nt_zerohole = p->nt_netbits;
    parent->nt_zeroptr = NULL;
  }
  else
  {
    parent->nt_onehole = p->nt_netbits;
    parent->nt_oneptr = NULL;
  }
  if (p->nt_name)
     free (p->nt_name);
  free (p);

  /* Iterate up the tree, adjusting hole sizes toward the root
   */
  for (p = parent->nt_parentptr; p; p = p->nt_parentptr)
  {
    p->nt_onehole  = MIN (p->nt_oneptr->nt_zerohole, p->nt_oneptr->nt_onehole);
    p->nt_zerohole = MIN (p->nt_zeroptr->nt_zerohole, p->nt_zeroptr->nt_onehole);
  }
}

