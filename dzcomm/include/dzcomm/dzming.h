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

#ifndef DZCOMM_MINGW32
   #error bad include
#endif

#define COMM_PORT_MINGW32_DRIVER     DZ_ID('D','Z','M','G')
