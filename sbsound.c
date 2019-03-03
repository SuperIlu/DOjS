/*
** Licensed under Attribution 4.0 International (CC BY 4.0)
** https://creativecommons.org/licenses/by/4.0/
**
** Code was taken from http://www.shdon.com/dos/sound
** by Steven Don.
** This is a derived/modified version by Andre Seidelt <superilu@yahoo.com>
*/
/****************************************************************************
** Demonstration of playing a single wave file through DMA using DJGPP     **
**  by Steven H Don                                                        **
**                                                                         **
** For questions, feel free to e-mail me.                                  **
**                                                                         **
**    shd@earthling.net                                                    **
**    http://shd.cjb.net                                                   **
**                                                                         **
****************************************************************************/
#include "sbsound.h"
#include <mujs.h>
#include "DOjS.h"
#include "sbdet.h"

#ifndef PLATFORM_UNIX

// Include files
#include <conio.h>
#include <dos.h>
#include <malloc.h>
#include <math.h>
#include <mem.h>
#include <stdint.h>
#include <stdio.h>

#include <dpmi.h>
#include <go32.h>
#include <pc.h>
#include <stdlib.h>
#include <string.h>

static void snd_SetVect(int Vector);
static void snd_ResetVect(int Vector);
static void snd_AssignBuffer();

volatile unsigned int snd_CBuffer;  // Clear Buffer indicator
unsigned char *snd_DMABuffer;       // Pointer to protected mode DMA Buffer
unsigned int snd_DMA;               // The DMA channel
unsigned int snd_IRQ;               // The IRQ level
unsigned int snd_Base;              // Sound Blaster base address, Word

struct WaveData {
    uint16_t format;
    uint16_t channels;
    uint32_t rate;
    uint16_t num_bits;
    unsigned int SoundLength;
    unsigned char *Sample;
};

// Pointers to old and new interrupt routines
_go32_dpmi_seginfo snd_OldIRQ, snd_MyIRQ;

// Pointer to DOS DMA buffer
_go32_dpmi_seginfo snd_DOSBuf;
int snd_DOSBufOfs;

/****************************************************************************
** Checks to see if a Sound Blaster exists at a given address, returns     **
** true if Sound Blaster found, false if not.                              **
****************************************************************************/
static bool snd_ResetDSP(unsigned int a, unsigned int i, unsigned int d) {
    // Reset the DSP
    outportb(a + 0x6, 1);
    delay(10);
    outportb(a + 0x6, 0);
    delay(10);
    // Check if (reset was succesfull
    if (((inportb(a + 0xE) & 0x80) == 0x80) && (inportb(a + 0xA) == 0xAA)) {
        // DSP was found
        snd_Base = a;
        snd_IRQ = i;
        snd_DMA = d;

        snd_AssignBuffer();

        // Save old/set new IRQ vector
        if (snd_IRQ == 2 || snd_IRQ == 10 || snd_IRQ == 11) {
            if (snd_IRQ == 2) snd_SetVect(0x71);
            if (snd_IRQ == 10) snd_SetVect(0x72);
            if (snd_IRQ == 11) snd_SetVect(0x73);
        } else {
            snd_SetVect(8 + snd_IRQ);
        }
        // Enable IRQ
        if (snd_IRQ == 2 || snd_IRQ == 10 || snd_IRQ == 11) {
            if (snd_IRQ == 2) outportb(0xA1, inportb(0xA1) & 253);
            if (snd_IRQ == 10) outportb(0xA1, inportb(0xA1) & 251);
            if (snd_IRQ == 11) outportb(0xA1, inportb(0xA1) & 247);
            outportb(0x21, inportb(0x21) & 251);
        } else {
            outportb(0x21, inportb(0x21) & ~(1 << snd_IRQ));
        }
        // Set clear buffer to last buffer
        snd_CBuffer = 3;

        return true;
    } else {
        // No DSP was found
        return false;
    }
}

/****************************************************************************
** Send a byte to the DSP (Digital Signal Processor) on the Sound Blaster  **
****************************************************************************/
static void snd_WriteDSP(unsigned char Value) {
    // Wait for the DSP to be ready to accept data
    while ((inportb(snd_Base + 0xC) & 0x80) == 0x80)
        ;
    // Send byte
    outportb(snd_Base + 0xC, Value);
}

