/*
** Licensed under Attribution 4.0 International (CC BY 4.0)
** https://creativecommons.org/licenses/by/4.0/
**
** Code was taken from http://www.shdon.com/dos/sound
** by Steven Don.
** This is a derived/modified version by Andre Seidelt <superilu@yahoo.com>
*/


/**
 * Intrument and frequency definition for FM sound system.
 * @namespace FMMUSIC
 * @property {*} Notes.C note definition.
 * @property {*} Notes.CSharp note definition.
 * @property {*} Notes.D note definition.
 * @property {*} Notes.DSharp note definition.
 * @property {*} Notes.E note definition.
 * @property {*} Notes.F note definition.
 * @property {*} Notes.FSharp note definition.
 * @property {*} Notes.G note definition.
 * @property {*} Notes.GSharp note definition.
 * @property {*} Notes.A note definition.
 * @property {*} Notes.ASharp note definition.
 * 
 * @property {*} Harp a harp like instrument
 * @property {*} Piano a piano like instrument
 * @property {*} Flute a flute like instrument
 */
FMMUSIC = {
    Notes: {
        C: 343,
        CSharp: 363,
        D: 385,
        DSharp: 408,
        E: 432,
        F: 458,
        FSharp: 485,
        G: 514,
        GSharp: 544,
        A: 577,
        ASharp: 611,
        B: 647
    },
    Harp: {
        Modulator: {
            Attack: 15,
            Decay: 5,
            Sustain: 8,
            Release: 5,
            Multi: 2,
            TotalLevel: 41
        },
        Carrier: {
            Attack: 15,
            Decay: 2,
            Release: 3,
            Multi: 1,
            KSL: 2,
            TotalLevel: 3
        }
    },
    Piano: {
        Modulator: {
            Attack: 15,
            Decay: 1,
            Sustain: 10,
            Release: 3,
            Multi: 1,
            KSL: 1,
            TotalLevel: 16
        },
        Carrier: {
            Attack: 13,
            Decay: 2,
            Sustain: 8,
            Release: 4,
            Multi: 1,
            TotalLevel: 0,
            KSR: 1
        },
        Feedback: 3
    },
    Flute: {
        Modulator: {
            Attack: 6,
            Decay: 14,
            Sustain: 7,
            Release: 15,
            Vibrato: 1,
            Tremolo: 1,
            EGType: 1,
            KSL: 3,
            TotalLevel: 44
        },
        Carrier: {
            Attack: 6,
            Decay: 5,
            Sustain: 13,
            Release: 10,
            Vibrato: 1,
            EGType: 1,
            Multi: 1,
        },
        Feedback: 7,
        SynType: 3
    }
};
