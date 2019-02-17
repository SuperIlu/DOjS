/*
** Licensed under Attribution 4.0 International (CC BY 4.0)
** https://creativecommons.org/licenses/by/4.0/
**
** FM.DAT is licensed under WTFPL: http://www.wtfpl.net/
**
** Code was taken from http://www.shdon.com/dos/sound
** by Steven Don.
** This is a derived/modified version by Andre Seidelt <superilu@yahoo.com>
*/
/****************************************************************************
** Background MIDI unit                                                    **
**  by Steven H Don                                                        **
*****************************************************************************
** The file FM.DAT is necessary to play through Sound Blaster/compatible   **
*****************************************************************************
** Unit for playing MIDI files in the background through either an FM      **
** sound card, such as the Sound Blaster (any version) or a General MIDI   **
** device, such as the Roland cards or a Gravis UltraSound using MegaEM.   **
** It does not implement all of the MIDI effects and asynchronous files    **
** are not supported. It works with most files I have, however. As you can **
** see from the size of the file, as well as the complexity of the code,   **
** MIDI is no easy stuff. This file should be useful though.               **
**                                                                         **
** For questions, feel free to e-mail me.                                  **
**                                                                         **
**    shd@earthling.net                                                    **
**    http://shd.cjb.net                                                   **
**                                                                         **
****************************************************************************/
/*
  Functions available to the calling program are:
    LoadMIDI   : loads the MIDI file into memory.
                   Expects a filename including .MID extension
                   Returns 1 if successful
    UnloadMIDI : unloads the MIDI file and fress the memory.
    PlayMIDI   : starts MIDI playback
    StopMIDI   : stops MIDI playback
    SetGM      : sets the playback device to General MIDI.
    SetFM      : sets the playback device to the FM synthesizer
    SetVol     : sets the playback volume, from 0..255
    Playing    : returns 0 when the MIDI file has ended
*/
#include "midiplay.h"
#include <mujs.h>
#include "DOjS.h"

#ifndef PLATFORM_UNIX
#include <dos.h>
#include <go32.h>
#include <malloc.h>
#include <stdio.h>
// Extra for protected mode interrupt handler in DJGPP
#include <dpmi.h>
#include <pc.h>

// Used to distinguish between General MIDI devices and FM synthesizers
#define None 0
#define FM 1
#define GM 2
// Used to activate tremolo and vibrato amplification
#define AM 1
#define VIB 0

// The MIDI File's header
struct FileHeaderType {
    long MTHd;  // 6468544Dh = "MTHd"
    long HeaderSize;
    unsigned short int FileFormat, NumberOfTracks, TicksPerQNote;
};

// The header of a track
struct TrackHeaderType {
    long MTrk;  // 6B72544Dh = "MTrk"
    long TrackSize;
};

typedef struct __midi {
    long TicksPerQNote;
    unsigned char NumberOfTracks;  // How many tracks are there in the file
    unsigned int TrackSize[64];    // Stores the byte length of each track
    char *TrackData[64];           // Stores the actual MIDI data
} midi_t;

// Variables for reading the MIDI file
// This is used to determine whether a track needs attention
static long mid_WaitingFor[64];
// Which byte is to be read next
static unsigned int mid_NextBytePtr[64];
// Stores the last MIDI command sent, this is necessary for "running" mode
static unsigned char mid_LastCommand[64];

// This stores a pointer to the original timer handler
// DJGPP requires some doing
static _go32_dpmi_seginfo pm_old_handler, pm_new_handler;

// This is used for counting the clock ticks
static long mid_ClockTicks, mid_TickCounter, mid_MIDICounter;
// This is used in case Windows is active
static long mid_WinTick;
static char mid_Windows;
// Variable for amplifying AM, VIB and percussion mode.
static unsigned char mid_ByteBD;

// Variables and constants necessary for playing the MIDI file through an FM
// based sound card such as Sound Blaster

// Addresses of the operators used to form voice data
static const int mid_OpAdr[9] = {0, 1, 2, 8, 9, 10, 16, 17, 18};
// F numbers - to form a specific note
static const unsigned short int mid_FNr[128] = {86,  91,  96,  102, 108, 114, 121, 128, 136, 144, 153, 162, 172, 182, 192, 204, 108, 114, 121, 128, 136, 144, 153, 162, 172, 182,
                                                192, 204, 216, 229, 242, 257, 136, 144, 153, 162, 172, 182, 192, 204, 216, 229, 242, 257, 272, 288, 306, 324, 172, 182, 192, 204,
                                                216, 229, 242, 257, 272, 288, 306, 324, 343, 363, 385, 408, 216, 229, 242, 257, 272, 288, 306, 324, 343, 363, 385, 408, 432, 458,
                                                485, 514, 272, 288, 306, 324, 343, 363, 385, 408, 432, 458, 485, 514, 544, 577, 611, 647, 343, 363, 385, 408, 432, 458, 485, 514,
                                                544, 577, 611, 647, 686, 726, 770, 816, 432, 458, 485, 514, 544, 577, 611, 647, 686, 726, 770, 816, 864, 916, 970, 1023};
/*
  Some operators are reserved for percussion. They are at the end of the
  SB's operators which means they're in the middle of the SB Pro's. The main
  program doesn't take this into account so this is used to convert from
  virtual voice number to real voice number
*/
static const int mid_RealVoice[15] = {0, 1, 2, 3, 4, 5, 9, 10, 11, 12, 13, 14, 15, 16, 17};

