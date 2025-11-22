// static done

#include "libcRSID.h"

void cRSID_generateSound(cRSID_C64instance* C64instance, unsigned char* buf, unsigned short len) {
    for (unsigned short i = 0; i < len; i += 2) {
        int Output = cRSID_generateSample(C64instance);  // cRSID_emulateC64(C64instance);
        // if (Output>=32767) Output=32767; else if (Output<=-32768) Output=-32768; //saturation logic on overflow
        buf[i] = Output & 0xFF;
        buf[i + 1] = Output >> 8;
    }
}

signed short cRSID_generateSample(cRSID_C64instance* C64) {  // call this from custom buffer-filler
    int Output = cRSID_emulateC64(C64);
    if (C64->PSIDdigiMode) Output += cRSID_playPSIDdigi(C64);
    if (Output >= 32767)
        Output = 32767;
    else if (Output <= -32768)
        Output = -32768;  // saturation logic on overflow
    return (signed short)Output;
}
