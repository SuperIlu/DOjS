/*
 * DZcomm : a serial port API.
 * file : dtimer.c
 *
 * Low level DOS timer implementation.
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
 *      DOS timer module.
 *
 *      By Shawn Hargreaves.
 *
 *      Accuracy improvements by Neil Townsend.
 *
 *      See readme.txt for copyright information.
 */


/* This #define tells dzcomm.h that this is source file for the library and
 * not to include things the are only for the users code.
 */
#define DZCOMM_LIB_SRC_FILE

#include "dzcomm.h"
#include "dzcomm/dzintern.h"

#ifndef DZCOMM_DOS
   #error something is wrong with the makefile
#endif



/* Error factor for retrace syncing. Reduce this to spend less time waiting
 * for the retrace, increase it to reduce the chance of missing retraces.
 */
#define VSYNC_MARGIN             1024

#define TIMER_INT                8

#define LOVEBILL_TIMER_SPEED     BPS_TO_TIMER(200)

#define REENTRANT_RECALL_GAP     2500

static long dzdos_bios_counter;                 /* keep BIOS time up to date */

static long dzdos_timer_delay;                  /* how long between interrupts */

static int dzdos_timer_semaphore = FALSE;       /* reentrant interrupt? */

static int dzdos_timer_clocking_loss = 0;       /* unmeasured time that gets lost */

static long dzdos_vsync_counter;                /* retrace position counter */
static long dzdos_vsync_speed;                  /* retrace speed */

int dz_i_love_bill = FALSE;



/* this driver is locked to a fixed rate, which works under win95 etc. */
int dzdos_fixed_timer_init(void);
void dzdos_fixed_timer_exit(void);


DZ_TIMER_DRIVER dzdos_timedrv_fixed_rate =
{
   DZ_TIMEDRV_FIXED_RATE,
   dz_empty_string,
   dz_empty_string,
   "Fixed-rate timer",
   dzdos_fixed_timer_init,
   dzdos_fixed_timer_exit,
   NULL, NULL, NULL, NULL, NULL, NULL, NULL
};



/* this driver varies the speed on the fly, which only works in clean DOS */
int dzdos_var_timer_init(void);
void dzdos_var_timer_exit(void);
int dzdos_var_timer_can_simulate_retrace(void);
void dzdos_var_timer_simulate_retrace(int enable);


DZ_TIMER_DRIVER dzdos_timedrv_variable_rate =
{
   DZ_TIMEDRV_VARIABLE_RATE,
   dz_empty_string,
   dz_empty_string,
   "Variable-rate timer",
   dzdos_var_timer_init,
   dzdos_var_timer_exit,
   NULL, NULL, NULL, NULL,
   dzdos_var_timer_can_simulate_retrace,
   dzdos_var_timer_simulate_retrace,
   NULL
};



/* list the available drivers */
_DZ_DRIVER_INFO _dz_timer_driver_list[] =
{
   { DZ_TIMEDRV_VARIABLE_RATE,   &dzdos_timedrv_variable_rate,    TRUE  },
   { DZ_TIMEDRV_FIXED_RATE,      &dzdos_timedrv_fixed_rate,       TRUE  },
   { 0,                          NULL,                            0     }
};

/* What type of 'DOS' is it ? FROM Allegro ... but we only care about i_love_bill */

/* detect_os:
 *  Operating system autodetection routine.
 */
