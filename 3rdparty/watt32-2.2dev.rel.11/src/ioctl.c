/*!\file ioctl.c
 * BSD ioctlsocket().
 */

/*  BSD sockets functionality for Watt-32 TCP/IP
 *
 *  Copyright (c) 1997-2002 Gisle Vanem <gvanem@yahoo.no>
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. All advertising materials mentioning features or use of this software
 *     must display the following acknowledgement:
 *       This product includes software developed by Gisle Vanem
 *       Bergen, Norway.
 *
 *  THIS SOFTWARE IS PROVIDED BY ME (Gisle Vanem) AND CONTRIBUTORS ``AS IS''
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED.  IN NO EVENT SHALL I OR CONTRIBUTORS BE LIABLE FOR ANY
 *  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 *  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 *  THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *  Version
 *
 *  0.5 : Dec 18, 1997 : G. Vanem - created
 */

#define BSD    /* in order to include SIOxx macros in <sys/ioctl.h> */

#include "socket.h"

#if defined(USE_BSD_API)

static int file_ioctrl  (Socket *socket, long cmd, char *argp);
static int iface_ioctrl (Socket *socket, long cmd, char *argp);
static int arp_ioctrl   (Socket *socket, long cmd, char *argp);
static int waterm_ioctrl(Socket *socket, long cmd, char *argp);

#define NO_CMD(cmd) SOCK_DEBUGF ((", unsupported cmd %d, group %c", \
                                  (int)((cmd) & IOCPARM_MASK),      \
                                  (char)IOCGROUP(cmd)))
#ifdef USE_DEBUG
static const char *get_ioctl_cmd (long cmd);
#endif

#ifndef NUM_IFACES
#define NUM_IFACES 1
#endif

#ifndef ARPHRD_FDDI
#define ARPHRD_FDDI 10
#endif

int W32_CALL ioctlsocket (int s, long cmd, char *argp)
{
  Socket *socket = _socklist_find (s);

  SOCK_PROLOGUE (socket, "\nioctlsocket:%d", s);

  SOCK_DEBUGF ((", %s", get_ioctl_cmd(cmd)));

  switch (IOCGROUP(cmd))
  {
    case 'f':
         return file_ioctrl (socket, cmd, argp);

    case 'I':
         return iface_ioctrl (socket, cmd, argp);

    case 's':
         return waterm_ioctrl (socket, cmd, argp);

    default:
         NO_CMD (cmd);
         SOCK_ERRNO (ESOCKTNOSUPPORT);
  }
  return (-1);
}

/*
 * IO-control for "file" handles (i.e. stream/datagram sockets)
 */
static int file_ioctrl (Socket *socket, long cmd, char *argp)
{
  int len;

  VERIFY_RW (argp, sizeof(*argp));

  switch ((DWORD)cmd)
  {
    case FIONREAD:
         if (socket->so_type != SOCK_DGRAM &&
             socket->so_type != SOCK_STREAM)
         {
           SOCK_ERRNO (EBADF);
           return (-1);
         }
         if (socket->so_type == SOCK_DGRAM)
         {
           if (socket->so_state & SS_PRIV)
                len = sock_recv_used ((sock_type*)socket->udp_sock);
           else len = sock_rbused ((sock_type*)socket->udp_sock);
         }
         else
           len = sock_rbused ((sock_type*)socket->tcp_sock);

         if (len < 0)
         {
           SOCK_ERRNO (EBADF);
           return (-1);
         }

         SOCK_DEBUGF ((" %d", len));
         if (len >= 0)
            *(u_long*)argp = len;
         break;

    case FIONBIO:                 /* set nonblocking I/O on/off */
         if (*argp)
         {
           socket->so_state |= SS_NBIO;
           socket->timeout = 0;
           if (socket->tcp_sock)
              socket->tcp_sock->timeout = 0;
         }
         else
         {
           socket->so_state &= ~SS_NBIO;
           if (socket->tcp_sock)  /* Only TCP sockets timeout on inactivety */
              socket->timeout = sock_delay;
         }
         SOCK_DEBUGF ((" %d", (socket->so_state & SS_NBIO) ? 1 : 0));
         break;

#if 0
    case FIOASYNC:
    case FIOCLEX:
    case FIONCLEX:
    case FIONREAD:
    case FIOSETOWN:
    case FIOGETOWN:
#endif

    default:
         NO_CMD (cmd);
         SOCK_ERRNO (ESOCKTNOSUPPORT);
         return (-1);
  }
  return (0);
}