/****************************************************************************
** Starts playback of the buffer. The DMA controller is programmed with    **
** a block length of 32K - the entire buffer. The DSP is instructed to     **
** play blocks of 8K and then generate an interrupt (which allows the      **
** program to clear the parts that have already been played)               **
****************************************************************************/
static void snd_StartPlayBack() {
    unsigned int Page, OffSet;

    snd_WriteDSP(0xD1);  // DSP-command D1h - Enable speaker, required
                         // on older SB's
    snd_WriteDSP(0x40);  // DSP-command 40h - Set sample frequency
    snd_WriteDSP(165);   // Write time constant

    // Convert pointer to linear address
    Page = snd_DOSBufOfs >> 16;       // Calculate page
    OffSet = snd_DOSBufOfs & 0xFFFF;  // Calculate offset in the page
    outportb(0x0A, 4 | snd_DMA);      // Mask DMA channel
    outportb(0x0C, 0);                // Clear byte pointer
    outportb(0x0B, 0x58 | snd_DMA);   // Set mode
    /*
        The mode consists of the following:
        0x58 + x = binary 01 00 10 xx
                          |  |  |  |
                          |  |  |  +- DMA channel
                          |  |  +---- Read operation (the DSP reads from memory)
                          |  +------- Single cycle mode
                          +---------- Block mode
    */

    outportb(snd_DMA << 1,
             OffSet & 0xFF);  // Write the offset to the DMA controller
    outportb(snd_DMA << 1, OffSet >> 8);

    if (snd_DMA == 0) outportb(0x87, Page);
    if (snd_DMA == 1) outportb(0x83, Page);
    if (snd_DMA == 3) outportb(0x82, Page);

    outportb((snd_DMA << 1) + 1,
             0xFF);  // Set the block length to 0x7FFF = 32 Kbyte
    outportb((snd_DMA << 1) + 1, 0x7F);

    outportb(0x0A, snd_DMA);  // Unmask DMA channel

    snd_WriteDSP(0x48);  // DSP-command 48h - Set block length
    snd_WriteDSP(0xFF);  // Set the block length to 0x1FFF = 8 Kbyte
    snd_WriteDSP(0x1F);
    snd_WriteDSP(0x1C);  // DSP-command 1Ch - Start auto-init playback
}

/****************************************************************************
** Clears an 8K part of the DMA buffer                                     **
****************************************************************************/
static void snd_ClearBuffer(unsigned int Buffer) {
    unsigned char *Address;
    // Fill an 8K block in the DMA buffer with 128's - silence
    Address = snd_DMABuffer + (Buffer << 13);
    memset(Address, 128, 8192);
    // Copy DMA buffer to DOS memory
    dosmemput(Address, 8192, snd_DOSBufOfs + (Buffer << 13));
}

/****************************************************************************
** Mixes a sample with the contents of the DMA buffer                      **
****************************************************************************/
static void snd_MixVoice(struct WaveData *Voice) {
    unsigned int Counter, beforeOffSet, OffSet, DMAPointer;

    // Read DMA pointer from DMA controller
    DMAPointer = inportb(1 + (snd_DMA << 1));
    DMAPointer = DMAPointer + (inportb(1 + (snd_DMA << 1)) << 8);

    /*
      DMAPointer contains amount that remains to be played.
      This is convert to the offset of the current sample
    */

    DMAPointer = 0x7FFF - DMAPointer;

    beforeOffSet = OffSet = DMAPointer;

    for (Counter = 0; Counter <= Voice->SoundLength; Counter++) {
        // Mix byte
        snd_DMABuffer[OffSet++] += Voice->Sample[Counter];
        OffSet &= 0x7FFF;  // Move on to next byte and keep it in the 32K range
    }

    // Copy as one or in parts
    if (OffSet > beforeOffSet)
        dosmemput(snd_DMABuffer + beforeOffSet, OffSet - beforeOffSet, snd_DOSBufOfs + beforeOffSet);
    else {
        dosmemput(snd_DMABuffer + beforeOffSet, 32768 - beforeOffSet, snd_DOSBufOfs + beforeOffSet);
        dosmemput(snd_DMABuffer, OffSet, snd_DOSBufOfs);
    }
}