static char mid_InUse[15];
static long mid_Activated[15];
static unsigned char mid_MIDILink[15], mid_NoteNumber[15], mid_NoteVelocity[15];
static unsigned char mid_NoteOffData[18][2];
// This stores which instrument is currently in use on a MIDI channel
static unsigned char mid_Instrument[16];
// This stores the FM instrument definitions
static unsigned char mid_M2[128], mid_M4[128], mid_M6[128], mid_M8[128], mid_ME[128], mid_MC[128], mid_C2[128], mid_C4[128], mid_C6[128], mid_C8[128], mid_CE[128];

// This indicates whether the file should be played through a General MIDI
// device or through an FM synthesizer, such as the Sound Blaster
static unsigned char mid_Device;
// This indicates whether we have a Sound Blaster (6 voices) or an SB Pro or
// better (15 voices) - the 5 drums are always available
static unsigned char mid_Voices;
// This stores the Base address of the Sound Blaster and GM device
static unsigned short int mid_FMBase, mid_GMBase;
// The master volume. Normal volume is 128.
static long mid_MasterVolume;

// the current playing song
static midi_t *mid_CurrentSong;

// This procedure compensates a given volume for the master volume
static unsigned char mid_DoVolume(unsigned char Before) {
    long After;

    After = Before;
    After *= mid_MasterVolume;
    After >>= 7;
    if (After > 127) After = 127;
    return (unsigned char)After;
}

// This procedure changes the speed of the timer to adjust the tempo of a song
// NewSpeed gives the amount of microseconds per quarter note
static void mid_ChangeSpeed(midi_t *midi, long NewSpeed) {
    long Divisor;

    Divisor = (midi->TicksPerQNote << 20) / NewSpeed;

    // If Windows is present, the timer frequency must remain below 1000
    if (mid_Windows) {
        if (Divisor > 1000) {
            mid_WinTick = 1 + (Divisor / 1000);
            Divisor = Divisor / mid_WinTick;
        }
    } else
        mid_WinTick = 1;
    // Set the appropriate values for the timer interrupt
    mid_TickCounter = 0x1234DD / Divisor;
    outportb(0x43, 0x34);
    outportb(0x40, mid_TickCounter & 0xFF);
    outportb(0x40, (mid_TickCounter >> 8) & 0xFF);
}

// Writes a value to a specified index register on the FM card
static void mid_WriteFM(unsigned char Chip, unsigned char Register, unsigned char Value) {
    unsigned int Counter;
    unsigned int Address;

    if (Chip == 0)
        Address = mid_FMBase;
    else
        Address = mid_FMBase + 2;
    // Select register
    outportb(Address, Register);
    // Wait for card to accept value
    for (Counter = 0; Counter < 25; Counter++) inportb(Address);
    // Send value
    outportb(Address + 1, Value);
    // Wait for card to accept value
    for (Counter = 0; Counter < 100; Counter++) inportb(Address);
}

// Sets a channel on the FM synthesizer to a specific instrument
static void mid_SetInstr(unsigned char Voice, unsigned char I, unsigned char Volume) {
    unsigned int Chip, Value;

    if (Voice > 8) {
        Chip = 1;
        Voice -= 9;
    } else
        Chip = 0;
    // Correction for volume
    Value = 63 - (mid_M4[I] & 63);
    Value = Value * Volume / 127;
    if (Value > 63)
        Value = 0;
    else
        Value = 63 - Value;
    Value = (mid_M4[I] & 0xC0) | Value;
    // Set up voice modulator
    mid_WriteFM(Chip, 0x20 + mid_OpAdr[Voice], mid_M2[I]);
    mid_WriteFM(Chip, 0x40 + mid_OpAdr[Voice], Value);
    mid_WriteFM(Chip, 0x60 + mid_OpAdr[Voice], mid_M6[I]);
    mid_WriteFM(Chip, 0x80 + mid_OpAdr[Voice], mid_M8[I]);
    mid_WriteFM(Chip, 0xE0 + mid_OpAdr[Voice], mid_ME[I]);
    // The "or 3 shl 4" is enables the voice on the OPL3
    mid_WriteFM(Chip, 0xC0 + mid_OpAdr[Voice], mid_MC[I] + (3 << 4));

    // Correction for volume
    Value = 63 - (mid_C4[I] & 63);
    Value = Value * Volume / 127;
    if (Value > 63)
        Value = 0;
    else
        Value = 63 - Value;
    Value = (mid_C4[I] & 0xC0) | Value;
    // Set up voice carrier
    mid_WriteFM(Chip, 0x23 + mid_OpAdr[Voice], mid_C2[I]);
    mid_WriteFM(Chip, 0x43 + mid_OpAdr[Voice], Value);
    mid_WriteFM(Chip, 0x63 + mid_OpAdr[Voice], mid_C6[I]);
    mid_WriteFM(Chip, 0x83 + mid_OpAdr[Voice], mid_C8[I]);
    mid_WriteFM(Chip, 0xE3 + mid_OpAdr[Voice], mid_CE[I]);
}

// Sets up a drum channel, in much the same way as a normal voice
static void mid_SetDrum(unsigned char Operator, unsigned char O2, unsigned char O4, unsigned char O6, unsigned char O8, unsigned char OE, unsigned char OC) {
    mid_WriteFM(0, 0x20 + Operator, O2);
    mid_WriteFM(0, 0x40 + Operator, O4);
    mid_WriteFM(0, 0x60 + Operator, O6);
    mid_WriteFM(0, 0x80 + Operator, O8);
    mid_WriteFM(0, 0xE0 + Operator, OE);
    mid_WriteFM(0, 0xC0 + Operator, OC);
}