/*
 * Return name of this interface.
 * We only support a single interface at a time.
 */
static char eth_ifname [IFNAMSIZ] = "eth0";
static char arc_ifname [IFNAMSIZ] = "arch0";
static char tok_ifname [IFNAMSIZ] = "tr0";
static char fddi_ifname[IFNAMSIZ] = "fddi0";
static char ppp_ifname [IFNAMSIZ] = "ppp0";
static char slp_ifname [IFNAMSIZ] = "slp0";

void __get_ifname (char *if_name)
{
  switch (_pktdevclass)
  {
    case PDCLASS_ETHER:
         strcpy (if_name, eth_ifname);
         break;
    case PDCLASS_TOKEN:
         strcpy (if_name, tok_ifname);
         break;
    case PDCLASS_FDDI:
         strcpy (if_name, fddi_ifname);
         break;
    case PDCLASS_ARCNET:
         strcpy (if_name, arc_ifname);
         break;
    case PDCLASS_SLIP:
         strcpy (if_name, slp_ifname);
         break;
    case PDCLASS_PPP:
         strcpy (if_name, ppp_ifname);
         break;
    default:
         strcpy (if_name, "??");
         break;
  }
}

#ifdef NOT_USED_YET
/*
 * Set a new name for this interface.
 */
void __set_ifname (const char *if_name)
{
  switch (_pktdevclass)
  {
    case PDCLASS_ETHER:
         strcpy (eth_ifname, if_name);
         break;
    case PDCLASS_TOKEN:
         strcpy (tok_ifname, if_name);
         break;
    case PDCLASS_FDDI:
         strcpy (fddi_name, if_ifname);
         break;
    case PDCLASS_ARCNET:
         strcpy (arc_name, if_ifname);
         break;
    case PDCLASS_SLIP:
         strcpy (slp_ifname, if_name);
         break;
    case PDCLASS_PPP:
         strcpy (ppp_ifname, if_name);
         break;
    default:
         break;
  }
}

static struct ifnet *eth_ifnet (void)
{
  static struct ifnet net;

  /** \todo fill info */
  return (&net);
}

static struct ifnet *tok_ifnet (void)
{
  static struct ifnet net;

  /** \todo fill info */
  return (&net);
}
#endif

/*
 * Handler for interface request get/set commands
 */
