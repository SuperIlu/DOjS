/*
 * A simple 'uname' program for DOS and Watt-32
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dos.h>
#include <tcp.h>

#define SYSTEM_NAME     "MS-DOS"
#define UNAME_MACHINE   "pc"    /* what configure normally expects */
#define UNAME_SYSTEM    "MSDOS"
#define PROCESSOR_TYPE  "x86"


int a_flag = 0;
int m_flag = 0;
int n_flag = 0;
int p_flag = 0;
int r_flag = 0;
int s_flag = 0;
int v_flag = 0;

char *dosver_str (void)
{
  static char buf[20];
#ifdef __DJGPP__
  _get_dos_version (0);
#endif
  sprintf (buf, "%u %u", _osmajor, _osminor);
  return (buf);
}

char *dosvendor_str (void)
{
#if 0
  /* to-do: test for PCDOS, MSDOS, DRDOS, PTS-DOS, FreeDOS,
   * Win9x/NT/OS2/Linux DOS-box etc.
   */
#endif
  return (UNAME_SYSTEM);
}

void usage (void)
{
  fprintf (stderr, "uname [-amnprsv]\n");
  exit (-1);
}

int W32_CALL nul_print (char c)
{
  (void)c;
  return (0);
}

void my_sock_init (void)
{
  W32_DATA int _watt_do_exit;
  int rc;

  _watt_do_exit = 0;   /* don't exit in sock_init() */
  _outch = nul_print;  /* silence printouts */
  rc = sock_init();
  if (rc)
  {
    fprintf (stderr, "Watt-32 init failed (code %d)\n", rc);
    exit (-1);
  }
}

int main (int argc, char **argv)
{
  int ch;

  while ((ch = getopt(argc, argv, "h?amnprsv")) != EOF)
     switch (ch)
     {
       case 'a':
            a_flag = 1;
            m_flag = n_flag = p_flag = 1;
            r_flag = s_flag = v_flag = 1;
            break;
       case 'm':
            m_flag = 1;
            break;
       case 'n':
            n_flag = 1;
            break;
       case 'p':
            p_flag = 1;
            break;
       case 'r':
            r_flag = 1;
            break;
       case 's':
            s_flag = 1;
            break;
       case 'v':
            v_flag = 1;
            break;
       case 'h':
       case '?':
       default : usage();
     }

  if (a_flag || n_flag)
     my_sock_init();

  printf (SYSTEM_NAME);

  if (n_flag)
  {
    char *p, name[256] = "<unknown host>";

    gethostname (name, sizeof(name));
    p = strchr (name, '.');
    if (p)
       *p = '\0';
    printf (" %s", name);
  }
  if (r_flag || v_flag)
     printf (" %s", dosver_str());

  if (m_flag)
     printf (" %s", UNAME_MACHINE);

  if (p_flag)
     printf (" %s", PROCESSOR_TYPE);

  if (s_flag)
     printf (" %s", dosvendor_str());

  fflush (stdout);
  return (0);
}
