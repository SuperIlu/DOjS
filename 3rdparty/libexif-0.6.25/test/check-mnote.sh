#!/bin/sh
# Parses test EXIF files and manipulates maker notes
# This doesn't perform any explicit checks other than the OS's exceptions due
# to NULL pointer exceptions, segmentation faults, divide-by-zero, etc.
#
# Copyright (C) 2024 Dan Fandrich <dan@coneharvesters.com>, et. al.
# SPDX-License-Identifier: LGPL-2.0-or-later
set -e

srcdir="${srcdir:-.}"

for fn in "${srcdir}"/testdata/*.jpg ; do
    ./test-mnote$EXEEXT "${fn}" >/dev/null
done