static int iface_ioctrl (Socket *socket, long cmd, char *argp)
{
  struct ifreq       *ifr = (struct ifreq *) argp;
  struct ifconf      *ifc = (struct ifconf*) argp;
  struct sockaddr_in *sin;
  const eth_address  *eth;
  int   len, i;

  VERIFY_RW (argp, sizeof(*ifr));

  switch ((DWORD)cmd)
  {
    case SIOCSARP:
    case SIOCGARP:
    case SIOCDARP:
         return arp_ioctrl (socket, cmd, argp);

    case SIOCGIFADDR:                /* get interface address */
    case OSIOCGIFADDR:
         __get_ifname (ifr->ifr_name);

         if (ifr->ifr_addr.sa_family == AF_INET)
         {
           struct sockaddr_in *sin = (struct sockaddr_in*) &ifr->ifr_addr;
           sin->sin_addr.s_addr = htonl (my_ip_addr);
           break;
         }
#if defined(USE_IPV6)
         if (ifr->ifr_addr.sa_family == AF_INET6)
         {
           struct sockaddr_in6 *sin = (struct sockaddr_in6*) &ifr->ifr_addr;
           memcpy (&sin->sin6_addr, &in6addr_my_ip, sizeof(sin->sin6_addr));
           break;
         }
#endif
         if (_pktdevclass == PDCLASS_TOKEN  || /* otherwise return MAC addr? */
             _pktdevclass == PDCLASS_ETHER  ||
             _pktdevclass == PDCLASS_ARCNET ||
             _pktdevclass == PDCLASS_FDDI)
              memcpy (ifr->ifr_addr.sa_data, _eth_addr, sizeof(_eth_addr));
         else memset (ifr->ifr_addr.sa_data, 0, sizeof(ifr->ifr_addr.sa_data));
         break;

    case SIOCGIFMTU:                 /* get interface MTU */
         ifr->ifr_mtu = _mtu;
         break;

#if 0
    case SIOCGIFNAME:                /* get interface name */
         break;
#endif

    case SIOCSIFADDR:                /* set interface address */
         if (ifr->ifr_addr.sa_family == AF_INET)
         {
           sin = (struct sockaddr_in*) &ifr->ifr_addr;
           my_ip_addr = ntohl (sin->sin_addr.s_addr);
           break;
         }
#if defined(USE_IPV6)
         if (ifr->ifr_addr.sa_family == AF_INET6)
         {
           struct sockaddr_in6 *sin = (struct sockaddr_in6*) &ifr->ifr_addr;
           memcpy ((void*)&in6addr_my_ip, &sin->sin6_addr, sizeof(in6addr_my_ip));
           break;
         }
#endif
         eth = (const eth_address*) ifr->ifr_addr.sa_data;  /* ?? */
         if (!_eth_set_addr(eth))
         {
           SOCK_ERRNO (EINVAL);
           return (-1);
         }
         break;

    case OSIOCGIFDSTADDR:
    case SIOCGIFDSTADDR:
         /** \todo Get point-to-point address */
         break;

    case SIOCSIFDSTADDR:
         /** \todo Set point-to-point address */
         break;

    case SIOCSIFFLAGS:   /* set iface flags */
         /* Allow other socket types to do this?
          */
         if (socket->so_type == SOCK_PACKET)
         {
           BOOL rc;

           if (ifr->ifr_flags & IFF_PROMISC)
                rc = _sock_set_promisc_rx_mode();
           else if (ifr->ifr_flags & IFF_ALLMULTI)
                rc = _sock_set_mcast_rx_mode();
           else rc = _sock_set_normal_rx_mode (socket);
           if (!rc)
           {
             SOCK_ERRNO (ENETDOWN);
             return (-1);
           }
         }
         break;

    case SIOCGIFFLAGS:               /* get iface flags */
         ifr->ifr_flags = 0;
         if (_eth_is_init)
         {
           ifr->ifr_flags |= (IFF_UP | IFF_RUNNING);

           if (_pkt_rxmode == RXMODE_PROMISCOUS)
              ifr->ifr_flags |= IFF_PROMISC;

           if (_pkt_rxmode >= RXMODE_MULTICAST2)
              ifr->ifr_flags |= IFF_ALLMULTI;

           if (_pktdevclass == PDCLASS_PPP  ||
               _pktdevclass == PDCLASS_SLIP ||
               _pktdevclass == PDCLASS_AX25)
                ifr->ifr_flags |= IFF_POINTOPOINT;
           else ifr->ifr_flags |= IFF_BROADCAST;  /* ARCNET broadcast? */

#if defined(USE_MULTICAST)
           if (_multicast_on)
              ifr->ifr_flags |= IFF_MULTICAST;
#endif
         }
         break;

    case SIOCGIFBRDADDR:             /* get IP broadcast address */
    case OSIOCGIFBRDADDR:
         sin = (struct sockaddr_in*) &ifr->ifr_broadaddr;
         sin->sin_addr.s_addr = htonl (my_ip_addr | ~sin_mask);
         sin->sin_family      = AF_INET;
         __get_ifname (ifr->ifr_name);
         break;

    case SIOCSIFBRDADDR:             /* set IP broadcast address */
         break;

    case SIOCGIFMETRIC:              /* get interface metric */
         ifr->ifr_metric = 1;
         __get_ifname (ifr->ifr_name);
         break;

    case SIOCSIFMETRIC:
         /** \todo Set interface metric */
         break;

    case SIOCDIFADDR:                /* delete interface addr */
#if defined(USE_IPV6)
         if (ifr->ifr_addr.sa_family == AF_INET6)
         {
           memset ((void*)&in6addr_my_ip, 0, sizeof(in6addr_my_ip));
           break;
         }
#endif
         if (ifr->ifr_addr.sa_family == AF_INET)
         {
           my_ip_addr = 0;
           break;
         }

         /** \todo Handle deleting interface address */
         break;

    case SIOCAIFADDR:
         /** \todo Handle add/change interface alias */
         break;

    case SIOCGIFNETMASK:             /* get interface net-mask */
    case OSIOCGIFNETMASK:
         sin = (struct sockaddr_in*) &ifr->ifr_addr;
         sin->sin_addr.s_addr = htonl (sin_mask);
         sin->sin_family      = AF_INET;
         __get_ifname (ifr->ifr_name);
         break;

    case SIOCSIFNETMASK:             /* set interface net-mask */
         sin = (struct sockaddr_in*) &ifr->ifr_addr;
         sin_mask = ntohl (sin->sin_addr.s_addr);
         break;

    case SIOCGIFCONF:                /* get interfaces config */
    case OSIOCGIFCONF:
         len = ifc->ifc_len = min (ifc->ifc_len, NUM_IFACES*SIZEOF(*ifr));
         ifc = (struct ifconf*) ifc->ifc_buf; /* user's buffer */
         VERIFY_RW (ifc, len);

         i = 0;
         for (ifr = (struct ifreq*)ifc; i < len; ifr++, i += sizeof(*ifr))
         {
           __get_ifname (ifr->ifr_name);
           sin = (struct sockaddr_in*) &ifr->ifr_addr;
           sin->sin_addr.s_addr = htonl (my_ip_addr);
           sin->sin_family      = AF_INET;
         }
         break;

    case SIOCGIFHWADDR:
         switch (_pktdevclass)
         {
           case PDCLASS_ETHER:
                ifr->ifr_hwaddr.sa_family = ARPHRD_ETHER;
                memcpy (ifr->ifr_hwaddr.sa_data, _eth_addr,
                        sizeof(ifr->ifr_hwaddr.sa_data));
                break;
           case PDCLASS_TOKEN:
                ifr->ifr_hwaddr.sa_family = ARPHRD_TOKEN;
                memcpy (ifr->ifr_hwaddr.sa_data, _eth_addr,
                        sizeof(ifr->ifr_hwaddr.sa_data));
                break;
           case PDCLASS_FDDI:
                ifr->ifr_hwaddr.sa_family = ARPHRD_FDDI;
                memcpy (ifr->ifr_hwaddr.sa_data, _eth_addr,
                        sizeof(ifr->ifr_hwaddr.sa_data));
                break;
           case PDCLASS_ARCNET:
                ifr->ifr_hwaddr.sa_family  = ARPHRD_ARCNET;
                ifr->ifr_hwaddr.sa_data[0] = _eth_addr[0];
                break;
           case PDCLASS_SLIP:
           case PDCLASS_PPP:
                ifr->ifr_hwaddr.sa_family = 0;
                memset (ifr->ifr_hwaddr.sa_data, 0,
                        sizeof(ifr->ifr_hwaddr.sa_data));
                break;
           default:
                return (-1);
         }
         break;

    default:
         NO_CMD (cmd);
         SOCK_ERRNO (ESOCKTNOSUPPORT);
         return (-1);
  }
  ARGSUSED (len);
  return (0);
}


