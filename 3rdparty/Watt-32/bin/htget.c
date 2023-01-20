/*
 *   HTGET
 *
 *   Get a document via HTTP
 *
 *   This program was compiled under Borland C++ 3.1 in C mode for the small
 *   model. You will require the WATTCP libraries. A copy is included in the
 *   distribution. For the sources of WATTCP, ask archie where the archives
 *   are.
 *
 *   Please send bug fixes, ports and enhancements to the author for
 *   incorporation in newer versions.
 *
 *   Copyright (C) 1996  Ken Yap (ken@syd.dit.csiro.au)
 *
 *   This program is free software; you can redistribute it and/or modify it
 *   under the terms of the Artistic License, a copy of which is included
 *   with this distribution.
 *
 *   THIS PACKAGE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS OR
 *   IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 *   WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <io.h>
#include <tcp.h>

#if defined(__MINGW32__) || defined(__DJGPP__)
  #define write _write
  #define close _close
#endif

#if defined(__GNUC__)
  #pragma GCC diagnostic ignored "-Wpointer-sign"
  #pragma GCC diagnostic ignored "-Waddress"
#endif

#if defined(__CYGWIN__)
  #define stricmp(s1, s2)        strcasecmp (s1, s2)
  #define strnicmp(s1, s2, len)  strncasecmp (s1, s2, len)
  #define STARTS_WITH(s, what)   (strncasecmp (s, what, sizeof(what)-1) == 0)
#else
  #define STARTS_WITH(s, what)   (strnicmp (s, what, sizeof(what)-1) == 0)
#endif


#if (defined(__SMALL__) && !defined(DOS386)) || defined(__LARGE__)
  #define SAVE_SPACE
#else
  #define perror perror_s  /* prevent duplicate symbols */
#endif

#define WINDOWSIZE  (16*1024)
#define BUFFERSIZE  2048
#define PROGRAM     "HTGET"
#define VERSION     "1.05"
#define AUTHOR      "Copyright (C) 1996 Ken Yap (ken@syd.dit.csiro.au)"

// #define HTTPVER  "HTTP/1.[01]"   /* Apache doesn't like this */
#define HTTPVER     "HTTP/1.0"
#define HTTPVER10   "HTTP/1.0"
#define HTTPVER11   "HTTP/1.1"

char       *buffer;
tcp_Socket *sock;

struct tm *mtime;
int        output;
int        headeronly      = 0;
int        debugMode       = 0;
int        verboseMode     = 0;
int        ifModifiedSince = 0;
char      *outputfile      = NULL;
char      *userPass        = NULL;
char      *dayname         = "SunMonTueWedThuFriSat";
char      *monthname       = "JanFebMarAprMayJunJulAugSepOctNovDec";
char       proxy_host[80]  = { 0 };
int        proxy_port      = 0;

void (W32_CALL *prev_hook) (const char*, const char*);

void base64encode (const char *in, char *out)
{
  int    c1, c2, c3;
  int    len = 0;
  static char basis_64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"\
                           "abcdefghijklmnopqrstuvwxyz0123456789+/";
  while (*in)
  {
    c1 = *in++;
    if (*in == '\0')
       c2 = c3 = 0;
    else
    {
      c2 = *in++;
      if (*in == '\0')
           c3 = 0;
      else c3 = *in++;
    }
    *out++ = basis_64 [c1 >> 2];
    *out++ = basis_64 [((c1 & 0x3) << 4) | ((c2 & 0xF0) >> 4)];
    len += 2;
    if (c2 == 0 && c3 == 0)
    {
      *out++ = '=';
      *out++ = '=';
      len += 2;
    }
    else if (c3 == 0)
    {
      *out++ = basis_64 [((c2 & 0xF) << 2) | ((c3 & 0xC0) >> 6)];
      *out++ = '=';
      len += 2;
    }
    else
    {
      *out++ = basis_64 [((c2 & 0xF) << 2) | ((c3 & 0xC0) >> 6)];
      *out++ = basis_64 [c3 & 0x3F];
      len += 2;
    }
  }
  *out = '\0';
}

