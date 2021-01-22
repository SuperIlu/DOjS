/*
MIT License

Copyright (c) 2019-2021 Andre Seidelt <superilu@yahoo.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#include "comport.h"

#include <dzcomm.h>
#include <errno.h>
#include <mujs.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "DOjS.h"

/************
** defines **
************/
#define COM_NUM_PORTS 8       //!< max number of ports
#define COM_PORT "PORT"       //!< field name for 'port number'
#define COM_BUFFER_SIZE 4096  //!< max line length in Com_ReadBuffer()

/*********************
** static variables **
*********************/
static bool com_used[COM_NUM_PORTS];  //!< keep track of opened ports

/************
** structs **
************/
//! file userdata definition
typedef struct __comport {
    comm_port *port;  //!< the port pointer
    int port_num;     //!< port number
} comport_t;

#define COM_CLEANUP(p)                 \
    {                                  \
        comm_port_uninstall(p->port);  \
        comm_port_delete(p->port);     \
        com_used[p->port_num] = false; \
        free(p);                       \
    }

/*********************
** static functions **
*********************/
/**
 * @brief finalize a file and free resources.
 *
 * @param J VM state.
 */
static void Com_Finalize(js_State *J, void *data) {
    comport_t *p = (comport_t *)data;

    DEBUGF("fin COM%d=%p\n", p->port_num + 1, p->port);

    COM_CLEANUP(p);
}

/**
 * @brief open a com port.
 * new COMPort(port:number, baud:number, bits:number, parity:number, stop:number, flow:number[, addr:number, irq:number])
 *
 * @param J VM state.
 */
static void new_Com(js_State *J) {
    NEW_OBJECT_PREP(J);

    int com = js_tointeger(J, 1);
    if (com < _com1 || com > _com4) {
        js_error(J, "COM parameter must be one of COM.PORT.COMx!");
        return;

    } else if (com_used[com]) {
        js_error(J, "COM port already opened!");
        return;
    }

    comport_t *p = malloc(sizeof(comport_t));
    if (!p) {
        JS_ENOMEM(J);
        return;
    }
    p->port = NULL;
    p->port_num = com;

    DEBUGF("init COM%d=%p\n", p->port_num + 1, p->port);
    com_used[com] = true;
    p->port = comm_port_init(com);
    if (!p->port) {
        js_error(J, "COM error: %s!", szDZCommErr);
        COM_CLEANUP(p);
        return;
    }

    DEBUGF("set baud COM%d=%p\n", p->port_num + 1, p->port);
    int baud = js_tointeger(J, 2);
    if (baud < _75 || baud > _115200) {
        js_error(J, "Baud rate must be one of COM.BAUD.Bx!");
        COM_CLEANUP(p);
        return;
    }
    comm_port_set_baud_rate(p->port, baud);

    DEBUGF("set bits COM%d=%p\n", p->port_num + 1, p->port);
    int bits = js_tointeger(J, 3);
    if (bits < BITS_5 || bits > BITS_8) {
        js_error(J, "Word length must be one of COM.BIT.BITS_x!");
        COM_CLEANUP(p);
        return;
    }
    comm_port_set_data_bits(p->port, bits);

    DEBUGF("set parity COM%d=%p\n", p->port_num + 1, p->port);
    int par = js_tointeger(J, 4);
    if (par < NO_PARITY || par > SPACE_PARITY) {
        js_error(J, "Word length must be one of COM.PARITY.x_PARITY!");
        COM_CLEANUP(p);
        return;
    }
    comm_port_set_parity(p->port, par);

    DEBUGF("set stop COM%d=%p\n", p->port_num + 1, p->port);
    int stop = js_tointeger(J, 5);
    if (stop != STOP_1 && stop != STOP_2) {
        js_error(J, "Stop bits must be one of COM.STOP.STOP_x!");
        COM_CLEANUP(p);
        return;
    }
    comm_port_set_stop_bits(p->port, stop);

    DEBUGF("set flow COM%d=%p\n", p->port_num + 1, p->port);
    int flow = js_tointeger(J, 6);
    if (flow < NO_CONTROL || flow > RTS_CTS) {
        js_error(J, "Flow control must be one of COM.FLOW.x!");
        COM_CLEANUP(p);
        return;
    }
    comm_port_set_flow_control(p->port, flow);

    DEBUGF("set addr/irq COM%d=%p\n", p->port_num + 1, p->port);
    if (js_isnumber(J, 7) && js_isnumber(J, 8)) {
        int addr = js_tointeger(J, 7);
        int irq = js_tointeger(J, 8);
        comm_port_set_port_address(p->port, addr);
        if (irq < 0 || irq > 15) {
            js_error(J, "Irq must be between 0..15!");
            COM_CLEANUP(p);
            return;
        }
        comm_port_set_irq_num(p->port, irq);
    }

    DEBUGF("Opening COM%d with irq=%d and port=0x%X as %p\n", com + 1, comm_port_get_irq_num(p->port), comm_port_get_port_address(p->port), p->port);

    if (!comm_port_install_handler(p->port)) {
        js_error(J, "COM error: %s!", szDZCommErr);
        COM_CLEANUP(p);
        return;
    }

    js_currentfunction(J);
    js_getproperty(J, -1, "prototype");
    js_newuserdata(J, TAG_COM, p, Com_Finalize);
}

