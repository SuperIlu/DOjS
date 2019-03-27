/*
** Licensed under Attribution 4.0 International (CC BY 4.0)
** https://creativecommons.org/licenses/by/4.0/
**
** Code was taken from http://www.shdon.com/dos/sound
** by Steven Don.
** This is a derived/modified version by Andre Seidelt <superilu@yahoo.com>
*/
/****************************************************************************
** Demonstration of automatically detecting the Sound Blaster settings     **
**  by Steven H Don                                                        **
**                                                                         **
** For questions, feel free to e-mail me.                                  **
**                                                                         **
**    shd@earthling.net                                                    **
**    http://shd.cjb.net                                                   **
**                                                                         **
*****************************************************************************
** Original download URL: http://www.shdon.com/dos/sound                   **
** Modified for DJGPP in 2019 by superilu@yahoo.com                        **
****************************************************************************/
#include "sbdet.h"
#include "DOjS.h"

#ifndef PLATFORM_UNIX

#include <dos.h>
#include <dpmi.h>
#include <go32.h>
#include <string.h>

unsigned int det_Base;               // Sound Blaster base address
char det_CType[80];                  // Card type
unsigned char det_LoDMA, det_HiDMA;  // DMA Channel and IRQ
volatile unsigned char det_IRQ;

/****************************************************************************
** Checks to see if a Sound Blaster exists at a given address, returns     **
** true if Sound Blaster found, false if not.                              **
****************************************************************************/
static char det_ResetDSP(unsigned int Test) {
    // Reset the DSP
    outportb(Test + 0x6, 1);
    delay(10);
    outportb(Test + 0x6, 0);
    delay(10);
    // Check if reset was succesfull
    if (((inportb(Test + 0xE) & 0x80) == 0x80) && (inportb(Test + 0xA) == 0xAA)) {
        // DSP was found
        det_Base = Test;
        return (1);
    } else
        // No DSP was found
        return (0);
}

/****************************************************************************
** Read a byte from the DSP (Digital Signal Processor) on the Sound Blaster**
****************************************************************************/
static unsigned char det_ReadDSP() {
    while (!(inportb(det_Base + 0xE) & 0x80)) {
        ;
    }
    return (inportb(det_Base + 0xA));
}

/****************************************************************************
** Send a byte to the DSP (Digital Signal Processor) on the Sound Blaster  **
****************************************************************************/
static void det_WriteDSP(unsigned char Value) {
    // Wait for the DSP to be ready to accept data
    while ((inportb(det_Base + 0xC) & 0x80) == 0x80) {
        ;
    }
    // Send byte
    outportb(det_Base + 0xC, Value);
}

/****************************************************************************
** Returns the version number of the DSP.                                  **
****************************************************************************/
static unsigned int det_DSPVersion() {
    unsigned int Version;
    det_WriteDSP(0xE1);
    Version = det_ReadDSP();
    return (det_ReadDSP() + (Version << 8));
}

/****************************************************************************
** This tells the DSP to start playing a sample. However, the DMA          **
** controller has not been set up. Therefore the DSP signals the DMA       **
** controller for data. We check to see where that call takes place and    **
** thus determine the DMA channel the DSP uses                             **
****************************************************************************/
static unsigned char det_TestLoDMA() {
    unsigned char Before, After;
    unsigned int Count;

    // Reset DSP
    det_ResetDSP(det_Base);
    det_WriteDSP(0xD3);  // DSP-command D3h - Speaker Off
    det_WriteDSP(0x40);  // DSP-command 40h - Set sample frequency
    det_WriteDSP(165);   // Write time constant for 11025Hz
    det_WriteDSP(0x14);  // DSP-command 14h - 8bit playback
    det_WriteDSP(0x01);  // Set the block length to 1

    Count = 0;

    // Disable interrupts
    asm("cli");

    // Look at the current state of the DMA requests
    Before = inportb(0x8) & 0xE0;

    // Start playback, * note : this is still part of "set block length"
    det_WriteDSP(0x00);

    do {
        // Check them again until there is a change or it takes too long
        After = inportb(0x8) & 0xE0;
    } while ((Before == After) && (++Count < 60000));

    // Reenable interrupts
    asm("sti");

    // Reset DSP
    det_ResetDSP(det_Base);

    // Is there a change?
    if (Before != After) {
        Count = 0;
        After ^= (Before);
        // If yes, which bit is set?
        while (After > 0) {
            Count++;
            After >>= 1;
        }
        return (Count - 5);
    }
    // No DMA???
    return (0xFF);
}

