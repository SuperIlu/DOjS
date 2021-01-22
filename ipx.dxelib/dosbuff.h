/*
    Copyright (C) 2001 Hotwarez LLC, Goldtree Enterprises

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation;
    version 2 of the License.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

    Modified for DOjS 2019 by Andre Seidelt <superilu@yahoo.com>
*/

#ifndef DOSBUFF_H
#define DOSBUFF_H

#include <stdbool.h>

#define DOS_BUFFER_SIZE 200 /* 300 bytes */
#define DOS_BUFFER_TEST 100 /* The amount we REALLY need for the dos buffer */

/* from dosbuff.c */
extern unsigned short segment_of_dos_buffer, offset_of_dos_buffer;
extern unsigned long address_of_dos_buffer;
extern unsigned long linear_address_of_dos_buffer;

extern unsigned short segment_of_dos_buffer_two, offset_of_dos_buffer_two;
extern unsigned long address_of_dos_buffer_two;
extern unsigned long linear_address_of_dos_buffer_two;

extern unsigned short segment_of_ecb_send_buffer, offset_of_ecb_send_buffer;
extern unsigned long address_of_ecb_send_buffer;
extern unsigned long linear_address_of_ecb_send_buffer;

extern unsigned short segment_of_ecb_receive_buffer, offset_of_ecb_receive_buffer;
extern unsigned long address_of_ecb_receive_buffer;
extern unsigned long linear_address_of_ecb_receive_buffer;

extern unsigned short segment_of_send_buffer, offset_of_send_buffer;
extern unsigned long address_of_send_buffer;
extern unsigned long linear_address_of_send_buffer;

extern unsigned short segment_of_receive_buffer, offset_of_receive_buffer;
extern unsigned long address_of_receive_buffer;
extern unsigned long linear_address_of_receive_buffer;

bool Allocate_Dos_Buffers(void);
void Copy_Into_Dos_Buffer(void *block, short length);
void Copy_From_Dos_Buffer(void *block, short length);
void Copy_Into_Dos_Buffer_Two(void *block, short length);
void Copy_From_Dos_Buffer_Two(void *block, short length);
void Copy_Into_Send_Buffer(void *block, short length);
void Copy_From_Send_Buffer(void *block, short length);
void Copy_Into_Receive_Buffer(void *block, short length);
void Copy_From_Receive_Buffer(void *block, short length);
void Copy_Into_Ecb_Send_Buffer(void *block, short length);
void Copy_From_Ecb_Send_Buffer(void *block, short length);
void Copy_Into_Ecb_Receive_Buffer(void *block, short length);
void Copy_From_Ecb_Receive_Buffer(void *block, short length);
unsigned long Make_Far_Pointer(unsigned short segment, unsigned short offset);

#endif
