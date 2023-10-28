/*
The newlib subdirectory is a collection of software from several sources.

Each file may have its own copyright/license that is embedded in the source
file.  Unless otherwise noted in the body of the source file(s), the following copyright
notices will apply to the contents of the newlib subdirectory:

(1) Red Hat Incorporated

Copyright (c) 1994-2009  Red Hat, Inc. All rights reserved.

This copyrighted material is made available to anyone wishing to use,
modify, copy, or redistribute it subject to the terms and conditions
of the BSD License.   This program is distributed in the hope that
it will be useful, but WITHOUT ANY WARRANTY expressed or implied,
including the implied warranties of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.  A copy of this license is available at
http://www.opensource.org/licenses. Any Red Hat trademarks that are
incorporated in the source code or documentation are not subject to
the BSD License and may only be used or replicated with the express
permission of Red Hat, Inc.
*/
/*
 * mktime.c
 * Original Author:	G. Haley
 *
 * Converts the broken-down time, expressed as local time, in the structure
 * pointed to by tim_p into a calendar time value. The original values of the
 * tm_wday and tm_yday fields of the structure are ignored, and the original
 * values of the other fields have no restrictions. On successful completion
 * the fields of the structure are set to represent the specified calendar
 * time. Returns the specified calendar time. If the calendar time can not be
 * represented, returns the value (time_t) -1.
 *
 * Modifications:	Fixed tm_isdst usage - 27 August 2008 Craig Howland.
 */

/*
FUNCTION
<<mktime>>---convert time to arithmetic representation

INDEX
        mktime

SYNOPSIS
        #include <time.h>
        time_t mktime(struct tm *<[timp]>);

DESCRIPTION
<<mktime>> assumes the time at <[timp]> is a local time, and converts
its representation from the traditional representation defined by
<<struct tm>> into a representation suitable for arithmetic.

<<localtime>> is the inverse of <<mktime>>.

RETURNS
If the contents of the structure at <[timp]> do not form a valid
calendar time representation, the result is <<-1>>.  Otherwise, the
result is the time, converted to a <<time_t>> value.

PORTABILITY
ANSI C requires <<mktime>>.

<<mktime>> requires no supporting OS subroutines.
*/

#include <stdlib.h>
#include <time.h>
#include <dpmi.h>
#include <string.h>

#include "DOjS.h"

#define _SEC_IN_MINUTE 60L
#define _SEC_IN_HOUR 3600L
#define _SEC_IN_DAY 86400L

static const int DAYS_IN_MONTH[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

#define _DAYS_IN_MONTH(x) ((x == 1) ? days_in_feb : DAYS_IN_MONTH[x])

static const int _DAYS_BEFORE_MONTH[12] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};

#define _ISLEAP(y) (((y) % 4) == 0 && (((y) % 100) != 0 || (((y) + 1900) % 400) == 0))
#define _DAYS_IN_YEAR(year) (_ISLEAP(year) ? 366 : 365)

typedef struct __tzrule_struct {
    char ch;
    int m; /* Month of year if ch=M */
    int n; /* Week of month if ch=M */
    int d; /* Day of week if ch=M, day of year if ch=J or ch=D */
    int s; /* Time of day in seconds */
    time_t change;
    long offset; /* Match type of _timezone. */
} __tzrule_type;

typedef struct __tzinfo_struct {
    int __tznorth;
    int __tzyear;
    __tzrule_type __tzrule[2];
} __tzinfo_type;

/* Shared timezone information for libc/time functions.  */
static __tzinfo_type tzinfo = {1, 0, {{'J', 0, 0, 0, 0, (time_t)0, 0L}, {'J', 0, 0, 0, 0, (time_t)0, 0L}}};

static __tzinfo_type *__gettzinfo(void) { return &tzinfo; }