static void detect_os()
{
   static int done = 0;

   __dpmi_regs r;
   char buf[16];
   char *p;
   int i;

   if (done) return;

   done = 1;

   /* check for Windows 3.1 or 95 */
   r.x.ax = 0x1600; 
   __dpmi_int(0x2F, &r);

   if ((r.h.al != 0) && (r.h.al != 1) && (r.h.al != 0x80) && (r.h.al != 0xFF)) {
      dz_i_love_bill = TRUE;
      return;
   }

   /* check for Windows NT */
   p = getenv("OS");

   if (((p) && (stricmp(p, "Windows_NT") == 0)) || (_get_dos_version(1) == 0x0532)) {
      dz_i_love_bill = TRUE;
      return;
   }

   /* check for OS/2 */
   r.x.ax = 0x4010;
   __dpmi_int(0x2F, &r);

   if (r.x.ax != 0x4010) {
      dz_i_love_bill = TRUE;
      return;
   }

   /* check for Linux DOSEMU */
   _farsetsel(_dos_ds);

   for (i=0; i<8; i++) 
      buf[i] = _farnspeekb(0xFFFF5+i);

   buf[8] = 0;

   if (!strcmp(buf, "02/25/93")) {
      r.x.ax = 0;
      __dpmi_int(0xE6, &r);

      if (r.x.ax == 0xAA55) {
         dz_i_love_bill = TRUE;     /* (evil chortle) */
         return;
      }
   }

   /* check if running under OpenDOS */
   r.x.ax = 0x4452;
   __dpmi_int(0x21, &r);

   if ((r.x.ax >= 0x1072) && !(r.x.flags & 1)) {

      /* now check for OpenDOS EMM386.EXE */
      r.x.ax = 0x12FF;
      r.x.bx = 0x0106;
      __dpmi_int(0x2F, &r);

      if ((r.x.ax == 0) && (r.x.bx == 0xEDC0)) {
         dz_i_love_bill = TRUE;
      }

      return;
   }

   /* check for that stupid win95 "stealth mode" setting */
   r.x.ax = 0x8102;
   r.x.bx = 0;
   r.x.dx = 0;
   __dpmi_int(0x4B, &r);

   if ((r.x.bx == 3) && !(r.x.flags & 1)) {
      dz_i_love_bill = TRUE;
      return;
   }
}


/* set_timer:
 *  Sets the delay time for PIT channel 1 in one-shot mode.
 */
static INLINE void dzdos_set_timer(long time)
{
    outportb(0x43, 0x30);
    outportb(0x40, time & 0xff);
    outportb(0x40, time >> 8);
}



/* set_timer_rate:
 *  Sets the delay time for PIT channel 1 in cycle mode.
 */
static INLINE void dzdos_set_timer_rate(long time)
{
    outportb(0x43, 0x34);
    outportb(0x40, time & 0xff);
    outportb(0x40, time >> 8);
}



/* read_timer:
 *  Reads the elapsed time from PIT channel 1.
 */
static INLINE long dzdos_read_timer()
{
   long x;

   outportb(0x43, 0x00);
   x = inportb(0x40);
   x += inportb(0x40) << 8;

   return (0xFFFF - x + 1) & 0xFFFF;
}



/* fixed_timer_handler:
 *  Interrupt handler for the fixed-rate timer driver.
 */
static int dzdos_fixed_timer_handler()
{
   int bios;

   dzdos_bios_counter -= LOVEBILL_TIMER_SPEED; 

   if (dzdos_bios_counter <= 0) {
      dzdos_bios_counter += 0x10000;
      bios = TRUE;
   }
   else {
      outportb(0x20, 0x20);
      ENABLE();
      bios = FALSE;
   }

   _dz_handle_timer_tick(LOVEBILL_TIMER_SPEED);

   if (!bios)
      DISABLE();

   return bios;
}

END_OF_STATIC_FUNCTION(dzdos_fixed_timer_handler);



/* fixed_timer_init:
 *  Installs the fixed-rate timer driver.
 */
int dzdos_fixed_timer_init()
{
   int i;

   detect_os();

   LOCK_VARIABLE(dzdos_timedrv_fixed_rate);
   LOCK_VARIABLE(dzdos_bios_counter);
   LOCK_FUNCTION(dzdos_fixed_timer_handler);

   dzdos_bios_counter = 0x10000;

   if (_dzdos_install_irq(TIMER_INT, dzdos_fixed_timer_handler) != 0)
      return -1;

   DISABLE();

   /* sometimes it doesn't seem to register if we only do this once... */
   for (i=0; i<4; i++)
      dzdos_set_timer_rate(LOVEBILL_TIMER_SPEED);

   ENABLE();

   return 0;
}



/* fixed_timer_exit:
 *  Shuts down the fixed-rate timer driver.
 */
void dzdos_fixed_timer_exit()
{
   DISABLE();

   dzdos_set_timer_rate(0);

   _dzdos_remove_irq(TIMER_INT);

   dzdos_set_timer_rate(0);

   ENABLE();
}



/* var_timer_handler:
 *  Interrupt handler for the variable-rate timer driver.
 */
