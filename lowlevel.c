/*
MIT License

Copyright (c) 2019-2020 Andre Seidelt <superilu@yahoo.com>

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

#include "lowlevel.h"

#include <bios.h>

#include "DOjS.h"

/************
** defines **
************/
#define LL_HDD_FLAG (1 << 7)  //!< HDDs start at 0x80, FDDs at 0

#define LL_INT13_42_SIZE 0x10  //!< size of INT13,42 data structure
#define LL_INT13_48_SIZE 0x1E  //!< size of INT13,48 data structure

#define LL_BLOCKSIZE 512                                 //!< block size is hardcoded to 512 byte for now
#define LL_DOSMEMSIZE (LL_BLOCKSIZE + LL_INT13_42_SIZE)  //!< size of dosmem is blocksize + size for int13,42 structure size

#define LL_DISK_INT 0x13  //!< BIOS disk access INT

/************
** structs **
************/
//!< see https://en.wikipedia.org/wiki/INT_13H#INT_13h_AH=42h:_Extended_Read_Sectors_From_Drive
typedef struct {
    uint8_t size;
    uint8_t unused;
    uint16_t num_sectors;
    uint16_t offset;
    uint16_t segment;
    uint64_t start_sector;
} __attribute__((packed)) ll_int13_42_t;

//!< https://en.wikipedia.org/wiki/INT_13H#INT_13h_AH=48h:_Extended_Read_Drive_Parameters
typedef struct {
    uint16_t size;
    uint16_t flags;
    uint32_t cylinders;
    uint32_t heads;
    uint32_t sectors_track;
    uint64_t total_sectors;
    uint16_t bytes_sector;
} __attribute__((packed)) ll_int13_48_t;

/*********************
** static variables **
*********************/
static int ll_dos_segment = 0;   //!< dos mem segment
static int ll_dos_selector = 0;  //!< dos mem selector

/*********************
** static functions **
*********************/
/**
 * free buffer (called at exit)
 */
static void ll_free_dos_buffer(void) {
    __dpmi_free_dos_memory(ll_dos_selector);
    ll_dos_segment = ll_dos_selector = 0;
}

/**
 * allocate static buffer for all raw disk access
 */
static void ll_alloc_dos_buffer(void) {
    if (ll_dos_segment) {
        return;
    }
    ll_dos_segment = __dpmi_allocate_dos_memory(LL_DOSMEMSIZE, &ll_dos_selector);
    if (ll_dos_segment == -1) {
        ll_dos_segment = 0;
        return;
    }
    atexit(ll_free_dos_buffer);
}

/**
 * @brief transfer data from linear to dosmem
 *
 * @param buff pointer to linear buffer
 * @param len bytes to TX, must not exceed LL_DOSMEMSIZE!
 */
static void ll_dosput(const char *buff, int len) { dosmemput(buff, len, 16 * ll_dos_segment); }

/**
 * @brief transfer data from dosmem to linear.
 *
 * @param buff pointer to linear buffer
 * @param len bytes to TX, must not exceed LL_DOSMEMSIZE!
 */
static void ll_dosget(char *buff, int len) { dosmemget(16 * ll_dos_segment, len, buff); }

/**
 * @brief get drive number from JS withs anity checks
 *
 * @param J VM state.
 * @param idx stack index
 * @return drive number or -1 for failure
 */
static int ll_get_drive(js_State *J, int idx) {
    if (!js_isnumber(J, idx)) {
        js_error(J, "drive parameter must be a number!");
        return -1;
    }

    int drive = js_toint16(J, idx);

    if (drive < 0) {
        js_error(J, "drive parameter must be > 0!");
        return -1;
    }

    if (drive < LL_HDD_FLAG) {
        // FDD
        unsigned long ptraddr = 0x0410;  // Base Address: segment is zero
        unsigned int eq_list = _farpeekw(_dos_ds, ptraddr);

        int num_fdd = (0x01 & (eq_list >> 7)) + 1;
        if (drive > num_fdd) {
            js_error(J, "drive parameter exeeds number of FDD!");
            return -1;
        } else {
            return drive;
        }
    } else {
        // HDD
        unsigned long ptraddr = 0x0475;  // Base Address: segment is zero
        int num_hdd = _farpeekb(_dos_ds, ptraddr);
        if (drive > num_hdd + LL_HDD_FLAG) {
            js_error(J, "drive parameter exeeds number of HDD!");
            return -1;
        } else {
            return drive;
        }
    }
}

/**
 * @brief call int13,41 to check for 4x extensions
 *
 * @param drive drive number
 * @return true if the extension is detected, else false
 */
