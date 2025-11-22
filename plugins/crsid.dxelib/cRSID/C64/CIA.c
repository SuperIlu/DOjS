// static done

// cRSID CIA emulation

void cRSID_createCIAchip(cRSID_C64instance* C64, cRSID_CIAinstance* CIA, unsigned short baseaddress) {
    CIA->C64 = C64;
    CIA->ChipModel = 0;
    CIA->BaseAddress = baseaddress;
    CIA->BasePtrWR = &C64->IObankWR[baseaddress];
    CIA->BasePtrRD = &C64->IObankRD[baseaddress];
    cRSID_initCIAchip(CIA);
}

void cRSID_initCIAchip(cRSID_CIAinstance* CIA) {
    unsigned char i;
    for (i = 0; i < 0x10; ++i) CIA->BasePtrWR[i] = CIA->BasePtrRD[i] = 0x00;
}

static inline char cRSID_emulateCIA(cRSID_CIAinstance* CIA, char cycles) {
    int Tmp;

    enum CIAregisters {
        PORTA = 0,
        PORTB = 1,
        DDRA = 2,
        DDRB = 3,
        TIMERAL = 4,
        TIMERAH = 5,
        TIMERBL = 6,
        TIMERBH = 7,  // Write:Set Timer-latch, Read: read Timer
        TOD_TENTHSECONDS = 8,
        TOD_SECONDS = 9,
        TOD_MINUTES = 0xA,
        TOD_HOURS = 0xB,
        SERIAL_DATA = 0xC,
        INTERRUPTS = 0xD,
        CONTROLA = 0xE,
        CONTROLB = 0xF
    };

    enum InterruptBitVal {
        INTERRUPT_HAPPENED = 0x80,
        SET_OR_CLEAR_FLAGS = 0x80,  //(Read or Write operation determines which one:)
        FLAGn = 0x10,
        SERIALPORT = 0x08,
        ALARM = 0x04,
        TIMERB = 0x02,
        TIMERA = 0x01
    };  // flags/masks of interrupt-sources

    enum ControlAbitVal {
        ENABLE_TIMERA = 0x01,
        PORTB6_TIMERA = 0x02,
        TOGGLED_PORTB6 = 0x04,
        ONESHOT_TIMERA = 0x08,
        FORCELOADA_STROBE = 0x10,
        TIMERA_FROM_CNT = 0x20,
        SERIALPORT_IS_OUTPUT = 0x40,
        TIMEOFDAY_50Hz = 0x80
    };

    enum ControlBbitVal {
        ENABLE_TIMERB = 0x01,
        PORTB7_TIMERB = 0x02,
        TOGGLED_PORTB7 = 0x04,
        ONESHOT_TIMERB = 0x08,
        FORCELOADB_STROBE = 0x10,
        TIMERB_FROM_CPUCLK = 0x00,
        TIMERB_FROM_CNT = 0x20,
        TIMERB_FROM_TIMERA = 0x40,
        TIMERB_FROM_TIMERA_AND_CNT = 0x60,
        TIMEOFDAY_WRITE_SETS_ALARM = 0x80
    };

    // TimerA
    if (CIA->BasePtrWR[CONTROLA] & FORCELOADA_STROBE) {  // force latch into counter (strobe-input)
        CIA->BasePtrRD[TIMERAH] = CIA->BasePtrWR[TIMERAH];
        CIA->BasePtrRD[TIMERAL] = CIA->BasePtrWR[TIMERAL];
    } else if ((CIA->BasePtrWR[CONTROLA] & (ENABLE_TIMERA | TIMERA_FROM_CNT)) == ENABLE_TIMERA) {       // Enabled, counts Phi2
        Tmp = ((CIA->BasePtrRD[TIMERAH] << 8) + CIA->BasePtrRD[TIMERAL]) - cycles;                      // count timer
        if (Tmp <= 0) {                                                                                 // Timer counted down
            Tmp += (CIA->BasePtrWR[TIMERAH] << 8) + CIA->BasePtrWR[TIMERAL];                            // reload timer
            if (CIA->BasePtrWR[CONTROLA] & ONESHOT_TIMERA) CIA->BasePtrWR[CONTROLA] &= ~ENABLE_TIMERA;  // disable if one-shot
            CIA->BasePtrRD[INTERRUPTS] |= TIMERA;
            if (CIA->BasePtrWR[INTERRUPTS] & TIMERA) {  // generate interrupt if mask allows
                CIA->BasePtrRD[INTERRUPTS] |= INTERRUPT_HAPPENED;
            }
        }
        CIA->BasePtrRD[TIMERAH] = (Tmp >> 8);
        CIA->BasePtrRD[TIMERAL] = Tmp & 0xFF;
    }
    CIA->BasePtrWR[CONTROLA] &= ~FORCELOADA_STROBE;       // strobe is edge-sensitive
    CIA->BasePtrRD[CONTROLA] = CIA->BasePtrWR[CONTROLA];  // control-registers are readable

    // TimerB
    if (CIA->BasePtrWR[CONTROLB] & FORCELOADB_STROBE) {  // force latch into counter (strobe-input)
        CIA->BasePtrRD[TIMERBH] = CIA->BasePtrWR[TIMERBH];
        CIA->BasePtrRD[TIMERBL] = CIA->BasePtrWR[TIMERBL];
    }  // what about clocking TimerB by TimerA? (maybe not used in any music)
    else if ((CIA->BasePtrWR[CONTROLB] & (ENABLE_TIMERB | TIMERB_FROM_TIMERA)) == ENABLE_TIMERB) {      // Enabled, counts Phi2
        Tmp = ((CIA->BasePtrRD[TIMERBH] << 8) + CIA->BasePtrRD[TIMERBL]) - cycles;                      // count timer
        if (Tmp <= 0) {                                                                                 // Timer counted down
            Tmp += (CIA->BasePtrWR[TIMERBH] << 8) + CIA->BasePtrWR[TIMERBL];                            // reload timer
            if (CIA->BasePtrWR[CONTROLB] & ONESHOT_TIMERB) CIA->BasePtrWR[CONTROLB] &= ~ENABLE_TIMERB;  // disable if one-shot
            CIA->BasePtrRD[INTERRUPTS] |= TIMERB;
            if (CIA->BasePtrWR[INTERRUPTS] & TIMERB) {  // generate interrupt if mask allows
                CIA->BasePtrRD[INTERRUPTS] |= INTERRUPT_HAPPENED;
            }
        }
        CIA->BasePtrRD[TIMERBH] = (Tmp >> 8);
        CIA->BasePtrRD[TIMERBL] = Tmp & 0xFF;
    }
    CIA->BasePtrWR[CONTROLB] &= ~FORCELOADB_STROBE;       // strobe is edge-sensitive
    CIA->BasePtrRD[CONTROLB] = CIA->BasePtrWR[CONTROLB];  // control-registers are readable

    return (CIA->BasePtrRD[INTERRUPTS] & INTERRUPT_HAPPENED);
}

static inline void cRSID_writeCIAIRQmask(cRSID_CIAinstance* CIA, unsigned char value) {
    if (value & 0x80)
        CIA->BasePtrWR[0xD] |= (value & 0x1F);
    else
        CIA->BasePtrWR[0xD] &= ~(value & 0x1F);
}

static inline void cRSID_acknowledgeCIAIRQ(cRSID_CIAinstance* CIA) {
    CIA->BasePtrRD[0xD] = 0x00;  // reading a CIA interrupt-register clears its read-part and IRQ-flag
}
