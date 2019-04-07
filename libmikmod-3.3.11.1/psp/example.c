#include <pspkernel.h>
#include <pspdebug.h>
#include <pspctrl.h>
#include <pspdisplay.h>
#include <stdlib.h>
#include <string.h>

#include "mikmod.h"


/* Define the module info section */
PSP_MODULE_INFO("LIBMIKMODTEST", 0x1000, 1, 1);

/* Define the main thread's attribute value (optional) */
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER | THREAD_ATTR_VFPU);

/* Define printf, just to make typing easier */
#define printf	pspDebugScreenPrintf

int done = 0;
int mikModThreadID = -1;


/* Exit callback */
int exit_callback(int arg1, int arg2, void *common)
{
	done = 1;
	return 0;
}

/* Callback thread */
int CallbackThread(SceSize args, void *argp)
{
	int cbid;

	cbid = sceKernelCreateCallback("Exit Callback", exit_callback, NULL);
	sceKernelRegisterExitCallback(cbid);
	sceKernelSleepThreadCB();

	return 0;
}

/* Sets up the callback thread and returns its thread id */
int SetupCallbacks(void)
{
	int thid = 0;

	thid = sceKernelCreateThread("update_thread", CallbackThread,
				     0x11, 0xFA0, 0, 0);
	if(thid >= 0)
	{
		sceKernelStartThread(thid, 0, 0);
	}

	return thid;
}

void my_error_handler(void)
{
	printf("_mm_critical %d\n", MikMod_critical);
	printf("_mm_errno %d\n", MikMod_errno);
	printf("%s\n", MikMod_strerror(MikMod_errno));
	return;
}

static int AudioChannelThread(int args, void *argp)
{
	while (!done)
	{
		MikMod_Update();
		/* have to sleep here to allow other threads a chance to process.
		 * with no sleep, this thread will take over when the output is
		 * disabled via MikMod_DisableOutput()
		 * co-operative threading sucks bigtime... */
		sceKernelDelayThread(1);
	}
	return (0);
}

int main(void)
{
	SceCtrlData pad, lastpad;

	int maxchan = 128;
	BOOL outputEnabled;
	MODULE *mf = NULL;
	SAMPLE *sf = NULL;
	int voice = 0;
	int pan = 127;
	int vol = 127;
	int freq = 22000;

	pspDebugScreenInit();
	SetupCallbacks();

	sceCtrlSetSamplingCycle(0);
	sceCtrlSetSamplingMode(1);
	if (!MikMod_InitThreads()) {
		printf("MikMod thread init failed\n");
	}

	MikMod_RegisterErrorHandler(my_error_handler);
	/* register all the drivers */
	MikMod_RegisterAllDrivers();
	/* register all the module loaders */
	MikMod_RegisterAllLoaders();

	/* initialize the library */
	md_mode = DMODE_16BITS|DMODE_STEREO|DMODE_SOFT_SNDFX|DMODE_SOFT_MUSIC; 
	md_reverb = 0;
	md_pansep = 128;
	if (MikMod_Init("")) {
		printf("Could not initialize sound, reason: %s\n", MikMod_strerror(MikMod_errno));
		sceKernelExitGame();
		return 0;
	}

	MikMod_SetNumVoices(-1, 8);
	/* get ready to play */
	sf = Sample_Load("ms0:/sound.wav");

	printf("Starting.\n");
	MikMod_EnableOutput();
	outputEnabled = 1;

	if ((mikModThreadID = sceKernelCreateThread("MikMod" ,(void*)&AudioChannelThread,0x12,0x10000,0,NULL)) > 0) {
		sceKernelStartThread(mikModThreadID, 0 , NULL);
	}
	else {
		printf("Play thread create failed!\n");
	}

	sceCtrlReadBufferPositive(&lastpad, 1);
	do {
		sceCtrlReadBufferPositive(&pad, 1);

		if(pad.Buttons != lastpad.Buttons) {
			if(pad.Buttons & PSP_CTRL_CROSS) {
				voice = Sample_Play(sf,0,0);
				Voice_SetPanning(voice, pan);
			}

			if(pad.Buttons & PSP_CTRL_SQUARE) {
				outputEnabled = !outputEnabled;
				if(outputEnabled)
					MikMod_EnableOutput();
				else	MikMod_DisableOutput();
			}

			if(pad.Buttons & PSP_CTRL_CIRCLE) {
				mf = Player_Load("ms0:/MUSIC.XM", maxchan, 0);
				if (NULL != mf) {
					mf->wrap = 1;
					Player_Start(mf);
				}
			}

			if(pad.Buttons & PSP_CTRL_TRIANGLE) {
				if (NULL != mf) {
					Player_Stop();
					Player_Free(mf); /* To stop the song for real, it needs to be freed. I know, weird... */
					mf = NULL;
				}
			}

			if(pad.Buttons & PSP_CTRL_SELECT)
				printf("Player is %s\n", Player_Active()?"On":"Off");

			lastpad = pad;
		}

		if(pad.Buttons & PSP_CTRL_LTRIGGER) {
			Voice_SetPanning(voice, (pan<2)?pan:--pan);
			printf("pan is %d\n", pan);
		}

		if(pad.Buttons & PSP_CTRL_RTRIGGER) {
			Voice_SetPanning(voice, (pan>254)?pan:++pan);
			printf("pan is %d\n", pan);
		}

		if(pad.Buttons & PSP_CTRL_UP) {
			Voice_SetVolume(voice, (vol>254)?vol:++vol);
			printf("vol is %d\n", vol);
		}

		if(pad.Buttons & PSP_CTRL_DOWN) {
			Voice_SetVolume(voice, (vol<2)?vol:--vol);
			printf("vol is %d\n", vol);
		}

		if(pad.Buttons & PSP_CTRL_LEFT) {
			Voice_SetFrequency(voice, (freq<1001)?freq:(freq -=1000));
			printf("freq is %d\n", freq);
		}

		if(pad.Buttons & PSP_CTRL_RIGHT) {
			Voice_SetFrequency(voice, (freq>44000)?freq:(freq +=1000));
			printf("freq is %d\n", freq);
		}
		sceDisplayWaitVblankStart();
		
	} while(!((pad.Buttons & PSP_CTRL_START) || done));

	printf("Stopping.\n");

	/* allow audio thread to terminate cleanly */
	done = 1;
	if (mikModThreadID > 0) {
		SceUInt timeout = 100000;
		sceKernelWaitThreadEnd(mikModThreadID, &timeout);
		/* not 100% sure if this is necessary after a clean exit,
		 * but just to make sure any resources are freed: */
		sceKernelDeleteThread(mikModThreadID);
	}
	Player_Stop();
	Player_Free(mf);
	MikMod_Exit();

	sceKernelExitGame();
	return 0;
}
