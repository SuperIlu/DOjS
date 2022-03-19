/*
 * DZcomm : a serial port API
 * file : dzdos.h
 *
 * DOS-specific header defines.
 * 
 * By Neil Townsend.
 *
 * See readme.txt for copyright information.
 */


#ifndef DZCOMM_DOS
   #error bad include
#endif


/********************************************/
/************ Platform-specifics ************/
/********************************************/

#define COMM_PORT_DOS_DRIVER     DZ_ID('D','Z','C','D')

DZ_VAR(int, dz_i_love_bill);

#define DZ_TIMEDRV_FIXED_RATE     DZ_ID('F','I','X','T')
#define DZ_TIMEDRV_VARIABLE_RATE  DZ_ID('V','A','R','T')

/* Note that we will use some routines from allegro. This means that we
 * provide alternatives for these routines and then decide at user
 * compiler time whether to the allegro versions (which we must do if it
 * is there) or our versions.
 */

/* Check that this is the user compiling and not the library being built */
#ifndef DZCOMM_LIB_SRC_FILE
/* check that this is the main #include by the user */
#ifndef DZCOMM_SECONDARY_INCLUDE

#ifdef ALLEGRO_H

#ifndef AINTERN_H
/* Allegro functions not declared in allegro.h but are in aintern.h */
DZ_FUNC(int,  _install_irq, (int num, DZ_METHOD(int, handler, (void))));
DZ_FUNC(void, _remove_irq,  (int num));
#endif /* !AINTERN_H */

int  _dzdos_install_irq(int num, int (*handler)());
int  _dzdos_install_irq(int num, int (*handler)()) {
   return _install_irq(num, handler);
}

void _dzdos_remove_irq(int num);
void _dzdos_remove_irq(int num) {
   _remove_irq(num);
}

/* An Allegro and a dzcomm internal variable that we need here */
DZ_VAR(int, _dzdos_timer_installed);
AL_VAR(int, _timer_installed);

int dzdos_install_timer(void);
int dzdos_install_timer(void) {
   int rv = install_timer();
   if (_timer_installed) _dzdos_timer_installed = TRUE;
   else                  _dzdos_timer_installed = FALSE;
   return rv;
}

int dzdos_install_param_int(void (*proc)(void *param), void *param, long speed);
int dzdos_install_param_int(void (*proc)(void *param), void *param, long speed) {
   return install_param_int(proc, param, speed);
}

void dzdos_remove_param_int(void (*proc)(void *param), void *param);
void dzdos_remove_param_int(void (*proc)(void *param), void *param) {
   remove_param_int(proc, param);
}

#else

/* Prototypes for the dz versions of the function which would ideally be hidden */
DZ_FUNC(int,  _dzdos_real_install_irq, (int num, DZ_METHOD(int, handler, (void))));
DZ_FUNC(void, _dzdos_real_remove_irq,  (int num));
DZ_FUNC(int, dz_install_timer, (void));
DZ_FUNC(int, dz_install_param_int, (DZ_METHOD(void, proc, (void *param)), void *param, long speed));
DZ_FUNC(void, dz_remove_param_int, (DZ_METHOD(void, proc, (void *param)), void *param));

int  _dzdos_install_irq(int num, int (*handler)()) {
   return _dzdos_real_install_irq(num, handler);
}

void _dzdos_remove_irq(int num) {
   _dzdos_real_remove_irq(num);
}

int dzdos_install_timer(void) {
   return dz_install_timer();
}

int dzdos_install_param_int(void (*proc)(void *param), void *param, long speed) {
   return dz_install_param_int(proc, param, speed);
}

void dzdos_remove_param_int(void (*proc)(void *param), void *param) {
   dz_remove_param_int(proc, param);
}

#endif /* ALLEGRO_H */

#endif /* !DZCOMM_SECONDARY_INCLUDE */
#endif /* !DZCOMM_LIB_SRC_FILE      */