/*
 * Handler for buffer hi/lo watermark and urgent data (OOB)
 */
static int waterm_ioctrl (Socket *socket, long cmd, char *argp)
{
  VERIFY_RW (argp, sizeof(*argp));

  switch ((DWORD)cmd)
  {
    case SIOCSHIWAT:
         /** \todo set high watermark */
         break;

    case SIOCGHIWAT:
         /** \todo get high watermark */
         break;

    case SIOCSLOWAT:
         /** \todo set low watermark */
         break;

    case SIOCGLOWAT:
         /** \todo get low watermark */
         break;

    case SIOCATMARK:
         /** \todo OOB data available? */
         break;

    default:
         NO_CMD (cmd);
         SOCK_ERRNO (ESOCKTNOSUPPORT);
         return (-1);
  }
  ARGSUSED (socket);
  ARGSUSED (argp);
  return (0);
}


/*
 * Handler for ARP-cache interface commands
 */
static int arp_ioctrl (Socket *socket, long cmd, char *argp)
{
  struct arpreq *arp = (struct arpreq*) argp;
  eth_address   *eth;
  DWORD  ip;

  switch ((DWORD)cmd)
  {
    case SIOCSARP:      /* add given IP/MAC-addr pair to ARP cache */
         ip  = intel (*(DWORD*)arp->arp_pa.sa_data);
         eth = (eth_address*) arp->arp_ha.sa_data;
         if (!_arp_cache_add (ip, (const eth_address*)eth, FALSE))
         {
           SOCK_ERRNO (EINVAL);
           return (-1);
         }
         break;

    case SIOCGARP:      /* return ARP entry for given ip */
    case OSIOCGARP:
         ip  = intel (*(DWORD*)arp->arp_pa.sa_data);
         eth = (eth_address*) arp->arp_ha.sa_data;
         if (_arp_lookup_fixed (ip, eth))
         {
           arp->arp_flags |= (ATF_INUSE | ATF_COM | ATF_PERM);  /* fixed addr */
         }
         else if (_arp_lookup (ip, eth))
         {
           arp->arp_flags |= (ATF_INUSE | ATF_COM);   /* dynamic addr */
         }
         else
         {
           arp->arp_flags = 0;
           SOCK_ERRNO (ENOENT);
           return (-1);
         }
         break;

    case SIOCDARP:      /* delete ARP-entry for given ip */
         ip = intel (*(DWORD*)arp->arp_pa.sa_data);
         if (!_arp_cache_del(ip))
         {
           SOCK_ERRNO (ENOENT);
           return (-1);
         }
         break;

    default:
         NO_CMD (cmd);
         SOCK_ERRNO (ESOCKTNOSUPPORT);
         return (-1);
  }
  ARGSUSED (socket);
  return (0);
}

