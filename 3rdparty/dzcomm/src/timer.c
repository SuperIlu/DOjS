/*
 * DZcomm : a serial port API.
 * file : timer.c
 *
 * Timer implementation if needed.
 * 
 * See readme.txt for copyright information.
 *
 * v1.0 Directly from the allegro distribution:
 */
/*         ______   ___    ___ 
 *        /\  _  \ /\_ \  /\_ \ 
 *        \ \ \L\ \\//\ \ \//\ \      __     __   _ __   ___ 
 *         \ \  __ \ \ \ \  \ \ \   /'__`\ /'_ `\/\`'__\/ __`\
 *          \ \ \/\ \ \_\ \_ \_\ \_/\  __//\ \L\ \ \ \//\ \L\ \
 *           \ \_\ \_\/\____\/\____\ \____\ \____ \ \_\\ \____/
 *            \/_/\/_/\/____/\/____/\/____/\/___L\ \/_/ \/___/
 *                                           /\____/
 *                                           \_/__/
 *
 *      Timer interrupt routines.
 *
 *      By Shawn Hargreaves.
 *
 *      See readme.txt for copyright information.
 */


#include <time.h>

/* This #define tells dzcomm.h that this is source file for the library and
 * not to include things the are only for the users code.
 */
#define DZCOMM_LIB_SRC_FILE

#include "dzcomm.h"
#include "dzcomm/dzintern.h"



DZ_TIMER_DRIVER *dz_timer_driver = NULL;        /* the active driver */

int _dzdos_timer_installed = FALSE;

DZ_TIMER_QUEUE _dz_timer_queue[MAX_TIMERS];     /* list of active callbacks */

volatile int dz_retrace_count = 0;           /* used for retrace syncing */
void (*dz_retrace_proc)(void) = NULL;

static long dz_vsync_counter;                /* retrace position counter */

int _dz_timer_use_retrace = FALSE;           /* are we synced to the retrace? */

volatile int _dz_retrace_hpp_value = -1;     /* to set during next retrace */

static int dz_timer_semaphore = FALSE;       /* reentrant interrupt? */

static volatile long dz_timer_delay = 0;     /* lost interrupt rollover */


/* This should really be handled in a more complete manner, but it's only needed here */
static void dz_yield_timeslice()
{
   #ifdef ALLEGRO_DJGPP

      __dpmi_yield();

   #endif
}


/* _handle_timer_tick:
 *  Called by the driver to handle a timer tick.
 */
long _dz_handle_timer_tick(int interval)
{
   long new_delay = 0x8000;
   long d;
   int i;

   dz_timer_delay += interval;

   /* reentrant interrupt? */
   if (dz_timer_semaphore)
      return 0x2000;

   dz_timer_semaphore = TRUE;
   d = dz_timer_delay;

   /* deal with retrace synchronisation */
   dz_vsync_counter -= d; 

   while (dz_vsync_counter <= 0) {
      dz_vsync_counter += BPS_TO_TIMER(70);
      dz_retrace_count++;
      if (dz_retrace_proc)
	 dz_retrace_proc();
   }

   /* process the user callbacks */
   for (i=0; i<MAX_TIMERS; i++) { 
      if (((_dz_timer_queue[i].proc) || (_dz_timer_queue[i].param_proc)) &&
	  (_dz_timer_queue[i].speed > 0)) {

	 _dz_timer_queue[i].counter -= d;

	 while ((_dz_timer_queue[i].counter <= 0) && 
		((_dz_timer_queue[i].proc) || (_dz_timer_queue[i].param_proc)) && 
		(_dz_timer_queue[i].speed > 0)) {
	    _dz_timer_queue[i].counter += _dz_timer_queue[i].speed;
	    if (_dz_timer_queue[i].param_proc)
	       _dz_timer_queue[i].param_proc(_dz_timer_queue[i].param);
	    else
	       _dz_timer_queue[i].proc();
	 }

	 if ((_dz_timer_queue[i].counter > 0) && 
	     ((_dz_timer_queue[i].proc) || (_dz_timer_queue[i].param_proc)) && 
	     (_dz_timer_queue[i].counter < new_delay)) {
	    new_delay = _dz_timer_queue[i].counter;
	 }
      }
   }

   dz_timer_delay -= d;
   dz_timer_semaphore = FALSE;

   return new_delay;
}

END_OF_FUNCTION(_dz_handle_timer_tick);



/* simple interrupt handler for the rest() function */
static volatile long dz_rest_count;

static void dz_rest_int(void)
{
   dz_rest_count--;
}

END_OF_STATIC_FUNCTION(dz_rest_int);



/* rest_callback:
 *  Waits for time milliseconds.
 */
void dz_rest_callback(long time, void (*callback)(void))
{
   if (dz_timer_driver) {
      if (dz_timer_driver->rest) {
	 dz_timer_driver->rest(time, callback);
      }
      else {
	 dz_rest_count = time;

	 if (dz_install_int(dz_rest_int, 1) < 0)
	    return;

	 do {
	    if (callback)
	       callback();
	    else
	       dz_yield_timeslice();

	 } while (dz_rest_count > 0);

	 dz_remove_int(dz_rest_int);
      }
   }
   else {
      time = clock() + MIN(time * CLOCKS_PER_SEC / 1000, 2);
      do {
      } while (clock() < time);
   }
}



