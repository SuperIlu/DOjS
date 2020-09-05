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

#include "ipx.h"
#include "DOjS.h"

#include <dos.h>
#include <dpmi.h>
#include <go32.h>
#include "dosbuff.h"

#include <stdio.h>
#include <stdlib.h> /* For swab */
#include <string.h>

/*********************
** static variables **
*********************/
static local_address_t local_address;
static packet_buffer_t packet_buffer;

static unsigned short socket = 0;
static packet_t send_packet, Receive_packet;

static unsigned long address_of_callback;
static unsigned short segment_of_callback;
static unsigned short offset_of_callback;
static unsigned long Receive_count = 0;
static bool ipx_initialized = false;

static _go32_dpmi_registers callback_regs;
static _go32_dpmi_seginfo callback_info;

/*********************
** static prototypes **
*********************/
static void Get_Local_Address();
static bool Compare_Nodes(node_address_t node_one, node_address_t node_two);
static void Packet_Receive_Callback(_go32_dpmi_registers *callback_regs);
static void Allocate_Receive_Callback(void);
static unsigned short Endian_Swap(unsigned short old_short);
static bool Init_IPX(void);
static int IPX_Open_Socket(unsigned char longetivity, unsigned short *socket_number);
static void IPX_Close_Socket(unsigned short *socket_number);
static void IPX_Send_Packet(ECB_t *ecb);
static int IPX_Listen_For_Packet(ECB_t *ecb);
static void IPX_Idle(void);
static void Init_Send_Packet(ECB_t *ecb, IPX_header_t *ipx_header, unsigned short size, unsigned short sock);
static bool Init_Receive_Packet(ECB_t *ecb, IPX_header_t *ipx_header, unsigned short size, unsigned short sock);

static bool Jonipx_Init();
// static bool Jonipx_Get_Packet(string_t string, node_address_t source_node);
static void Jonipx_Send_Packet(string_t string, int length, node_address_t dest_node);
static void Jonipx_Close(void);
static bool Jonipx_Packet_Ready(void);
static bool Jonipx_Pop_Packet(game_packet_t *packet);

/*********************
** static functions **
*********************/
static void Get_Local_Address() {
    _go32_dpmi_registers r;

    // Clear out the registers
    memset(&r, 0, sizeof(r));
    memset(&local_address, 0, sizeof(local_address));

    r.x.bx = 0x0009;  // Subfunction 0x0009, get internetwork address
    r.x.es = segment_of_dos_buffer;
    r.x.si = offset_of_dos_buffer;

    Copy_Into_Dos_Buffer(&local_address, sizeof(local_address));
    _go32_dpmi_simulate_int(0x7a, &r);
    Copy_From_Dos_Buffer(&local_address, sizeof(local_address));

    LOGF("net  %02x:%02x:%02x:%02x\n", local_address.net[0], local_address.net[1], local_address.net[2], local_address.net[3]);
    LOGF("node %02x:%02x:%02x:%02x:%02x:%02x\n", local_address.node_address[0], local_address.node_address[1], local_address.node_address[2], local_address.node_address[3],
         local_address.node_address[4], local_address.node_address[5]);
}

/* Return true if the nodes are the same, false if they are not */
static bool Compare_Nodes(node_address_t node_one, node_address_t node_two) {
    int i;

    for (i = 0; i < sizeof(node_address_t); i++) {
        if (node_one[i] != node_two[i]) {
            return false;
        }
    }

    return true;
}

/* This function should get called every time a packet is received */
static void Packet_Receive_Callback(_go32_dpmi_registers *callback_regs) {
    callback_regs->d.eax = 4;  // Doing this cuz I saw it in the dox
    Receive_count++;

    // This should copy the packet into the Receive buffer
    IPX_Listen_For_Packet(&Receive_packet.ecb);

    // Copy packet from Receive buffer into ipx header and buffer
    Copy_From_Receive_Buffer(&Receive_packet.ipx_header, sizeof(IPX_header_t) + sizeof(string_t));

    // Ignore all packets from ourselves
    if (!Compare_Nodes(local_address.node_address, Receive_packet.ipx_header.source.node_address)) {
        // Copy from there into the packet buffer, we skip the header
        for (int i = 0; i < sizeof(string_t); i++) {
            packet_buffer.packets[packet_buffer.buffer_pos].string[i] = Receive_packet.string[i];
        }

        for (int i = 0; i < sizeof(node_address_t); i++) {
            packet_buffer.packets[packet_buffer.buffer_pos].source_node[i] = Receive_packet.ipx_header.source.node_address[i];
        }

        packet_buffer.buffer_pos++;

        if (packet_buffer.buffer_pos >= MAX_PACKETS_IN_BUFFER) packet_buffer.buffer_pos = 0;

    }  // End of if from me
}

