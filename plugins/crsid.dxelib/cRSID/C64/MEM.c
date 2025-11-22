// static done

// Emulation of C64 memories and memory bus (PLA & MUXes)

static inline unsigned char* cRSID_getMemReadPtr(cRSID_C64instance* C64, register unsigned short address) {
    // cRSID_C64instance* const C64 = &cRSID_C64; //for faster (?) operation we use a global object as memory
    if (address < 0xA000)
        return &C64->RAMbank[address];
    else if (0xD000 <= address && address < 0xE000 && (C64->RAMbank[1] & 3)) {
        if (0xD400 <= address && address < 0xD419) return &C64->IObankWR[address];  // emulate bitfading aka SID-read of last written reg (e.g. Lift Off ROR $D400,x)
        return &C64->IObankRD[address];
    } else if ((address < 0xC000 && (C64->RAMbank[1] & 3) == 3) || (0xE000 <= address && (C64->RAMbank[1] & 2)))
        return &C64->ROMbanks[address];
    return &C64->RAMbank[address];
}

static inline unsigned char* cRSID_getMemReadPtrC64(cRSID_C64instance* C64, register unsigned short address) {
    if (address < 0xA000)
        return &C64->RAMbank[address];
    else if (0xD000 <= address && address < 0xE000 && (C64->RAMbank[1] & 3)) {
        if (0xD400 <= address && address < 0xD419) return &C64->IObankWR[address];  // emulate peculiar SID-read (e.g. Lift Off)
        return &C64->IObankRD[address];
    } else if ((address < 0xC000 && (C64->RAMbank[1] & 3) == 3) || (0xE000 <= address && (C64->RAMbank[1] & 2)))
        return &C64->ROMbanks[address];
    return &C64->RAMbank[address];
}

static inline unsigned char* cRSID_getMemWritePtr(cRSID_C64instance* C64, register unsigned short address) {
    // cRSID_C64instance* const C64 = &cRSID_C64; //for faster (?) operation we use a global object as memory
    if (address < 0xD000 || 0xE000 <= address)
        return &C64->RAMbank[address];
    else if (C64->RAMbank[1] & 3) {                   // handle SID-mirrors! (CJ in the USA workaround (writing above $d420, except SID2/SID3))
        if (0xD420 <= address && address < 0xD800) {  // CIA/VIC mirrors needed?
            if (!(C64->PSIDdigiMode && 0xD418 <= address && address < 0xD500) && !(C64->SID[2].BaseAddress <= address && address < C64->SID[2].BaseAddress + 0x20) &&
                !(C64->SID[3].BaseAddress <= address && address < C64->SID[3].BaseAddress + 0x20)) {
                return &C64->IObankWR[0xD400 + (address & 0x1F)];  // write to $D400..D41F if not in SID2/SID3 address-space
            } else
                return &C64->IObankWR[address];
        } else
            return &C64->IObankWR[address];
    }
    return &C64->RAMbank[address];
}

static inline unsigned char* cRSID_getMemWritePtrC64(cRSID_C64instance* C64, register unsigned short address) {
    if (address < 0xD000 || 0xE000 <= address)
        return &C64->RAMbank[address];
    else if (C64->RAMbank[1] & 3) {                   // handle SID-mirrors! (CJ in the USA workaround (writing above $d420, except SID2/SID3/PSIDdigi))
        if (0xD420 <= address && address < 0xD800) {  // CIA/VIC mirrors needed?
            if (!(C64->PSIDdigiMode && 0xD418 <= address && address < 0xD500) && !(C64->SID[2].BaseAddress <= address && address < C64->SID[2].BaseAddress + 0x20) &&
                !(C64->SID[3].BaseAddress <= address && address < C64->SID[3].BaseAddress + 0x20)) {
                return &C64->IObankWR[0xD400 + (address & 0x1F)];  // write to $D400..D41F if not in SID2/SID3 address-space
            } else
                return &C64->IObankWR[address];
        } else
            return &C64->IObankWR[address];
    }
    return &C64->RAMbank[address];
}

static inline unsigned char cRSID_readMem(cRSID_C64instance* C64, register unsigned short address) { return *cRSID_getMemReadPtr(C64, address); }

static inline unsigned char cRSID_readMemC64(cRSID_C64instance* C64, register unsigned short address) { return *cRSID_getMemReadPtrC64(C64, address); }

static inline void cRSID_writeMem(cRSID_C64instance* C64, register unsigned short address, register unsigned char data) { *cRSID_getMemWritePtr(C64, address) = data; }

static inline void cRSID_writeMemC64(cRSID_C64instance* C64, register unsigned short address, register unsigned char data) { *cRSID_getMemWritePtrC64(C64, address) = data; }

