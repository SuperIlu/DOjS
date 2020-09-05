#!/usr/bin/perl -w
#
use Socket;
use Net::protoent;
use Net::servent;
use Net::netent;
# use Net::Ping;

$AF_INET = Socket::AF_INET;
# $SOCK_STREAM = 1;

#
# Main
#
{
  print "Simple tests for Watt-32 and Perl\n";
  &test_getXbyY_func();

  #&test_ping_func();
}

#
# test gethostbyname() / gethostbyaddr()
#
sub test_getXbyY_func
{
  local ($name, $addr, $ip);
  local ($aliases, $type, $len);

  print "Testing gethostbyname(\"localhost\") : ";
  ($name, $aliases, $type, $len, $addr) = gethostbyname("localhost");
  die "gethostbyname" unless $addr;
  print_it (inet_ntoa($addr), $aliases);

  print "Testing gethostbyaddr(127.0.0.1)   : ";
  $ip = inet_aton ("127.0.0.1");
  ($name, $aliases, $type, $len, $addr) = gethostbyaddr($ip,$AF_INET);
  die "gethostbyaddr" unless $name;
  print_it ($name, $aliases);

  print "Testing getservbyname(\"echo\",\"tcp\"): ";
  $s = getservbyname ("echo", "tcp");
  die "getservbyname" unless $s;
  printf ("port %d\n", $s->port);

  print "Testing getservbyport(7,\"tcp\")     : ";
  $s = getservbyport (7, "tcp");
  die "getservbyport" unless $s;
  printf ("name \"%s\"\n", $s->name);

  print "Testing getprotobyname(\"udp\")      : ";
  $s = getprotobyname ("udp");
  die "getprotobyname" unless $s;
  printf ("proto %d\n", $s->proto);

  print "Testing getprotobynumber(17)       : ";
  $s = getprotobynumber (17);
  die "getprotobynumber" unless $s;
  printf ("name \"%s\"\n", $s->name);

  if ($^O =~ /Win32/)
  {
    print "getnetbyaddr() and getnetbyname() unsupported\n";
  }
  else
  {
    print "Testing getnetbyname(\"loopback\")   : ";
    $n = getnetbyname ("loopback");
    die "getnetbyaddr" unless $n;
    printf ("net %d\n", $n->net);

    print "Testing getnetbyaddr(127)          : ";
    $n = getnetbyaddr (127, $AF_INET);
    die "getnetbyaddr" unless $n;
    printf ("name \"%s\"\n", $n->name);
  }

}

#
# Test some ping routines in Net::Ping (icmp+udp+tcp ping)
#
sub test_ping_func
{
  $host = inet_aton ("127.0.0.1");

  # do and ICMP ping, 10sec timeout, 1000 bytes
  $p = Net::Ping->new ("icmp",10,1000);
  print "ping %s\n", $p->ping($host) ? "ok" : "fail";
  $p->close();

}

sub print_it
{
  my ($name, $alias) = @_;
  local $len = printf ("%s, ", $name);

  printf ("%.*s", 100-$len, "alias: ");
  if ($alias) {
    printf ("\"%s\"\n", $alias);
  }
  else {
    print "<none>\n";
  }
}