static bool ll_int41_extensions_check(int drive) {
    __dpmi_regs r;

    r.h.ah = 0x41;  // http://www.ctyme.com/intr/rb-0706.htm
    r.h.dl = drive;
    int ret = __dpmi_int(LL_DISK_INT, &r);
    if (ret == 0) {
        if (r.x.flags & 1) { /* is carry flag set?  */
            return false;
        }
        return r.x.bx == 0xAA55 && r.h.ah >= 0x01;
    }
    return false;
}

/**
 * @brief determine number of sectors for drives without int13,4x extension
 *
 * @param drive drive number
 * @return uint32_t number of sectors
 */
static uint32_t ll_int08_drive_parameters(int drive) {
    __dpmi_regs r;

    r.h.ah = 0x08;  // http://www.ctyme.com/intr/rb-0621.htm
    r.h.dl = drive;
    r.x.es = 0x00;
    r.x.di = 0x00;
    int ret = __dpmi_int(LL_DISK_INT, &r);
    if (ret == 0) {
        if (r.x.flags & 1) { /* is carry flag set?  */
            return false;
        }
        uint16_t heads = r.h.dh + 1;
        uint16_t sectors = r.h.cl & 0x3F;
        uint32_t cylinders = ((uint32_t)r.h.ch | ((uint32_t)(r.h.cl & 0xC0) << 2)) + 1;

        return cylinders * sectors * heads;
    }
    return 0;
}

/**
 * @brief determine number of sectors for drives with int13,4x extension
 *
 * @param drive drive number
 * @return uint32_t number of sectors
 */
static uint32_t ll_int48_extended_drive_parameters(int drive) {
    char buffer[LL_DOSMEMSIZE];
    __dpmi_regs r;
    ll_int13_48_t *data = (ll_int13_48_t *)buffer;

    ll_alloc_dos_buffer();
    if (!ll_dos_segment) {
        DEBUGF("Error getting DOSMEM\n");
        return 0;  // error
    }
    data->size = LL_INT13_48_SIZE;
    ll_dosput(buffer, data->size);
    r.h.ah = 0x48;  // http://www.ctyme.com/intr/rb-0715.htm
    r.h.dl = drive;
    r.x.ds = ll_dos_segment;
    r.x.si = 0;
    int ret = __dpmi_int(LL_DISK_INT, &r);
    if (ret == 0) {
        if (r.x.flags & 1) { /* is carry flag set?  */
            return false;
        }
        ll_dosget(buffer, data->size);
        return data->total_sectors;
    }
    return 0;
}

/**
 * @brief read a sector for drives without in13,4x extension
 *
 * @param drive drive number
 * @param lba block number
 * @param buff destination buffer, must be able to hold LL_BLOCKSIZE bytes.
 * @return true if reading is successfull, else false
 */
static bool ll_int02_read_sector(int drive, uint32_t lba, char *buff) {
    __dpmi_regs r;

    uint16_t heads;
    uint16_t sectors;

    ll_alloc_dos_buffer();
    if (!ll_dos_segment) {
        DEBUGF("Error getting DOSMEM\n");
        return false;  // error
    }

    // get drive info
    r.h.ah = 0x08;  // http://www.ctyme.com/intr/rb-0621.htm
    r.h.dl = drive;
    r.x.es = 0x00;
    r.x.di = 0x00;
    int ret = __dpmi_int(LL_DISK_INT, &r);
    if (ret == 0) {
        if (r.x.flags & 1) { /* is carry flag set?  */
            return false;
        }
        heads = r.h.dh + 1;
        sectors = r.h.cl & 0x3F;
    } else {
        return false;
    }

    // calculate CHS
    uint16_t cylinder = lba / (heads * sectors);
    uint16_t temp = lba % (heads * sectors);
    uint16_t head = temp / sectors;
    uint16_t sector = temp % sectors + 1;

    DEBUGF("LBA=%ld reading CHS=%d/%d/%d\n", lba, cylinder, head, sector);

    // read sector
    r.h.ah = 0x02;  // http://www.ctyme.com/intr/rb-0607.htm
    r.h.al = 1;
    r.h.dl = drive;
    r.x.es = ll_dos_segment;
    r.x.bx = 0;
    r.h.ch = cylinder & 0xff;
    r.h.cl = sector | ((cylinder >> 2) & 0xc0);
    r.h.dh = head;
    ret = __dpmi_int(LL_DISK_INT, &r);
    if (ret == 0) {
        if (r.x.flags & 1) { /* is carry flag set?  */
            return false;
        }
        ll_dosget(buff, LL_BLOCKSIZE);
        return true;
    }
    return false;
}