static void Allocate_Receive_Callback(void) {
    memset(&callback_info, 0, sizeof(_go32_dpmi_seginfo));
    memset(&callback_regs, 0, sizeof(_go32_dpmi_registers));

    callback_info.pm_offset = (unsigned int)Packet_Receive_Callback;

    _go32_dpmi_allocate_real_mode_callback_retf(&callback_info, &callback_regs);

    segment_of_callback = callback_info.rm_segment;
    offset_of_callback = callback_info.rm_offset;
    address_of_callback = Make_Far_Pointer(segment_of_callback, offset_of_callback);
}

static unsigned short Endian_Swap(unsigned short old_short) {
    unsigned short temp_short;
    unsigned short swap_short;
    unsigned char *char_ptr;
    unsigned char *char_ptr_two;

    temp_short = old_short;

    char_ptr = (unsigned char *)&temp_short;
    char_ptr_two = (unsigned char *)&swap_short;

    char_ptr_two[0] = char_ptr[1];
    char_ptr_two[1] = char_ptr[0];

    return (swap_short);
}

static bool Init_IPX(void) {
    _go32_dpmi_registers r;

    // Clear out the registers
    memset(&r, 0, sizeof(r));

    r.x.ax = 0x7A00;
    _go32_dpmi_simulate_int(0x2f, &r);

    if (r.h.al != 255) {
        DEBUGF("Error in installing IPX %x \n", r.h.al);
        return false;
    }

    Get_Local_Address();

    return true;
}

/*
   Open a socket...
   longetivity   == 0x00 for open till close or terminate
                 == 0xff for open till close use for tsr
   socket_number == 0 for dynamic allocation
                 == anything else
   returns  0x0  == success
            0xfe == socket table full
            0xff == socket already open
*/
static int IPX_Open_Socket(unsigned char longetivity, unsigned short *socket_number) {
    _go32_dpmi_registers r;

    // Clear out the registers
    memset(&r, 0, sizeof(r));

    r.x.bx = 0x0000;  // Function open socket till close or terminate?
    r.h.al = longetivity;

    r.x.dx = Endian_Swap(*socket_number);

    // Call the interrupt
    _go32_dpmi_simulate_int(0x7A, &r);

    if (*socket_number == 0x0000) {
        *socket_number = Endian_Swap(r.x.dx);
    }

    return (r.h.al);
}

static void IPX_Close_Socket(unsigned short *socket_number) {
    _go32_dpmi_registers r;

    // Clear out the registers
    memset(&r, 0, sizeof(r));

    r.x.bx = 0x0001;
    r.x.dx = Endian_Swap(*socket_number);

    _go32_dpmi_simulate_int(0x7A, &r);
}

static void IPX_Send_Packet(ECB_t *ecb) {
    _go32_dpmi_registers r;

    // Clear out the registers
    memset(&r, 0, sizeof(r));

    r.x.bx = 0x0003;  // Function 3 is send packet?
    r.x.es = segment_of_ecb_send_buffer;
    r.x.si = offset_of_ecb_send_buffer;

    Copy_Into_Ecb_Send_Buffer(ecb, sizeof(ECB_t));
    _go32_dpmi_simulate_int(0x7a, &r);
    Copy_From_Ecb_Send_Buffer(ecb, sizeof(ECB_t));
}

static int IPX_Listen_For_Packet(ECB_t *ecb) {
    _go32_dpmi_registers r;

    // Clear out the registers
    memset(&r, 0, sizeof(r));

    r.x.bx = 0x0004;  // Subfunction 4 is listen for packet?
    r.x.es = segment_of_ecb_receive_buffer;
    r.x.si = offset_of_ecb_receive_buffer;

    _go32_dpmi_simulate_int(0x7A, &r);
    Copy_From_Ecb_Receive_Buffer(ecb, sizeof(ECB_t));

    return (r.h.al);
}

/* Tell driver we are idle */
static void IPX_Idle(void) {
    _go32_dpmi_registers r;

    // Clear out the registers
    memset(&r, 0, sizeof(r));

    r.x.bx = 0x000A;

    _go32_dpmi_simulate_int(0x7A, &r);
}

/* Initialize all the basic shizit we need to send packets...store the
   pointer to memory where we will put packets */
