/*
** Licensed under Attribution 4.0 International (CC BY 4.0)
** https://creativecommons.org/licenses/by/4.0/
**
** Code was taken from http://www.shdon.com/dos/sound
** by Steven Don.
** This is a derived/modified version by Andre Seidelt <superilu@yahoo.com>
*/
/****************************************************************************
** Demonstration of directly programming the FM synthesizer on AdLib and   **
** Sound Blaster compatible cards                                          **
**  by Steven H Don                                                        **
**                                                                         **
** For questions, feel free to e-mail me.                                  **
**                                                                         **
**    shd@earthling.net                                                    **
**    http://shd.home.ml.org                                               **
**                                                                         **
*****************************************************************************
** Original download URL: http://www.shdon.com/dos/sound                   **
** Modified for DJGPP in 2019 by superilu@yahoo.com                        **
****************************************************************************/
#include "DOjS.h"
#include <mujs.h>

#ifndef PLATFORM_UNIX
#include <dos.h>

struct Operator {
  unsigned char
      // Tremolo
      Tremolo,
      // Vibrato
      Vibrato,
      // Envelop generator type
      EGType,
      // KSR
      KSR,
      // Frequency multiplier
      Multi,
      // Key scaling level
      KSL,
      // Volume
      TotalLevel,
      // Shape of the wave envelop
      Attack, Decay, Sustain, Release,
      // Type of wave
      WaveShape;
};

struct Instrument {
  struct Operator Modulator, Carrier;

  unsigned char
      // Feedback strength
      Feedback,
      // Synthesis type
      SynType;
};

/**************************************************************************
** Writes a value to a specified index register on the FM card           **
**************************************************************************/
void WriteFM(unsigned char Register, unsigned char Value) {
  unsigned char Counter;

  // Select register
  outportb(0x388, Register);
  // Wait for card to accept value
  for (Counter = 1; Counter < 25; Counter++) {
    inportb(0x388);
  }
  // Send value
  outportb(0x389, Value);
  // Wait for card to accept value
  for (Counter = 1; Counter < 100; Counter++) {
    inportb(0x388);
  }
}

/**************************************************************************
** Checks for the presence of an FM card                                 **
**************************************************************************/
unsigned char FMInstalled() {
  unsigned char A, B;

  WriteFM(1, 0);
  WriteFM(4, 0x60);
  WriteFM(4, 0x80);
  A = inportb(0x388);
  WriteFM(2, 0xFF);
  WriteFM(4, 0x21);
  B = inportb(0x388);
  WriteFM(4, 0x60);
  WriteFM(4, 0x80);
  if ((A & 0xE0) == 0 && (B & 0xE0) == 0xC0) {
    return (1);
  } else {
    return (0);
  }
}

/**************************************************************************
** Activates a voice on the FM card                                      **
***************************************************************************
** Voice selects one of the 9 FM voices                                  **
** FNumber selects the note to be played                                 **
** Block selects the octave for the specified note                       **
**************************************************************************/
void NoteOn(unsigned char Voice, unsigned int Note, unsigned char Block) {
  WriteFM(0xA0 + Voice, Note & 0xFF);
  WriteFM(0xB0 + Voice, (Note >> 8) + (Block << 2) + 32);
}

/**************************************************************************
** Deactivates a voice on the FM card                                    **
***************************************************************************
** Make sure to give the same values for Note and Block or this will     **
** sound very odd.                                                       **
**************************************************************************/
void NoteOff(unsigned char Voice, unsigned int Note, unsigned char Block) {
  WriteFM(0xA0 + Voice, Note & 0xFF);
  WriteFM(0xB0 + Voice, (Note >> 8) + (Block << 2));
}

/**************************************************************************
** Sets instrument settings for a voice on the FM card                   **
**************************************************************************/
void SetInstrument(unsigned char Voice, struct Instrument Instr) {
  const
      // Addresses of the operators used to form voice data
      unsigned char OpAdr[9] = {0, 1, 2, 8, 9, 10, 16, 17, 18};

  unsigned char Value;

  // Set up voice modulator
  Value = Instr.Modulator.Tremolo << 8 | Instr.Modulator.Vibrato << 7 |
          Instr.Modulator.EGType << 6 | Instr.Modulator.KSR << 5 |
          Instr.Modulator.Multi;
  WriteFM(0x20 + OpAdr[Voice], Value);

  Value = Instr.Modulator.KSL << 7 | Instr.Modulator.TotalLevel;
  WriteFM(0x40 + OpAdr[Voice], Value);

  Value = Instr.Modulator.Attack << 4 | Instr.Modulator.Decay;
  WriteFM(0x60 + OpAdr[Voice], Value);

  Value = Instr.Modulator.Sustain << 4 | Instr.Modulator.Release;
  WriteFM(0x80 + OpAdr[Voice], Value);

  WriteFM(0xE0 + OpAdr[Voice], Instr.Modulator.WaveShape);

  Value = Instr.Feedback << 1 | Instr.SynType;
  WriteFM(0xC0 + OpAdr[Voice], Value);

  // Set up voice carrier
  Value = Instr.Carrier.Tremolo << 8 | Instr.Carrier.Vibrato << 7 |
          Instr.Carrier.EGType << 6 | Instr.Carrier.KSR << 5 |
          Instr.Carrier.Multi;
  WriteFM(0x23 + OpAdr[Voice], Value);

  Value = Instr.Carrier.KSL << 7 | Instr.Carrier.TotalLevel;
  WriteFM(0x43 + OpAdr[Voice], Value);

  Value = Instr.Carrier.Attack << 4 | Instr.Carrier.Decay;
  WriteFM(0x63 + OpAdr[Voice], Value);

  Value = Instr.Carrier.Sustain << 4 | Instr.Carrier.Release;
  WriteFM(0x83 + OpAdr[Voice], Value);

  WriteFM(0xE3 + OpAdr[Voice], Instr.Carrier.WaveShape);
}