// Enables a note on the FM synthesizer
static void mid_EnableNote(unsigned char Voice, unsigned char Number) {
    unsigned char Chip, Note, Block;

    // Calculate which part of the OPL3 chip should receive the data
    if (Voice > 8) {
        Chip = 1;
        Voice -= 9;
    } else
        Chip = 0;
    // Calculate appropriate data for FM synthesizer
    Note = Number;
    Block = Number >> 4;
    // Store data to disable the note when necessary
    mid_NoteOffData[Voice][0] = (mid_FNr[Note] & 0xFF);
    mid_NoteOffData[Voice][1] = (mid_FNr[Note] >> 8) + (Block << 2);
    // Write data to FM synthesizer
    mid_WriteFM(Chip, 0xA0 + Voice, (mid_FNr[Note] & 0xFF));
    mid_WriteFM(Chip, 0xB0 + Voice, (mid_FNr[Note] >> 8) + (Block << 2) + 32);
}

// Disables a note on the FM synthesizer
static void mid_DisableNote(unsigned char Voice) {
    unsigned char Chip;

    // Calculate which part of the OPL3 chip should receive the data
    if (Voice > 8) {
        Chip = 1;
        Voice -= 9;
    } else
        Chip = 0;
    // Write data to FM synthesizer
    mid_WriteFM(Chip, 0xA0 + Voice, mid_NoteOffData[Voice][0]);
    mid_WriteFM(Chip, 0xB0 + Voice, mid_NoteOffData[Voice][1]);
}

// Cuts a note on the FM synthesizer immediately
static void mid_CutNote(unsigned char Voice) {
    unsigned char Chip;

    // Calculate which part of the OPL3 chip should receive the data}
    if (Voice > 8)
        Chip = 1;
    else
        Chip = 0;
    // Set decay rate to fast - to avoid "plink" sound
    mid_WriteFM(Chip, 0x80 + mid_OpAdr[Voice % 9], 0xF);
    mid_WriteFM(Chip, 0x83 + mid_OpAdr[Voice % 9], 0xF);
    // Disable the note
    mid_DisableNote(Voice);
}

// Processes a "NoteOff" event for the FM synthesizer
static void mid_NoteOff(unsigned char MIDIChannel, unsigned char Number, unsigned char Velocity) {
    unsigned char FoundChannel, FMChannel;

    // Assume the note can't be found
    FoundChannel = 255;
    // Scan for note on FM channels
    for (FMChannel = 0; FMChannel <= mid_Voices; FMChannel++) {
        if (mid_InUse[FMChannel] != 0) {
            // Is this the correct channel?
            if (mid_MIDILink[FMChannel] == MIDIChannel && mid_NoteNumber[FMChannel] == Number) {
                // If the correct channel has been found then report that
                FoundChannel = FMChannel;
                break;
            }
        }
    }
    if (FoundChannel != 255) {
        // Disable the note
        mid_DisableNote(mid_RealVoice[FoundChannel]);
        // Store appropriate information
        mid_InUse[FoundChannel] = 0;  // InUse flag
    }
    // Done to prevent "Parameter 'Velocity' never used
    (void)Velocity;
}

// Processes a "NoteOn" event for the FM synthesizer
static void mid_NoteOn(unsigned char MIDIChannel, unsigned char Number, unsigned char Velocity) {
    unsigned char FreeChannel, FMChannel;

    // Velocity of zero means note off
    if (Velocity == 0) {
        mid_NoteOff(MIDIChannel, Number, Velocity);
        return;
    }
    // Assume no free channel
    FreeChannel = 255;
    // Scan for free channel
    for (FMChannel = 0; FMChannel <= mid_Voices; FMChannel++) {
        if (mid_InUse[FMChannel] == 0) {
            // If a free channel has been found then report that
            FreeChannel = FMChannel;
            break;
        }
    }
    // If there was no free channel, the SB's 6/15 voice polyphony
    // has been exceeded and the "oldest" note must be deactivated
    if (FreeChannel == 255) {
        long Oldest = 0x7FFFFFFF;
        // Scan for the oldest note
        for (FMChannel = 0; FMChannel <= mid_Voices; FMChannel++) {
            if (mid_Activated[FMChannel] < Oldest) {
                FreeChannel = FMChannel;
                Oldest = mid_Activated[FMChannel];
            }
        }
        // Disable the note currently playing
        mid_CutNote(mid_RealVoice[FreeChannel]);
    }
    // Change the instrument settings for the FM channel chosen
    mid_SetInstr(mid_RealVoice[FreeChannel], mid_Instrument[MIDIChannel], Velocity);
    // Start playing the note
    mid_EnableNote(mid_RealVoice[FreeChannel], Number);
    // Store appropriate information
    mid_InUse[FreeChannel] = 1;                    // InUse flag
    mid_Activated[FreeChannel] = mid_MIDICounter;  // Activation time
    mid_MIDILink[FreeChannel] = MIDIChannel;       // Link FM channel to MIDI channel
    mid_NoteNumber[FreeChannel] = Number;          // Note number (which note is being played)
    mid_NoteVelocity[FreeChannel] = Velocity;      // Velocity (=volume)
}

