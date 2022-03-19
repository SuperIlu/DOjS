/*
 * DZcomm : a serial port API
 * file : dzwin.h
 *
 * DOS-specific header defines.
 * 
 * By Neil Townsend.
 *
 * See readme.txt for copyright information.
 */

#ifndef DZCOMM_WIN
   #error bad include
#endif


#ifndef USE_CONSOLE
   #undef END_OF_MAIN

   #ifdef ALLEGRO_GCC
   #ifdef __cplusplus

      /* GCC version for C++ programs, using __attribute__ ((stdcall)) */
      #define END_OF_MAIN()                                                  \
									     \
	 extern "C" {                                                        \
	    int __attribute__ ((stdcall)) WinMain(void *hI, void *hP, char *Cmd, int nShow); \
									     \
	 }                                                                   \
									     \
	 int __attribute__ ((stdcall)) WinMain(void *hI, void *hP, char *Cmd, int nShow) \
	 {                                                                   \
	    return _WinMain((void *)_mangled_main, hI, hP, Cmd, nShow);      \
	 }

   #else    /* not C++ */

      /* GCC version for C programs, using __attribute__ ((stdcall)) */
      #define END_OF_MAIN()                                                  \
									     \
	 int __attribute__ ((stdcall)) WinMain(void *hI, void *hP, char *Cmd, int nShow); \
									     \
	 int __attribute__ ((stdcall)) WinMain(void *hI, void *hP, char *Cmd, int nShow) \
	 {                                                                   \
	    return _WinMain((void *)_mangled_main, hI, hP, Cmd, nShow);      \
	 }

   #endif   /* end of not C++ */
   #else    /* end of GCC */

      /* MSVC version, using __stdcall */
      #define END_OF_MAIN()                                                  \
									     \
	 int __stdcall WinMain(void *hI, void *hP, char *Cmd, int nShow)     \
	 {                                                                   \
	    return _WinMain((void *)_mangled_main, hI, hP, Cmd, nShow);      \
	 }

   #endif

#endif



/*******************************************/
/************* system drivers **************/
/*******************************************/
#define SYSTEM_DIRECTX           AL_ID('D','X',' ',' ')

AL_VAR(SYSTEM_DRIVER, system_directx);



/*******************************************/
/************** timer drivers **************/
/*******************************************/
#define TIMER_WIN32_ST           AL_ID('W','3','2','S')
#define TIMER_WIN32_MT           AL_ID('W','3','2','M')

AL_VAR(TIMER_DRIVER, timer_win32_st);
AL_VAR(TIMER_DRIVER, timer_win32_mt);



/*******************************************/
/************ keyboard drivers *************/
/*******************************************/
#define KEYBOARD_DIRECTX         AL_ID('D','X',' ',' ')

AL_VAR(KEYBOARD_DRIVER, keyboard_directx);



/*******************************************/
/************* mouse drivers ***************/
/*******************************************/
#define MOUSE_DIRECTX            AL_ID('D','X',' ',' ')

AL_VAR(MOUSE_DRIVER, mouse_directx);



/*******************************************/
/*************** gfx drivers ***************/
/*******************************************/
#define GFX_DIRECTX              AL_ID('D','X','A','C')
#define GFX_DIRECTX_ACCEL        AL_ID('D','X','A','C')
#define GFX_DIRECTX_SAFE         AL_ID('D','X','S','A')
#define GFX_DIRECTX_SOFT         AL_ID('D','X','S','O')
#define GFX_DIRECTX_OVL          AL_ID('D','X','O','V')
#define GFX_DIRECTX_DBLBUF       AL_ID('D','X','D','B')
#define GFX_GDI                  AL_ID('G','D','I','B')

AL_VAR(GFX_DRIVER, gfx_directx_accel);
AL_VAR(GFX_DRIVER, gfx_directx_safe);
AL_VAR(GFX_DRIVER, gfx_directx_soft);
AL_VAR(GFX_DRIVER, gfx_directx_ovl);
AL_VAR(GFX_DRIVER, gfx_gdi);

#define GFX_DRIVER_DIRECTX                                              \
   {  GFX_DIRECTX_ACCEL,   &gfx_directx_accel,     TRUE  },             \
   {  GFX_DIRECTX_SOFT,    &gfx_directx_soft,      TRUE  },             \
   {  GFX_DIRECTX_SAFE,    &gfx_directx_safe,      TRUE  },             \
   {  GFX_GDI,             &gfx_gdi,               FALSE },             \
   {  GFX_DIRECTX_OVL,     &gfx_directx_ovl,       FALSE }, 

#define GFX_SAFE_ID              GFX_DIRECTX_SAFE
#define GFX_SAFE_DEPTH           8
#define GFX_SAFE_W               640
#define GFX_SAFE_H               480



/********************************************/
/*************** sound drivers **************/
/********************************************/
#define DIGI_DIRECTX(n)          AL_ID('D','X','A'+(n),' ')
#define MIDI_WIN32(n)            AL_ID('W','3','2','A'+(n))



/*******************************************/
/************ joystick drivers *************/
/*******************************************/
#define JOY_TYPE_WIN32           AL_ID('W','3','2',' ')

AL_VAR(JOYSTICK_DRIVER, joystick_win32);

#define JOYSTICK_DRIVER_WIN32                                     \
      { JOY_TYPE_WIN32,          &joystick_win32,  TRUE  },

AL_FUNC(int, calibrate_joystick_tl, (void));
AL_FUNC(int, calibrate_joystick_br, (void));
AL_FUNC(int, calibrate_joystick_throttle_min, (void));
AL_FUNC(int, calibrate_joystick_throttle_max, (void));
AL_FUNC(int, calibrate_joystick_hat, (int direction));