/**
 * @brief close port.
 * com.Close()
 *
 * @param J VM state.
 */
static void Com_Close(js_State *J) {
    comport_t *p = js_touserdata(J, 0, TAG_COM);

    DEBUGF("close COM%d=%p\n", p->port_num + 1, p->port);

    // close and delete port if still open, mark port as free again
    if (p->port) {
        comm_port_uninstall(p->port);
        comm_port_delete(p->port);
        p->port = NULL;
    } else {
        js_error(J, "Port is already closed!");
    }
    com_used[p->port_num] = false;
}

/**
 * @brief flush.
 * com.FlushInput()
 *
 * @param J VM state.
 */
static void Com_FlushInput(js_State *J) {
    comport_t *p = js_touserdata(J, 0, TAG_COM);

    if (p->port) {
        comm_port_flush_input(p->port);
    } else {
        js_error(J, "Port is closed!");
    }
}

/**
 * @brief flush.
 * com.FlushOutput()
 *
 * @param J VM state.
 */
static void Com_FlushOutput(js_State *J) {
    comport_t *p = js_touserdata(J, 0, TAG_COM);

    if (p->port) {
        comm_port_flush_output(p->port);
    } else {
        js_error(J, "Port is closed!");
    }
}

/**
 * @brief com.IsOutputEmpty():boolean
 *
 * @param J VM state.
 */
static void Com_IsOutputEmpty(js_State *J) {
    comport_t *p = js_touserdata(J, 0, TAG_COM);

    if (p->port) {
        js_pushboolean(J, comm_port_out_empty(p->port) == -1);
    } else {
        js_error(J, "Port is closed!");
    }
}

/**
 * @brief com.IsOutputFull():boolean
 *
 * @param J VM state.
 */
static void Com_IsOutputFull(js_State *J) {
    comport_t *p = js_touserdata(J, 0, TAG_COM);

    if (p->port) {
        js_pushboolean(J, comm_port_out_full(p->port) == -1);
    } else {
        js_error(J, "Port is closed!");
    }
}

/**
 * @brief com.WriteByte(number)
 *
 * @param J VM state.
 */
static void Com_WriteByte(js_State *J) {
    comport_t *p = js_touserdata(J, 0, TAG_COM);

    if (p->port) {
        const char ch = js_toint16(J, 1);

        int ret = comm_port_out(p->port, ch);
        if (ret != 1) {
            js_error(J, "TX buffer overflow!");
        }
    } else {
        js_error(J, "Port is closed!");
    }
}

/**
 * @brief com.WriteString(string):number
 *
 * @param J VM state.
 */
static void Com_WriteString(js_State *J) {
    comport_t *p = js_touserdata(J, 0, TAG_COM);

    if (p->port) {
        const char *str = js_tostring(J, 1);

        js_pushnumber(J, comm_port_string_send(p->port, str));
    } else {
        js_error(J, "Port is closed!");
    }
}

/**
 * @brief com.IsInputEmpty():boolean
 *
 * @param J VM state.
 */
static void Com_IsInputEmpty(js_State *J) {
    comport_t *p = js_touserdata(J, 0, TAG_COM);

    if (p->port) {
        js_pushboolean(J, comm_port_in_empty(p->port) == -1);
    } else {
        js_error(J, "Port is closed!");
    }
}

/**
 * @brief com.IsInputFull():boolean
 *
 * @param J VM state.
 */
static void Com_IsInputFull(js_State *J) {
    comport_t *p = js_touserdata(J, 0, TAG_COM);

    if (p->port) {
        js_pushboolean(J, comm_port_in_full(p->port) == -1);
    } else {
        js_error(J, "Port is closed!");
    }
}

/**
 * @brief com.ReadByte():number
 *
 * @param J VM state.
 */
static void Com_ReadByte(js_State *J) {
    comport_t *p = js_touserdata(J, 0, TAG_COM);

    if (p->port) {
        int ret = comm_port_test(p->port);
        DEBUGF("RC 0x%02X\n", ret);
        if (ret < 0) {
            js_pushnull(J);
        } else {
            js_pushnumber(J, ((int)ret) & 0xFF);
        }
    } else {
        js_error(J, "Port is closed!");
    }
}

/**
 * @brief com.ReadBuffer():string
 *
 * @param J VM state.
 */
