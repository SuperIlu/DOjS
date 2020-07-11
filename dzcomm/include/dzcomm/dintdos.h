/*
 * DZcomm :  a serial port API
 *
 * Some definitions for internal use by the DOS library code.
 *
 * By Neil Townsend.
 *
 * See readme.txt for copyright information.
 */


#ifndef DINTDOS_H
#define DINTDOS_H

#ifndef DZCOMM_H
   #error must include allegro.h first
#endif

#ifndef DZCOMM_DOS
   #error bad include
#endif

#ifdef __cplusplus
   extern "C" {
#endif


/* macros to enable and disable interrupts */
#if defined DZCOMM_GCC

   #define DISABLE()    asm volatile ("cli")
   #define ENABLE()     asm volatile ("sti")

#elif defined DZCOMM_WATCOM

   void DISABLE(void);
   void ENABLE(void);

   #pragma aux DISABLE  = "cli";
   #pragma aux ENABLE   = "sti";

#else

   #define DISABLE()    asm { cli }
   #define ENABLE()     asm { sti }

#endif

DZ_VAR(volatile int, dz_interrupt_depth);

#define dz_disable_interrupts() {       \
if (dz_interrupt_depth == 0) DISABLE(); \
dz_interrupt_depth++;                   \
}
#define dz_enable_interrupts() {        \
dz_interrupt_depth--;                   \
if (dz_interrupt_depth == 0) ENABLE();  \
}
#define dz_set_interrupt_depth(x) {     \
dz_interrupt_depth = x;                 \
}

/*
 * These are the low level system functions to do with timing and irqs.
 * If allegro is being used then we must use allegro's versions if not
 * our versions. The functions given here catch the calls in dcomm.c and
 * decide which version to call.
 */

/* My versions (or the wrappers) */
DZ_FUNC(int,  _dzdos_install_irq, (int num, DZ_METHOD(int, handler, (void))));
DZ_FUNC(void, _dzdos_remove_irq,  (int num));

typedef struct _DZ_IRQ_HANDLER
{
   DZ_METHOD(int, handler, (void));    /* our C handler */
   int number;                         /* irq number */

   #ifdef DZCOMM_DJGPP
      __dpmi_paddr old_vector;         /* original protected mode vector */
   #else
      void (__interrupt __far *old_vector)();
   #endif
} _DZ_IRQ_HANDLER;


typedef struct DZ_TIMER_DRIVER
{
   int  id;
   char *name;
   char *desc;
   char *ascii_name;
   DZ_METHOD(int,  init, (void));
   DZ_METHOD(void, exit, (void));
   DZ_METHOD(int,  install_int, (DZ_METHOD(void, proc, (void)), long speed));
   DZ_METHOD(void, remove_int, (DZ_METHOD(void, proc, (void))));
   DZ_METHOD(int,  install_param_int, (DZ_METHOD(void, proc, (void *param)), void *param, long speed));
   DZ_METHOD(void, remove_param_int, (DZ_METHOD(void, proc, (void *param)), void *param));
   DZ_METHOD(int,  can_simulate_retrace, (void));
   DZ_METHOD(void, simulate_retrace, (int enable));
   DZ_METHOD(void, rest, (long time, DZ_METHOD(void, callback, (void))));
} DZ_TIMER_DRIVER;

/* The real routines */
DZ_VAR(DZ_TIMER_DRIVER *, dz_timer_driver);
DZ_ARRAY(_DZ_DRIVER_INFO, _dz_timer_driver_list);

DZ_VAR(int, _dzdos_timer_installed);

DZ_FUNC(int, dz_install_int_ex, (DZ_METHOD(void, proc, (void)), long speed));
DZ_FUNC(int, dz_install_int, (DZ_METHOD(void, proc, (void)), long speed));
DZ_FUNC(void, dz_remove_int, (DZ_METHOD(void, proc, (void))));

DZ_FUNC(int, dz_install_timer, (void));
DZ_FUNC(int, dz_install_param_int, (DZ_METHOD(void, proc, (void *param)), void *param, long speed));
DZ_FUNC(void, dz_remove_param_int, (DZ_METHOD(void, proc, (void *param)), void *param));

DZ_FUNC(int,  dz_timer_can_simulate_retrace, (void));
DZ_FUNC(void, dz_timer_simulate_retrace, (int enable));
DZ_FUNC(int,  dz_timer_is_using_retrace, (void));

DZ_VAR(volatile int, dz_retrace_count);
DZ_FUNCPTR(void, dz_retrace_proc, (void));

DZ_FUNC(void, dz_rest, (long time));
DZ_FUNC(void, dz_rest_callback, (long time, DZ_METHOD(void, callback, (void))));

/* And the wrappers for the ones I actually use */
DZ_FUNC(int, dzdos_install_timer, (void));
DZ_FUNC(int, dzdos_install_param_int, (DZ_METHOD(void, proc, (void *param)), void *param, long speed));
DZ_FUNC(void, dzdos_remove_param_int, (DZ_METHOD(void, proc, (void *param)), void *param));


#ifdef __cplusplus
   }
#endif

#endif          /* ifndef DINTDOS_H */