/****************************************************************************
** Loads a wave file into memory. This procedure treats any file as a      **
** standard 11025Hz, 8bit, mono .WAV file. It doesn't perform any error    **
** checking.                                                               **
****************************************************************************/
static bool snd_LoadVoice(struct WaveData *Voice, const char *FileName) {
    FILE *WAVFile;
    char buffer[16];

    // If it can't be opened...
    WAVFile = fopen(FileName, "rb");
    if (WAVFile == NULL) {
        //..display error message
        LOGF("Unable to open wave file %s\n", FileName);
        return false;
    }

    // check file header for "RIFF    WAVEfmt "
    fread(buffer, sizeof(buffer), 1, WAVFile);
    if (buffer[0] != 'R' || buffer[1] != 'I' || buffer[2] != 'F' || buffer[3] != 'F' || buffer[8] != 'W' || buffer[9] != 'A' || buffer[10] != 'V' || buffer[11] != 'E' ||
        buffer[12] != 'f' || buffer[13] != 'm' || buffer[14] != 't' || buffer[15] != ' ') {
        LOGF("No wave file %s\n", FileName);
        fclose(WAVFile);
        return false;
    }

    // read header into buffer, description from
    // http://soundfile.sapp.org/doc/WaveFormat/
    fseek(WAVFile, 20L, SEEK_SET);
    fread(buffer, sizeof(buffer), 1, WAVFile);

    Voice->format = *((uint16_t *)&buffer[0]);
    Voice->channels = *((uint16_t *)&buffer[2]);
    Voice->rate = *((uint32_t *)&buffer[4]);
    Voice->num_bits = *((uint16_t *)&buffer[14]);

    LOGF("WAV %s is format=%d, channels=%d, rate=%ld and %d bits\n", FileName, Voice->format, Voice->channels, Voice->rate, Voice->num_bits);

    if (Voice->format != 1 || Voice->channels != 1 || Voice->num_bits != 8) {
        LOGF("WAV %s must be PCM, 8bit, mono!\n", FileName);
        fclose(WAVFile);
        return false;
    }
    if (Voice->rate != 11025) {
        LOGF("WARNING: WAV %s samplerate != 11025!\n", FileName);
    }

    // Return length of file for sound length minus 48 bytes for .WAV header
    fseek(WAVFile, 0L, SEEK_END);
    Voice->SoundLength = ftell(WAVFile) - 48;
    fseek(WAVFile, 0L, SEEK_SET);

    Voice->Sample = (unsigned char *)malloc(Voice->SoundLength + 2);  // Assign memory
    if (!Voice->Sample) {
        //..display error message
        LOG("Can't malloc() sample\n");
        fclose(WAVFile);
        return false;
    }

    fseek(WAVFile, 46L, SEEK_SET);  // Skip the header

    // Load the sample data
    fread(Voice->Sample, Voice->SoundLength + 2, 1, WAVFile);

    fclose(WAVFile);  // Close the file

    return true;
}

static void snd_UnloadVoice(struct WaveData *Voice) { free(Voice->Sample); }

/****************************************************************************
** Converts a wave file so it can be mixed easily                          **
****************************************************************************/
static void snd_ConvertVoice(struct WaveData *Voice) {
    unsigned int OffSet;

    // for each sample, decrease sample value to avoid overflow

    for (OffSet = 0; OffSet <= Voice->SoundLength; OffSet++) {
        Voice->Sample[OffSet] >>= 2;
        Voice->Sample[OffSet] -= 32;
    }
}

/****************************************************************************
** IRQ service routine - this is called when the DSP has finished playing  **
** a block                                                                 **
****************************************************************************/
void snd_ServiceIRQ() {
    // Relieve DSP
    inportb(snd_Base + 0xE);
    // Acknowledge hardware interrupt
    outportb(0x20, 0x20);
    // Acknowledge cascade interrupt for IRQ 2, 10 and 11
    if (snd_IRQ == 2 || snd_IRQ == 10 || snd_IRQ == 11) outportb(0xA0, 0x20);
    // Increase pointer to clear buffer and keep it in the range 0..3
    snd_CBuffer++;
    snd_CBuffer &= 3;
    // Clear buffer
    snd_ClearBuffer(snd_CBuffer);
}

