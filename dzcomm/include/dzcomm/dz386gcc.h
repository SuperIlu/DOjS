/*
 * DZcomm : a serial port API.
 * file : dz386gcc.h
 *
 * Inline functions (gcc style 386 asm).
 *
 * By Shawn Hargreaves.
 * 
 * See readme.txt for copyright information.
 */


#if (!defined DZCOMM_GCC) || (!defined DZCOMM_I386)
   #error bad include
#endif



/* bmp_read_line:
 *  Bank switch function.
 */
// Left as an example
//AL_INLINE(unsigned long, bmp_read_line, (BITMAP *bmp, int line),
//{
//   unsigned long result;
//
//   asm volatile (
//      "  call *%3 "
//
//   : "=a" (result)                     /* result in eax */
//
//   : "d" (bmp),                        /* bitmap in edx */
//     "0" (line),                       /* line number in eax */
//     "r" (bmp->read_bank)              /* the bank switch routine */
//   );
//
//   return result;
//})

