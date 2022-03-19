/*
 * IPv6 tunneling over UDP/IPv4
 *
 * Refs:
 *  http://www.ietf.org/internet-drafts/draft-huitema-v6ops-teredo-00.txt
 *  http://www.microsoft.com/technet/treeview/default.asp?url=/technet/prodtechnol/winxppro/maintain/Teredo.asp
 */

#include "socket.h"
#include "teredo64.h"

/* 3FFE:831F::/32 */
const struct in6_addr in6addr_teredo_pfx = {{ 0x3F, 0xFE, 0x83, 0x1F,
                                              0,0,0,0, 0,0,0,0,
                                              0,0,0,0 }};

/*
 * make_ipv6 teredo addr from
 *  dest_host, port, is_cone_nat:
 *
 *   prefix + ip4_host + (0x8000 * is_cone_nat) + (port ^0xFFFF)
 */

