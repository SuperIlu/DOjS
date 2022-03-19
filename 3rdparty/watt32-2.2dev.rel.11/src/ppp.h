/*!\file ppp.h
 */
#ifndef _w32_PPP_H
#define _w32_PPP_H

#include <time.h>
#include <sys/socket.h>     /* <sys/wtypes.h> */
#include <net/ppp_defs.h>   /* PPP_MRU etc. */

void ppp_start (int debug);
void ppp_input (const BYTE *inbuf, WORD len);


/* Standard link phases
 */
enum ppp_phase {
     phDead, phEstablish, phAuthenticate,
     phNetwork, phTerminate
   };

/* Standard set of events
 */
enum ppp_event {
     evUp, evDown, evOpen, evClose, evTOp,
     evTOm, evRCRp, evRCRm, evRCA, evRCN,
     evRTR, evRTA, evRUC, evRXJp, evRXJm, evRXR
   };

/* Standard negotiation states
 */
enum xcp_state {
     stInitial, stStarting, stClosed, stStopped,
     stClosing, stStopping, stReqSent, stAckRcvd,
     stAckSent, stOpened, stNoChange
   };

/* Standard set of actions
 */
enum xcp_action {
     acNull, acIrc, acScr, acTls, acTlf, acStr, acSta,
     acSca, acScn, acScj, acTld, acTlu, acZrc, acSer
   };

/* Standard message code numbers
 */
#define CODE_CONF_REQ     1
#define CODE_CONF_ACK     2
#define CODE_CONF_NAK     3
#define CODE_CONF_REJ     4
#define CODE_TERM_REQ     5
#define CODE_TERM_ACK     6
#define CODE_CODE_REJ     7
#define CODE_PROTO_REJ    8
#define CODE_ECHO_REQ     9
#define CODE_ECHO_REP    10
#define CODE_DISCARD_REQ 11


struct ppp_xcp;
struct ppp_state;
struct xcp_type;

typedef void (*xcp_func) (struct ppp_state *state,
                          struct ppp_xcp *xcp,
                          const struct xcp_type *tp,
                          const BYTE *buf, int len);

typedef void (*ppp_func) (struct ppp_state *state,
                          struct ppp_xcp *xcp);

/* Basic information about an option.
 */
struct xcp_type {
       char    *name;            /* Printable name */
       short    type;            /* option number (-1 to end) */
       short    flag;            /* set to ignore default value */
       BYTE     minlen;          /* minimum overall length */
       BYTE     maxlen;          /* maximum overall length */
       DWORD    default_value;   /* default */
       xcp_func validate_req;
       xcp_func validate_nak;
       xcp_func show;
     };

enum os_state {
     osUsable,
     osUnusable
   };

struct option_state {
       DWORD    my_value;             /* sent in Configure-Request */
       DWORD    peer_value;           /* last received */
       enum os_state state;           /* flag for rejects */
     };

struct ppp_xcp {
       const char          *name;     /* name of control protocol */
       enum xcp_state       state;    /* state machine */
       int                  restart;  /* standard restart count */
       time_t               timeout;  /* standard restart timer */
       int                  naks_sent;/* consecutive naks sent */
       ppp_func             deliver;  /* handler */
       struct xcp_type     *types;    /* option types supported here */
       struct option_state *opts;     /* current negotiation state */
       WORD                 proto;    /* PPP protocol ID */
       BYTE                 ident;    /* current ID number */
     };

enum parse_state {
     psOK, psNak,
     psRej, psBad
   };

struct ppp_state {
       enum ppp_phase   phase;        /* link phase */
       enum parse_state parse_state;  /* request parsing state */
       struct ppp_xcp   xcps[2];      /* installed XCPs */
#define XCP_LCP  0
#define XCP_IPCP 1
       int    timeout_period;         /* value for restart timer */
       int    mlen;                   /* packet length */
       DWORD  mymagic, hismagic;      /* magic numbers (for echo) */
       BYTE  *bp, *up;                /* pointers to packet buffers */
       BYTE  *inbuffer;
       BYTE   outbuffer [PPP_MRU];
     };

#endif  /* _w32_PPP_H */
