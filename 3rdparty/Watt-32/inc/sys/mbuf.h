#ifndef __SYS_MBUF_H
#define __SYS_MBUF_H

/* moved from <sys/cdefs.h>
 */
struct mbuf {
       struct mbuf  *next;    /* Links mbufs belonging to single packets */
       struct mbuf  *anext;   /* Links packets on queues */
       unsigned      size;    /* Size of associated data buffer */
       int           refcnt;  /* Reference count */
       struct mbuf  *dup;     /* Pointer to duplicated mbuf */
       char         *data;    /* Active working pointers */
       unsigned      cnt;
     };

#endif
