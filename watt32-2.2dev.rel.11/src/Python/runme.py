#! /usr/bin/env python

from __future__ import print_function

import sys, inspect

from watt32   import *
from win32api import Sleep

try:
  from colorama import init, Fore, Style
  init()
  FG_BRIGHTRED = Style.BRIGHT + Fore.RED
  FG_YELLOW    = Style.BRIGHT + Fore.YELLOW
  FG_NORMAL    = Style.RESET_ALL

except ImportError:
  print ("Failed to import colorama: %s" % e)
  FG_BRIGHTRED = ''
  FG_YELLOW    = ''
  FG_NORMAL    = ''

debug = 0
host_to_ping = "www.google.com"

def __LINE__():
  try:
    raise Exception
  except:
    return sys.exc_info()[2].tb_frame.f_back.f_lineno

def __FILE__():
  return inspect.currentframe().f_code.co_filename

def ping (host):
  idx = 100
  ip = lookup_host (host, None)
  num = 0
  if ip == 0:
    err = cvar.dom_errno
    print ('%sUnknown host %s, dom_errno: %d: %s%s' % (FG_YELLOW, host, err, dom_strerror(err), FG_NORMAL))
    return

  while num < 5:
    if _ping(ip, idx, None, 0):
      print ("sent PING # %lu " % idx)
    idx += 1
    num += 1
    tcp_tick (None)
    if _chk_ping(ip,None) != -1:
      print ('%sGot ICMP echo%s' % (FG_YELLOW, FG_NORMAL))
    Sleep (1000)

def show_version (details = 0):
  if details:
    show_content()
  print ("Version:   %s"        % wattcpVersion())
  print ("$(CC):     %s (-D%s)" % (wattcpBuildCCexe(), wattcpBuildCC()))
  print ("$(CFLAGS): %s"        % wattcpBuildCflags())

  capa = wattcpCapabilities().split('/')
  length = 0
  print ("Features: ", end="")
  for c in capa[1:]:
    print (c, end="")
    length += len(c)
    if length > 60:
       length = 0
       print ('\n', ' '*10, end="")
  sys.exit (0)

def show_help():
  print ('''Usage: %s [-dN] [-v] [-h] host to ping
    -d: sets debug level 1.
    -d1: sets debug level 1.
    -d2: sets debug level 2.
    -d3: sets debug level 3.
    -d4: sets debug level 4.
    -v:  show version info and exit.''' % sys.argv[0])
  sys.exit(0)

def set_debug():
  global debug
  print ('set_debug()')
  debug = 1

def set_debug2():
  global debug
  print ('set_debug2()')
  debug = 2

def set_debug3():
  global debug
  print ('set_debug3()')
  debug = 3

def set_debug4():
  global debug
  print ('set_debug4()')
  debug = 4


#
# Parse command-line. '-dN' -> sets debug level.
#                     '-v'  -> show version info and exit.
#                     '-h'  -> show short help and exit.
def parse_cmdline():
  global host_to_ping

  ARGS = { '-d'  : set_debug,
           '-d1' : set_debug,
           '-d2' : set_debug2,
           '-d3' : set_debug3,
           '-d4' : set_debug4,
           '-v'  : show_version,
           '-h'  : show_help }

  if len(sys.argv) < 2:
    return

  print ('%s (%u): sys.argv[1]: %s' % (__FILE__(), __LINE__(), sys.argv[1]))

  # exec (ARGS [sys.argv[1]])

  if sys.argv[1] == '-v':
    show_version ()
  if sys.argv[1] == '-v2':
    show_version (1)
  elif sys.argv[1] == '-h':
    show_help()
  elif sys.argv[1] == '-d':
    set_debug()
  elif sys.argv[1] == '-d1':
    set_debug()
  elif sys.argv[1] == '-d2':
    set_debug2()
  elif sys.argv[1] == '-d3':
    set_debug3()
  elif sys.argv[1] == '-d4':
    set_debug4()

  if len(sys.argv) >= 2:
     host_to_ping = sys.argv[2]

#
# Show the contents of _watt32.pyd
#
def show_content():
  print ("s%s contains" % (FG_YELLOW, watt32._watt32.__file__))
  for s in dir(watt32):
    if debug < 2 and s.startswith('_swig'):
       continue
    print (s)
    if debug > 2:
      if s.startswith('__') and s.endswith('__'):
        print (dir(s))
  print (FG_NORMAL)

#
# Our main() function
#
def main():
  parse_cmdline()
  print ('%s (%u): debug %d:' % (__FILE__(), __LINE__(), debug))

  if debug > 0:
    watt32.cvar.debug_on = debug
    dbug_init()
  sock_init()
  ping (host_to_ping)

if __name__ == '__main__':
  main()