static void Init_Send_Packet(ECB_t *ecb, IPX_header_t *ipx_header, unsigned short size, unsigned short sock) {
    int i;

    memset(ipx_header, 0, sizeof(IPX_header_t));
    memset(ecb, 0, sizeof(ECB_t));

    ecb->socket = Endian_Swap(sock);  // Big endian socket number
    ecb->fragment_count = 1;          // Fragment count??
    ecb->ESR = 0;                     // NULL;

    // Pointer to data fragment(ipx header)
    ecb->fragment_data = address_of_send_buffer;

    ecb->fragment_size = sizeof(IPX_header_t) + size;

    for (i = 0; i < 6; i++) {
        ecb->immediate_address[i] = 0xff;  // Broadcast
    }

    ipx_header->checksum = 0xffff;  // No checksum
    ipx_header->packet_type = 0;    // Packet exchange packet

    for (i = 0; i < 4; i++) {
        ipx_header->destination.net[i] = local_address.net[i];  // Send to this network
    }

    for (i = 0; i < 6; i++) {
        ipx_header->destination.node_address[i] = 0xff;  // Send to everybody!
    }

    // USE A DEFINE FOR THE WRITE SOCKET!!!
    ipx_header->destination.socket = Endian_Swap(sock);  // Send to my socket

    for (i = 0; i < 4; i++) {
        ipx_header->source.net[i] = local_address.net[i];
    }

    for (i = 0; i < 6; i++) {
        ipx_header->source.node_address[i] = local_address.node_address[i];  // From me
    }

    ipx_header->source.socket = Endian_Swap(sock);  // From my socket
}

/* Initialize all the basic shizit we need to Receive packets...store the
   pointer to memory where we will put packets */
static bool Init_Receive_Packet(ECB_t *ecb, IPX_header_t *ipx_header, unsigned short size, unsigned short sock) {
    int error_code;

    memset(ecb, 0, sizeof(ECB_t));
    memset(ipx_header, 0, sizeof(IPX_header_t));

    ecb->in_use = 0x1D;  // ??
    ecb->socket = Endian_Swap(sock);
    ecb->fragment_count = 1;

    // Set a real mode callback for my ESR function
    Allocate_Receive_Callback();

    ecb->ESR = address_of_callback;

    ecb->fragment_data = address_of_receive_buffer;

    ecb->fragment_size = sizeof(IPX_header_t) + size;

    Copy_Into_Receive_Buffer(ipx_header, sizeof(IPX_header_t) + size);
    Copy_Into_Ecb_Receive_Buffer(ecb, sizeof(ECB_t));

    if ((error_code = IPX_Listen_For_Packet(ecb)) == 0xFF) {
        DEBUGF("Error in listen for packet function (in init Receive) 0x%x \n", error_code);
        return false;
    }

    Copy_From_Receive_Buffer(ipx_header, sizeof(IPX_header_t) + size);

    return true;
}

static bool Jonipx_Init() {
    int temp_int;
    int i;

    if (Init_IPX()) {
        DEBUG("IPX driver Detected! \n");
    } else {
        DEBUG("No IPX driver detected! \n");
        return false;
    }

    DEBUGF("Opening socket at %x!! \n", socket);

    // longetivity == 0x00 for open till close or terminate , 0xff for open till close
    if (!(temp_int = IPX_Open_Socket(0x00, &socket))) {
        DEBUGF("Socket opened at 0x%x (error code 0x%d)\n", socket, temp_int);
    } else {
        DEBUGF("Error 0x%x opening socket \n", temp_int);
        return false;
    }

    for (i = 0; i < sizeof(string_t); i++) {
        send_packet.string[i] = '\0';
    }
    Init_Send_Packet(&send_packet.ecb, &send_packet.ipx_header, sizeof(string_t), socket);

    Copy_Into_Send_Buffer(&send_packet.ipx_header, sizeof(IPX_header_t));
    Copy_Into_Ecb_Send_Buffer(&send_packet.ecb, sizeof(ECB_t));

    Init_Receive_Packet(&Receive_packet.ecb, &Receive_packet.ipx_header, sizeof(string_t), socket);

    packet_buffer.buffer_pos = 0;
    packet_buffer.buffer_start = 0;

    ipx_initialized = true;
    return true;
}