// Plays a drum note
static void mid_DrumOn(unsigned char MIDIChannel, unsigned char Number, unsigned char Velocity) {
    // If velocity is 0, note is turned off, this is ignored
    if (Velocity == 0) return;
    // Convert velocity to "level" needed by SB and reduce the volume slightly
    Velocity = (int)(Velocity << 3) / 10;
    Velocity = 63 - (Velocity >> 1);
    // Bass drum
    if (Number == 35 || Number == 36 || Number == 41 || Number == 43) {
        // Set channel 6 to bass, allowing for volume
        mid_SetDrum(16, 0, 13, 248, 102, 0, 48);
        mid_SetDrum(19, 0, Velocity, 246, 87, 0, 16);
        // Enable bass and immediately deactivate
        mid_WriteFM(0, 0xBD, mid_ByteBD | 16);
        mid_WriteFM(0, 0xBD, mid_ByteBD);
    }
    // HiHat
    if (Number == 37 || Number == 39 || Number == 42 || Number == 44 || Number == 46 || Number == 56 || Number == 62 || Number == 69 || Number == 70 || Number == 71 ||
        Number == 72 || Number == 78) {
        // Set channel 7 to hihat, allowing for volume
        mid_SetDrum(17, 0, Velocity, 240, 6, 0, 16);
        // Enable hihat and immediately deactivate
        mid_WriteFM(0, 0xBD, mid_ByteBD | 1);
        mid_WriteFM(0, 0xBD, mid_ByteBD);
    }
    // Snare drum
    if (Number == 38 || Number == 40) {
        // Set channel 7 to snare drum, allowing for volume
        mid_SetDrum(20, 0, Velocity, 240, 7, 2, 16);
        // Enable hihat and immediately deactivate
        mid_WriteFM(0, 0xBD, mid_ByteBD | 8);
        mid_WriteFM(0, 0xBD, mid_ByteBD);
    }
    // TomTom
    if (Number == 45 || Number == 47 || Number == 48 || Number == 50 || Number == 60 || Number == 61 || Number == 63 || Number == 64 || Number == 65 || Number == 66 ||
        Number == 67 || Number == 68 || Number == 73 || Number == 74 || Number == 75 || Number == 76 || Number == 77) {
        // Set channel 8 to tomtom, allowing for volume
        mid_SetDrum(18, 2, Velocity, 240, 6, 0, 16);
        // Enable tomtom and immediately deactivate
        mid_WriteFM(0, 0xBD, mid_ByteBD | 4);
        mid_WriteFM(0, 0xBD, mid_ByteBD);
    }
    // Cymbal
    if (Number == 49 || Number == 51 || Number == 52 || Number == 53 || Number == 54 || Number == 55 || Number == 57 || Number == 58 || Number == 59 || Number == 79 ||
        Number == 80 || Number == 81) {
        // Set channel 8 to cymbal, allowing for volume
        mid_SetDrum(21, 4, Velocity, 240, 6, 0, 16);
        // Enable cymbal and immediately deactivate
        mid_WriteFM(0, 0xBD, mid_ByteBD | 2);
        mid_WriteFM(0, 0xBD, mid_ByteBD);
    }
    // Done to avoid "Parameter 'MIDIChannel' never used
    (void)MIDIChannel;
}

// Disables a drum note, well, it actually does nothing since drum notes
// do not need to be disabled
static void mid_DrumOff(unsigned char MIDIChannel, unsigned char Number, unsigned char Velocity) {
    (void)MIDIChannel;
    (void)Number;
    (void)Velocity;
}

// Sends a GM command to the GM device
static void mid_SendGM(unsigned char c) {
    while ((inportb(mid_GMBase + 1) & 0x40) != 0)
        ;
    outportb(mid_GMBase, c);
}

// This function reads a byte from a specific track
static unsigned char mid_ReadByte(midi_t *midi, unsigned char TrackNumber) {
    if (mid_WaitingFor[TrackNumber] < 0xFFFFFF)
        return midi->TrackData[TrackNumber][mid_NextBytePtr[TrackNumber]++];
    else
        return 0;
}

// This function reads a Variable Length Encoded (VLE) number from the track
static long mid_GetVLE(midi_t *midi, unsigned char TrackNumber) {
    unsigned char ByteRead;
    long Result;

    // Assume zero
    Result = 0;
    do {
        // Read first byte
        ByteRead = mid_ReadByte(midi, TrackNumber);
        // Store 7bit part
        Result = (Result << 7) + (ByteRead & 0x7F);
    } while ((ByteRead & 0x80) != 0);
    return Result;
}

// This void stores the time for the next event
static void mid_GetDeltaTime(midi_t *midi, unsigned char TrackNumber) { mid_WaitingFor[TrackNumber] += mid_GetVLE(midi, TrackNumber); }