/****************************************************************************
** This does the same thing for the 16bit DMA channel, if used             **
****************************************************************************/
static unsigned char det_TestHiDMA() {
    unsigned char Before, After;
    unsigned int Count;

    // Reset DSP
    det_ResetDSP(det_Base);
    det_WriteDSP(0xB0);  // DSP-command B0h - 16bit DMA
    det_WriteDSP(0x10);  // Mono
    det_WriteDSP(0x03);  // Set the block length to 3 = 1 DWORD

    Count = 0;

    // Disable interrupts
    asm("cli");

    // Look at the current state of the DMA requests
    Before = inportb(0xD0) & 0xF0;

    // Start playback, * note : this is still part of "set block length"
    det_WriteDSP(0x00);

    do {
        // Check them again until there is a change or it takes too long
        After = inportb(0xD0) & 0xF0;
    } while ((Before == After) && (++Count < 60000));

    // Reenable interrupts
    asm("sti");

    // Reset DSP
    det_ResetDSP(det_Base);

    // Is there a change?
    if (Before != After) {
        Count = 0;
        After ^= (Before);
        // If yes, which bit is set?
        while (After > 0) {
            Count++;
            After >>= 1;
        }
        return (Count - 1);
    }
    // No DMA???
    return (0xFF);
}

/****************************************************************************
** This tells the DSP to start playing a sample. And waits to see which    **
** one of the IRQ service routines is called.                              **
****************************************************************************/
static void det_TestIRQ() {
    unsigned int Count;
    det_IRQ = 0;
    det_WriteDSP(0xD1);                    // DSP-command D1h - Enable speaker, required
                                           // on older SB's
    det_WriteDSP(0x40);                    // DSP-command 40h - Set sample frequency
    det_WriteDSP(165);                     // Write time constant for 11025Hz
    outportb(0x0A, 4 + det_LoDMA);         // Mask DMA channel
    outportb(0x0C, 0);                     // Clear byte pointer
    outportb(0x0B, 0x49);                  // Set mode
    outportb(1 + (det_LoDMA << 1), 0x03);  // Set the block length to 3 == 1 DWORD
    outportb(1 + (det_LoDMA << 1), 0x00);

    outportb(0x0A, det_LoDMA);  // Unmask DMA channel

    det_WriteDSP(0x14);  // DSP-command 14h - Single cycle playback
    det_WriteDSP(0x03);
    det_WriteDSP(0x00);

    Count = 0;
    do
        // Wait until either one of the IRQ service routines has been called
        // or it takes too long
        Count++;
    while ((!det_IRQ) && (Count != 0xFFFF));
}

/****************************************************************************
** IRQ service routine - this is called when the DSP has finished playing  **
** a block.                                                                **
****************************************************************************/
void det_ServiceIRQ2() {
    inportb(det_Base + 0xE);  // Relieve DSP
    outportb(0x20, 0x20);     // Acknowledge hardware interrupt
    outportb(0xA0, 0x20);     // Since IRQ 2 has been redirected, we need to
                              // acknowledge two interrupt controllers
    det_IRQ = 2;              // Save interrupt channel number
}

void det_ServiceIRQ3() {
    inportb(det_Base + 0xE);
    outportb(0x20, 0x20);
    det_IRQ = 3;
}

void det_ServiceIRQ5() {
    inportb(det_Base + 0xE);
    outportb(0x20, 0x20);
    det_IRQ = 5;
}

