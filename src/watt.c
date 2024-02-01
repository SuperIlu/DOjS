/*
MIT License

Copyright (c) 2019-2021 Andre Seidelt <superilu@yahoo.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "watt.h"

#if LINUX == 1
#include <ifaddrs.h>
#endif  // LINUX == 1

#define WATT_NAME_BUFFER_SIZE 1024

/**************
** variables **
**************/
extern int _watt_do_exit; /* in sock_ini.h, but not in public headers. */

/*********************
** static functions **
*********************/

#ifdef DEBUG_ENABLED
/**
 * @brief debug test function
 *
 * @param J VM state.
 */
static void f_IpDebug(js_State *J) { watt_pushipaddr(J, watt_toipaddr(J, 1)); }
#endif

/**
 * @brief create a map of all known ipv4 interfaces with their ip-addresses. DOS only knows a single fake 'eth0' interface containing he IP settings of Watt and a fake 'lo'
 * interface.
 *
 * GetNetworkInterfaces(): {"eth0":{"inet":[192.168.1.2], "netmask":[255,255,255,0]}, "lo":{"inet":[127.0.0.1], "netmask":[255,0,0,0]}}
 */
static void f_GetNetworkInterfaces(js_State *J) {
#if LINUX == 1
    struct ifaddrs *ifaddr;
    if (getifaddrs(&ifaddr) == -1) {
        js_error(J, "getifaddrs() failed");
        return;
    }

    js_newobject(J);
    {
        for (struct ifaddrs *ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
            if (ifa->ifa_addr == NULL) {
                continue;
            }

            int family = ifa->ifa_addr->sa_family;

            if (family == AF_INET) {
                js_newobject(J);
                {
                    struct sockaddr_in *inet = (struct sockaddr_in *)ifa->ifa_addr;
                    watt_pushipaddr(J, ntohl(inet->sin_addr.s_addr));
                    js_setproperty(J, -2, "inet");

                    struct sockaddr_in *netmask = (struct sockaddr_in *)ifa->ifa_netmask;
                    watt_pushipaddr(J, ntohl(netmask->sin_addr.s_addr));
                    js_setproperty(J, -2, "netmask");
                }
                js_setproperty(J, -2, ifa->ifa_name);
            }
        }
    }

    freeifaddrs(ifaddr);
#else
    js_newobject(J);
    {
        js_newobject(J);
        {
            watt_pushipaddr(J, _gethostid());
            js_setproperty(J, -2, "inet");
            watt_pushipaddr(J, sin_mask);
            js_setproperty(J, -2, "netmask");
        }
        js_setproperty(J, -2, "eth0");

        js_newobject(J);
        {
            watt_pushipaddr(J, IP(127, 0, 0, 1));
            js_setproperty(J, -2, "inet");
            watt_pushipaddr(J, IP(255, 0, 0, 0));
            js_setproperty(J, -2, "netmask");
        }
        js_setproperty(J, -2, "lo");
    }
#endif
}

/**
 * @brief resolve an IP address using DNS.
 * Resolve(hostname:string):number[]
 *
 * @param J VM state.
 */
static void f_Resolve(js_State *J) {
    const char *host = js_tostring(J, 1);
    DWORD ip = resolve(host);
    if (ip) {
        watt_pushipaddr(J, ip);
    } else {
        js_error(J, "Can't resolve '%s': %s", host, dom_strerror(dom_errno));
    }
}

/**
 * @brief reverse lookup IP address using DNS.
 * ResolveIp(ip:number[]):string
 *
 * @param J VM state.
 */
static void f_ResolveIp(js_State *J) {
    char buffer[WATT_NAME_BUFFER_SIZE];

    DWORD ip = watt_toipaddr(J, 1);

    // HACK: apparently resolve_ip() is broken and resolves the IP in veverse order!
    ip = IP(IP4(ip), IP3(ip), IP2(ip), IP1(ip));

    if (resolve_ip(ip, buffer, sizeof(buffer))) {
        js_pushstring(J, buffer);
    } else {
        js_error(J, "Can't reverse lookup '%d.%d.%d.%d'", IP1(ip), IP2(ip), IP3(ip), IP4(ip));
    }
}

/**
 * @brief get hostname.
 * GetHostname():string
 *
 * @param J VM state.
 */