/*
 * Return string for ioctlsocket() command
 */
#if defined(USE_DEBUG)
static const struct search_list commands[] = {
                  { FIOCLEX,         "FIOCLEX"         },
                  { FIONCLEX,        "FIONCLEX"        },
                  { FIONREAD,        "FIONREAD"        },
                  { FIONBIO,         "FIONBIO"         },
                  { FIOASYNC,        "FIOASYNC"        },
                  { FIOSETOWN,       "FIOSETOWN"       },
                  { FIOGETOWN,       "FIOGETOWN"       },
                  { SIOCSPGRP,       "SIOCSPGRP"       },
                  { SIOCGPGRP,       "SIOCGPGRP"       },
                  { SIOCADDRT,       "SIOCADDRT"       },
                  { SIOCDELRT,       "SIOCDELRT"       },
                  { SIOCSIFADDR,     "SIOCSIFADDR"     },
                  { OSIOCGIFADDR,    "OSIOCGIFADDR"    },
                  { SIOCGIFADDR,     "SIOCGIFADDR"     },
                  { SIOCSIFDSTADDR,  "SIOCSIFDSTADDR"  },
                  { OSIOCGIFDSTADDR, "OSIOCGIFDSTADDR" },
                  { SIOCGIFDSTADDR,  "SIOCGIFDSTADDR"  },
                  { SIOCSIFFLAGS,    "SIOCSIFFLAGS"    },
                  { SIOCGIFFLAGS,    "SIOCGIFFLAGS"    },
                  { OSIOCGIFBRDADDR, "OSIOCGIFBRDADDR" },
                  { SIOCGIFBRDADDR,  "SIOCGIFBRDADDR"  },
                  { SIOCSIFBRDADDR,  "SIOCSIFBRDADDR"  },
                  { OSIOCGIFCONF,    "OSIOCGIFCONF"    },
                  { SIOCGIFCONF,     "SIOCGIFCONF"     },
                  { OSIOCGIFNETMASK, "OSIOCGIFNETMASK" },
                  { SIOCGIFNETMASK,  "SIOCGIFNETMASK"  },
                  { SIOCSIFNETMASK,  "SIOCSIFNETMASK"  },
                  { SIOCGIFMETRIC,   "SIOCGIFMETRIC"   },
                  { SIOCSIFMETRIC,   "SIOCSIFMETRIC"   },
                  { SIOCDIFADDR,     "SIOCDIFADDR"     },
                  { SIOCAIFADDR,     "SIOCAIFADDR"     },
                  { SIOCGIFMTU,      "SIOCGIFMTU"      },
                  { SIOCSARP,        "SIOCSARP"        },
                  { OSIOCGARP,       "OSIOCGARP"       },
                  { SIOCGARP,        "SIOCGARP"        },
                  { SIOCDARP,        "SIOCDARP"        },
                  { SIOCSHIWAT,      "SIOCSHIWAT"      },
                  { SIOCGHIWAT,      "SIOCGHIWAT"      },
                  { SIOCSLOWAT,      "SIOCSLOWAT"      },
                  { SIOCGLOWAT,      "SIOCGLOWAT"      },
                  { SIOCATMARK,      "SIOCATMARK"      },
                  { SIOCGIFHWADDR,   "SIOCGIFHWADDR"   }
                };

static const char *get_ioctl_cmd (long cmd)
{
  static char buf[50];

  switch (IOCGROUP(cmd))
  {
    case 'f':
         strcpy (buf, "file cmd: ");
         break;

    case 'I':
         strcpy (buf, "iface cmd: ");
         break;

    case 's':
         strcpy (buf, "waterm cmd: ");
         break;

    default:
         return ("??unknown");
  }
  strcat (buf, list_lookup(cmd, commands, DIM(commands)));
  return (buf);
}
#endif


/*
 * A small test program for above functions.
 */
