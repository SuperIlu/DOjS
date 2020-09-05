/*!\file pcslip.c
 *  Simple SLIP handler.
 *
 *  This stuff will probably never take off --gv
 *  Not called during init.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "wattcp.h"
#include "pcqueue.h"
#include "pcsed.h"
#include "pcpkt.h"
#include "pcconfig.h"
#include "strings.h"
#include "misc.h"
#include "timer.h"
#include "language.h"
#include "ioport.h"
#include "pcslip.h"

#if defined(__MSDOS__)  /* rest of file */

static void (W32_CALL *prev_hook) (const char*, const char*);

static WORD slip_base_reg = 0x3F8;      /* COM1 */
static int  slip_timeout  = 40;         /* CONNECT timeout etc. */
static char slip_user   [MAX_VALUELEN] = "";
static char slip_passwd [MAX_VALUELEN] = "";

static void do_dial (const char *val);

/*****************************************************************/

static void W32_CALL slip_config (const char *name, const char *value)
{
  static const struct config_table slip_cfg[] = {
            { "DIAL",    ARG_FUNC,   (void*)do_dial        },
            { "USER",    ARG_STRCPY, (void*)&slip_user     },
            { "PASSWD",  ARG_STRCPY, (void*)&slip_passwd   },
            { "TIMEOUT", ARG_ATOI,   (void*)&slip_timeout  },
            { "BASE",    ARG_ATOX_W, (void*)&slip_base_reg },
            { NULL }
          };

  if ((_pktdevclass != PDCLASS_SLIP ||
       !parse_config_table(&slip_cfg[0], "SLIP.", name, value)) && prev_hook)
     (*prev_hook) (name, value);
}

/*****************************************************************/

int slip_init (void)
{
  prev_hook = usr_init;
  usr_init  = slip_config;
  return (0);
}

/*****************************************************************/

static BOOL modem_command (const char *cmd, const char *resp, int timeout)
{
  DWORD timer;
  BOOL  rc;
  int   len = cmd ? strlen (cmd) : strlen (resp);

  if (cmd)
     rc = pkt_send (cmd, len);  /**<\todo Bypass PKTDRVR ? */

  if (rc < len || !_eth_is_init)
     return (FALSE);

  timer = set_timeout (1000 * timeout);

  while (!chk_timeout(timer))
  {
    char *pkt = (char*) _eth_arrived (NULL, NULL);

    if (!pkt)
       continue;

    outsn (pkt, len);                             /* print the modem echo */
    _eth_free (pkt);
    return (strncmp(pkt,resp,strlen(resp)) == 0); /* got modem response */
  }
  return (FALSE);
}

/*****************************************************************/

static int slip_dial (const char *str)
{
  char dial_str[80];
  WORD mcr = slip_base_reg + 4;
  WORD lcr = slip_base_reg + 3;

  _outportb (lcr, _inportb(lcr) & 0x43);  /* 8N1 */
  _outportb (mcr, _inportb(mcr) | 1);     /* raise DTR */

  if (!modem_command("ATZ\r","OK",5))
     return (0);

  _strlcpy (dial_str, str, sizeof(dial_str)-3);
  strcat (dial_str, "\r");
  outs (_LANG("SLIP dialing.."));

  if (!modem_command(dial_str,"OK",2))
     return (-1);

  if (!modem_command(NULL,"CONNECT",slip_timeout))
     return (-2);
  return (0);
}

static void do_dial (const char *value)
{
  int rc = slip_dial (value);

  if (rc == -1)
     outsnl (_LANG("Modem not responding\7"));
  if (rc == -2)
     outsnl (_LANG("Connect failed"));
}

#endif /* __MSDOS__ */