void det_ServiceIRQ7() {
    inportb(det_Base + 0xE);
    outportb(0x20, 0x20);
    det_IRQ = 7;
}

void det_ServiceIRQ10() {
    inportb(det_Base + 0xE);
    outportb(0x20, 0x20);
    outportb(0xA0, 0x20);
    det_IRQ = 10;
}

void det_ServiceIRQ11() {
    inportb(det_Base + 0xE);
    outportb(0x20, 0x20);
    outportb(0xA0, 0x20);
    det_IRQ = 11;
}

/****************************************************************************
** set IRQ vector                                                          **
****************************************************************************/
static void det_SetVect(int Vector, _go32_dpmi_seginfo *MyIRQ, _go32_dpmi_seginfo *OldIRQ) {
    // Save the old interrupt handler
    _go32_dpmi_get_protected_mode_interrupt_vector(Vector, OldIRQ);
    // Set the new interrupt handler in the chain
    _go32_dpmi_chain_protected_mode_interrupt_vector(Vector, MyIRQ);
}

/****************************************************************************
** restore IRQ vector                                                      **
****************************************************************************/
// TODO: detection of 2, 10 and 11 hangs DOSBox
static void det_ResetVect(int Vector, _go32_dpmi_seginfo *OldIRQ) {
    // Set interrupt vector to the BIOS handler
    _go32_dpmi_set_protected_mode_interrupt_vector(Vector, OldIRQ);
}

/**
 * @brief detect the presence and the parameters of a SoundBlaster compatible
 * soundcard.
 *
 * @param blaster where to store the detected parameters
 *
 * @return true if a card was found
 * @return false if no card was found
 */