/**
 * @brief write a sector for drives without in13,4x extension
 *
 * @param drive drive number
 * @param lba block number
 * @param buff data buffer, must be at least LL_BLOCKSIZE bytes.
 * @return true if writing is successfull, else false
 */
static bool ll_int03_write_sector(int drive, uint32_t lba, char *buff) {
    __dpmi_regs r;

    uint16_t heads;
    uint16_t sectors;

    ll_alloc_dos_buffer();
    if (!ll_dos_segment) {
        DEBUGF("Error getting DOSMEM\n");
        return false;  // error
    }

    // get drive info
    r.h.ah = 0x08;  // http://www.ctyme.com/intr/rb-0621.htm
    r.h.dl = drive;
    r.x.es = 0x00;
    r.x.di = 0x00;
    int ret = __dpmi_int(LL_DISK_INT, &r);
    if (ret == 0) {
        if (r.x.flags & 1) { /* is carry flag set?  */
            return false;
        }
        heads = r.h.dh + 1;
        sectors = r.h.cl & 0x3F;
    } else {
        return false;
    }

    // calculate CHS
    uint16_t cylinder = lba / (heads * sectors);
    uint16_t temp = lba % (heads * sectors);
    uint16_t head = temp / sectors;
    uint16_t sector = temp % sectors + 1;

    DEBUGF("LBA=%ld reading CHS=%d/%d/%d\n", lba, cylinder, head, sector);

    // write sector
    r.h.ah = 0x03;  // http://www.ctyme.com/intr/rb-0608.htm
    r.h.al = 1;
    r.h.dl = drive;
    r.x.es = ll_dos_segment;
    r.x.bx = 0;
    r.h.ch = cylinder & 0xff;
    r.h.cl = sector | ((cylinder >> 2) & 0xc0);
    r.h.dh = head;
    ll_dosput(buff, LL_BLOCKSIZE);
    ret = __dpmi_int(LL_DISK_INT, &r);
    if (ret == 0) {
        if (r.x.flags & 1) { /* is carry flag set?  */
            return false;
        }
        return true;
    }
    return false;
}

/**
 * @brief read a sector for drives with in13,4x extension
 *
 * @param drive drive number
 * @param lba block number
 * @param buff destination buffer, must be able to hold LL_BLOCKSIZE bytes.
 * @return true if reading is successfull, else false
 */
static bool ll_int42_extended_read(int drive, uint64_t lba, char *buff) {
    char buffer[LL_DOSMEMSIZE];
    __dpmi_regs r;
    ll_int13_42_t *data = (ll_int13_42_t *)buffer;

    ll_alloc_dos_buffer();
    if (!ll_dos_segment) {
        DEBUGF("Error getting DOSMEM\n");
        return false;  // error
    }
    data->size = LL_INT13_42_SIZE;
    data->offset = LL_INT13_42_SIZE;
    data->segment = ll_dos_segment;
    data->num_sectors = 1;
    data->start_sector = lba;
    ll_dosput(buffer, data->size);
    r.h.ah = 0x42;  // http://www.ctyme.com/intr/rb-0708.htm
    r.h.dl = drive;
    r.x.ds = ll_dos_segment;
    r.x.si = 0;
    int ret = __dpmi_int(LL_DISK_INT, &r);
    if (ret == 0) {
        if (r.x.flags & 1) { /* is carry flag set?  */
            return false;
        }
        ll_dosget(buffer, LL_DOSMEMSIZE);
        memcpy(buff, &buffer[LL_INT13_42_SIZE], LL_BLOCKSIZE);
        return true;
    }
    return false;
}

/**
 * @brief write a sector for drives with in13,4x extension
 *
 * @param drive drive number
 * @param lba block number
 * @param buff data buffer, must be at least LL_BLOCKSIZE bytes.
 * @return true if writing is successfull, else false
 */
static bool ll_int43_extended_write(int drive, uint64_t lba, char *buff) {
    char buffer[LL_DOSMEMSIZE];
    __dpmi_regs r;
    ll_int13_42_t *data = (ll_int13_42_t *)buffer;

    ll_alloc_dos_buffer();
    if (!ll_dos_segment) {
        DEBUGF("Error getting DOSMEM\n");
        return false;  // error
    }
    data->size = LL_INT13_42_SIZE;  // same struct as for reading
    data->offset = LL_INT13_42_SIZE;
    data->segment = ll_dos_segment;
    data->num_sectors = 1;
    data->start_sector = lba;
    memcpy(&buffer[LL_INT13_42_SIZE], buff, LL_BLOCKSIZE);  // copy data behind struct
    ll_dosput(buffer, LL_DOSMEMSIZE);                       // copy everything to dosmem
    r.h.ah = 0x43;                                          // http://www.ctyme.com/intr/rb-0710.htm
    r.h.dl = drive;
    r.x.ds = ll_dos_segment;
    r.x.si = 0;
    int ret = __dpmi_int(LL_DISK_INT, &r);
    if (ret == 0) {
        if (r.x.flags & 1) { /* is carry flag set?  */
            return false;
        }
        return true;
    }
    return false;
}