static void Com_ReadBuffer(js_State *J) {
    comport_t *p = js_touserdata(J, 0, TAG_COM);

    if (p->port) {
        int pos = 0;
        char buf[COM_BUFFER_SIZE];

        while (pos < COM_BUFFER_SIZE) {
            int ret = comm_port_test(p->port);
            DEBUGF("RB %0x02X\n", ret);
            if (ret < 0) {
                break;
            } else {
                buf[pos] = ((int)ret) & 0xFF;
                pos++;
            }
        }
        js_pushlstring(J, buf, pos);
    } else {
        js_error(J, "Port is closed!");
    }
}

/***********************
** exported functions **
***********************/
#define COM_PUSH_BAUD(b)               \
    {                                  \
        js_pushnumber(J, _##b);        \
        js_setproperty(J, -2, "B" #b); \
    }

#define COM_PUSH_VALUE(n)          \
    {                              \
        js_pushnumber(J, n);       \
        js_setproperty(J, -2, #n); \
    }

#define COM_PUSH_VALUE_NAME(n, v) \
    {                             \
        js_pushnumber(J, v);      \
        js_setproperty(J, -2, n); \
    }

/**
 * @brief initialize file subsystem.
 *
 * @param J VM state.
 */
void init_comport(js_State *J) {
    DEBUGF("%s\n", __PRETTY_FUNCTION__);

    // mark all ports as unused
    for (int i = 0; i < COM_NUM_PORTS; i++) {
        com_used[i] = false;
    }

    dzcomm_init();  // initialize DZComm

    // push the enums into COM object in 'global'
    js_newobject(J);
    {
        {
            js_newobject(J);
            COM_PUSH_BAUD(50);
            COM_PUSH_BAUD(75);
            COM_PUSH_BAUD(110);
            COM_PUSH_BAUD(134);
            COM_PUSH_BAUD(150);
            COM_PUSH_BAUD(200);
            COM_PUSH_BAUD(300);
            COM_PUSH_BAUD(600);
            COM_PUSH_BAUD(1200);
            COM_PUSH_BAUD(1800);
            COM_PUSH_BAUD(2400);
            COM_PUSH_BAUD(4800);
            COM_PUSH_BAUD(9600);
            COM_PUSH_BAUD(19200);
            COM_PUSH_BAUD(38400);
            COM_PUSH_BAUD(57600);
            COM_PUSH_BAUD(115200);
            js_setproperty(J, -2, "BAUD");
        }

        {
            js_newobject(J);
            COM_PUSH_VALUE(NO_PARITY);
            COM_PUSH_VALUE(ODD_PARITY);
            COM_PUSH_VALUE(EVEN_PARITY);
            COM_PUSH_VALUE(MARK_PARITY);
            COM_PUSH_VALUE(SPACE_PARITY);
            js_setproperty(J, -2, "PARITY");
        }

        {
            js_newobject(J);
            COM_PUSH_VALUE(STOP_1);
            COM_PUSH_VALUE(STOP_2);
            js_setproperty(J, -2, "STOP");
        }

        {
            js_newobject(J);
            COM_PUSH_VALUE(NO_CONTROL);
            COM_PUSH_VALUE(XON_XOFF);
            COM_PUSH_VALUE(RTS_CTS);
            js_setproperty(J, -2, "FLOW");
        }

        {
            js_newobject(J);
            COM_PUSH_VALUE(BITS_5);
            COM_PUSH_VALUE(BITS_6);
            COM_PUSH_VALUE(BITS_7);
            COM_PUSH_VALUE(BITS_8);
            js_setproperty(J, -2, "BIT");
        }

        {
            js_newobject(J);
            COM_PUSH_VALUE_NAME("COM1", _com1);
            COM_PUSH_VALUE_NAME("COM2", _com2);
            COM_PUSH_VALUE_NAME("COM3", _com3);
            COM_PUSH_VALUE_NAME("COM4", _com4);
            js_setproperty(J, -2, "PORT");
        }
    }
    js_setglobal(J, "COM");

    js_newobject(J);
    {
        NPROTDEF(J, Com, Close, 0);
        NPROTDEF(J, Com, FlushInput, 0);
        NPROTDEF(J, Com, FlushOutput, 0);
        NPROTDEF(J, Com, IsOutputEmpty, 0);
        NPROTDEF(J, Com, IsOutputFull, 0);
        NPROTDEF(J, Com, WriteByte, 1);
        NPROTDEF(J, Com, WriteString, 1);
        NPROTDEF(J, Com, IsInputEmpty, 0);
        NPROTDEF(J, Com, IsInputFull, 0);
        NPROTDEF(J, Com, ReadByte, 0);
        NPROTDEF(J, Com, ReadBuffer, 0);
    }
    CTORDEF(J, new_Com, TAG_COM, 8);

    DEBUGF("%s DONE\n", __PRETTY_FUNCTION__);
}

/**
 * @brief shutdown DZComm.
 */
void shutdown_comport() {
    DEBUGF("%s\n", __PRETTY_FUNCTION__);
    dzcomm_closedown();
    DEBUGF("%s DONE\n", __PRETTY_FUNCTION__);
}
