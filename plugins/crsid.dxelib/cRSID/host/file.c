// static DONE

#include "libcRSID.h"

#include <stdio.h>
#include <stddef.h>

cRSID_SIDheader* cRSID_processSIDfile(cRSID_C64instance* C64, unsigned char* filedata, int filesize) {
    int i;
    unsigned short SIDdataOffset;
    cRSID_SIDheader* SIDheader;
    const char MagicStringPSID[] = "PSID";
    // const char MagicStringRSID[]="RSID";

    C64->SIDheader = SIDheader = (cRSID_SIDheader*)filedata;

    for (i = 0x0000; i < 0xA000; ++i) C64->RAMbank[i] = 0;  // fresh start (maybe some bugged SIDs want 0 at certain RAM-locations)
    for (i = 0xC000; i < 0xD000; ++i) C64->RAMbank[i] = 0;

    if (SIDheader->MagicString[0] != 'P' && SIDheader->MagicString[0] != 'R') return NULL;
    for (i = 1; i < (int)(sizeof(MagicStringPSID) - 1); ++i) {
        if (SIDheader->MagicString[i] != MagicStringPSID[i]) return NULL;
    }
    C64->RealSIDmode = (SIDheader->MagicString[0] == 'R');

    if (SIDheader->LoadAddressH == 0 && SIDheader->LoadAddressH == 0) {  // load-address taken from first 2 bytes of the C64 PRG
        C64->LoadAddress = (filedata[SIDheader->HeaderSize + 1] << 8) + (filedata[SIDheader->HeaderSize + 0]);
        SIDdataOffset = SIDheader->HeaderSize + 2;
    } else {  // load-adress taken from SID-header
        C64->LoadAddress = (SIDheader->LoadAddressH << 8) + (SIDheader->LoadAddressL);
        SIDdataOffset = SIDheader->HeaderSize;
    }

    for (i = SIDdataOffset; i < filesize; ++i) C64->RAMbank[C64->LoadAddress + (i - SIDdataOffset)] = filedata[i];

    i = C64->LoadAddress + (filesize - SIDdataOffset);
    C64->EndAddress = (i < 0x10000) ? i : 0xFFFF;

    C64->PSIDdigiMode = (!C64->RealSIDmode && (SIDheader->ModelFormatStandard & 2));

    return C64->SIDheader;
}
