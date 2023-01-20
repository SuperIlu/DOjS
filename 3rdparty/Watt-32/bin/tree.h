#ifndef __TREE_H
#define __TREE_H

#ifdef __HIGHC__
#  define MAX(x,y)  _max(x,y)
#  define MIN(x,y)  _min(x,y)
#else
#  define MAX(x,y)  ((x) > (y) ? (x) : (y))
#  define MIN(x,y)  ((x) < (y) ? (x) : (y))
#endif

#define IMPOSSIBLE_HOLE 33   /* = 1 + number of bits in IP address */
#define FALSE           0
#define TRUE            1

typedef char bool;

struct net_tree {
       unsigned long    nt_addr;        /* IP address this node */
       short            nt_netbits;     /* # net bits */
       char            *nt_name;        /* DNS name of this net */
       short            nt_hostbits;    /* # of host bits in use */
       struct net_tree *nt_zeroptr;     /* ptr to left subtree */
       short            nt_zerohole;    /* max hole size in bits to left */
       struct net_tree *nt_oneptr;      /* ptr to right subtree */
       short            nt_onehole;     /* max hole size in bits to right */
       struct net_tree *nt_parentptr;   /* ptr to parent node */
     };

typedef struct net_tree TREE;

extern TREE   root;
extern char  *errormsg;

extern int    get_syms  (void);
extern void   print_tree(TREE *, FILE *);
extern TREE  *alloc_net (int, TREE *, DWORD *);
extern TREE  *find_net  (TREE *, DWORD);
extern TREE  *grow_net  (TREE *);
extern void   del_net   (TREE *);
extern DWORD  ddtolong  (char *);
extern char  *longtodd  (DWORD);

#endif