void cRSID_setROMcontent(cRSID_C64instance* C64) {  // fill KERNAL/BASIC-ROM areas with content needed for SID-playback
    int i;
    const unsigned char ROM_IRQreturnCode[9] = {0xAD, 0x0D, 0xDC, 0x68, 0xA8, 0x68, 0xAA, 0x68, 0x40};  // CIA1-acknowledge IRQ-return
    const unsigned char ROM_NMIstartCode[5] = {0x78, 0x6c, 0x18, 0x03, 0x40};                           // SEI and jmp($0318)
    const unsigned char ROM_IRQBRKstartCode[19] = {                                                     // Full IRQ-return (handling BRK with the same RAM vector as IRQ)
                                                          0x48, 0x8A, 0x48, 0x98, 0x48, 0xBA, 0xBD, 0x04, 0x01, 0x29, 0x10, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0x6C, 0x14, 0x03};

    for (i = 0xA000; i < 0x10000; ++i) C64->ROMbanks[i] = 0x60;  // RTS (at least return if some unsupported call is made to ROM)
    for (i = 0xEA31; i < 0xEA7E; ++i) C64->ROMbanks[i] = 0xEA;   // NOP (full IRQ-return leading to simple IRQ-return without other tasks)
    for (i = 0; i < 9; ++i) C64->ROMbanks[0xEA7E + i] = ROM_IRQreturnCode[i];
    for (i = 0; i < 4; ++i) C64->ROMbanks[0xFE43 + i] = ROM_NMIstartCode[i];
    for (i = 0; i < 19; ++i) C64->ROMbanks[0xFF48 + i] = ROM_IRQBRKstartCode[i];

    C64->ROMbanks[0xFFFB] = 0xFE;
    C64->ROMbanks[0xFFFA] = 0x43;  // ROM NMI-vector
    C64->ROMbanks[0xFFFF] = 0xFF;
    C64->ROMbanks[0xFFFE] = 0x48;  // ROM IRQ-vector

    // copy KERNAL & BASIC ROM contents into the RAM under them? (So PSIDs that don't select bank correctly will work better.)
    for (i = 0xA000; i < 0x10000; ++i) C64->RAMbank[i] = C64->ROMbanks[i];
}

void cRSID_initMem(cRSID_C64instance* C64) {  // set default values that normally KERNEL ensures after startup/reset (only SID-playback related)
    // data required by both PSID and RSID (according to HVSC SID_file_format.txt):
    cRSID_writeMemC64(C64, 0x02A6, C64->VideoStandard);  //$02A6 should be pre-set to: 0:NTSC / 1:PAL
    cRSID_writeMemC64(C64, 0x0001, 0x37);                // initialize bank-reg. (ROM-banks and IO enabled)

    // if (C64->ROMbanks[0xE000]==0) { //wasn't a KERNAL-ROM loaded? (e.g. PSID)
    cRSID_writeMemC64(C64, 0x00CB, 0x40);  // Some tunes might check for keypress here (e.g. Master Blaster Intro)
    // if(C64->RealSIDmode) {
    cRSID_writeMemC64(C64, 0x0315, 0xEA);
    cRSID_writeMemC64(C64, 0x0314, 0x31);  // IRQ
    cRSID_writeMemC64(C64, 0x0319, 0xEA /*0xFE*/);
    cRSID_writeMemC64(C64, 0x0318, 0x81 /*0x47*/);  // NMI
    //}

    for (int i = 0xD000; i < 0xD7FF; ++i) C64->IObankRD[i] = C64->IObankWR[i] = 0;  // initialize the whole IO area for a known base-state
    if (C64->RealSIDmode) {
        C64->IObankWR[0xD012] = 0x37;
        C64->IObankWR[0xD011] = 0x8B;
    }  // else C64->IObankWR[0xD012] = 0;
    // C64->IObankWR[0xD019] = 0; //PSID: rasterrow: any value <= $FF, IRQ:enable later if there is VIC-timingsource

    C64->IObankRD[0xDC00] = 0x10;
    C64->IObankRD[0xDC01] = 0xFF;  // Imitate CIA1 keyboard/joy port, some tunes check if buttons are not pressed
    if (C64->VideoStandard) {
        C64->IObankWR[0xDC04] = 0x24;
        C64->IObankWR[0xDC05] = 0x40;
    }  // initialize CIAs
    else {
        C64->IObankWR[0xDC04] = 0x95;
        C64->IObankWR[0xDC05] = 0x42;
    }
    if (C64->RealSIDmode) C64->IObankWR[0xDC0D] = 0x81;    // Reset-default, but for PSID CIA1 TimerA IRQ should be enabled anyway if SID is CIA-timed
    C64->IObankWR[0xDC0E] = 0x01;                          // some tunes (and PSID doc) expect already running CIA (Reset-default)
    C64->IObankWR[0xDC0F] = 0x00;                          // All counters other than CIA1 TimerA should be disabled and set to 0xFF for PSID:
    C64->IObankWR[0xDD04] = C64->IObankWR[0xDD05] = 0xFF;  // C64->IObankWR[0xDD0E] = C64->IObank[0xDD0F] = 0x00;
    //}
}
