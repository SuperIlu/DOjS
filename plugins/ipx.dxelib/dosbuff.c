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

#include "dosbuff.h"
#include "DOjS.h"

#include <dpmi.h>
#include <go32.h>

#include <string.h>
//#include <conio.h>
#include <stdio.h>

/* Globals */
unsigned short segment_of_dos_buffer, offset_of_dos_buffer;
unsigned long address_of_dos_buffer;
unsigned long linear_address_of_dos_buffer;

unsigned short segment_of_dos_buffer_two, offset_of_dos_buffer_two;
unsigned long address_of_dos_buffer_two;
unsigned long linear_address_of_dos_buffer_two;

unsigned short segment_of_ecb_send_buffer, offset_of_ecb_send_buffer;
unsigned long address_of_ecb_send_buffer;
unsigned long linear_address_of_ecb_send_buffer;

unsigned short segment_of_ecb_receive_buffer, offset_of_ecb_receive_buffer;
unsigned long address_of_ecb_receive_buffer;
unsigned long linear_address_of_ecb_receive_buffer;

unsigned short segment_of_send_buffer, offset_of_send_buffer;
unsigned long address_of_send_buffer;
unsigned long linear_address_of_send_buffer;

unsigned short segment_of_receive_buffer, offset_of_receive_buffer;
unsigned long address_of_receive_buffer;
unsigned long linear_address_of_receive_buffer;

/* End of Globals */

bool Allocate_Dos_Buffers(void) {
    _go32_dpmi_seginfo info;
    unsigned int address, page;  // Address is linear
    unsigned int new_segment, new_offset;

    info.size = (DOS_BUFFER_SIZE + 15) / 16;

    if (_go32_dpmi_allocate_dos_memory(&info)) {
        DEBUG("ERROR Allocate dos buffer Alloc failed \n");
        return false;
    }

    // What is the 20 bit address?
    address = info.rm_segment << 4;
    // Does it cross a 64K boundary?

    page = address & 0xffff;
    if ((page + DOS_BUFFER_TEST) > 0xffff) address = (address - page) + 0x10000;

    new_segment = address / 16;
    new_offset = address % 16;

    segment_of_dos_buffer = new_segment;
    offset_of_dos_buffer = new_offset;

    linear_address_of_dos_buffer = address;
    address_of_dos_buffer = ((unsigned long)new_segment << 16) | new_offset;

    // Do the same for buffer two
    memset(&info, 0, sizeof(info));

    info.size = (DOS_BUFFER_SIZE + 15) / 16;

    if (_go32_dpmi_allocate_dos_memory(&info)) {
        DEBUG("ERROR Allocate dos buffer Alloc failed \n");
        return false;
    }

    // What is the 20 bit address?
    address = info.rm_segment << 4;
    // Does it cross a 64K boundary?

    page = address & 0xffff;
    if ((page + DOS_BUFFER_TEST) > 0xffff) address = (address - page) + 0x10000;

    new_segment = address / 16;
    new_offset = address % 16;

    segment_of_dos_buffer_two = new_segment;
    offset_of_dos_buffer_two = new_offset;

    linear_address_of_dos_buffer_two = address;
    address_of_dos_buffer_two = ((unsigned long)new_segment << 16) | new_offset;

    // Do the same for send buffer

    info.size = (DOS_BUFFER_SIZE + 15) / 16;

    if (_go32_dpmi_allocate_dos_memory(&info)) {
        DEBUG("ERROR Allocate dos buffer Alloc failed \n");
        return false;
    }

    // What is the 20 bit address?
    address = info.rm_segment << 4;
    // Does it cross a 64K boundary?

    page = address & 0xffff;
    if ((page + DOS_BUFFER_TEST) > 0xffff) address = (address - page) + 0x10000;

    new_segment = address / 16;
    new_offset = address % 16;

    segment_of_send_buffer = new_segment;
    offset_of_send_buffer = new_offset;

    linear_address_of_send_buffer = address;
    address_of_send_buffer = ((unsigned long)new_segment << 16) | new_offset;

    // Do the same for Receive buffer

    info.size = (DOS_BUFFER_SIZE + 15) / 16;

    if (_go32_dpmi_allocate_dos_memory(&info)) {
        DEBUG("ERROR Allocate dos buffer Alloc failed \n");
        return false;
    }

    // What is the 20 bit address?
    address = info.rm_segment << 4;
    // Does it cross a 64K boundary?

    page = address & 0xffff;
    if ((page + DOS_BUFFER_TEST) > 0xffff) address = (address - page) + 0x10000;

    new_segment = address / 16;
    new_offset = address % 16;

    segment_of_receive_buffer = new_segment;
    offset_of_receive_buffer = new_offset;

    linear_address_of_receive_buffer = address;
    address_of_receive_buffer = ((unsigned long)new_segment << 16) | new_offset;

    // Do the same for ecb send buffer

    info.size = (DOS_BUFFER_SIZE + 15) / 16;

    if (_go32_dpmi_allocate_dos_memory(&info)) {
        DEBUG("ERROR Allocate dos buffer Alloc failed \n");
        return false;
    }

    // What is the 20 bit address?
    address = info.rm_segment << 4;
    // Does it cross a 64K boundary?

    page = address & 0xffff;
    if ((page + DOS_BUFFER_TEST) > 0xffff) address = (address - page) + 0x10000;

    new_segment = address / 16;
    new_offset = address % 16;

    segment_of_ecb_send_buffer = new_segment;
    offset_of_ecb_send_buffer = new_offset;

    linear_address_of_ecb_send_buffer = address;
    address_of_ecb_send_buffer = ((unsigned long)new_segment << 16) | new_offset;

    // Do the same for Receive buffer

    info.size = (DOS_BUFFER_SIZE + 15) / 16;

    if (_go32_dpmi_allocate_dos_memory(&info)) {
        DEBUG("ERROR Allocate dos buffer Alloc failed \n");
        return false;
    }

    // What is the 20 bit address?
    address = info.rm_segment << 4;
    // Does it cross a 64K boundary?

    page = address & 0xffff;
    if ((page + DOS_BUFFER_TEST) > 0xffff) address = (address - page) + 0x10000;

    new_segment = address / 16;
    new_offset = address % 16;

    segment_of_ecb_receive_buffer = new_segment;
    offset_of_ecb_receive_buffer = new_offset;

    linear_address_of_ecb_receive_buffer = address;
    address_of_ecb_receive_buffer = ((unsigned long)new_segment << 16) | new_offset;

    return true;
} /* End of Allocate_Dos_Buffer() */

