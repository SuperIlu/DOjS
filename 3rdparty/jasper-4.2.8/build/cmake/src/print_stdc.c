#include <stdio.h>
int main(void)
{
	long stdc_version;

#if defined(__STDC__) && defined(__STDC_VERSION__)
	stdc_version = __STDC_VERSION__;
#else
	/*
	stdc_version = 198900L;
	*/
	stdc_version = 0L;
#endif

	printf("%ldL", stdc_version);
	if (fflush(stdout)) {
		return 1;
	}
	return 0;
}
