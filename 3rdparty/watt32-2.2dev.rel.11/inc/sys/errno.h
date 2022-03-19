/*!\file sys/errno.h
 *
 * Compatibility header.
 */

/*
 * The naming <sys/w??.h> is required for those compilers that
 * have <sys/??.h> in the usual place but doesn't define stuff
 * related to Watt-32's BSD-socket interface.
 */

#ifndef __SYS_WERRNO_H
#include <sys/werrno.h>
#endif

/*
 * This file shadows CygWin's <sys/errno.h>. This hack pulls
 * in the default <sys/errno.h>
 */
#ifdef __CYGWIN__
#include_next <sys/errno.h>
#endif

