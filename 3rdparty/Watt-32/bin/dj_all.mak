#
# GNU Makefile for making all Watt-32/djgpp programs *under*
# `$(WATT_ROOT)/bin/'. Use `xxx/makefile.dj' to make samples in
# these directories itself. See also `.\readme.txt' for usage.
#

SUBDIRS = archie bing dig dos-vnc finger2 fping ftp ftpsrv      \
          host http icmppush ioctrl mass-dns mathopd mtr netcat \
          netkala nslookup popmail rdate rdebug smtpd suck      \
          syslogd syslogd2 talk tftpd ttcp verify wget.182      \
        # rsync

#
# For /bin/bash ?
#
COMMAND = cd $(dir); $(MAKE) -f Makefile.dj $@; cd ..;

#
# Use make's internal cd
#
COMMAND = $(MAKE) -C $(dir) -f Makefile.dj $@ ; echo ;

.PHONY: clean depend $(SUBDIRS)

all clean depend:
	- $(foreach dir, $(SUBDIRS), $(COMMAND) )