/**
 * @brief query disk status
 *
 * @param drive drive number
 * @return status as returned by BIOS
 */
static int ll_int01_disk_status(int drive) {
    __dpmi_regs r;

    r.h.ah = 0x01;
    r.h.dl = drive;
    int ret = __dpmi_int(LL_DISK_INT, &r);
    if (ret == 0) {
        return 0xFF & r.h.ah;
    }
    return -1;
}

/**
 * @brief query the disk status
 * GetDiskStatus():number
 *
 * @param J VM state.
 */
static void f_GetDiskStatus(js_State *J) {
    int drive = ll_get_drive(J, 1);
    if (drive < 0) {
        return;
    }
    js_pushnumber(J, ll_int01_disk_status(drive));
}

/**
 * @brief get number of sectors for a drive.
 * GetRawSectorSize(drive:number):number
 *
 * @param J VM state.
 */
static void f_GetRawSectorSize(js_State *J) {
    // check for valid drive number
    int drive = ll_get_drive(J, 1);
    if (drive < 0) {
        return;
    }

    // check if the drive has INT13/$4x extension
    uint32_t num_sectors;
    if (ll_int41_extensions_check(drive)) {
        num_sectors = ll_int48_extended_drive_parameters(drive);
    } else {
        num_sectors = ll_int08_drive_parameters(drive);
    }

    if (num_sectors) {
        js_pushnumber(J, num_sectors);
    } else {
        js_error(J, "Could not determine number of sectors");
    }
}

/**
 * @brief read a sector from a disk.
 * RawRead(disk:number, sector:number):number[]
 *
 * @param J VM state.
 */
static void f_RawRead(js_State *J) {
    char buffer[LL_BLOCKSIZE];

    // check for valid drive number
    int drive = ll_get_drive(J, 1);
    if (drive < 0) {
        return;
    }

    int sector = js_toint32(J, 2);

    // check if the drive has INT13/$4x extension
    bool success;
    if (ll_int41_extensions_check(drive)) {
        success = ll_int42_extended_read(drive, sector, buffer);
    } else {
        success = ll_int02_read_sector(drive, sector, buffer);
    }

    if (success) {
        js_newarray(J);
        for (int j = 0; j < LL_BLOCKSIZE; j++) {
            js_pushnumber(J, 0xFF & buffer[j]);
            js_setindex(J, -2, j);
        }
    } else {
        js_error(J, "Could not read disk sector");
    }
}

/**
 * @brief write a sector to disk
 * RawWrite(disk:number, sector:number, data:number[])
 *
 * @param J VM state.
 */
static void f_RawWrite(js_State *J) {
    char buffer[LL_BLOCKSIZE];

    if (!DOjS.params.raw_write) {
        js_error(J, "Raw write is disabled, needs to be enabled at command line!");
        return;
    }

    // check for valid drive number
    int drive = ll_get_drive(J, 1);
    if (drive < 0) {
        return;
    }

    int sector = js_toint32(J, 2);

    if (!js_isarray(J, 3)) {
        JS_ENOARR(J);
        return;
    }

    int len = js_getlength(J, 3);
    if (len < LL_BLOCKSIZE) {
        js_error(J, "data array must contain at least %d bytes", LL_BLOCKSIZE);
        return;
    }

    for (int i = 0; i < len; i++) {
        js_getindex(J, 3, i);
        buffer[i] = js_toint16(J, -1);
        js_pop(J, 1);
    }

    // check if the drive has INT13/$4x extension
    bool success;
    if (ll_int41_extensions_check(drive)) {
        success = ll_int43_extended_write(drive, sector, buffer);
    } else {
        success = ll_int03_write_sector(drive, sector, buffer);
    }

    if (!success) {
        js_error(J, "Could not write disk sector");
    }
}

/**
 * @brief get number of FDDs
 * GetNumberOfFDD():number
 *
 * @param J VM state.
 */
static void f_GetNumberOfFDD(js_State *J) {
    unsigned long ptraddr = 0x0410;  // Base Address: segment is zero
    unsigned int eq_list = _farpeekw(_dos_ds, ptraddr);

    js_pushnumber(J, (0x01 & (eq_list >> 7)) + 1);
}