/*
static bool Jonipx_Get_Packet(string_t string, node_address_t source_node) {
    int i;
    int from_local = 1;  // Is this packet from the local computer?

    IPX_Idle();

    Copy_From_Ecb_Receive_Buffer(&Receive_packet.ecb, sizeof(ECB_t));

    if (Receive_packet.ecb.in_use == 0) {
        Copy_From_Receive_Buffer(&Receive_packet.ipx_header, sizeof(IPX_header_t) + sizeof(string_t));

        DEBUGF("receiving %s", Receive_packet.string);

        if (IPX_Listen_For_Packet(&Receive_packet.ecb)) {
            DEBUG("Error in loop listening for packets... \n");
            return false;
        }
        Copy_From_Receive_Buffer(&Receive_packet.ipx_header, sizeof(IPX_header_t) + sizeof(string_t));

        // Ignore shit that I sent
        for (i = 0; i < 6; i++) {
            if (Receive_packet.ipx_header.source.node_address[i] != local_address.node_address[i]) {
                from_local = 0;
            }
        }

        // Copy the source node address of the packet
        for (i = 0; i < 6; i++) {
            source_node[i] = Receive_packet.ipx_header.source.node_address[i];
        }

        if (from_local) {
            return false;
        }

        for (i = 0; i < sizeof(string_t); i++) {
            string[i] = Receive_packet.string[i];
        }

        return true;
    } else {  // End if packet ready
        return false;
    }
}
*/

static void Jonipx_Send_Packet(string_t string, int length, node_address_t dest_node) {
    int i;

    IPX_Idle();
    Copy_From_Ecb_Send_Buffer(&send_packet.ecb, sizeof(ECB_t));

    // while (send_packet.ecb.in_use != 0) {
    //     DEBUGF("waiting for clear to send  0x%x \n", send_packet.ecb.in_use);
    //     Copy_From_Ecb_Send_Buffer(&send_packet.ecb, sizeof(ECB_t));
    // }

    for (i = 0; i < 6; i++) {
        send_packet.ecb.immediate_address[i] = dest_node[i];  // 0xff; Broadcast
    }

    for (i = 0; i < 6; i++) {
        send_packet.ipx_header.destination.node_address[i] = dest_node[i];  // 0xff; Broadcast
    }

    for (i = 0; i < length; i++) {
        send_packet.string[i] = string[i];
    }

    Copy_Into_Send_Buffer(&send_packet.ipx_header, sizeof(IPX_header_t) + sizeof(string_t));
    IPX_Send_Packet(&send_packet.ecb);

    IPX_Idle();  // HACK??

    delay(IPX_DELAY);
}

static void Jonipx_Close(void) {
    if (ipx_initialized) {
        DEBUG("Closing socket \n");
        IPX_Close_Socket(&socket);
        socket = 0;
    }
}

/* Is there a packet ready in the packet buffer? */
static bool Jonipx_Packet_Ready(void) {
    if (packet_buffer.buffer_start != packet_buffer.buffer_pos) {
        return true;
    } else {
        return false;
    }
}

/* Copy the next available packet into string */
static bool Jonipx_Pop_Packet(game_packet_t *packet) {
    int i;

    if (!Jonipx_Packet_Ready()) {
        return false;
    }

    for (i = 0; i < sizeof(string_t); i++) {
        packet->string[i] = packet_buffer.packets[packet_buffer.buffer_start].string[i];
    }

    for (i = 0; i < sizeof(node_address_t); i++) {
        packet->source_node[i] = packet_buffer.packets[packet_buffer.buffer_start].source_node[i];
    }

    packet_buffer.buffer_start++;

    if (packet_buffer.buffer_start >= MAX_PACKETS_IN_BUFFER) {
        packet_buffer.buffer_start = 0;
    }

    return true;
}

static bool ipx_convertAddress(js_State *J, int idx, node_address_t addr) {
    if (!js_isarray(J, idx)) {
        JS_ENOARR(J);
        return false;
    } else {
        int len = js_getlength(J, idx);
        if (len != ADDRESS_SIZE) {
            js_error(J, "address must be of length %d", ADDRESS_SIZE);
            return false;
        }
        // DEBUG("Copying address data\n");
        for (int i = 0; i < ADDRESS_SIZE; i++) {
            js_getindex(J, idx, i);
            addr[i] = js_toint16(J, -1);
            js_pop(J, 1);
        }
        return true;
    }
}

/***********************
** exported functions **
***********************/
/**
 * @brief open an IPX socket.
 *
 * IpxSocketOpen(num:number)
 *
 * @param J the JS context.
 */
static void ipx_IpxSocketOpen(js_State *J) {
    if (DOjS.ipx_available) {
        if (!socket) {
            socket = js_toint16(J, 1);

            Jonipx_Init();
        } else {
            js_error(J, "Socket already open with address 0x%04X.", socket);
        }
    } else {
        js_error(J, "IPX not available");
    }
}

