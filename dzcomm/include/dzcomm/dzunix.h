/*
 * DZcomm : a serial port API.
 * file : dzucfg.h
 *
 * Unix-specific header defines.
 *
 * By Michael Bukin. Dzcommport by Neil Townsend
 * 
 * See readme.txt for copyright information.
 */

#ifndef _DZCOMM_DZUNIX_H
#define _DZCOMM_DZUNIX_H

#ifndef DZCOMM_UNIX
   #error bad include
#endif



/**************************************/
/************ General Unix ************/
/**************************************/

#define  COMM_PORT_UNIX_DRIVER   DZ_ID('D', 'Z', 'C', 'U')

/****************************************/
/************ Linux-specific ************/
/****************************************/

#ifdef DZCOMM_LINUX

#define  COMM_PORT_LINUX_DRIVER  DZ_ID('D', 'Z', 'C', 'L')


/* We may already have the following from allegro. If not define them */
#ifndef ALLEGRO_LINUX

/* Port I/O functions -- maybe these should be internal */

static inline void outportb(unsigned short port, unsigned char value)
{
   __asm__ volatile ("outb %0, %1" : : "a" (value), "d" (port));
}

static inline void outportw(unsigned short port, unsigned short value)
{
   __asm__ volatile ("outw %0, %1" : : "a" (value), "d" (port));
}

static inline void outportl(unsigned short port, unsigned long value)
{
   __asm__ volatile ("outl %0, %1" : : "a" (value), "d" (port));
}

static inline unsigned char inportb(unsigned short port)
{
   unsigned char value;
   __asm__ volatile ("inb %1, %0" : "=a" (value) : "d" (port));
   return value;
}

static inline unsigned short inportw(unsigned short port)
{
   unsigned short value;
   __asm__ volatile ("inw %1, %0" : "=a" (value) : "d" (port));
   return value;
}

static inline unsigned long inportl(unsigned short port)
{
   unsigned long value;
   __asm__ volatile ("inl %1, %0" : "=a" (value) : "d" (port));
   return value;
}

#endif /* !ALLEEGRO_LINUX */

#endif /* DZCOMM_LINUX */



#endif /* !_DZCOMM_DZUNIX_H */

