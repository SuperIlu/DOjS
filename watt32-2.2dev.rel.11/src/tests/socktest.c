#include "socket.h"
#include "loopback.h"
#include "pcdbug.h"

#include <rpc/types.h>

#undef  assert
#define assert(x) ((x) ? (void)0 : AssertFail(__LINE__))

WORD   test_port = 1234;
int    sock = -1;
struct sockaddr_in sock_name;

int  sock_select   (int sock);
int  sock_loopback (in_Header *ip);
void AssertFail   (unsigned line);

/*--------------------------------------------------------------------------*/

void setup (void)
{
  dbug_init();
  tcp_set_debug_state (1);
  loopback_handler = sock_loopback;
}

/*--------------------------------------------------------------------------*/

int main (void)
{
  int  rc;
  char buf[100] = { 0 };

  setup();
  sock = socket (AF_INET, SOCK_STREAM, 0);
  assert (sock >= 0);

  sock_name.sin_family      = AF_INET;
  sock_name.sin_port        = htons (test_port);
  sock_name.sin_addr.s_addr = htonl (INADDR_LOOPBACK);
  assert (bind(sock, (struct sockaddr*)&sock_name, sizeof(sock_name)) != -1);

  sock_name.sin_family      = AF_INET;
  sock_name.sin_port        = htons (test_port);
  sock_name.sin_addr.s_addr = htonl (INADDR_LOOPBACK);
  assert (connect(sock, (struct sockaddr*)&sock_name, sizeof(sock_name)) != -1);

  do
  {
    rc = sock_select (sock);
  }
  while (rc < 0);

  assert (write_s(sock,"HELO",4) != -1);
  assert (read_s (sock,buf,sizeof(buf)) != -1);
  printf ("buf = `%s'\n", buf);

  assert (write_s(sock,"QUIT",4) != -1);
  assert (close_s(sock) != -1);
  sock = -1;
  return (0);
}

/*--------------------------------------------------------------------------*/

int sock_select (int sock)
{
  fd_set write_fd;
  fd_set read_fd;
  struct timeval timeout;

  FD_ZERO (&write_fd);
  FD_SET (sock, &write_fd);
  FD_ZERO (&read_fd);
  FD_SET (sock, &read_fd);
  timeout.tv_sec  = 1;
  timeout.tv_usec = 0;
  return select_s (sock+1, &read_fd, &write_fd, NULL, &timeout);
}

/*--------------------------------------------------------------------------*/

int fix_tcp_packet (in_Header *ip, tcp_Header *tcp, int ack_len, int data_len)
{
  static DWORD seq_num = 12345678;  /* starting sequence */
  tcp_PseudoHeader ph;
  WORD src_port;

  tcp->checksum = 0;
  src_port      = tcp->srcPort;
  tcp->srcPort  = tcp->dstPort;
  tcp->dstPort  = src_port;
  tcp->acknum   = intel (intel(tcp->seqnum) + ack_len);
  tcp->seqnum   = intel (seq_num);
  tcp->offset   = sizeof(*tcp) / 4;    /* in dwords */
  seq_num      += data_len;

  ph.src        = ip->source;
  ph.dst        = ip->destination;
  ph.mbz        = 0;
  ph.protocol   = TCP_PROTO;
  ph.length     = intel16 (sizeof(*tcp) + data_len);
  ph.checksum   = CHECKSUM (tcp, sizeof(*tcp) + data_len);

  ip->length    = intel16 (data_len + sizeof(*tcp) + sizeof(*ip));
  ip->checksum  = 0;
  ip->checksum  = ~CHECKSUM (ip, sizeof(*ip));
  tcp->checksum = ~CHECKSUM (&ph, sizeof(ph));
  return (sizeof(*ip) + sizeof(*tcp) + data_len);
}

/*--------------------------------------------------------------------------*/

int sock_loopback (in_Header *ip)
{
  if (ip->proto == TCP_PROTO)
  {
    tcp_Header *tcp = (tcp_Header*) ((BYTE*)ip + in_GetHdrLen(ip));

    if (intel16(tcp->dstPort) == test_port)
    {
      BYTE flags = tcp->flags & tcp_FlagMASK;

      if (flags == tcp_FlagSYN)
      {
        tcp->flags = tcp_FlagACK | tcp_FlagSYN;
        return fix_tcp_packet (ip, tcp, 1, 0);
      }

      if (flags & tcp_FlagACK)
      {
        char *data = (char*)tcp + (tcp->offset << 2);

        if (!strncmp(data,"HELO",4))
        {
          strcpy (data, "Welcome to loop-back handler");
          return fix_tcp_packet (ip, tcp, 4, strlen(data) + 1);
        }
        if (!strncmp(data,"QUIT",4))
        {
          strcpy (data, "Bye from loop-back handler");
          return fix_tcp_packet (ip, tcp, 4, strlen(data) + 1);
        }
      }
    }
  }
  return (-1);
}

/*----------------------------------------------------------------*/

void AssertFail (unsigned line)
{
  fprintf (stderr, "\nAssert fail at line %d\n", line);
  if (sock != -1)
     close_s (sock);
  exit (-1);
}

