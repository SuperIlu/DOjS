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

#ifndef __IPX_H__
#define __IPX_H__

#include <mujs.h>
#include "DOjS.h"

/*************
** defines **
*************/
#define PACKED_STRUCT
#define MAX_PACKETS_IN_BUFFER 100
#define MAX_PACKET_LEN 80
#define ADDRESS_SIZE 6
#define IPX_DELAY 3 /* Amount we delay between sending packets */

#define JSINC_IPX BOOT_DIR "ipx.js"  //!< boot script for ipx subsystem

/*************
** typedefs **
*************/
typedef unsigned char net_t[4];
typedef unsigned char node_address_t[ADDRESS_SIZE];
typedef char string_t[MAX_PACKET_LEN];
typedef unsigned short address_t[2];

/************
** structs **
************/
typedef struct {
    net_t net PACKED_STRUCT;                   /* Network address */
    node_address_t node_address PACKED_STRUCT; /* Node address */
    unsigned short socket PACKED_STRUCT;       /* Big endian socket number */
} net_address_t;

typedef struct {
    net_t net PACKED_STRUCT;                   /* My network address */
    node_address_t node_address PACKED_STRUCT; /* My node address */
} local_address_t;

typedef struct {
    address_t link PACKED_STRUCT;                   /* Pointer to next ECB */
    unsigned long ESR PACKED_STRUCT;                /* Event service routine 00000000h if none */
    unsigned char in_use PACKED_STRUCT;             /* In use flag */
    unsigned char complete PACKED_STRUCT;           /* Completing flag */
    unsigned short socket PACKED_STRUCT;            /* Big endian socket number */
    unsigned char IPX_work[4] PACKED_STRUCT;        /* IPX work space */
    unsigned char D_work[12] PACKED_STRUCT;         /* Driver work space */
    node_address_t immediate_address PACKED_STRUCT; /* Immediate local node address */
    unsigned short fragment_count PACKED_STRUCT;    /* Fragment count */
    unsigned long fragment_data PACKED_STRUCT;      /* Pointer to data fragment */
    unsigned short fragment_size PACKED_STRUCT;     /* Size of data fragment */
} ECB_t;

typedef struct {
    unsigned short checksum PACKED_STRUCT;         /* Big endian checksum */
    unsigned short length PACKED_STRUCT;           /* Big endian length in bytes */
    unsigned char transport_control PACKED_STRUCT; /* Transport control */
    unsigned char packet_type PACKED_STRUCT;       /* Packet type */
    net_address_t destination PACKED_STRUCT;       /* Destination network address */
    net_address_t source PACKED_STRUCT;            /* Source network address */
} IPX_header_t;

typedef struct {
    string_t string;
    node_address_t source_node;
} game_packet_t;

typedef struct {
    ECB_t ecb PACKED_STRUCT;
    IPX_header_t ipx_header PACKED_STRUCT;
    string_t string PACKED_STRUCT;
} packet_t;

typedef struct {
    game_packet_t packets[MAX_PACKETS_IN_BUFFER]; /* Array of packets */
    unsigned short buffer_start;                  /* The last packet we read */
    unsigned short buffer_pos;                    /* The most recent packet */
} packet_buffer_t;

/***********************
** exported functions **
***********************/
extern bool init_ipx(js_State *J);
extern void shutdown_ipx(void);

#endif  // __IPX_H__
