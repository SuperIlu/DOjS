// cRSID - a lightweight (integer-only) RealSID playback environment by Hermit

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "libcRSID.h"

#ifndef CRSID_PLATFORM_PC
#define CRSID_PLATFORM_PC
#endif

#define DEFAULT_SUBTUNE 1  // 1..
#define DEFAULT_SAMPLERATE 44100
#define DEFAULT_BUFFERSIZE 16384

int main(int argc, char *argv[]) {
    char *SIDfileName;
    char SubTune = DEFAULT_SUBTUNE, CIAisSet;
    short PrevFrameCycles;
    cRSID_SIDheader *SIDheader;
    cRSID_C64instance *C64;

    C64 = cRSID_init(DEFAULT_SAMPLERATE, DEFAULT_BUFFERSIZE);
    if (C64 == NULL) exit(CRSID_ERROR_INIT);

    if (argc < 2) {
        printf("Usage: crsid <Filename.sid> [subtune]\n");
        return 0;
    }
    SIDfileName = argv[1];

    if (argc == 2)
        cRSID_playSIDfile(C64, SIDfileName, SubTune);
    else if (argc == 3) {
        sscanf(argv[2], "%d", &SubTune);
        if ((SIDheader = cRSID_loadSIDtune(C64, SIDfileName)) == NULL) {
            printf("Load error!\n");
            return CRSID_ERROR_LOAD;
        }
        cRSID_initSIDtune(C64, SIDheader, SubTune);

        printf("Author: %s , Title: %s , Info: %s\n", SIDheader->Author, SIDheader->Title, SIDheader->ReleaseInfo);

        printf("Load-address:$%4.4X, End-address:$%4.4X, Size:%d bytes\n", C64->LoadAddress, C64->EndAddress, C64->EndAddress - C64->LoadAddress);
        printf("Init-address:$%4.4X, ", C64->InitAddress);
        if (!C64->RealSIDmode) {
            printf("Play-address:$%4.4X, ", C64->PlayAddress);
            if (SIDheader->PlayAddressH == 0 && SIDheader->PlayAddressL == 0) printf("(IRQ), ");
        }
        printf("Subtune:%d (of %d)", C64->SubTune, SIDheader->SubtuneAmount);
        if (C64->RealSIDmode)
            printf(", RealSID");
        else if (C64->PSIDdigiMode)
            printf(", PSID-digi");
        printf("\n");

        printf("SID1:$%4.4X,%d ", C64->SID[1].BaseAddress, C64->SID[1].ChipModel);
        if (C64->SID[2].BaseAddress) printf("SID2:$%4.4X,%d ", C64->SID[2].BaseAddress, C64->SID[2].ChipModel);
        if (C64->SID[3].BaseAddress) printf("SID3:$%4.4X,%d ", C64->SID[3].BaseAddress, C64->SID[3].ChipModel);
        printf("\n");

        PrevFrameCycles = C64->FrameCycles;
        if (!C64->RealSIDmode) {
            printf("Speed: %.1fx (player-call at every %d cycle) TimerSource:%s ", (C64->VideoStandard <= 1 ? 19656.0 : 17095.0) / C64->FrameCycles, C64->FrameCycles,
                   C64->TimerSource ? "CIA" : "VIC");
        }
        printf("Standard:%s\n", C64->VideoStandard ? "PAL" : "NTSC");

        cRSID_playSIDtune();

        usleep(100000);
        if (C64->FrameCycles != PrevFrameCycles) {
            if (!CIAisSet) {
                CIAisSet = 1;
                printf("New FrameSpeed: %.1fx (%d cycles between playercalls)\n", (C64->VideoStandard <= 1 ? 19656.0 : 17095.0) / C64->FrameCycles, C64->FrameCycles);
            }
        }
    }

    printf("Press Enter to abort playback...\n");
    getchar();

    cRSID_close();
    return 0;
}
