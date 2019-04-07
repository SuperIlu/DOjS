#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "mikmod_internals.h"
#include "mikmod_ctype.h"

int _mm_strcasecmp(const char *__s1, const char *__s2)
{
	const char *p1 = __s1;
	const char *p2 = __s2;
	char c1, c2;

	if (p1 == p2) return 0;

	do {
		c1 = mik_tolower(*p1++);
		c2 = mik_tolower(*p2++);
		if (c1 == '\0') break;
	} while (c1 == c2);

	return (int)(c1 - c2);
}