static int dzdos_var_timer_handler()
{
   long new_delay = 0x8000;
   int callback[MAX_TIMERS];
   int bios = FALSE;
   int x;

   /* reentrant interrupt? */
   if (dzdos_timer_semaphore) {
      dzdos_timer_delay += REENTRANT_RECALL_GAP + dzdos_read_timer();
      dzdos_set_timer(REENTRANT_RECALL_GAP - dzdos_timer_clocking_loss/2);
      outportb(0x20, 0x20); 
      return 0;
   }

   dzdos_timer_semaphore = TRUE;

   /* deal with retrace synchronisation */
   dzdos_vsync_counter -= dzdos_timer_delay; 

   if (dzdos_vsync_counter <= 0) {
      if (_dz_timer_use_retrace) {
	 /* wait for retrace to start */
	 do { 
	 } while (!(inportb(0x3DA)&8));

	 /* update the VGA pelpan register? */
	 if (_dz_retrace_hpp_value >= 0) {
	    inportb(0x3DA);
	    outportb(0x3C0, 0x33);
	    outportb(0x3C0, _dz_retrace_hpp_value);
	    _dz_retrace_hpp_value = -1;
	 }

	 /* user callback */
	 dz_retrace_count++;
	 if (dz_retrace_proc)
	    dz_retrace_proc();

	 dzdos_vsync_counter = dzdos_vsync_speed - VSYNC_MARGIN;
      }
      else {
	 dzdos_vsync_counter += dzdos_vsync_speed;
	 dz_retrace_count++;
	 if (dz_retrace_proc)
	    dz_retrace_proc();
      }
   }

   if (dzdos_vsync_counter < new_delay)
      new_delay = dzdos_vsync_counter;

   dzdos_timer_delay += dzdos_read_timer() + dzdos_timer_clocking_loss;
   dzdos_set_timer(0);

   /* process the user callbacks */
   for (x=0; x<MAX_TIMERS; x++) { 
      callback[x] = FALSE;

      if (((_dz_timer_queue[x].proc) || (_dz_timer_queue[x].param_proc)) && 
	  (_dz_timer_queue[x].speed > 0)) {
	 _dz_timer_queue[x].counter -= dzdos_timer_delay;

	 if (_dz_timer_queue[x].counter <= 0) {
	    _dz_timer_queue[x].counter += _dz_timer_queue[x].speed;
	    callback[x] = TRUE;
	 }

	 if ((_dz_timer_queue[x].counter > 0) && (_dz_timer_queue[x].counter < new_delay))
	    new_delay = _dz_timer_queue[x].counter;
      }
   }

   /* update bios time */
   dzdos_bios_counter -= dzdos_timer_delay; 

   if (dzdos_bios_counter <= 0) {
      dzdos_bios_counter += 0x10000;
      bios = TRUE;
   }

   if (dzdos_bios_counter < new_delay)
      new_delay = dzdos_bios_counter;

   /* fudge factor to prevent interrupts coming too close to each other */
   if (new_delay < 1024)
      dzdos_timer_delay = 1024;
   else
      dzdos_timer_delay = new_delay;

   /* start the timer up again */
   new_delay = dzdos_read_timer();
   dzdos_set_timer(dzdos_timer_delay);
   dzdos_timer_delay += new_delay;

   if (!bios) {
      outportb(0x20, 0x20);      /* ack. the interrupt */
      ENABLE();
   }

   /* finally call the user timer routines */
   for (x=0; x<MAX_TIMERS; x++) {
      if (callback[x]) {
	 if (_dz_timer_queue[x].param_proc)
	    _dz_timer_queue[x].param_proc(_dz_timer_queue[x].param);
	 else
	    _dz_timer_queue[x].proc();

	 while (((_dz_timer_queue[x].proc) || (_dz_timer_queue[x].param_proc)) && 
		(_dz_timer_queue[x].counter <= 0)) {
	    _dz_timer_queue[x].counter += _dz_timer_queue[x].speed;
	    if (_dz_timer_queue[x].param_proc)
	       _dz_timer_queue[x].param_proc(_dz_timer_queue[x].param);
	    else
	       _dz_timer_queue[x].proc();
	 }
      }
   }

   if (!bios)
      DISABLE();

   dzdos_timer_semaphore = FALSE;

   return bios;
}

END_OF_STATIC_FUNCTION(dzdos_var_timer_handler);



/* var_timer_can_simulate_retrace:
 *  Checks whether it is cool to enable retrace syncing at the moment.
 * For the sake of this dzcomm version, it is never cool because we
 * don't have all the gfx stuff ... the orignal is left there for reference.
 */
int dzdos_var_timer_can_simulate_retrace()
{
   return FALSE;
/*    return ((gfx_driver) && ((gfx_driver->id == GFX_VGA) ||  */
/* 			    (gfx_driver->id == GFX_MODEX))); */
}