/**
 * @brief get number of HDDs
 * GetNumberOfHDD():number
 *
 * @param J VM state.
 */
static void f_GetNumberOfHDD(js_State *J) {
    unsigned long ptraddr = 0x0475;  // Base Address: segment is zero
    js_pushnumber(J, _farpeekb(_dos_ds, ptraddr));
}

/**
 * @brief get a list of parallel port addresses from BIOS
 * GetParallelPorts():number[]
 *
 * @param J VM state.
 *
 * @see https://www.lowlevel.eu/wiki/BIOS_Data_Area
 */
static void f_GetParallelPorts(js_State *J) {
    unsigned long ptraddr = 0x0408;  // Base Address: segment is zero
    int idx = 0;

    js_newarray(J);
    for (int j = 0; j < 3; j++) {
        unsigned int port = _farpeekw(_dos_ds, ptraddr);
        ptraddr += 2;
        if (port) {
            js_pushnumber(J, port);
            js_setindex(J, -2, idx);
            idx++;
        }
    }
}

/**
 * @brief get a list of serial port addresses from BIOS
 * GetSerialPorts():number[]
 *
 * @param J VM state.
 *
 * @see https://www.lowlevel.eu/wiki/BIOS_Data_Area
 */
static void f_GetSerialPorts(js_State *J) {
    unsigned long ptraddr = 0x0400;  // Base Address: segment is zero
    int idx = 0;

    js_newarray(J);
    for (int j = 0; j < 3; j++) {
        unsigned int port = _farpeekw(_dos_ds, ptraddr);
        ptraddr += 2;
        if (port) {
            js_pushnumber(J, port);
            js_setindex(J, -2, idx);
            idx++;
        }
    }
}

/**
 * reset lpt port
 * @param J VM state.
 */
static void f__LPTReset(js_State *J) { biosprint(1, 0, js_toint16(J, 1)); }

/**
 * send data to lpt port
 * @param J VM state.
 */
static void f__LPTSend(js_State *J) {
    int port = js_toint16(J, 1);
    const char *data = js_tostring(J, 2);

    while (*data) {
        biosprint(0, *data++, port);
    }
}

/**
 * get status of lpt port
 * @param J VM state.
 */
static void f__LPTStatus(js_State *J) { js_pushnumber(J, biosprint(2, 0, js_toint16(J, 1))); }

static void f_OutPortByte(js_State *J) { outportb(js_toint16(J, 1), js_toint16(J, 2) & 0xFF); }
static void f_OutPortWord(js_State *J) { outportw(js_toint16(J, 1), js_toint16(J, 2)); }
static void f_OutPortLong(js_State *J) { outportl(js_toint16(J, 1), js_toint32(J, 2)); }

static void f_InPortByte(js_State *J) { js_pushnumber(J, inportb(js_toint16(J, 1)) & 0xFF); }
static void f_InPortWord(js_State *J) { js_pushnumber(J, inportw(js_toint16(J, 1)) & 0xFFFF); }
static void f_InPortLong(js_State *J) { js_pushnumber(J, inportl(js_toint16(J, 1))); }

/***********************
** exported functions **
***********************/
/**
 * @brief initialize grx subsystem.
 *
 * @param J VM state.
 */
void init_lowlevel(js_State *J) {
    DEBUGF("%s\n", __PRETTY_FUNCTION__);

    // define global functions
    NFUNCDEF(J, OutPortByte, 2);
    NFUNCDEF(J, OutPortWord, 2);
    NFUNCDEF(J, OutPortLong, 2);
    NFUNCDEF(J, InPortByte, 1);
    NFUNCDEF(J, InPortWord, 1);
    NFUNCDEF(J, InPortLong, 1);
    NFUNCDEF(J, GetParallelPorts, 0);
    NFUNCDEF(J, GetSerialPorts, 0);
    NFUNCDEF(J, _LPTReset, 1);
    NFUNCDEF(J, _LPTSend, 2);
    NFUNCDEF(J, _LPTStatus, 1);

    NFUNCDEF(J, GetNumberOfFDD, 0);
    NFUNCDEF(J, GetNumberOfHDD, 0);
    NFUNCDEF(J, GetDiskStatus, 1);
    NFUNCDEF(J, GetRawSectorSize, 1);
    NFUNCDEF(J, RawRead, 2);
    NFUNCDEF(J, RawWrite, 3);

    DEBUGF("%s DONE\n", __PRETTY_FUNCTION__);
}