/* rest:
 *  Waits for time milliseconds.
 */
void dz_rest(long time)
{
   dz_rest_callback(time, NULL);
}



/* timer_can_simulate_retrace:
 *  Checks whether the current driver is capable of a video retrace
 *  syncing mode.
 */
int dz_timer_can_simulate_retrace()
{
   if ((dz_timer_driver) && (dz_timer_driver->can_simulate_retrace))
      return dz_timer_driver->can_simulate_retrace();
   else
      return FALSE;
}



/* timer_simulate_retrace:
 *  Turns retrace simulation on or off, and if turning it on, calibrates
 *  the retrace timer.
 */
void dz_timer_simulate_retrace(int enable)
{
   if (!dz_timer_can_simulate_retrace())
      return;

   dz_timer_driver->simulate_retrace(enable);
}



/* timer_is_using_retrace:
 *  Tells the user whether the current driver is providing a retrace
 *  sync.
 */
int dz_timer_is_using_retrace()
{
   return _dz_timer_use_retrace;
}



/* find_timer_slot:
 *  Searches the list of user timer callbacks for a specified function, 
 *  returning the position at which it was found, or -1 if it isn't there.
 */
static int dz_find_timer_slot(void (*proc)(void))
{
   int x;

   for (x=0; x<MAX_TIMERS; x++)
      if (_dz_timer_queue[x].proc == proc)
	 return x;

   return -1;
}

END_OF_STATIC_FUNCTION(dz_find_timer_slot);



/* find_param_timer_slot:
 *  Searches the list of user timer callbacks for a specified paramater 
 *  function, returning the position at which it was found, or -1 if it 
 *  isn't there.
 */
static int dz_find_param_timer_slot(void (*proc)(void *param), void *param)
{
   int x;

   for (x=0; x<MAX_TIMERS; x++)
      if ((_dz_timer_queue[x].param_proc == proc) && (_dz_timer_queue[x].param == param))
	 return x;

   return -1;
}

END_OF_STATIC_FUNCTION(dz_find_param_timer_slot);



/* find_empty_timer_slot:
 *  Searches the list of user timer callbacks for an empty slot.
 */
static int dz_find_empty_timer_slot(void)
{
   int x;

   for (x=0; x<MAX_TIMERS; x++)
      if ((!_dz_timer_queue[x].proc) && (!_dz_timer_queue[x].param_proc))
	 return x;

   return -1;
}

END_OF_STATIC_FUNCTION(dz_find_empty_timer_slot);



/* install_timer_int:
 *  Installs a function into the list of user timers, or if it is already 
 *  installed, adjusts its speed. This function will be called once every 
 *  speed timer ticks. Returns a negative number if there was no room to 
 *  add a new routine.
 */
int dz_install_timer_int(void *proc, void *param, long speed, int param_used)
{
   int x;

   if (!dz_timer_driver) {                   /* make sure we are installed */
      if (dz_install_timer() != 0)
	 return -1;
   }

   if (param_used) {
      if (dz_timer_driver->install_param_int) 
	 return dz_timer_driver->install_param_int(proc, param, speed);

      x = dz_find_param_timer_slot(proc, param);
   }
   else {
      if (dz_timer_driver->install_int) 
	 return dz_timer_driver->install_int(proc, speed);

      x = dz_find_timer_slot(proc); 
   }

   if (x < 0)
      x = dz_find_empty_timer_slot();

   if (x < 0)
      return -1;

   if ((proc == _dz_timer_queue[x].proc) || (proc == _dz_timer_queue[x].param_proc)) { 
      _dz_timer_queue[x].counter -= _dz_timer_queue[x].speed;
      _dz_timer_queue[x].counter += speed;
   }
   else {
      _dz_timer_queue[x].counter = speed;
      if (param_used) {
	 _dz_timer_queue[x].param = param;
	 _dz_timer_queue[x].param_proc = proc;
      }
      else
	 _dz_timer_queue[x].proc = proc;
   }

   _dz_timer_queue[x].speed = speed;

   return 0;
}

END_OF_FUNCTION(dz_install_timer_int);



/* install_int:
 *  Wrapper for install_timer_int, which takes the speed in milliseconds,
 *  no paramater.
 */
int dz_install_int(void (*proc)(void), long speed)
{
   return dz_install_timer_int((void *)proc, NULL, MSEC_TO_TIMER(speed), 0);
}

END_OF_FUNCTION(dz_install_int);



/* install_int_ex:
 *  Wrapper for install_timer_int, which takes the speed in timer ticks,
 *  no paramater.
 */
int dz_install_int_ex(void (*proc)(void), long speed)
{
   return dz_install_timer_int((void *)proc, NULL, speed, 0);
}

END_OF_FUNCTION(dz_install_int_ex);