static void validate_structure(struct tm *tim_p) {
    div_t res;
    int days_in_feb = 28;

    /* calculate time & date to account for out of range values */
    if (tim_p->tm_sec < 0 || tim_p->tm_sec > 59) {
        res = div(tim_p->tm_sec, 60);
        tim_p->tm_min += res.quot;
        if ((tim_p->tm_sec = res.rem) < 0) {
            tim_p->tm_sec += 60;
            --tim_p->tm_min;
        }
    }

    if (tim_p->tm_min < 0 || tim_p->tm_min > 59) {
        res = div(tim_p->tm_min, 60);
        tim_p->tm_hour += res.quot;
        if ((tim_p->tm_min = res.rem) < 0) {
            tim_p->tm_min += 60;
            --tim_p->tm_hour;
        }
    }

    if (tim_p->tm_hour < 0 || tim_p->tm_hour > 23) {
        res = div(tim_p->tm_hour, 24);
        tim_p->tm_mday += res.quot;
        if ((tim_p->tm_hour = res.rem) < 0) {
            tim_p->tm_hour += 24;
            --tim_p->tm_mday;
        }
    }

    if (tim_p->tm_mon < 0 || tim_p->tm_mon > 11) {
        res = div(tim_p->tm_mon, 12);
        tim_p->tm_year += res.quot;
        if ((tim_p->tm_mon = res.rem) < 0) {
            tim_p->tm_mon += 12;
            --tim_p->tm_year;
        }
    }

    if (_DAYS_IN_YEAR(tim_p->tm_year) == 366) days_in_feb = 29;

    if (tim_p->tm_mday <= 0) {
        while (tim_p->tm_mday <= 0) {
            if (--tim_p->tm_mon == -1) {
                tim_p->tm_year--;
                tim_p->tm_mon = 11;
                days_in_feb = ((_DAYS_IN_YEAR(tim_p->tm_year) == 366) ? 29 : 28);
            }
            tim_p->tm_mday += _DAYS_IN_MONTH(tim_p->tm_mon);
        }
    } else {
        while (tim_p->tm_mday > _DAYS_IN_MONTH(tim_p->tm_mon)) {
            tim_p->tm_mday -= _DAYS_IN_MONTH(tim_p->tm_mon);
            if (++tim_p->tm_mon == 12) {
                tim_p->tm_year++;
                tim_p->tm_mon = 0;
                days_in_feb = ((_DAYS_IN_YEAR(tim_p->tm_year) == 366) ? 29 : 28);
            }
        }
    }
}

static time_t mytime(struct tm *tim_p) {
    time_t tim = 0;
    long days = 0;
    int year, isdst = 0;
    __tzinfo_type *tz = __gettzinfo();

    /* validate structure */
    validate_structure(tim_p);

    /* compute hours, minutes, seconds */
    tim += tim_p->tm_sec + (tim_p->tm_min * _SEC_IN_MINUTE) + (tim_p->tm_hour * _SEC_IN_HOUR);

    /* compute days in year */
    days += tim_p->tm_mday - 1;
    days += _DAYS_BEFORE_MONTH[tim_p->tm_mon];
    if (tim_p->tm_mon > 1 && _DAYS_IN_YEAR(tim_p->tm_year) == 366) days++;

    /* compute day of the year */
    tim_p->tm_yday = days;

    if (tim_p->tm_year > 10000 || tim_p->tm_year < -10000) return (time_t)-1;

    /* compute days in other years */
    if ((year = tim_p->tm_year) > 70) {
        for (year = 70; year < tim_p->tm_year; year++) days += _DAYS_IN_YEAR(year);
    } else if (year < 70) {
        for (year = 69; year > tim_p->tm_year; year--) days -= _DAYS_IN_YEAR(year);
        days -= _DAYS_IN_YEAR(year);
    }

    /* compute total seconds */
    tim += (time_t)days * _SEC_IN_DAY;

    /* add appropriate offset to put time in gmt format */
    if (isdst == 1)
        tim += (time_t)tz->__tzrule[1].offset;
    else /* otherwise assume std time */
        tim += (time_t)tz->__tzrule[0].offset;

    /* reset isdst flag to what we have calculated */
    tim_p->tm_isdst = isdst;

    /* compute day of the week */
    if ((tim_p->tm_wday = (days + 4) % 7) < 0) tim_p->tm_wday += 7;

    return tim;
}

/**
 * @brief time() replacement that clears the temporary variables
 *
 * @param t
 * @return time_t
 */
time_t time(time_t *t) {
    time_t tt;
    struct tm tmblk;
    __dpmi_regs r;

    memset(&tmblk, 0, sizeof(tmblk));

    memset(&r, 0, sizeof(r));
    r.h.ah = 0x2c;
    __dpmi_int(0x21, &r);
    // LOGF("INT21h, 0x2c %d:%d:%d / ", r.h.ch, r.h.cl, r.h.dh);

    tmblk.tm_sec = r.h.dh;
    tmblk.tm_min = r.h.cl;
    tmblk.tm_hour = r.h.ch;

    memset(&r, 0, sizeof(r));
    r.h.ah = 0x2a;
    __dpmi_int(0x21, &r);
    // LOGF("INT21h, 0x2a %d-%d-%d / ", r.x.cx & 0x7ff, r.h.dh, r.h.dl);

    tmblk.tm_mday = r.h.dl;
    tmblk.tm_mon = r.h.dh - 1;
    tmblk.tm_year = (r.x.cx & 0x7ff) - 1900;

    tmblk.tm_wday = tmblk.tm_yday = tmblk.tm_gmtoff = 0;
    tmblk.tm_zone = 0;
    tmblk.tm_isdst = -1;

    // LOGF("%d:%d:%d %d-%d-%d / ", tmblk.tm_hour, tmblk.tm_min, tmblk.tm_sec, tmblk.tm_year, tmblk.tm_mon, tmblk.tm_mday);

    tt = mytime(&tmblk);
    // LOGF("mytime() := %d\n", tt);
    if (t) {
        *t = tt;
    }

    return tt;
}
