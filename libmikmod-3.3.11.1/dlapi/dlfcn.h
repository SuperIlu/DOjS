#ifndef _MIKMOD_DLFCN_H_
#define	_MIKMOD_DLFCN_H_

#ifdef MIKMOD_DLAPI_HP
#define	RTLD_LAZY	1

extern void *dlopen(const char *, int);
extern int dlclose(void *);
extern void *dlsym(void *, const char *);
#endif	/* MIKMOD_DLAPI_HP */

#endif	/* _MIKMOD_DLFCN_H_ */
