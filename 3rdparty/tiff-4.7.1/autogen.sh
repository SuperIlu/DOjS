#!/bin/sh

retval=0

set -x

autoreconf --install --force || retval=$?

# Get latest config.guess and config.sub from upstream master since
# these are often out of date.  This requires network connectivity and
# sometimes the site is down, a failure here does not result in
# failure of the whole script.
for file in config.guess config.sub
do
    echo "$0: getting $file..."
    wget --timeout=5 -O config/$file.tmp \
      "https://git.savannah.gnu.org/cgit/config.git/plain/${file}" \
      && mv -f config/$file.tmp config/$file \
      && chmod a+x config/$file
    rm -f config/$file.tmp
done

exit $retval