static void f_GetHostname(js_State *J) {
    char buffer[WATT_NAME_BUFFER_SIZE];
    gethostname(buffer, sizeof(buffer));
    js_pushstring(J, buffer);
}

/**
 * @brief get domainname.
 * GetDomainname():string
 *
 * @param J VM state.
 */
static void f_GetDomainname(js_State *J) {
    char buffer[WATT_NAME_BUFFER_SIZE];
    if (getdomainname(buffer, sizeof(buffer)) == 0) {
        js_pushstring(J, buffer);
    } else {
        js_pushnull(J);
    }
}

/***********************
** exported functions **
***********************/
/**
 * @brief initialize color subsystem.
 *
 * @param J VM state.
 */
void init_watt(js_State *J) {
    DEBUGF("%s\n", __PRETTY_FUNCTION__);

    if (!DOjS.params.no_tcpip) {
        // WATT32 init
#ifdef DEBUG_ENABLED
        dbug_init();
#endif
        _watt_do_exit = 0;
        int err = sock_init();
#if LINUX != 1
        if (!err) {
            char buffer[WATT_NAME_BUFFER_SIZE];
            LOGF("WATTCP init         : %s\n", sock_init_err(err));
            LOGF("WATTCP Address      : %s\n", _inet_ntoa(NULL, _gethostid()));
            LOGF("WATTCP Network Mask : %s\n", _inet_ntoa(NULL, sin_mask));
            gethostname(buffer, sizeof(buffer));
            LOGF("WATTCP hostname     : %s\n", buffer);
            getdomainname(buffer, sizeof(buffer));
            LOGF("WATTCP domainname   : %s\n", buffer);
            LOGF("WATTCP              : %s / %s\n", wattcpVersion(), wattcpCapabilities());
        } else {
            LOGF("WATTCP init: %s\n", sock_init_err(err));
        }
#else   // LINUX != 1
        (void)err;
#endif  // LINUX != 1

        // functions
        NFUNCDEF(J, GetNetworkInterfaces, 0);
        NFUNCDEF(J, GetHostname, 0);
        NFUNCDEF(J, GetDomainname, 0);
        NFUNCDEF(J, Resolve, 1);
        NFUNCDEF(J, ResolveIp, 1);

#ifdef DEBUG_ENABLED
        NFUNCDEF(J, IpDebug, 1);
#endif
    } else {
        LOGF("TCP/IP stack disabled\n");
    }

    DEBUGF("%s DONE\n", __PRETTY_FUNCTION__);
}

/**
 * @brief push an IP address as array onto the JS stack
 *
 * @param J VM state.
 * @param ip IP address in DWORD format
 */
void watt_pushipaddr(js_State *J, uint32_t ip) {
    DEBUGF("%s(): %08lX\n", __PRETTY_FUNCTION__, ip);

    js_newarray(J);
    js_pushnumber(J, IP1(ip));
    js_setindex(J, -2, 0);
    js_pushnumber(J, IP2(ip));
    js_setindex(J, -2, 1);
    js_pushnumber(J, IP3(ip));
    js_setindex(J, -2, 2);
    js_pushnumber(J, IP4(ip));
    js_setindex(J, -2, 3);
}

/**
 * @brief get an IP address from JS stack.
 *
 * @param J VM state.
 * @param idx JS stack index.
 * @return DWORD convert IP address in JS stack format to DWORD.
 */
uint32_t watt_toipaddr(js_State *J, int idx) {
    if (!js_isarray(J, idx)) {
        JS_ENOARR(J);
        return 0;
    } else {
        int len = js_getlength(J, idx);
        if (len < 4) {
            js_error(J, "IP address must have 4 values");
            return 0;
        }

        js_getindex(J, idx, 0);
        uint8_t ip1 = js_touint16(J, -1);
        js_pop(J, 1);
        js_getindex(J, idx, 1);
        uint8_t ip2 = js_touint16(J, -1);
        js_pop(J, 1);
        js_getindex(J, idx, 2);
        uint8_t ip3 = js_touint16(J, -1);
        js_pop(J, 1);
        js_getindex(J, idx, 3);
        uint8_t ip4 = js_touint16(J, -1);
        js_pop(J, 1);

        DEBUGF("%s(): %08lX\n", __PRETTY_FUNCTION__, IP(ip1, ip2, ip3, ip4));

        return IP(ip1, ip2, ip3, ip4);
    }
}