bool detect_sb(sblaster_t *blaster) {
    int Temp;
    //_go32_dpmi_seginfo Old71;  // Pointers to old interrupt routines
    _go32_dpmi_seginfo Old11;
    _go32_dpmi_seginfo Old13;
    _go32_dpmi_seginfo Old15;
    //_go32_dpmi_seginfo Old72;
    //_go32_dpmi_seginfo Old73;

    //_go32_dpmi_seginfo New71;  // Pointers to new interrupt routines
    _go32_dpmi_seginfo New11;
    _go32_dpmi_seginfo New13;
    _go32_dpmi_seginfo New15;
    //_go32_dpmi_seginfo New72;
    //_go32_dpmi_seginfo New73;

    // New71.pm_offset = (int)det_ServiceIRQ2;
    // New71.pm_selector = _go32_my_cs();
    New11.pm_offset = (int)det_ServiceIRQ2;
    New11.pm_selector = _go32_my_cs();
    New13.pm_offset = (int)det_ServiceIRQ5;
    New13.pm_selector = _go32_my_cs();
    New15.pm_offset = (int)det_ServiceIRQ7;
    New15.pm_selector = _go32_my_cs();
    // New72.pm_offset = (int)det_ServiceIRQ10;
    // New72.pm_selector = _go32_my_cs();
    // New73.pm_offset = (int)det_ServiceIRQ11;
    // New73.pm_selector = _go32_my_cs();

    // Check for Sound Blaster, address: ports 220, 230, 240, 250, 260 or 280
    for (Temp = 1; Temp < 9; Temp++) {
        if (Temp != 7)
            if (det_ResetDSP(0x200 + (Temp << 4))) {
                break;
            }
    }
    if (Temp == 9) {
        // or none at all
        DEBUG("No Sound Blaster found\n");
        blaster->type = SB_NONE;
        return false;
    }

    // There are several different type of Sound Blaster
    blaster->type = SB_UNKOWN;
    if (det_DSPVersion() == 0x100) {
        strcpy(det_CType, "Sound Blaster");
        blaster->type = SBLASTER;
    }
    if (det_DSPVersion() == 0x105) {
        strcpy(det_CType, "Sound Blaster 1.5");
        blaster->type = SBLASTER_15;
    }
    if (det_DSPVersion() == 0x200) {
        strcpy(det_CType, "Sound Blaster Pro 2");
        blaster->type = SBLASTER_PRO2;
    }
    if (det_DSPVersion() == 0x300) {
        strcpy(det_CType, "Sound Blaster Pro 3");
        blaster->type = SBLASTER_PRO3;
    }
    if ((det_DSPVersion() >> 8) >= 4) {
        strcpy(det_CType, "Sound Blaster 16/ASP/AWE 32/AWE 64");
        blaster->type = SBLASTER_AWE;
    }

    // Check for DMA Channel
    det_LoDMA = det_TestLoDMA();
    if ((det_DSPVersion() >> 8) >= 4) {
        det_HiDMA = det_TestHiDMA();
        if (det_HiDMA == 0xFF) {
            det_HiDMA = det_LoDMA;
        }
    }
    // Set new IRQ vectors, saving old ones
    // det_SetVect(0x71, &New71, &Old71);  // 2
    det_SetVect(11, &New11, &Old11);  // 3
    det_SetVect(13, &New13, &Old13);  // 5
    det_SetVect(15, &New15, &Old15);  // 7
    // det_SetVect(0x72, &New72, &Old72);  // 10
    // det_SetVect(0x73, &New73, &Old73);  // 11

    // Enable IRQs
    // outportb(0xA1, inportb(0xA1) & 253);        // 2
    // outportb(0xA1, inportb(0xA1) & 251);        // 10
    // outportb(0xA1, inportb(0xA1) & 247);        // 11
    // outportb(0x21, inportb(0x21) & 251);        // 2, 10 & 11
    outportb(0x21, inportb(0x21) & ~(1 << 3));  // 3
    outportb(0x21, inportb(0x21) & ~(1 << 5));  // 5
    outportb(0x21, inportb(0x21) & ~(1 << 7));  // 7

    // Scan which IRQ is called
    det_TestIRQ();

    // Free interrupt vectors used to service IRQs
    // det_ResetVect(0x71, &Old71);
    det_ResetVect(11, &Old11);
    det_ResetVect(13, &Old13);
    det_ResetVect(15, &Old15);
    // det_ResetVect(0x72, &Old72);
    // det_ResetVect(0x73, &Old73);

    // Mask IRQs
    /// outportb(0xA1, inportb(0xA1) | 2);  // 2
    // outportb(0xA1, inportb(0xA1) | 4);  // 10
    // outportb(0xA1, inportb(0xA1) | 8);  // 11
    // outportb(0x21, inportb(0x21) | 4);  // 2, 10 & 11

    outportb(0x21, inportb(0x21) | (1 << 3));  // 3
    outportb(0x21, inportb(0x21) | (1 << 5));  // 5
    outportb(0x21, inportb(0x21) | (1 << 7));  // 7

    // Stops DMA-transfer
    det_WriteDSP(0xD0);
    det_WriteDSP(0xDA);

    if (det_IRQ == 0) {
        // or none at all
        DEBUG("No Sound Blaster found %d\n");
        blaster->type = SB_NONE;
        return false;
    }

    // Display results
    LOG("SBlaster: Detection result\n");
    LOGF("Card                : %s\n", det_CType);
    LOGF("Base port           : 0x%03X\n", det_Base);
    blaster->port = det_Base;
    LOGF("IRQ                 : %d\n", det_IRQ);
    blaster->irq = det_IRQ;
    LOGF("DMA channel (8bit)  : %d\n", det_LoDMA);
    blaster->dma = det_LoDMA;
    if ((det_DSPVersion() >> 8) >= 4) {
        LOGF("DMA channel (16bit) : %d\n", det_HiDMA);
        blaster->dma_high = det_HiDMA;
        blaster->bit16 = true;
    } else {
        blaster->bit16 = false;
    }
    return true;
}
#else
bool detect_sb(sblaster_t *blaster) { return false; }
#endif  // PLATFORM_UNIX