/*
 * WATTCP's sock_gets() doesn't do the right thing.
 * That's because sock isn't set to ASCII mode.
 */
int sock_getline (void *sock, char *buf, int len)
{
  int  i;
  char ch;

  for (i = 0, --len; i <= len && sock_read(sock,(BYTE*)&ch,1) > 0; )
  {
    if (ch == '\n')
       break;
    if (ch != '\r')
    {
      *buf++ = ch;
      ++i;
    }
  }
  *buf = '\0';
  return (i);
}

long header (const char *path)
{
  int   i, len, response;
  long  contentlength;
  char *s;

  contentlength = LONG_MAX;     /* largest int, in case no CL header */
  len = sock_getline (sock, buffer, BUFFERSIZE);
  if (len <= 0)
  {
    puts ("EOF from server");
    return (-1L);
  }

  s = buffer;
  if (!STARTS_WITH(s, HTTPVER10) && /* no HTTP/1.[01]? */
      !STARTS_WITH(s, HTTPVER11))
  {
    puts ("Not a HTTP/1.[01] server");
    return (-1L);
  }

  s += sizeof(HTTPVER10)-1;
  i = strspn (s, " \t");
  if (i <= 0)             /* no whitespace? */
  {
    puts ("Malformed HTTP/1.[01] line");
    return (-1L);
  }

  s += i;
  response = 500;
  sscanf (s, "%3d", &response);
  if (response == 401)
  {
    printf ("%s: authorisation failed!\n", path);
    return (-1L);
  }
  else if (response != 200 && response != 301 &&
           response != 302 && response != 304)
  {
    printf ("%s: %s\n", path, s);
    contentlength = -1L;
  }

  if (headeronly)
  {
    write (output, buffer, len);
    write (output, "\r\n", 2);
  }

  /* eat up the other header lines here */
  while ((len = sock_getline(sock, buffer, BUFFERSIZE)) > 0)
  {
    if (headeronly)
    {
      write (output, buffer, len);
      write (output, "\r\n", 2);
    }
    s = buffer;
    if (STARTS_WITH(s, "Content-Length:"))
    {
      s += sizeof("Content-Length:")-1;
      contentlength = atol(s);
    }
    else if (STARTS_WITH(buffer, "Location:"))
    {
      if (response == 301 || response == 302)
         printf ("At %s\n", buffer);
    }
    else if (strchr(" \t", buffer[0]))
            printf ("Warning: continuation line encountered\n");
  }
  return (response == 304 ? 0L : contentlength);
}

static void W32_CALL callback (void)
{
  if (_watt_cbroke)
  {
    puts ("\n^C/^Break detected\n");
    sock_exit();
  }
}