/**
 * @brief check if the given property is provided by the JS object and store the
 * value into dest.
 *
 * @param J VM state.
 * @param idx stack index.
 * @param name name of the property.
 * @param dest where to store the value.
 */
static void check_get_property(js_State *J, int idx, char *name,
                               unsigned char *dest) {
  if (!js_hasproperty(J, idx, name)) {
    *dest = 0;
  } else {
    *dest = js_toint16(J, -1);
    // DEBUGF("Found %s=%d\n", name, *dest);
    js_pop(J, 1);
  }
}

/**
 * @brief find a child object in a JS object.
 *
 * @param J VM state.
 * @param idx stack index.
 * @param name name of the property.
 *
 * @return true if the object was found.
 * @return false if the object was not found.
 */
static bool check_get_object(js_State *J, int idx, char *name) {
  // DEBUGF("Object %s\n", name);
  if (!js_hasproperty(J, idx, name)) {
    js_error(J, "Missing property: %s\n", name);
    return false;
  } else {
    return true;
  }
}

/**
 * @brief convert the contents of a JS object into struct Instrument and set the
 * parameters to the given voice.
 * SetInstrument(voice:number, instrument:object)
 *
 * @param J VM state.
 */
static void fm_SetInstrument(js_State *J) {
  struct Instrument inst;

  if (synth_available) {
    int voice = js_toint16(J, 1);
    if (voice < 0 || voice > 8) {
      js_error(J, "Voice must be between 0 and 8");
      return;
    }
    bool valid = true;
    valid &= check_get_object(J, 2, "Modulator");
    {
      check_get_property(J, -1, "Attack", &inst.Modulator.Attack);
      check_get_property(J, -1, "Decay", &inst.Modulator.Decay);
      check_get_property(J, -1, "Sustain", &inst.Modulator.Sustain);
      check_get_property(J, -1, "Release", &inst.Modulator.Release);
      check_get_property(J, -1, "Vibrato", &inst.Modulator.Vibrato);
      check_get_property(J, -1, "Tremolo", &inst.Modulator.Tremolo);
      check_get_property(J, -1, "EGType", &inst.Modulator.EGType);
      check_get_property(J, -1, "KSL", &inst.Modulator.KSL);
      check_get_property(J, -1, "TotalLevel", &inst.Modulator.TotalLevel);
    }
    js_pop(J, 1);
    valid &= check_get_object(J, 2, "Carrier");
    {
      check_get_property(J, -1, "Attack", &inst.Carrier.Attack);
      check_get_property(J, -1, "Decay", &inst.Carrier.Decay);
      check_get_property(J, -1, "Sustain", &inst.Carrier.Sustain);
      check_get_property(J, -1, "Release", &inst.Carrier.Release);
      check_get_property(J, -1, "Vibrato", &inst.Carrier.Vibrato);
      check_get_property(J, -1, "Tremolo", &inst.Carrier.Tremolo);
      check_get_property(J, -1, "EGType", &inst.Carrier.EGType);
      check_get_property(J, -1, "KSL", &inst.Carrier.KSL);
      check_get_property(J, -1, "TotalLevel", &inst.Carrier.TotalLevel);
    }
    js_pop(J, 1);
    check_get_property(J, 2, "Feedback", &inst.Feedback);
    check_get_property(J, 2, "SynType", &inst.SynType);

    if (valid) {
      SetInstrument(voice, inst);
    }
  }
}

/**
 * @brief start playing a note.
 * NoteOn(voice:number, note:number, octave: number)
 *
 * @param J VM state.
 */
static void fm_NoteOn(js_State *J) {
  if (synth_available) {
    unsigned int voice = js_toint16(J, 1);
    unsigned int note = js_toint16(J, 2);
    unsigned int block = js_toint16(J, 3);

    NoteOn(voice, note, block);
  }
}

/**
 * @brief stop playing a note.
 * NoteOff(voice:number, note:number, octave: number)
 *
 * @param J VM state.
 */
static void fm_NoteOff(js_State *J) {
  if (synth_available) {
    unsigned int voice = js_toint16(J, 1);
    unsigned int note = js_toint16(J, 2);
    unsigned int block = js_toint16(J, 3);

    NoteOff(voice, note, block);
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
bool init_fmmusic(js_State *J) {
  // create global functions
  FUNCDEF(J, fm_SetInstrument, "SetInstrument", 2);
  FUNCDEF(J, fm_NoteOn, "NoteOn", 3);
  FUNCDEF(J, fm_NoteOff, "NoteOff", 3);

  // Check to see if an FM sound card is present
  if (FMInstalled()) {
    DEBUG("AdLib/SB or compatible FM sound card found.\n");
    return true;
  } else {
    // If not, end the programme
    DEBUG("AdLib/SB or compatible FM sound card not found.\n");
    return false;
  }
}

/**
 * @brief shutdown fm music subsystem.
 */
void shutdown_fmmusic() {
  for (int v = 0; v < 9; v++) {
    NoteOff(v, 0, 0);
  }
}

#else
void init_fmmusic(js_State *J) {}
#endif // PLATFORM_UNIX
