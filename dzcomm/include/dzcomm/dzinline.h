/*
 * DZcomm : a serial port API.
 * file : dzucfg.h
 *
 * Inline functions (generic C).
 *
 * By Shawn Hargreaves. Moved to Dzcomm by Neil Townsend.
 * 
 * See readme.txt for copyright information.
 */


/*
 * Inlines which may be direct asm coded :
 */
#if (defined DZCOMM_GCC) && (defined DZCOMM_I386)

   /* use i386 asm, GCC syntax */
   #include "dz386gcc.h"

#elif (defined DZCOMM_MSVC) && (defined DZCOMM_I386)

   /* use i386 asm, MSVC syntax */
   #include "dz386vc.h"

#elif (defined DZCOMM_WATCOM) && (defined DZCOMM_I386)

   /* use i386 asm, Watcom syntax */
   #include "dz386wat.h"

#else

   /* use generic C versions */


#endif



/*****************************************/
/************* Routines ******************/
/*****************************************/

     /* Left here as an exmaple:
AL_INLINE(void, get_executable_name, (char *output, int size),
{
   ASSERT(system_driver);

   if (system_driver->get_executable_name) {
      system_driver->get_executable_name(output, size);
   }
   else {
      output += usetc(output, '.');
      output += usetc(output, '/');
      usetc(output, 0);
   }
})
*/


/****************************************************/
/******* defines for backward compatibility *********/
/****************************************************/