/* timer_calibrate_retrace:
 *  Times several vertical retraces, and calibrates the retrace syncing
 *  code accordingly.
 */
static void dzdos_timer_calibrate_retrace()
{
   int ot = _dz_timer_use_retrace;
   int c;

   #define AVERAGE_COUNT   4

   _dz_timer_use_retrace = FALSE;
   dzdos_vsync_speed = 0;

   /* time several retraces  - taken out as irrelevant, see above.*/
   for (c=0; c<AVERAGE_COUNT; c++) {
/*       _enter_critical(); */
/*       _vga_vsync(); */
/*       dzdos_set_timer(0); */
/*       _vga_vsync(); */
/*       dzdos_vsync_speed += dzdos_read_timer(); */
/*       _exit_critical(); */
   }

   dzdos_set_timer(dzdos_timer_delay);

   dzdos_vsync_speed /= AVERAGE_COUNT;

   /* sanity check to discard bogus values */
   if ((dzdos_vsync_speed > BPS_TO_TIMER(40)) || (dzdos_vsync_speed < BPS_TO_TIMER(110)))
      dzdos_vsync_speed = BPS_TO_TIMER(70);

   dzdos_vsync_counter = dzdos_vsync_speed;
   _dz_timer_use_retrace = ot;
}



/* var_timer_simulate_retrace:
 *  Turns retrace syncing mode on or off.
 * However, we always fail, see above.
 */
void dzdos_var_timer_simulate_retrace(int enable)
{
   if (enable) {
/*       dzdos_timer_calibrate_retrace(); */
/*       _dz_timer_use_retrace = TRUE; */
      _dz_timer_use_retrace = FALSE;
   }
   else {
      _dz_timer_use_retrace = FALSE;
      dzdos_vsync_counter = dzdos_vsync_speed = BPS_TO_TIMER(70);
   }
}



/* var_timer_init:
 *  Installs the variable-rate timer driver.
 */
int dzdos_var_timer_init()
{
   int x, y;

   detect_os();

   if (dz_i_love_bill)
      return -1;

   LOCK_VARIABLE(dzdos_timedrv_variable_rate);
   LOCK_VARIABLE(dzdos_bios_counter);
   LOCK_VARIABLE(dzdos_timer_delay);
   LOCK_VARIABLE(dzdos_timer_semaphore);
   LOCK_VARIABLE(dzdos_timer_clocking_loss);
   LOCK_VARIABLE(dzdos_vsync_counter);
   LOCK_VARIABLE(dzdos_vsync_speed);
   LOCK_FUNCTION(dzdos_var_timer_handler);

   dzdos_vsync_counter = dzdos_vsync_speed = BPS_TO_TIMER(70);

   dzdos_bios_counter = 0x10000;
   dzdos_timer_delay = 0x10000;

   if (_dzdos_install_irq(TIMER_INT, dzdos_var_timer_handler) != 0)
      return -1;

   DISABLE();

   /* now work out how much time calls to the 8254 clock chip take 
    * on this CPU/motherboard combination. It is impossible to time 
    * a set_timer call so we approximate it by a read_timer call with 
    * 4/5ths of a read_timer (no maths and no final read wait). This 
    * gives 9/10ths of 4 read_timers. Do it three times all over to 
    * get an averaging effect.
    */
   x = dzdos_read_timer();
   dzdos_read_timer();
   dzdos_read_timer();
   dzdos_read_timer();
   y = dzdos_read_timer();
   dzdos_read_timer();
   dzdos_read_timer();
   dzdos_read_timer();
   y = dzdos_read_timer();
   dzdos_read_timer();
   dzdos_read_timer();
   dzdos_read_timer();
   y = dzdos_read_timer();

   if (y >= x)
      dzdos_timer_clocking_loss = y - x;
   else
      dzdos_timer_clocking_loss = 0x10000 - x + y;

   dzdos_timer_clocking_loss = (9*dzdos_timer_clocking_loss)/30;

   /* sometimes it doesn't seem to register if we only do this once... */
   for (x=0; x<4; x++)
      dzdos_set_timer(dzdos_timer_delay);

   ENABLE();

   return 0;
}



/* var_timer_exit:
 *  Shuts down the variable-rate timer driver.
 */
void dzdos_var_timer_exit()
{
   DISABLE();

   dzdos_set_timer_rate(0);

   _dzdos_remove_irq(TIMER_INT);

   dzdos_set_timer_rate(0);

   ENABLE();
}

