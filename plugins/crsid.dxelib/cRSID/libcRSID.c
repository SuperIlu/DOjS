// static done

// cRSID lightweight (integer-only) RealSID library (with API-calls) by Hermit (Mihaly Horvath), Year 2022
// License: WTF - do what the fuck you want with the code, but please mention me as the original author

#include <stdlib.h>
// #include <stdint.h>
#ifdef CRSID_PLATFORM_PC
#include <stdio.h>
#endif

#include "libcRSID.h"

static inline unsigned char* cRSID_getMemReadPtr(cRSID_C64instance* C64, register unsigned short address);                   // for global cSID_C64 fast-access
static inline unsigned char* cRSID_getMemReadPtrC64(cRSID_C64instance* C64, register unsigned short address);                // maybe slower
static inline unsigned char* cRSID_getMemWritePtr(cRSID_C64instance* C64, register unsigned short address);                  // for global cSID_C64 fast-access
static inline unsigned char* cRSID_getMemWritePtrC64(cRSID_C64instance* C64, register unsigned short address);               // maybe slower
static inline unsigned char cRSID_readMem(cRSID_C64instance* C64, register unsigned short address);                          // for global cSID_C64 fast-access
static inline unsigned char cRSID_readMemC64(cRSID_C64instance* C64, register unsigned short address);                       // maybe slower
static inline void cRSID_writeMem(cRSID_C64instance* C64, register unsigned short address, register unsigned char data);     // for global cSID_C64 fast-access
static inline void cRSID_writeMemC64(cRSID_C64instance* C64, register unsigned short address, register unsigned char data);  // maybe slower

#include "C64/C64.c"
#include "host/file.c"
#include "host/audio.c"

cRSID_C64instance* cRSID_init(unsigned short samplerate, unsigned short buflen) {
    cRSID_C64instance* C64 = calloc(1, sizeof(cRSID_C64instance));
    if (!C64) {
        return NULL;
    }

    C64 = cRSID_createC64(C64, samplerate);
    return C64;
}

void cRSID_free(cRSID_C64instance* C64) { free(C64); }

void cRSID_initSIDtune(cRSID_C64instance* C64, cRSID_SIDheader* SIDheader, char subtune) {  // subtune: 1..255
    const unsigned char PowersOf2[] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80};
    unsigned int InitTimeout = 10000000;  // allowed instructions, value should be selected to allow for long-running memory-copiers in init-routines (e.g. Synth Sample)

    if (subtune == 0)
        subtune = 1;
    else if (subtune > SIDheader->SubtuneAmount)
        subtune = SIDheader->SubtuneAmount;
    C64->SubTune = subtune;

    cRSID_setC64(C64);
    cRSID_initC64(C64);  // cRSID_writeMemC64(C64,0xD418,0xF); //set C64 hardware and init (reset) it

    // determine init-address:
    C64->InitAddress = ((SIDheader->InitAddressH) << 8) + (SIDheader->InitAddressL);  // get info from BASIC-startupcode for some tunes
    if (C64->RAMbank[1] == 0x37) {                                                    // are there SIDs with routine under IO area? some PSIDs don't set bank-registers themselves
        if ((0xA000 <= C64->InitAddress && C64->InitAddress < 0xC000) || (C64->LoadAddress < 0xC000 && C64->EndAddress >= 0xA000))
            C64->RAMbank[1] = 0x36;
        else if (C64->InitAddress >= 0xE000 || C64->EndAddress >= 0xE000)
            C64->RAMbank[1] = 0x35;
    }
    cRSID_initCPU(&C64->CPU, C64->InitAddress);  // prepare init-routine call
    C64->CPU.A = subtune - 1;

    if (!C64->RealSIDmode) {
        // call init-routine:
        for (InitTimeout = 10000000; InitTimeout > 0; InitTimeout--) {
            if (cRSID_emulateCPU(C64) >= 0xFE) break;
        }  // give error when timed out?
    }

    // determine timing-source, if CIA, replace FrameCycles previouisly set to VIC-timing
    if (subtune > 32)
        C64->TimerSource = C64->SIDheader->SubtuneTimeSources[0] & 0x80;  // subtunes above 32 should use subtune32's timing
    else
        C64->TimerSource = C64->SIDheader->SubtuneTimeSources[(32 - subtune) >> 3] & PowersOf2[(subtune - 1) & 7];
    if (C64->TimerSource || C64->IObankWR[0xDC05] != 0x40 || C64->IObankWR[0xDC04] != 0x24) {  // CIA1-timing (probably multispeed tune)
        C64->FrameCycles = ((C64->IObankWR[0xDC04] + (C64->IObankWR[0xDC05] << 8)));           //<< 4) / C64->ClockRatio;
        C64->TimerSource = 1;                                                                  // if init-routine changed DC04 or DC05, assume CIA-timing
    }

    // determine playaddress:
    C64->PlayAddress = (SIDheader->PlayAddressH << 8) + SIDheader->PlayAddressL;
    if (C64->PlayAddress) {             // normal play-address called with JSR
        if (C64->RAMbank[1] == 0x37) {  // are there SIDs with routine under IO area?
            if (0xA000 <= C64->PlayAddress && C64->PlayAddress < 0xC000) C64->RAMbank[1] = 0x36;
        } else if (C64->PlayAddress >= 0xE000)
            C64->RAMbank[1] = 0x35;  // player under KERNAL (e.g. Crystal Kingdom Dizzy)
    } else {                         // IRQ-playaddress for multispeed-tunes set by init-routine (some tunes turn off KERNAL ROM but doesn't set IRQ-vector!)
        C64->PlayAddress = (C64->RAMbank[1] & 3) < 2 ? cRSID_readMemC64(C64, 0xFFFE) + (cRSID_readMemC64(C64, 0xFFFF) << 8)  // for PSID
                                                     : cRSID_readMemC64(C64, 0x314) + (cRSID_readMemC64(C64, 0x315) << 8);
        if (C64->PlayAddress == 0) {                     // if 0, still try with RSID-mode fallback
            cRSID_initCPU(&C64->CPU, C64->PlayAddress);  // point CPU to play-routine
            C64->Finished = 1;
            C64->Returned = 1;
            return;
        }
    }

    if (!C64->RealSIDmode) {                         // prepare (PSID) play-routine playback:
        cRSID_initCPU(&C64->CPU, C64->PlayAddress);  // point CPU to play-routine
        C64->FrameCycleCnt = 0;
        C64->Finished = 1;
        C64->SampleCycleCnt = 0;  // C64->CIAisSet=0;
    } else {
        C64->Finished = 0;
        C64->Returned = 0;
    }
}

#ifdef CRSID_PLATFORM_PC

#endif