void Copy_Into_Dos_Buffer(void *block, short length) { dosmemput(block, length, linear_address_of_dos_buffer); }

void Copy_From_Dos_Buffer(void *block, short length) { dosmemget(linear_address_of_dos_buffer, length, block); }

void Copy_Into_Dos_Buffer_Two(void *block, short length) { dosmemput(block, length, linear_address_of_dos_buffer_two); }

void Copy_From_Dos_Buffer_Two(void *block, short length) { dosmemget(linear_address_of_dos_buffer_two, length, block); }

void Copy_Into_Send_Buffer(void *block, short length) { dosmemput(block, length, linear_address_of_send_buffer); }

void Copy_From_Send_Buffer(void *block, short length) { dosmemget(linear_address_of_send_buffer, length, block); }

void Copy_Into_Receive_Buffer(void *block, short length) { dosmemput(block, length, linear_address_of_receive_buffer); }

void Copy_From_Receive_Buffer(void *block, short length) { dosmemget(linear_address_of_receive_buffer, length, block); }

void Copy_Into_Ecb_Send_Buffer(void *block, short length) { dosmemput(block, length, linear_address_of_ecb_send_buffer); }

void Copy_From_Ecb_Send_Buffer(void *block, short length) { dosmemget(linear_address_of_ecb_send_buffer, length, block); }

void Copy_Into_Ecb_Receive_Buffer(void *block, short length) { dosmemput(block, length, linear_address_of_ecb_receive_buffer); }

void Copy_From_Ecb_Receive_Buffer(void *block, short length) { dosmemget(linear_address_of_ecb_receive_buffer, length, block); }

unsigned long Make_Far_Pointer(unsigned short segment, unsigned short offset) {
    unsigned long temp;

    temp = ((unsigned long)segment << 16) | offset;

    return (temp);
} /* End of Make_Far_Pointer() */
