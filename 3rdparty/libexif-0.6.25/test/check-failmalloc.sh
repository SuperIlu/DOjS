#!/bin/sh
# Use Failmalloc to test behaviour in the face of out-of-memory conditions.
# The test runs a binary multiple times while configuring Failmalloc to fail a
# different malloc() call each time, while looking for abnormal program exits
# due to segfaults. See https://www.nongnu.org/failmalloc/
#
# Ideally, it would ensure that the test binary returns an error code on each
# failure, but this often doesn't happen. This is a problem that should be
# rectified, but the API doesn't allow returning an error code in many
# functions that could encounter a problem. The issue could be solved in more
# cases with more judicious use of log calls with EXIF_LOG_CODE_NO_MEMORY
# codes.
#
# Copyright (C) 2018-2021 Dan Fandrich <dan@coneharvesters.com>, et. al.
# SPDX-License-Identifier: LGPL-2.0-or-later

srcdir="${srcdir:-.}"

VERBOSE=
if [ "$1" = "-v" ] ; then
    VERBOSE=1
fi

if [ x"$FAILMALLOC_PATH" = x ]; then
    echo "libfailmalloc is not available"
    exit 77
fi

BINARY_PREFIX=./
if [ -e .libs/lt-test-value ]; then
    # If libtool is in use, the normal "binary" is actually a shell script which
    # would be interfered with by libfailmalloc. Instead, use the special lt-
    # binary which should work properly.
    BINARY_PREFIX=".libs/lt-"
fi

# Usage: failmalloc_binary_test #iterations binary <optional arguments>
# FIXME: auto-determine #iterations by comparing the output of each run
# with the output of a normal run, and exiting when that happens.
failmalloc_binary_test () {
  binary="$BINARY_PREFIX$2"
  iterations="$1"
  shift
  shift
  echo Checking "$binary" for "$iterations" iterations
  for n in $(seq "$iterations"); do
      test "$VERBOSE" = 1 && { echo "$n"; set -x; }
      FAILMALLOC_INTERVAL="$n" LD_PRELOAD="${LD_PRELOAD:+$LD_PRELOAD:}$FAILMALLOC_PATH" "$binary" "$@" >/dev/null
      s=$?
      test "$VERBOSE" = 1 && set +x;
      if test "$s" -ge 125; then
          # Such status codes only happen due to termination due to a signal
          # like SIGSEGV or ASAN errors (ignoring a couple that the shell
          # itself produces).
          echo "Abnormal binary exit status $s at malloc #$n on $binary"
          echo FAILURE
          exit 1
      fi
  done
}

# Make ASAN errors return a high number to differentiate them from regular test
# errors (which are ignored). This only does something if ASAN was configured
# in the build.
export ASAN_OPTIONS="exitcode=125${ASAN_OPTIONS:+:$ASAN_OPTIONS}"

# The number of iterations is determined empirically to be about twice as
# high as the maximum number of mallocs performed by the test program in order
# to avoid lowering code coverage in the case of future code changes that cause
# more allocations.

failmalloc_binary_test 700 test-mem
failmalloc_binary_test 500 test-value

for f in ${srcdir}/testdata/*jpg; do
    echo "Testing `basename "$f"`"
    failmalloc_binary_test 600 test-parse$EXEEXT "$f"
done
# N.B. adding the following binaries doesn't actually increase code coverage:
#  test-extract -o /dev/null
#  test-gps
#  test-mnote
#  test-parse --swap-byte-order

echo PASSED