// This void handles the MIDI events
static void mid_DoEvent(midi_t *midi, unsigned char TrackNumber) {
    unsigned char MIDIByte, MIDICommand, MIDIChannel;
    unsigned char n1, n2;

    // Get the MIDI event command from the track
    MIDIByte = mid_ReadByte(midi, TrackNumber);
    // If this is not a command, we are in "running" mode and the last
    // command issued on the track is assumed
    if ((MIDIByte & 0x80) == 0) {
        MIDIByte = mid_LastCommand[TrackNumber];
        mid_NextBytePtr[TrackNumber]--;
    }
    // Store the command for running mode
    mid_LastCommand[TrackNumber] = MIDIByte;
    /*
      META-EVENTS
      ===========
      Special commands controlling timing etc.
    */
    if (MIDIByte == 0xFF) {
        unsigned char MetaEvent = mid_ReadByte(midi, TrackNumber);
        long DataLength = mid_GetVLE(midi, TrackNumber);
        if (MetaEvent == 0x2F)  // End of track}
            mid_WaitingFor[TrackNumber] = 0xFFFFFF;
        if (MetaEvent == 0x51) {  // Tempo change
            long Data = mid_ReadByte(midi, TrackNumber);
            Data = (Data << 8) + mid_ReadByte(midi, TrackNumber);
            Data = (Data << 8) + mid_ReadByte(midi, TrackNumber);
            mid_ChangeSpeed(midi, Data);
        }
        if ((MetaEvent != 0x2F) && (MetaEvent != 0x51)) {
            // Others (text events, track sequence numbers etc. - ignore
            for (long Counter = 1; Counter <= DataLength; Counter++) mid_ReadByte(midi, TrackNumber);
        }
    }
    /*
      CHANNEL COMMANDS
      ================
      Upper nibble contains command, lower contains channel
    */
    MIDIChannel = MIDIByte & 0xF;
    MIDICommand = MIDIByte >> 4;
    if (MIDICommand == 8) {  // Note off
        n1 = mid_ReadByte(midi, TrackNumber);
        n2 = mid_DoVolume(mid_ReadByte(midi, TrackNumber));
        /*This allows the use of a wavetable General MIDI instrument (such
        as the Roland SCC1 (or an emulation thereof) or the FM synthesizer*/
        if (mid_Device == FM) {  // FM - Sound Blaster or AdLib
            if (MIDIChannel == 9 || MIDIChannel == 15) {
                mid_DrumOff(MIDIChannel, n1, n2);
            } else
                mid_NoteOff(MIDIChannel, n1, n2);
        } else {  // GM - General MIDI device
            mid_SendGM(MIDIByte);
            mid_SendGM(n1);
            mid_SendGM(n2);
        }
    }
    if (MIDICommand == 9) {  // Note on
        n1 = mid_ReadByte(midi, TrackNumber);
        n2 = mid_DoVolume(mid_ReadByte(midi, TrackNumber));
        if (mid_Device == FM) {
            if (MIDIChannel == 9 || MIDIChannel == 15)
                mid_DrumOn(MIDIChannel, n1, n2);
            else
                mid_NoteOn(MIDIChannel, n1, n2);
        } else {
            mid_SendGM(MIDIByte);
            mid_SendGM(n1);
            mid_SendGM(n2);
        }
    }
    if (MIDICommand == 0xA) {  // Key Aftertouch - only supported for GM device
        n1 = mid_ReadByte(midi, TrackNumber);
        n2 = mid_DoVolume(mid_ReadByte(midi, TrackNumber));
        if (mid_Device == GM) {
            mid_SendGM(MIDIByte);
            mid_SendGM(n1);
            mid_SendGM(n2);
        }
    }
    if (MIDICommand == 0xB) {  // Control change - only supported for GM device
        n1 = mid_ReadByte(midi, TrackNumber);
        n2 = mid_ReadByte(midi, TrackNumber);
        if (mid_Device == GM) {
            mid_SendGM(MIDIByte);
            mid_SendGM(n1);
            mid_SendGM(n2);
        }
    }
    if (MIDICommand == 0xC) {  // Patch change - this changes the instrument on a channel
        n1 = mid_ReadByte(midi, TrackNumber);
        if (mid_Device == FM) {
            mid_Instrument[MIDIChannel] = n1;
        } else {
            mid_SendGM(MIDIByte);
            mid_SendGM(n1);
        }
    }
    if (MIDICommand == 0xD) {  // Channel aftertouch - only supported on GM device
        n1 = mid_ReadByte(midi, TrackNumber);
        if (mid_Device == GM) {
            mid_SendGM(MIDIByte);
            mid_SendGM(n1);
        }
    }
    if (MIDICommand == 0xE) {  // Pitch wheel change - only supported on GM device
        n1 = mid_ReadByte(midi, TrackNumber);
        n2 = mid_ReadByte(midi, TrackNumber);
        if (mid_Device == GM) {
            mid_SendGM(MIDIByte);
            mid_SendGM(n1);
            mid_SendGM(n2);
        }
    }
    /*
      SYSTEM COMMANDS
      ===============
      These are ignored.
    */
    if (MIDICommand == 0xF) {
        if (MIDIChannel == 0) do
                ;
            while (mid_ReadByte(midi, TrackNumber) != 0x7F);  // System Exclusive
        if (MIDIChannel == 2) {
            mid_ReadByte(midi, TrackNumber);
            mid_ReadByte(midi, TrackNumber);
        }  // Song Position Pointer
        if (MIDIChannel == 3) {
            mid_ReadByte(midi, TrackNumber);
        }  // Song Select
    }
}

// Returns TRUE if the MIDI file is still playing. FALSE if it has stopped
static bool mid_Playing(midi_t *midi) {
    unsigned char CurrentTrack;

    // Check for at least one track still playing
    for (CurrentTrack = 1; CurrentTrack <= midi->NumberOfTracks; CurrentTrack++) {
        if (mid_WaitingFor[CurrentTrack] < 0xFFFFFF) {
            return (true);
        }
    }
    return false;
}

