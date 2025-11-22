# -*- sh -*-
#
# cmp(1) creates the same exit code as diff in case of files being
# different or the same, even though the diff(1) output is more
# informative
#
# Copyright (c) 2021 Hans Ulrich Niedermann <gp@n-dimensional.de>
# SPDX-License-Identifier: LGPL-2.0-or-later

if test "x$DIFF_U" != x && test "x$DIFF_U" != xno
then
    comparetool="$DIFF_U"
elif test "x$DIFF" != x && test "x$DIFF" != xno
then
    comparetool="$DIFF"
else
    comparetool=cmp
fi