/**
 * @brief close IPX socket (if any).
 *
 * IpxSocketClose()
 *
 * @param J the JS context.
 */
static void ipx_IpxSocketClose(js_State *J) {
    if (socket && DOjS.ipx_available) {
        Jonipx_Close();
    }
}

/**
 * @brief send packet via IPX. Max length 79 byte.
 *
 * IpxSendPacket(data:string, dest:array[6:number])
 *
 * @param J the JS context.
 */
static void ipx_IpxSend(js_State *J) {
    if (socket) {
        const char *str = js_tostring(J, 1);

        int str_len = strlen(str) + 1;
        if (str_len > MAX_PACKET_LEN) {
            js_error(J, "Max packet length is %d byte, data length is %d", MAX_PACKET_LEN, str_len);
            return;
        }

        string_t data;
        memcpy(data, str, str_len);
        // DEBUGF("Data length is %d\n", str_len);

        node_address_t addr;
        if (!ipx_convertAddress(J, 2, addr)) {
            return;
        }
        // DEBUGF("sending to %02x:%02x:%02x:%02x:%02x:%02x\n", addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);

        Jonipx_Send_Packet(data, str_len, addr);
    } else {
        js_error(J, "No socket open.");
    }
}

/**
 * @brief check for packet in receive buffer
 *
 * IpxCheckPacket():boolean
 *
 * @param J the JS context.
 */
static void ipx_IpxCheckPacket(js_State *J) {
    if (socket) {
        js_pushboolean(J, Jonipx_Packet_Ready());
    } else {
        js_error(J, "No socket open.");
    }
}

/**
 * @brief get packet from receive buffer (or NULL).
 *
 * IpxGetPacket():{data:string, source:array[6:number]}
 *
 * @param J the JS context.
 */
static void ipx_IpxGetPacket(js_State *J) {
    if (socket) {
        if (Jonipx_Packet_Ready()) {
            game_packet_t packet;
            Jonipx_Pop_Packet(&packet);

            js_newobject(J);
            {
                js_pushstring(J, packet.string);
                js_setproperty(J, -2, "data");
                js_newarray(J);
                for (int i = 0; i < ADDRESS_SIZE; i++) {
                    js_pushnumber(J, packet.source_node[i]);
                    js_setindex(J, -2, i);
                }
                js_setproperty(J, -2, "source");
            }
        } else {
            js_pushnull(J);
        }
    } else {
        js_error(J, "No socket open.");
    }
}

/**
 * @brief get the local address.
 *
 * IpxGetLocalAddress():array[6:number]
 *
 * @param J the JS context.
 */
static void ipx_IpxGetLocalAddress(js_State *J) {
    if (socket) {
        js_newarray(J);
        for (int i = 0; i < ADDRESS_SIZE; i++) {
            js_pushnumber(J, local_address.node_address[i]);
            js_setindex(J, -2, i);
        }
    } else {
        js_error(J, "No socket open.");
    }
}

/**
 * @brief initialize fm music subsystem.
 *
 * @param J VM state.
 */
void init_ipx(js_State *J) {
    DEBUGF("%s\n", __PRETTY_FUNCTION__);

    FUNCDEF(J, ipx_IpxSocketOpen, "IpxSocketOpen", 1);
    FUNCDEF(J, ipx_IpxSocketClose, "IpxSocketClose", 0);
    FUNCDEF(J, ipx_IpxSend, "IpxSend", 2);
    FUNCDEF(J, ipx_IpxCheckPacket, "IpxCheckPacket", 0);
    FUNCDEF(J, ipx_IpxGetPacket, "IpxGetPacket", 0);
    FUNCDEF(J, ipx_IpxGetLocalAddress, "IpxGetLocalAddress", 0);

    // Check to see if IPX is present
    if (Allocate_Dos_Buffers() && Init_IPX()) {
        LOG("IPX available.\n");
        DOjS.ipx_available = true;
    } else {
        // If not, end the programme
        LOG("IPX not found.\n");
        DOjS.ipx_available = false;
    }
    PROPDEF_B(J, DOjS.ipx_available, "IPX_AVAILABLE");

    DEBUGF("%s DONE\n", __PRETTY_FUNCTION__);
}

/**
 * @brief shutdown fm music subsystem.
 */
void shutdown_ipx() {
    DEBUGF("%s\n", __PRETTY_FUNCTION__);
    Jonipx_Close();
    DEBUGF("%s DONE\n", __PRETTY_FUNCTION__);
}