int htget (const char *host, int port, const char *path)
{
#ifndef SAVE_SPACE
  struct in_addr a, b;
#endif
  DWORD  hostaddr;
  int    status = 0;
  int    connected = 0;
  int    completed = 0;
  int    i, use_proxy = proxy_host[0] != 0;
  long   length, contentlength = 0L;
  const  char *name;
  char  *buf = buffer;
  char  *rx_buf;
  char   url[512];

  if (proxy_host[0] && use_proxy)
       name = proxy_host;
  else name = host;

  hostaddr = lookup_host (name, NULL);
  if (!hostaddr)
  {
    printf ("%s", dom_strerror(dom_errno));
    return (1);
  }

#ifndef SAVE_SPACE
  a.s_addr = intel (my_ip_addr);      /* A-side is me   */
  inet_aton (host, &b);               /* B-side is host */

  if (inet_netof(a) == inet_netof(b)) /* on same network */
     use_proxy = 0;
#endif

#if 0   /* test */
  {
    struct in_addr na, nb;

    na.s_addr = inet_netof (a);
    nb.s_addr = inet_netof (b);

    printf ("A [%s]: netof: %s\n", inet_ntoa(a), inet_ntoa(na));
    printf ("B [%s]: netof: %s\n", inet_ntoa(b), inet_ntoa(nb));

    na.s_addr = htonl (inet_lnaof(a));
    nb.s_addr = htonl (inet_lnaof(b));

    printf ("A [%s]: lnaof: %s\n", inet_ntoa(a), inet_ntoa(na));
    printf ("B [%s]: lnaof: %s\n", inet_ntoa(b), inet_ntoa(nb));

    printf ("use_proxy = %d\n", use_proxy);
  }
#endif

  if (use_proxy && proxy_port && proxy_host[0])
     port = proxy_port;

  if (debugMode)
     printf ("%s:%d%s\n", host, port, path);

  if (!tcp_open(sock, 0, hostaddr, port, NULL))
  {
    printf ("Cannot connect to `%s'\n", name);
    return (1);
  }

  rx_buf = malloc (WINDOWSIZE);
  sock_setbuf (sock, (BYTE*)rx_buf, WINDOWSIZE);
  sock_yield (sock, callback);

  sock_wait_established (sock, sock_delay, NULL, &status);
  connected = 1;
  completed = 1;
  sock_tick (sock, &status);      /* in case they sent reset */

  if (verboseMode)
     puts ("Sending HTTP GET/HEAD request");

  if (proxy_host[0])
       sprintf (url, "http://%s%s", host, path);
  else strcpy (url, path);

  buf += sprintf (buf, "%s %s " HTTPVER "\r\n"
                  "User-Agent: " PROGRAM "-DOS/" VERSION "\r\n",
                  headeronly ? "HEAD" : "GET", url);

  /* Append a "Host: " header (unless proxied)
   */
  if (name == host)
     buf += sprintf (buf, "Host: %s\r\n", host);

  if (userPass)
  {
    char pass [100];

    base64encode (userPass, pass);
    if (debugMode)
       printf ("%s => %s\n", userPass, pass);

    buf += sprintf (buf, "Authorization: Basic %s\r\n", pass);
  }

  if (ifModifiedSince)
  {
    char *mod = buf;

    buf += sprintf (buf,
                    "If-Modified-Since: %.3s, %02d %.3s %04d %02d:%02d:%02d GMT\r\n",
                    dayname + 3*mtime->tm_wday,
                    mtime->tm_mday,
                    monthname + 3*mtime->tm_mon,
                    mtime->tm_year + 1900,
                    mtime->tm_hour, mtime->tm_min, mtime->tm_sec);
    if (debugMode || verboseMode)
       puts (mod);
  }

  buf += sprintf (buf, "\r\n");
  sock_fastwrite (sock, (const BYTE*)buffer, buf-buffer);

  contentlength = header (path);

  if (contentlength >= 0L && !headeronly)
  {
    /* We wait until the last moment to open the output file.
     * If any specified so that we won't overwrite the file
     * in case of error in contacting server.
     */
    if (outputfile)
    {
      FILE *of = fopen (outputfile, "wb");

      if (!of)
      {
        perror (outputfile);
        goto close_up;
      }
      output = fileno (of);
    }

    length = 0L;
    while ((i = sock_read(sock,(BYTE*)buffer,BUFFERSIZE)) > 0)
    {
      write (output, buffer, i);
      length += i;
      if (verboseMode)
      {
        printf ("Got %lu bytes\r", length);
        fflush (stdout);
      }
    }

    if (contentlength != LONG_MAX && length != contentlength)
       printf ("Warning, actual length = %ld, content length = %ld\n",
               length, contentlength);
  }

close_up:
  sock_close (sock);
  sock_wait_closed (sock, sock_delay, NULL, &status);

sock_err:
  free (rx_buf);

  if (status == -1)
     printf ("`%s' %s\n", name, sockerr(sock));
  if (!connected)
     puts ("Could not get connected");

  return (!completed || contentlength < 0L);
}