// This is the new timer interrupt handler and routines to install it
static void mid_TimerHandler() {
    midi_t *midi = mid_CurrentSong;

    unsigned char CurrentTrack;

    // Increase MIDI counter
    mid_MIDICounter += mid_WinTick;
    // Check all the channels for MIDI events
    for (CurrentTrack = 1; CurrentTrack <= midi->NumberOfTracks; CurrentTrack++) {
        // If it is time to handle an event, do so
        if (mid_NextBytePtr[CurrentTrack] < midi->TrackSize[CurrentTrack])
            while (mid_MIDICounter >= mid_WaitingFor[CurrentTrack]) {
                // Call the event handler
                mid_DoEvent(midi, CurrentTrack);
                // Store the time for the next event
                mid_GetDeltaTime(midi, CurrentTrack);
            }
    }

    // Check whether we need to call the original timer handler
    mid_ClockTicks += mid_TickCounter;
    // Do so if required
    if (mid_ClockTicks > 65535) {
        mid_ClockTicks -= 65536;
    } else
        outportb(0x20, 0x20);
}

// Installs the MIDI timer handler
static void mid_InstallTimer(midi_t *midi) {
    mid_CurrentSong = midi;
    // Assume 18.2 times a second
    mid_TickCounter = 0;
    // Assume tempo 120 according to MIDI spec
    mid_ChangeSpeed(midi, midi->TicksPerQNote * 25000 / 3);
    // Install new timer handler
    _go32_dpmi_chain_protected_mode_interrupt_vector(8, &pm_new_handler);
}

// Restores the BIOS timer handler
static void mid_RestoreTimer() {
    // Return to 18.2 times a second}
    outportb(0x43, 0x34);
    outportb(0x40, 0);
    outportb(0x40, 0);
    // Install old timer handler
    _go32_dpmi_set_protected_mode_interrupt_vector(8, &pm_old_handler);
}

// This converts a 32bit number from little-endian (Motorola) to big-endian
//(Intel) format
static long mid_L2B32(long L) {
    long B;
    unsigned char T;

    B = 0;
    for (T = 0; T <= 3; T++) {
        B = (B << 8) + (L & 0xFF);
        L >>= 8;
    }
    return B;
}

// This converts a 16bit number from little-endian (Motorola) to big-endian
//(Intel) format
static unsigned int mid_L2B16(unsigned int L) { return ((L & 0xFF) << 8) + (L >> 8); }

// This loads the MIDI file into memory
static midi_t *mid_LoadMIDI(const char *FileName) {
    // To access the file itself
    FILE *MIDIFile;
    struct FileHeaderType MIDIHeader;
    struct TrackHeaderType TrackHeader;
    // For loading the tracks
    unsigned char CurrentTrack;

    midi_t *midi = (midi_t *)malloc(sizeof(midi_t));
    if (!midi) {
        return NULL;
    }

    // Open the file
    MIDIFile = fopen(FileName, "rb");
    if (MIDIFile == NULL) {
        LOG("Midi file not found.");
        free(midi);
        return (NULL);
    }

    // Read in the header
    fread(&MIDIHeader, 14, 1, MIDIFile);

    // If the first four bytes do not constiture "MTHd", this is not a MIDI file
    if (MIDIHeader.MTHd != 0x6468544D) {
        LOG("Wrong header in midi file.");
        fclose(MIDIFile);
        free(midi);
        return (NULL);
    }

    // If the header size is other than 6, this is an unknown
    // type of MIDI file
    if (mid_L2B32(MIDIHeader.HeaderSize) != 6) {
        LOG("Unknown header size in midi file.");
        fclose(MIDIFile);
        free(midi);
        return (NULL);
    }

    // Convert file format identifier
    MIDIHeader.FileFormat = mid_L2B16(MIDIHeader.FileFormat);
    // If it is an asynchronous file (type 2), I don't know how to play it
    if (MIDIHeader.FileFormat == 2) {
        LOG("Unknown format in midi file.");
        fclose(MIDIFile);
        free(midi);
        return (NULL);
    }

    // Store the tempo of the file
    midi->TicksPerQNote = mid_L2B16(MIDIHeader.TicksPerQNote);
    // Store the number of tracks in the file
    midi->NumberOfTracks = mid_L2B16(MIDIHeader.NumberOfTracks);
    if (MIDIHeader.FileFormat == 0) midi->NumberOfTracks = 1;
    // When we reach this, we can start loading
    for (CurrentTrack = 1; CurrentTrack <= midi->NumberOfTracks; CurrentTrack++) {
        // Load track header
        fread(&TrackHeader, 8, 1, MIDIFile);
        // If the first 4 bytes do not form "MTrk", the track is invalid
        if (TrackHeader.MTrk != 0x6B72544D) {
            LOG("Unknown track type in midi file.");
            fclose(MIDIFile);
            free(midi);
            return (NULL);
        }
        // We need to convert little-endian to big endian
        TrackHeader.TrackSize = mid_L2B32(TrackHeader.TrackSize);
        // If it's too big, we can't load it
        midi->TrackSize[CurrentTrack] = TrackHeader.TrackSize;
        // Assign memory for the track
        midi->TrackData[CurrentTrack] = (char *)malloc(midi->TrackSize[CurrentTrack]);
        fread((char *)midi->TrackData[CurrentTrack], midi->TrackSize[CurrentTrack], 1, MIDIFile);
    }

    // Close it
    fclose(MIDIFile);
    return midi;
}

