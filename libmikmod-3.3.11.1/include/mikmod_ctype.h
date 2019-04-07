/* Locale insensitive ctype.h functions taken from the RPM library.
 * RPM is Copyright (c) 1998 by Red Hat Software, Inc.
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */

#ifndef MIKMOD_CTYPE_H
#define MIKMOD_CTYPE_H

static inline int mik_isascii(int c) {
    return ((c & ~0x7f) == 0);
}

static inline int mik_islower(int c) {
    return (c >= 'a' && c <= 'z');
}

static inline int mik_isupper(int c) {
    return (c >= 'A' && c <= 'Z');
}

static inline int mik_isalpha(int c) {
    return (mik_islower(c) || mik_isupper(c));
}

static inline int mik_isdigit(int c) {
    return (c >= '0' && c <= '9');
}

static inline int mik_isxdigit(int c) {
    return (mik_isdigit(c) || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'));
}

static inline int mik_isalnum(int c) {
    return (mik_isalpha(c) || mik_isdigit(c));
}

static inline int mik_isblank(int c) {
    return (c == ' ' || c == '\t');
}

static inline int mik_isspace(int c) {
    switch (c) {
    case ' ':  case '\t':
    case '\n': case '\r':
    case '\f': case '\v': return 1;
    }
    return 0;
}

static inline int mik_isgraph(int c) {
    return (c > 0x20 && c <= 0x7e);
}

static inline int mik_isprint(int c) {
    return (c >= 0x20 && c <= 0x7e);
}

static inline int mik_toascii(int c) {
    return (c & 0x7f);
}

static inline int mik_tolower(int c) {
    return ((mik_isupper(c)) ? (c | ('a' - 'A')) : c);
}

static inline int mik_toupper(int c) {
    return ((mik_islower(c)) ? (c & ~('a' - 'A')) : c);
}

#endif /* MIKMOD_CTYPE_H */
