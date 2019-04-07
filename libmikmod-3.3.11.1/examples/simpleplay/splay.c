/* splay.c
 * A very basic example on how to use libmikmod to play a module.
 *
 * (C) 2004, Raphael Assenat (raph@raphnet.net)
 *
 * This example is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRENTY; without event the implied warrenty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */

#include <mikmod.h>

#if defined(_WIN32)
#define MikMod_Sleep(ns) Sleep(ns / 1000)
#elif defined(_MIKMOD_AMIGA)
void amiga_sysinit (void);
void amiga_usleep (unsigned long timeout);
#define MikMod_Sleep(ns) amiga_usleep(ns)
#else
#include <unistd.h>  /* for usleep() */
#define MikMod_Sleep(ns) usleep(ns)
#endif

int main(int argc, char **argv)
{
	MODULE *module;

	if (argc < 2) {
		fprintf(stderr, "Usage: ./splay file\n");
		return 1;
	}

#ifdef _MIKMOD_AMIGA
	amiga_sysinit ();
#endif

	/* initialize MikMod threads */
	MikMod_InitThreads ();

	/* register all the drivers */
	MikMod_RegisterAllDrivers();

	/* register all the module loaders */
	MikMod_RegisterAllLoaders();

	/* init the library */
	md_mode |= DMODE_SOFT_MUSIC | DMODE_NOISEREDUCTION;
	if (MikMod_Init("")) {
		fprintf(stderr, "Could not initialize sound, reason: %s\n",
				MikMod_strerror(MikMod_errno));
		return 2;
	}

	/* load module */
	module = Player_Load(argv[1], 64, 0);
	if (module) {
		/* start module */
		printf("Playing %s (%d chn)\n", module->songname, (int) module->numchn);
		Player_Start(module);

		while (Player_Active()) {
			MikMod_Sleep(10000);
			MikMod_Update();
		}

		Player_Stop();
		Player_Free(module);
	} else
		fprintf(stderr, "Could not load module, reason: %s\n",
				MikMod_strerror(MikMod_errno));

	MikMod_Exit();

	return 0;
}