/****************************************************************************
** This procedure allocates 32K of memory to the DMA buffer and makes sure **
** that no page boundary is crossed                                        **
****************************************************************************/
static void snd_AssignBuffer() {
    _go32_dpmi_seginfo TempBuf;  // Temporary pointer
    unsigned int Page1, Page2;   // Words

    // Assign 32K to DMA Buffer
    snd_DMABuffer = (unsigned char *)malloc(32768);

    // Assign 32K (2048 paragraphs) of DOS memory
    TempBuf.size = 2048;
    _go32_dpmi_allocate_dos_memory(&TempBuf);

    // Calculate linear address
    snd_DOSBufOfs = TempBuf.rm_segment << 4;

    // Calculate page at start of buffer
    Page1 = snd_DOSBufOfs >> 16;

    // Calculate page at end of buffer}
    Page2 = (snd_DOSBufOfs + 32767) >> 16;

    // Check to see if a page boundary is crossed
    if (Page1 != Page2) {
        // If so, assign another part of memory to the buffer
        snd_DOSBuf.size = 2048;
        _go32_dpmi_allocate_dos_memory(&snd_DOSBuf);
        snd_DOSBufOfs = snd_DOSBuf.rm_segment << 4;
        _go32_dpmi_free_dos_memory(&TempBuf);
    } else  // otherwise, use the part we've already allocated
        snd_DOSBuf = TempBuf;

    // Clear DMA buffers
    memset((void *)snd_DMABuffer, 128, 0x7FFF);
    dosmemput((void *)snd_DMABuffer, 32768, snd_DOSBufOfs);
}

/****************************************************************************
** Cleanup Free the DMA buffer again                                       **
****************************************************************************/
static void snd_FreeBuffer() {
    // Stops DMA-transfer
    snd_WriteDSP(0xD0);
    snd_WriteDSP(0xDA);

    // Free the memory allocated to the sound buffer
    free((void *)snd_DMABuffer);

    // Free interrupt vectors used to service IRQs
    if (snd_IRQ == 2 || snd_IRQ == 10 || snd_IRQ == 11) {
        if (snd_IRQ == 2) snd_ResetVect(0x71);
        if (snd_IRQ == 10) snd_ResetVect(0x72);
        if (snd_IRQ == 11) snd_ResetVect(0x73);
    } else {
        snd_ResetVect(8 + snd_IRQ);
    }

    // Mask IRQs
    if (snd_IRQ == 2 || snd_IRQ == 10 || snd_IRQ == 11) {
        if (snd_IRQ == 2) outportb(0xA1, inportb(0xA1) | 2);
        if (snd_IRQ == 10) outportb(0xA1, inportb(0xA1) | 4);
        if (snd_IRQ == 11) outportb(0xA1, inportb(0xA1) | 8);
        outportb(0x21, inportb(0x21) | 4);
    } else {
        outportb(0x21, inportb(0x21) | (1 << snd_IRQ));
    }
}

/****************************************************************************
** set IRQ vector                                                          **
****************************************************************************/
static void snd_SetVect(int Vector) {
    // Get location of the new keyboard handler
    snd_MyIRQ.pm_offset = (int)snd_ServiceIRQ;
    snd_MyIRQ.pm_selector = _go32_my_cs();
    // Save the old interrupt handler
    _go32_dpmi_get_protected_mode_interrupt_vector(Vector, &snd_OldIRQ);
    // Set the new interrupt handler in the chain
    _go32_dpmi_chain_protected_mode_interrupt_vector(Vector, &snd_MyIRQ);
}

/****************************************************************************
** restore IRQ vector                                                      **
****************************************************************************/
static void snd_ResetVect(int Vector) {
    // Set interrupt vector to the BIOS handler
    _go32_dpmi_set_protected_mode_interrupt_vector(Vector, &snd_OldIRQ);
}

/************
** structs **
************/
//! WAV userdata definition
typedef struct __sbsound {
    struct WaveData voice;  //! ready to play sound data
} sbsound_t;

/*********************
** static functions **
*********************/
/**
 * @brief finalize an image and free resources.
 *
 * @param J VM state.
 */
static void SBSound_Finalize(js_State *J, void *data) {
    sbsound_t *snd = (sbsound_t *)data;
    snd_UnloadVoice(&snd->voice);
    free(snd);
}

