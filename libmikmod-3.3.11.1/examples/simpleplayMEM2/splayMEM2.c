/* splayMEM2.c
 * An example on how to use libmikmod to play a module
 * from a memory buffer using library-provided readers.
 *
 * (C) 2004, Raphael Assenat (raph@raphnet.net)
 *
 * This example is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRENTY; without event the implied warrenty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */

#include <stdlib.h>
#include <stdio.h>
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
	char *data_buf;
	long data_len;
	FILE *fptr;

	if (argc < 2) {
		fprintf(stderr, "Usage: ./splayMEM2 file\n");
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
	md_mode |= DMODE_HQMIXER;

	if (MikMod_Init("")) {
		fprintf(stderr, "Could not initialize sound, reason: %s\n",
				MikMod_strerror(MikMod_errno));
		return 2;
	}

	/* open the file */
	fptr = fopen(argv[1], "rb");
	if (fptr == NULL) {
		perror("fopen");
		MikMod_Exit();
		return 1;
	}

	/* calculate the file size */
	fseek(fptr, 0, SEEK_END);
	data_len = ftell(fptr);
	fseek(fptr, 0, SEEK_SET);

	/* allocate a buffer and load the file into it */
	data_buf = malloc(data_len);
	if (data_buf == NULL) {
		perror("malloc");
		fclose(fptr);
		MikMod_Exit();
		return 1;
	}
	if (fread(data_buf, data_len, 1, fptr) != 1)
	{
		perror("fread");
		fclose(fptr);
		free(data_buf);
		MikMod_Exit();
		return 1;
	}
	fclose(fptr);

	/* load module */
	module = Player_LoadMem(data_buf, data_len, 64, 0);
	free(data_buf);
	if (module) {
		/* start module */
		printf("Playing %s\n", module->songname);
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