// This resets the drums
static void mid_EnableDrums() {
    // Enable waveform select
    mid_WriteFM(0, 1, 0x20);
    // Enable percussion mode, amplify AM & VIB
    mid_ByteBD = (AM << 7) | (VIB << 6) | 0x20;
    mid_WriteFM(0, 0xBD, mid_ByteBD);
    // Set drum frequencies
    mid_WriteFM(0, 0xA6, 400 & 0xFF);
    mid_WriteFM(0, 0xB6, (400 >> 8) + (2 << 2));
    mid_WriteFM(0, 0xA7, 500 & 0xFF);
    mid_WriteFM(0, 0xB7, (500 >> 8) + (2 << 2));
    mid_WriteFM(0, 0xA8, 650 & 0xFF);
    mid_WriteFM(0, 0xB8, (650 >> 8) + (2 << 2));
}

// Guess!!
static void mid_StopMIDI() {
    unsigned char CurrentChannel;

    mid_RestoreTimer();
    // Send "All notes off" to each channel
    if (mid_Device == FM) {
        for (CurrentChannel = 0; CurrentChannel <= 17; CurrentChannel++) {
            mid_DisableNote(CurrentChannel);
        }
    } else {
        for (CurrentChannel = 0; CurrentChannel <= 15; CurrentChannel++) {
            mid_SendGM(0xB0 | CurrentChannel);
            mid_SendGM(123);
            mid_SendGM(0);
        }
    }
    mid_CurrentSong = NULL;
}

// This starts playing the MIDI file}
static void mid_PlayMIDI(midi_t *midi) {
    unsigned char CurrentTrack;

    // MIDI might already be playing, so stop it first
    mid_StopMIDI();
    // Clear read pointers for every track
    for (CurrentTrack = 1; CurrentTrack <= midi->NumberOfTracks; CurrentTrack++) {
        mid_NextBytePtr[CurrentTrack] = 0;
        mid_WaitingFor[CurrentTrack] = 0;
        mid_LastCommand[CurrentTrack] = 0xFF;
        mid_GetDeltaTime(midi, CurrentTrack);
    }
    mid_MIDICounter = 0;
    mid_WinTick = 1;
    mid_EnableDrums();
    mid_InstallTimer(midi);
}

// This unloads the MIDI file from memory
static void mid_UnloadMIDI(midi_t *midi) {
    unsigned char CurrentTrack;

    mid_StopMIDI();
    for (CurrentTrack = 1; CurrentTrack <= midi->NumberOfTracks; CurrentTrack++) {
        if (midi->TrackSize[CurrentTrack] != 0) {
            free(midi->TrackData[CurrentTrack]);
            midi->TrackSize[CurrentTrack] = 0;
        }
    }
    free(midi);
}

// Set the playback volume
static void mid_SetVol(unsigned char NewVol) { mid_MasterVolume = NewVol; }

// Check for the existence of an OPL2/3 chip
static int mid_TestOPL(unsigned int Test) {
    unsigned char A, B;
    int Result;

    // Assume no OPL was found
    Result = 0;

    // Find it
    outportb(Test, 0);
    delay(1);
    outportb(Test + 1, 0);
    delay(1);
    outportb(Test, 4);
    delay(1);
    outportb(Test + 1, 0x60);
    delay(1);
    outportb(Test, 4);
    delay(1);
    outportb(Test + 1, 0x60);
    delay(1);
    A = inportb(Test);
    outportb(Test, 2);
    delay(1);
    outportb(Test + 1, 0xFF);
    delay(1);
    outportb(Test, 4);
    delay(1);
    outportb(Test + 1, 0x21);
    delay(1);
    B = inportb(Test);
    outportb(Test, 4);
    delay(1);
    outportb(Test + 1, 0x60);
    delay(1);
    outportb(Test, 4);
    delay(1);
    outportb(Test + 1, 0x60);
    delay(1);

    if ((A & 0xE0) == 0 && (B & 0xE0) == 0xC0)
        // This might be an OPL2
        Result = 2;
    else
        // There's nothin here, so stop looking
        return 0;

    // Check for OPL3
    if ((inportb(Test) & 0x06) == 0) Result = 3;

    return Result;
}

// This function returns -1 if a GM device is detected at the specified port
static int mid_TestGM(unsigned int Base) {
    delay(10);
    if ((inportb(Base + 1) & 0x40) == 0) {
        outportb(Base, 0xF8);
        delay(10);
        if ((inportb(Base + 1) & 0x40) == 0) return -1;
    }
    return 0;
}

/*This function reports whether Windows is present. Windows interferes with
the timer interrupt and measures have to be taken.*/
static char mid_MSWindows() {
    union REGS r;
    r.x.ax = 0x1600;
    int86(0x2F, &r, &r);
    return r.h.al;
}