/* install_param_int:
 *  Wrapper for install_timer_int, which takes the speed in milliseconds,
 *  with paramater.
 */
int dz_install_param_int(void (*proc)(void *param), void *param, long speed)
{
   return dz_install_timer_int((void *)proc, param, MSEC_TO_TIMER(speed), 1);
}

END_OF_FUNCTION(dz_install_param_int);



/* install_param_int_ex:
 *  Wrapper for install_timer_int, which takes the speed in timer ticks,
 *  no paramater.
 */
int dz_install_param_int_ex(void (*proc)(void *param), void *param, long speed)
{
   return dz_install_timer_int((void *)proc, param, speed, 1);
}

END_OF_FUNCTION(dz_install_param_int_ex);



/* remove_int:
 *  Removes a function from the list of user timers.
 */
void dz_remove_int(void (*proc)(void))
{
   int x;

   if ((dz_timer_driver) && (dz_timer_driver->remove_int)) {
      dz_timer_driver->remove_int(proc);
      return;
   }

   x = dz_find_timer_slot(proc);

   if (x >= 0) {
      _dz_timer_queue[x].proc = NULL;
      _dz_timer_queue[x].speed = 0;
      _dz_timer_queue[x].counter = 0;
   }
}

END_OF_FUNCTION(dz_remove_int);



/* remove_param_int:
 *  Removes a function from the list of user timers.
 */
void dz_remove_param_int(void (*proc)(void *param), void *param)
{
   int x;

   if ((dz_timer_driver) && (dz_timer_driver->remove_param_int)) {
      dz_timer_driver->remove_param_int(proc, param);
      return;
   }

   x = dz_find_param_timer_slot(proc, param);

   if (x >= 0) {
      _dz_timer_queue[x].param_proc = NULL;
      _dz_timer_queue[x].param = NULL;
      _dz_timer_queue[x].speed = 0;
      _dz_timer_queue[x].counter = 0;
   }
}

END_OF_FUNCTION(dz_remove_param_int);


/* remove_timer:
 *  Removes our timer handler and resets the BIOS clock. You don't normally
 *  need to call this, because allegro_exit() will do it for you.
 */
void dz_remove_timer(void)
{
   if (!dz_timer_driver)
      return;

   _dz_timer_use_retrace = FALSE;

   dz_timer_driver->exit();
   dz_timer_driver = NULL;

   _dzdos_timer_installed = FALSE;
}


/* install_timer:
 *  Installs the timer interrupt handler. You must do this before installing
 *  any user timer routines. You must set up the timer before trying to 
 *  display a mouse pointer or using any of the GUI routines.
 */
int dz_install_timer()
{
   _DZ_DRIVER_INFO *driver_list;
   int i;

   if (dz_timer_driver)
      return 0;

   for (i=0; i<MAX_TIMERS; i++) {
      _dz_timer_queue[i].proc = NULL;
      _dz_timer_queue[i].param_proc = NULL;
      _dz_timer_queue[i].param = NULL;
      _dz_timer_queue[i].speed = 0;
      _dz_timer_queue[i].counter = 0;
   }

   dz_retrace_proc = NULL;
   dz_vsync_counter = BPS_TO_TIMER(70);
   _dz_timer_use_retrace = FALSE;
   _dz_retrace_hpp_value = -1;
   dz_timer_delay = 0;

   LOCK_VARIABLE(dz_timer_driver);
   LOCK_VARIABLE(dz_timer_delay);
   LOCK_VARIABLE(_dz_timer_queue);
   LOCK_VARIABLE(dz_timer_semaphore);
   LOCK_VARIABLE(dz_vsync_counter);
   LOCK_VARIABLE(_dz_timer_use_retrace);
   LOCK_VARIABLE(_dz_retrace_hpp_value);
   LOCK_VARIABLE(dz_retrace_count);
   LOCK_VARIABLE(dz_retrace_proc);
   LOCK_VARIABLE(dz_rest_count);
   LOCK_FUNCTION(dz_rest_int);
   LOCK_FUNCTION(_dz_handle_timer_tick);
   LOCK_FUNCTION(dz_find_timer_slot);
   LOCK_FUNCTION(dz_find_param_timer_slot);
   LOCK_FUNCTION(dz_find_empty_timer_slot);
   LOCK_FUNCTION(dz_install_timer_int);
   LOCK_FUNCTION(dz_install_int);
   LOCK_FUNCTION(dz_install_int_ex);
   LOCK_FUNCTION(dz_install_param_int);
   LOCK_FUNCTION(dz_install_param_int_ex);
   LOCK_FUNCTION(dz_remove_int);
   LOCK_FUNCTION(dz_remove_param_int);

   /* autodetect a driver */
   driver_list = _dz_timer_driver_list;

   for (i=0; driver_list[i].driver; i++) {
      dz_timer_driver = driver_list[i].driver;
      if (dz_timer_driver->init() == 0)
	 break;
   }

   if (!driver_list[i].driver) {
      dz_timer_driver = NULL;
      return -1;
   }

   atexit(dz_remove_timer);
   _dzdos_timer_installed = TRUE;

   return 0;
}