void version (void)
{
  puts (PROGRAM " " VERSION " " AUTHOR "\n"
        PROGRAM " comes with ABSOLUTELY NO WARRANTY.\n"
        "This is Postcardware, and you are welcome to redistribute it\n"
        "under certain conditions; see the file Artistic for details.");
}

void usage (void)
{
  version();
  puts (
    "Usage: " PROGRAM " [-hvVm] [-p ident:passwd] [-o file] URL\n"
    "\t -h option to get header only\n"
    "\t -V show version info\n"
    "\t -v show verbose messages\n"
    "\t -d enable debug mode\n"
    "\t -m fetch only if newer than file in -o\n"
    "\t -p ident:password  send authorisation\n"
    "\t -o write HTML output to file");
}

/*--------------------------------------------------------------*/

static void Exit (const char *str)
{
  puts (str);
  exit (1);
}

void W32_CALL cnf_hook (const char *name, const char *value)
{
  if (!strcmp(name,"HTTP.PROXY"))
  {
    if (sscanf(value,"%[^:]:%d",proxy_host,&proxy_port) != 2)
       Exit ("Config error: syntax is HTTP_PROXY=<host>:<port>");
  }
  else if (prev_hook)
         (*prev_hook) (name, value);
}

void set_cnf_hook (void)
{
  prev_hook = usr_init;
  usr_init  = cnf_hook;
}

int MS_CDECL main (int argc, char **argv)
{
  char *host, *path, *s;
  int   port = 80;
  int   ch, status;
  int   host_alloc = 0;

  while ((ch = getopt(argc, argv, "?hVvdmp:o:")) != EOF)
     switch (ch)
     {
       case 'V': version();
                 puts (wattcpVersion());
                 return (0);

       case 'v': verboseMode = 1;
                 break;

       case 'd': debugMode++;
                 break;

       case 'h': headeronly = 1;
                 break;

       case 'm': ifModifiedSince = 1;
                 break;

       case 'o': outputfile = optarg;
                 break;

       case 'p': userPass = optarg;
                 break;

       default : usage();
                 return (1);
     }

  argc -= optind;
  argv += optind;

  if (argc <= 0)
  {
    usage();
    return (1);
  }

  buffer = malloc (BUFFERSIZE);
  sock   = malloc (sizeof(*sock));
  if (!buffer || !sock)
  {
    puts ("No memory");
    return (-1);
  }

  if (STARTS_WITH(argv[0], "http://"))
     argv[0] += sizeof("http://") - 1;

  path = strchr (argv[0],'/');
  if (path == NULL)   /* separate out the path */
  {
    host = argv[0];
    path = "/";       /* top directory */
  }
  else
  {
    host = calloc (path-argv[0]+1, 1);
    if (!host)
    {
      printf (PROGRAM ": Out of memory\n");
      return (1);
    }
    host_alloc = 1;
    strncat (host, argv[0], path-argv[0]);
  }


  /* do we have a port number? */
  s = strchr (host,':');
  if (s)
  {
    *s++ = '\0';
    port = atoi(s);
  }

  if (ifModifiedSince)
  {
    struct stat statbuf;

    /* allow only if no -h and -o file specified and file exists */
    if (headeronly || outputfile == 0 || stat(outputfile,&statbuf) < 0)
        ifModifiedSince = 0;
    else if (verboseMode)
    {
      mtime = gmtime (&statbuf.st_mtime);
      printf ("%s last modified %s", outputfile,asctime(mtime));
    }
  }

  if (debugMode)
  {
    tcp_set_debug_state (1);
    dbug_init();
  }

#ifndef __POCC__
  tzset();
#endif

  set_cnf_hook();
  sock_init();

  if (debugMode > 1)
     debug_on = debugMode;

  output = fileno (stdout);
  status = htget (host, port, path);

  /* MS's VCRuntime doesn't like this.
   */
  if (output != fileno(stdout) && stricmp(outputfile,"NUL"))
     close (output);
  free (buffer);
  free (sock);
  if (host_alloc)
    free (host);

  return (status);
}