/**
 * @brief load a WAV and store it as userdata in JS object.
 * new Sound(filename:string)
 *
 * @param J VM state.
 */
static void new_SBSound(js_State *J) {
    const char *fname = js_tostring(J, 1);

    sbsound_t *snd = malloc(sizeof(sbsound_t));
    if (!snd) {
        js_error(J, "No memory for sound '%s'", fname);
        return;
    }

    if (!snd_LoadVoice(&snd->voice, fname)) {
        js_error(J, "Can't load sound '%s'", fname);
        free(snd);
        return;
    }
    snd_ConvertVoice(&snd->voice);

    js_currentfunction(J);
    js_getproperty(J, -1, "prototype");
    js_newuserdata(J, TAG_SBSOUND, snd, SBSound_Finalize);

    // add properties
    js_pushstring(J, fname);
    js_defproperty(J, -2, "filename", JS_READONLY | JS_DONTENUM | JS_DONTCONF);

    js_pushnumber(J, snd->voice.SoundLength);
    js_defproperty(J, -2, "length", JS_READONLY | JS_DONTENUM | JS_DONTCONF);

    js_pushnumber(J, snd->voice.format);
    js_defproperty(J, -2, "format", JS_READONLY | JS_DONTENUM | JS_DONTCONF);

    js_pushnumber(J, snd->voice.channels);
    js_defproperty(J, -2, "channels", JS_READONLY | JS_DONTENUM | JS_DONTCONF);

    js_pushnumber(J, snd->voice.rate);
    js_defproperty(J, -2, "rate", JS_READONLY | JS_DONTENUM | JS_DONTCONF);

    js_pushnumber(J, snd->voice.num_bits);
    js_defproperty(J, -2, "num_bits", JS_READONLY | JS_DONTENUM | JS_DONTCONF);
}

/**
 * @brief play back the sound.
 * snd.Play()
 *
 * @param J VM state.
 */
static void SBSound_play(js_State *J) {
    if (sound_available) {
        sbsound_t *snd = js_touserdata(J, 0, TAG_SBSOUND);
        snd_MixVoice(&snd->voice);
    }
}

/***********************
** exported functions **
***********************/
/**
 * @brief initialize fm music subsystem.
 *
 * @param J VM state.
 */
bool init_sbsound(js_State *J, char *sb_set) {
    sblaster_t blaster;

    js_newobject(J);
    { PROTDEF(J, SBSound_play, TAG_SBSOUND, "Play", 0); }
    js_newcconstructor(J, new_SBSound, new_SBSound, TAG_SBSOUND, 1);
    js_defglobal(J, TAG_SBSOUND, JS_DONTENUM);

    bool ret;
    if (sb_set) {
        if (sscanf(sb_set, "%X:%u:%u", &blaster.port, &blaster.irq, &blaster.dma) == 3) {
            ret = true;
            if (blaster.port < 0x220 || blaster.port > 0x280) {
                LOGF("SBlaster: Port out of range: 0x%03X\n", blaster.port);
                ret = false;
            }
            if (blaster.irq < 2 || blaster.irq > 11) {
                LOGF("SBlaster: IRQ out of range: %d\n", blaster.irq);
                ret = false;
            }
            if (blaster.dma < 0 || blaster.dma > 7) {
                LOGF("SBlaster: DMA out of range: %d\n", blaster.dma);
                ret = false;
            }
        } else {
            LOGF("SBlaster: autodetection override failed: Wrong format in '%s'", sb_set);
            ret = false;
        }
        if (ret) {
            LOGF("SBlaster: autodetection override with port 0x%3X, irq %d, dma %d\n", blaster.port, blaster.irq, blaster.dma);
        }
    } else {
        // try to detect
        ret = detect_sb(&blaster);
    }

    if (ret) {
        bool ret = snd_ResetDSP(blaster.port, blaster.irq, blaster.dma);

        snd_StartPlayBack();

        return ret;
    }
    return false;
}

/**
 * @brief shutdown WAV subsystem.
 */
void shutdown_sbsound() {
    if (sound_available) {
        snd_FreeBuffer();
    }
}
#else

bool init_sbsound(js_State *J) { return false; }
void shutdown_sbsound() {}

#endif  // PLATFORM_UNIX