#if defined(TEST_PROG)

#undef  assert
#define assert(x) ((x) ? (void)0 : __assert_fail(__LINE__))

void __assert_fail (unsigned line)
{
  fprintf (stderr, "\nAssert failed at line %d, errno = %d (%s)\n",
           line, errno, strerror(errno));
  exit (-1);
}

const char *eth_addr_string (struct ether_addr *eth)
{
  static char buf[20];

  sprintf (buf, "%02X:%02X:%02X:%02X:%02X:%02X",
           eth->ether_addr_octet[0],
           eth->ether_addr_octet[1],
           eth->ether_addr_octet[2],
           eth->ether_addr_octet[3],
           eth->ether_addr_octet[4],
           eth->ether_addr_octet[5]);
  return (buf);
}

const char *if_flags_string (unsigned short flags)
{
  static char buf[200];

  buf[0] = '\0';

  if (flags & IFF_UP)
     strcat (buf,"up,");
  if (flags & IFF_BROADCAST)
     strcat (buf,"broadcast,");
  if (flags & IFF_DEBUG)
     strcat (buf,"debug,");
  if (flags & IFF_LOOPBACK)
     strcat (buf,"loopback,");
  if (flags & IFF_POINTOPOINT)
     strcat (buf,"p-to-p,");
  if (flags & IFF_NOTRAILERS)
     strcat (buf,"no-trail,");
  if (flags & IFF_RUNNING)
     strcat (buf,"running,");
  if (flags & IFF_NOARP)
     strcat (buf,"no-arp,");
  if (flags & IFF_PROMISC)
     strcat (buf,"promisc,");
  if (flags & IFF_ALLMULTI)
     strcat (buf,"all-multi,");
  if (flags & IFF_OACTIVE)
     strcat (buf,"out-act,");
  if (flags & IFF_SIMPLEX)
     strcat (buf,"simplex,");
  if (flags & IFF_LINK0)
     strcat (buf,"link0,");
  if (flags & IFF_LINK1)
     strcat (buf,"link1,");
  if (flags & IFF_LINK2)
     strcat (buf,"link2,");
  if (flags & IFF_MULTICAST)
     strcat (buf,"mcast,");

  if (buf[0])
     buf[strlen(buf)-1] = '\0';
  return (buf);
}

int main (void)
{
  struct ifreq ifr;
  struct sockaddr_in *sin;
  int    sock, on = 1;

  dbug_init();

  sock = socket (AF_INET, SOCK_DGRAM, 0);
  assert (sock > 0);
  assert (setsockopt(sock, SOL_SOCKET, SO_DEBUG, &on,sizeof(on)) == 0);

  ifr.ifr_addr.sa_family = AF_UNSPEC;  /* get MAC-address */

  assert (ioctlsocket (sock, SIOCGIFADDR, (char*)&ifr) == 0);
  printf ("Interface `%s':\n\t ether-addr: %s\n",
          ifr.ifr_name,
          eth_addr_string ((struct ether_addr*)&ifr.ifr_hwaddr.sa_data));

  assert (ioctlsocket (sock, SIOCGIFBRDADDR, (char*)&ifr) == 0);
  sin = (struct sockaddr_in*) &ifr.ifr_broadaddr;
  printf ("\t bcast-addr: %s\n", inet_ntoa (sin->sin_addr));

  ifr.ifr_addr.sa_family = AF_INET;

  assert (ioctlsocket (sock, SIOCGIFADDR, (char*)&ifr) == 0);
  sin = (struct sockaddr_in*) &ifr.ifr_addr;
  printf ("\t inet-addr:  %s\n", inet_ntoa (sin->sin_addr));

  assert (ioctlsocket (sock, SIOCGIFNETMASK, (char*)&ifr) == 0);
  sin = (struct sockaddr_in*) &ifr.ifr_addr;
  printf ("\t net-mask :  %s\n", inet_ntoa (sin->sin_addr));

  assert (ioctlsocket (sock, SIOCGIFMTU, (char*)&ifr) == 0);
  printf ("\t MTU      :  %u\n",ifr.ifr_mtu);

  assert (ioctlsocket (sock, SIOCGIFFLAGS, (char*)&ifr) == 0);
  printf ("\t Flags    :  0x%04X: %s\n",
          ifr.ifr_flags, if_flags_string(ifr.ifr_flags));

  assert (close_s(sock) >= 0);
  return (0);
}

#endif  /* TEST_PROG */
#endif  /* USE_BSD_API */
