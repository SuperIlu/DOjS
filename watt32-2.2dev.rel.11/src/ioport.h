/*!\file ioport.h
 */
#ifndef _w32_IOPORT_H
#define _w32_IOPORT_H

/*
 * Make sure <conio.h> (or <pc.h> for djgpp) gets included
 */
#if !defined(_w32_TARGET_H)
#error You must include IOPORT.H after TARGET.H
#endif

/*
 * Macros necessary to bypass the limitation of Borland's BCC32 not
 * to allow inline port functions. Assignements should be volatile,
 * check the .asm output.
 * NB! macro-args must be TASM/MASM compatible
 */

#if defined(__BORLANDC__) && defined(__FLAT__) && \
   !(defined(W32_IS_EMBARCADERO) || defined(W32_IS_CODEGEARC))

  #define VOLATILE        volatile
  #define __in(p,t,z)     ( _DX = (unsigned short)(p),__emit__(0xEC+z),(unsigned t)_AX )
  #define __out(p,x,z)    { _AX = (unsigned short)(x); _DX = (unsigned short)(p);__emit__(0xEE+z); }
  #define _inportb(p)     __in(p,char,0)
  #define _inportw(p)     __in(p,short,1)
  #define _outportb(p,x)  __out(p,x,0)
  #define _outportw(p,x)  __out(p,x,1)

#elif defined(__HIGHC__)
  #define _inportb(p)     _inb(p)
  #define _inportw(p)     _inw(p)
  #define _outportb(p,x)  _outb(p,x)
  #define _outportw(p,x)  _outpw(p,x)
  #define VOLATILE

#elif defined(__DJGPP__) || defined(__EMX__)
  #define _inportb(p)     inportb(p)
  #define _inport(p)      inportw(p)
  #define _outportb(p,x)  outportb(p,x)
  #define _outportw(p,x)  outportw(p,x)
  #define VOLATILE

#elif defined(_MSC_VER) && defined(__clang__)

 /* Windows implied. Should be no need for this.
  */

#elif defined(_MSC_VER) && (_MSC_VER >= 800)
  #include <conio.h>
  #pragma intrinsic (_inp, _inpw, _outp, _outpw)
  #define _inportb(p)     _inp(p)
  #define _inportw(p)     _inpw(p)
  #define _outportb(p,x)  _outp(p,x)
  #define _outportw(p,x)  _outpw(p,x)
  #define VOLATILE

#else
  #define _inportb(p)     inp(p)
  #define _inportw(p)     inpw(p)
  #define _outportb(p,x)  outp(p,x)
  #define _outportw(p,x)  outpw(p,x)
  #define VOLATILE
#endif

#if defined(__EMX__)

/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */

W32_GCC_INLINE unsigned char inportb (unsigned short port)
{
  unsigned char rc;
  __asm__ __volatile__ (
            "inb %1, %0"
          : "=a" (rc)
          : "dN" (port));
  return (rc);
}

W32_GCC_INLINE unsigned short inportw (unsigned short port)
{
  unsigned short rc;
  __asm__ __volatile__ (
            "inw %1, %0"
          : "=a" (rc)
          : "dN" (port));
  return (rc);
}

W32_GCC_INLINE void outportb (unsigned short port, unsigned char data)
{
  __asm__ __volatile__ (
             "outb %1, %0"
          :: "dN" (port),
             "a" (data));
}

W32_GCC_INLINE void outportw (unsigned short port, unsigned short data)
{
  __asm__ __volatile__ (
             "outw %1, %0"
          :: "dN" (port),
             "a" (data));
}
#endif  /* __EMX__ */
#endif  /* _w32_IOPORT_H */

