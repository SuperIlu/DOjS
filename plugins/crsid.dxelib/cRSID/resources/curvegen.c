#include <stdio.h>
#include <math.h>

int main() {
    int i, Cutoff, Resonance, samplerate = 44100;
    float cutoff_ratio_8580 = -2 * 3.14 * (12500 / 2048) / samplerate;
    float rDS_VCR_FET, cutoff_steepness_6581, cap_6581_reciprocal;

    // generate resonance-multipliers

    // 8580
    printf("\n8580 Resonance-DAC (1/Q) curve:\n");
    for (i = 0; i <= 0xF; ++i) {
        Resonance = (pow(2, (4 - i) / 8.0)) * 0x1000;
        printf("0x%4.4X,", Resonance);
    }
    printf("\n");

    // 6581
    printf("\n6581 Resonance-DAC (1/Q) curve:\n");
    for (i = 0; i <= 0xF; ++i) {
        Resonance = (i > 5 ? 8.0 / i : 1.41) * 0x1000;
        printf("0x%4.4X,", Resonance);
    }
    printf("\n");

    // generate cutoff value curves

    // 8580
    printf("\n8580 Cutoff-curve (for %d samplerate):\n", samplerate);
    for (i = 0; i < 0x800; ++i) {
        Cutoff = (1 - exp((i + 2) * cutoff_ratio_8580)) * 0x1000;  // linear curve by resistor-ladder VCR (with a little leakage)
        printf("0x%3.3X,", Cutoff);
        if (i % 16 == 15) printf("\n");
    }

// 6581
#define VCR_SHUNT_6581 1500            // kOhm //cca 1.5 MOhm Rshunt across VCR FET drain and source (causing 220Hz bottom cutoff with 470pF integrator capacitors in old C64)
#define VCR_FET_TRESHOLD 192           // Vth (on cutoff numeric range 0..2048) for the VCR cutoff-frequency control FET below which it doesn't conduct
#define CAP_6581 0.470                 // nF //filter capacitor value for 6581
#define FILTER_DARKNESS_6581 22.0      // the bigger the value, the darker the filter control is (that is, cutoff frequency increases less with the same cutoff-value)
#define FILTER_DISTORTION_6581 0.0016  // the bigger the value the more of resistance-modulation (filter distortion) is applied for 6581 cutoff-control

    cap_6581_reciprocal = -1000000 / CAP_6581;
    cutoff_steepness_6581 = FILTER_DARKNESS_6581 * (2048.0 - VCR_FET_TRESHOLD);  // pre-scale for 0...2048 cutoff-value range

    printf("\n6581 Cutoff-curve: (for %d samplerate):\n", samplerate);
    for (i = 0; i < 0x800; ++i) {
        rDS_VCR_FET =
            i <= VCR_FET_TRESHOLD
                ? 100000000.0  // below Vth treshold Vgs control-voltage FET presents an open circuit
                : cutoff_steepness_6581 /
                      (i - VCR_FET_TRESHOLD);  // rDS ~ (-Vth*rDSon) / (Vgs-Vth)  //above Vth FET drain-source resistance is proportional to reciprocal of cutoff-control voltage

        Cutoff = (1 - exp(cap_6581_reciprocal / (VCR_SHUNT_6581 * rDS_VCR_FET / (VCR_SHUNT_6581 + rDS_VCR_FET)) / samplerate)) *
                 0x1000;  // curve with 1.5MOhm VCR parallel Rshunt emulation

        printf("0x%3.3X,", Cutoff);
        if (i % 16 == 15) printf("\n");
    }

    return 0;
}