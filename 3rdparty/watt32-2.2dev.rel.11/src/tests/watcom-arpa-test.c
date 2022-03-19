/*
 * Linux socket() test on Windows.
 * With the help of OpenWatcom and Linux Subsystem for Windows 10.
 * https://msdn.microsoft.com/en-us/commandline/wsl/install-win10?f=255&MSPPError=-2147217396
 */

#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>

int open_sock (const struct sockaddr_in *addr)
{
  int fd, ret;

  fd = socket (AF_INET, SOCK_DGRAM, 0);
  if (fd < 0)
       perror ("socket");
  else close (fd);
  return (0);
}

int main (void)
{
  struct sockaddr_in addr;

  memset (&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
//open_sock (&addr);
  return (-1);
}
