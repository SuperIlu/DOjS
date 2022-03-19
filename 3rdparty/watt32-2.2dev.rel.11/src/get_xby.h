/*!\file get_xby.h
 */
#ifndef _w32_GET_X_BY_Y_H
#define _w32_GET_X_BY_Y_H

#ifndef _REENTRANT
#define _REENTRANT   /* pull in the *_r() prototypes */
#endif

#ifndef __NETDB_H
#include <netdb.h>
#endif

#define MAX_SERV_ALIASES    5
#define MAX_HOST_ALIASES    5
#define MAX_NETENT_ALIASES  5
#define MAX_PROTO_ALIASES   0
#define MAX_CACHE_LIFE      (15*60)

#define ReadHostsFile     W32_NAMESPACE (ReadHostsFile)
#define GetHostsFile      W32_NAMESPACE (GetHostsFile)
#define CloseHostFile     W32_NAMESPACE (CloseHostFile)

#define ReadHosts6File    W32_NAMESPACE (ReadHosts6File)
#define CloseHost6File    W32_NAMESPACE (CloseHost6File)
#define GetHosts6File     W32_NAMESPACE (GetHosts6File)

#define ReadServFile      W32_NAMESPACE (ReadServFile)
#define GetServFile       W32_NAMESPACE (GetServFile)
#define CloseServFile     W32_NAMESPACE (CloseServFile)

#define ReadProtoFile     W32_NAMESPACE (ReadProtoFile)
#define GetProtoFile      W32_NAMESPACE (GetProtoFile)
#define CloseProtoFile    W32_NAMESPACE (CloseProtoFile)

#define ReadNetworksFile  W32_NAMESPACE (ReadNetworksFile)
#define CloseNetworksFile W32_NAMESPACE (CloseNetworksFile)
#define GetNetFile        W32_NAMESPACE (GetNetFile)

#define InitEthersFile    W32_NAMESPACE (InitEthersFile)
#define ReadEthersFile    W32_NAMESPACE (ReadEthersFile)
#define GetEthersFile     W32_NAMESPACE (GetEthersFile)
#define GetEtherName      W32_NAMESPACE (GetEtherName)
#define NumEtherEntries   W32_NAMESPACE (NumEtherEntries)

#define DumpHostsCache    W32_NAMESPACE (DumpHostsCache)
#define DumpHosts6Cache   W32_NAMESPACE (DumpHosts6Cache)
#define DumpEthersCache   W32_NAMESPACE (DumpEthersCache)

/*!\struct _hostent
 * Internal hostent structure.
 */
struct _hostent {
  char            *h_name;                         /* official name of host */
  char            *h_aliases[MAX_HOST_ALIASES+1];  /* hostname alias list */
  int              h_num_addr;                     /* how many real addr below */
  DWORD            h_address[MAX_ADDRESSES+1];     /* addresses (network order) */
  DWORD            h_real_ttl;                     /* TTL from udp_dom.c */
  time_t           h_timeout;                      /* cached entry expiry time */
  struct _hostent *h_next;                         /* next entry (or NULL) */
};

/*!\struct _hostent6
 * Internal hostent6 structure.
 */
struct _hostent6 {
  char             *h_name;                        /* official name of host */
  char             *h_aliases[MAX_HOST_ALIASES+1]; /* hostname alias list */
  int               h_num_addr;                    /* how many real addr below */
  ip6_address       h_address[MAX_ADDRESSES+1];    /* addresses */
  DWORD             h_real_ttl;                    /* TTL from udp_dom.c */
  time_t            h_timeout;                     /* cached entry expiry time */
  struct _hostent6 *h_next;
};

/*!\struct _netent
 * Internal netent structure.
 */
struct _netent {
  char           *n_name;                           /* official name of net */
  char           *n_aliases [MAX_NETENT_ALIASES+1]; /* alias list */
  int             n_addrtype;                       /* net address type */
  DWORD           n_net;                            /* network (host order) */
  struct _netent *n_next;
};

/*!\struct _protoent
 * Internal protoent structure.
 */
struct _protoent {
  char             *p_name;
  char             *p_aliases [MAX_PROTO_ALIASES+1];
  int               p_proto;
  struct _protoent *p_next;
};

/*!\struct _servent
 * Internal servent structure.
 */
struct _servent {
  char            *s_name;
  char            *s_aliases [MAX_SERV_ALIASES+1];
  int              s_port;
  char            *s_proto;
  struct _servent *s_next;
};

extern BOOL     called_from_getai;
extern unsigned netdbCacheLife;

/* These are not in any ./inc headers (<netdb.h> etc.). We export
 * them anyway since they could come in handy. Now, only ./bin/tcpinfo.c
 * uses some of them.
 */

W32_FUNC void        W32_CALL ReadHostsFile    (const char *fname);  /* gethost.c */
W32_FUNC void        W32_CALL ReadServFile     (const char *fname);  /* getserv.c */
W32_FUNC void        W32_CALL ReadProtoFile    (const char *fname);  /* getprot.c */
W32_FUNC void        W32_CALL ReadNetworksFile (const char *fname);  /* getnet.c */
W32_FUNC void        W32_CALL ReadEthersFile   (void);               /* geteth.c */
W32_FUNC int         W32_CALL NumEtherEntries  (void);               /* geteth.c */
W32_FUNC void        W32_CALL InitEthersFile   (const char *fname);  /* geteth.c */

W32_FUNC void        W32_CALL CloseHostFile    (void);
W32_FUNC void        W32_CALL CloseServFile    (void);
W32_FUNC void        W32_CALL CloseProtoFile   (void);
W32_FUNC void        W32_CALL CloseNetworksFile(void);

W32_FUNC const char *W32_CALL GetHostsFile (void);
W32_FUNC const char *W32_CALL GetServFile  (void);
W32_FUNC const char *W32_CALL GetProtoFile (void);
W32_FUNC const char *W32_CALL GetNetFile   (void);
W32_FUNC const char *W32_CALL GetEthersFile(void);
W32_FUNC const char *W32_CALL GetEtherName (const eth_address *eth);

W32_FUNC void        W32_CALL DumpHostsCache (void);
W32_FUNC void        W32_CALL DumpEthersCache (void);

#if defined(USE_BSD_API) && defined(USE_IPV6)  /* gethost6.c */
  W32_FUNC void        W32_CALL ReadHosts6File  (const char *fname);
  W32_FUNC void        W32_CALL CloseHost6File  (void);
  W32_FUNC void        W32_CALL DumpHosts6Cache (void);
  W32_FUNC const char *W32_CALL GetHosts6File   (void);
#endif

#endif