// Initialize FM driver
static char mid_SetFM() {
    unsigned char Temp;

    // Assume a standard SB or AdLib: 6 melodic voices, 5 percussion voices
    mid_Voices = 5;
    // Check for FM card
    if (mid_TestOPL(0x388) > 0) mid_FMBase = 0x388;
    // Check for OPL3 at 0x220 and 0x240
    Temp = mid_TestOPL(0x240);
    if (Temp == 2) mid_FMBase = 0x240;
    if (Temp == 3) {
        mid_FMBase = 0x240;
        mid_Voices = 14;
    }
    Temp = mid_TestOPL(0x220);
    if (Temp == 2) mid_FMBase = 0x220;
    if (Temp == 3) {
        mid_FMBase = 0x220;
        mid_Voices = 14;
    }
    if (mid_FMBase != 0) {
        // Enable OPL3 if present
        if (mid_Voices != 5) {
            mid_WriteFM(1, 5, 1);
            mid_WriteFM(1, 4, 0);
        }
        // Load FM instrument definitions
        FILE *Bnk = fopen(BOOT_DIR "FM.DAT", "rb");
        if (!Bnk) {
            LOG(BOOT_DIR "FM.DAT missing!");
            return mid_Device;
        }
        fread(&mid_M2[0], 128, 1, Bnk);
        fread(&mid_M4[0], 128, 1, Bnk);
        fread(&mid_M6[0], 128, 1, Bnk);
        fread(&mid_M8[0], 128, 1, Bnk);
        fread(&mid_ME[0], 128, 1, Bnk);
        fread(&mid_MC[0], 128, 1, Bnk);
        fread(&mid_C2[0], 128, 1, Bnk);
        fread(&mid_C4[0], 128, 1, Bnk);
        fread(&mid_C6[0], 128, 1, Bnk);
        fread(&mid_C8[0], 128, 1, Bnk);
        fread(&mid_CE[0], 128, 1, Bnk);
        fclose(Bnk);
        mid_Device = FM;
    }
    return mid_Device == FM;
}

// Initialize GM driver
static char mid_SetGM() {
    // Try detecting a GM device
    mid_GMBase = 0;
    if (mid_TestGM(0x300)) mid_GMBase = 0x300;
    if (mid_TestGM(0x330)) mid_GMBase = 0x330;
    // If it is detected, use it
    if (mid_GMBase) mid_Device = GM;
    return mid_Device == GM;
}

static void mid_InitMIDI() {
    // No device found yet
    mid_Device = None;
    // Start at normal volume
    mid_SetVol(128);
    // Check whether Windows is present
    mid_Windows = mid_MSWindows();
    // Save old timer handler
    _go32_dpmi_get_protected_mode_interrupt_vector(8, &pm_old_handler);
    pm_new_handler.pm_offset = (int)mid_TimerHandler;
    pm_new_handler.pm_selector = _go32_my_cs();
}

/*********************
** static functions **
*********************/
/**
 * @brief finalize an image and free resources.
 *
 * @param J VM state.
 */
static void Midi_Finalize(js_State *J, void *data) {
    midi_t *midi = (midi_t *)data;
    mid_UnloadMIDI(midi);
}

/**
 * @brief load a MIDI and store it as userdata in JS object.
 * new Midi(filename:string)
 *
 * @param J VM state.
 */
static void new_Midi(js_State *J) {
    const char *fname = js_tostring(J, 1);

    midi_t *midi = mid_LoadMIDI(fname);
    if (!midi) {
        js_error(J, "Can't load midi '%s'", fname);
        return;
    }

    js_currentfunction(J);
    js_getproperty(J, -1, "prototype");
    js_newuserdata(J, TAG_MIDI, midi, Midi_Finalize);

    // add properties
    js_pushstring(J, fname);
    js_defproperty(J, -2, "filename", JS_READONLY | JS_DONTENUM | JS_DONTCONF);

    js_pushnumber(J, midi->NumberOfTracks);
    js_defproperty(J, -2, "tracks", JS_READONLY | JS_DONTENUM | JS_DONTCONF);
}

/**
 * @brief play back midi.
 * midi.Play()
 *
 * @param J VM state.
 */
static void mid_Play(js_State *J) {
    if (midi_available) {
        midi_t *midi = js_touserdata(J, 0, TAG_MIDI);
        mid_PlayMIDI(midi);
    }
}

/**
 * @brief check if midi is still playing.
 * midi.IsPlaying():boolean
 *
 * @param J VM state.
 */
static void mid_IsPlaying(js_State *J) {
    if (midi_available && mid_CurrentSong) {
        js_pushboolean(J, mid_Playing(mid_CurrentSong));
    } else {
        js_pushboolean(J, false);
    }
}

static void mid_Stop(js_State *J) {
    if (midi_available && mid_CurrentSong) {
        mid_StopMIDI(mid_CurrentSong);
    }
}

/***********************
** exported functions **
***********************/
/**
 * @brief initialize midi subsystem.
 *
 * @param J VM state.
 */
bool init_midi(js_State *J) {
    FUNCDEF(J, mid_IsPlaying, "MidiIsPlaying", 0);
    FUNCDEF(J, mid_Stop, "MidiStop", 0);

    js_newobject(J);
    { PROTDEF(J, mid_Play, TAG_MIDI, "Play", 0); }
    js_newcconstructor(J, new_Midi, new_Midi, TAG_MIDI, 1);
    js_defglobal(J, TAG_MIDI, JS_DONTENUM);

    mid_InitMIDI();
    if (mid_SetFM() == FM) {
        LOGF("FM MIDI available at 0x%03X.\n", mid_FMBase);
        return true;
    } else if (mid_SetGM() == GM) {
        LOGF("General MIDI available at 0x%03X.\n", mid_GMBase);
        return true;
    } else {
        // If not, end the programme
        LOG("MIDI disabled.\n");
        return false;
    }
}

/**
 * @brief shutdown MIDI subsystem.
 */
void shutdown_midi() { mid_StopMIDI(); }

#else
bool init_midi(js_State *J) { return false; }
void shutdown_midi() {}
#endif  // PLATFORM_UNIX
