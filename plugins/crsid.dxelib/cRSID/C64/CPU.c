// static done

// cRSID CPU-emulation

void cRSID_initCPU(cRSID_CPUinstance* CPU, unsigned short mempos) {
    CPU->PC = mempos;
    CPU->A = 0;
    CPU->X = 0;
    CPU->Y = 0;
    CPU->ST = 0x04;
    CPU->SP = 0xFF;
    CPU->PrevNMI = 0;
}

unsigned char cRSID_emulateCPU(cRSID_C64instance* C64) {  // the CPU emulation for SID/PRG playback (ToDo: CIA/VIC-IRQ/NMI/RESET vectors, BCD-mode)

    enum StatusFlagBitValues { N = 0x80, V = 0x40, B = 0x10, D = 0x08, I = 0x04, Z = 0x02, C = 0x01 };

    const unsigned char FlagSwitches[] = {0x01, 0x21, 0x04, 0x24, 0x00, 0x40, 0x08, 0x28}, BranchFlags[] = {0x80, 0x40, 0x01, 0x02};

    cRSID_CPUinstance* const CPU = &C64->CPU;

    char Cycles, SamePage;
    unsigned char IR, ST, X, Y;
    short int A, SP, T;
    unsigned int PC, PrevPC;

    static unsigned int Addr;

    inline void loadReg(void) {
        PC = CPU->PC;
        SP = CPU->SP;
        ST = CPU->ST;
        A = CPU->A;
        X = CPU->X;
        Y = CPU->Y;
    }
    inline void storeReg(void) {
        CPU->PC = PC;
        CPU->SP = SP;
        CPU->ST = ST;
        CPU->A = A;
        CPU->X = X;
        CPU->Y = Y;
    }

    inline unsigned char rd(register unsigned short address) {
        unsigned char value;
        value = *cRSID_getMemReadPtr(C64, address);
        if (C64->RealSIDmode) {
            if (C64->RAMbank[1] & 3) {
                if (address == 0xDC0D) {
                    cRSID_acknowledgeCIAIRQ(&C64->CIA[1]);
                } else if (address == 0xDD0D) {
                    cRSID_acknowledgeCIAIRQ(&C64->CIA[2]);
                }
            }
        }
        return value;
    }

    inline void wr(register unsigned short address, register unsigned char data) {
        *cRSID_getMemWritePtr(C64, address) = data;
        if (C64->RealSIDmode && (C64->RAMbank[1] & 3)) {
            // if(data&1) { //only writing 1 to $d019 bit0 would acknowledge, not any value (but RMW instructions write $d019 back before mod.)
            if (address == 0xD019) {
                cRSID_acknowledgeVICrasterIRQ(&C64->VIC);
            }
            //}
        }
    }

    inline void wr2(register unsigned short address, register unsigned char data) {  // PSID-hack specific memory-write
        *cRSID_getMemWritePtr(C64, address) = data;
        if (C64->RAMbank[1] & 3) {
            if (C64->RealSIDmode) {
                if (address == 0xDC0D)
                    cRSID_writeCIAIRQmask(&C64->CIA[1], data);
                else if (address == 0xDD0D)
                    cRSID_writeCIAIRQmask(&C64->CIA[2], data);
                else if (address == 0xDD0C)
                    C64->IObankRD[address] = data;  // mirror WR to RD (e.g. Wonderland_XIII_tune_1.sid)
                // #ifdef CRSID_PLATFORM_PC //just for info displayer
                //  else if (address==0xDC05 || address==0xDC04) C64->FrameCycles = ( (C64->IObankWR[0xDC04] + (C64->IObankWR[0xDC05]<<8)) );
                // #endif
                else if (address == 0xD019 && data & 1) {  // only writing 1 to $d019 bit0 would acknowledge
                    cRSID_acknowledgeVICrasterIRQ(&C64->VIC);
                }
            }

            else {
                switch (address) {
                    case 0xDC05:
                    case 0xDC04:
                        if (C64->TimerSource) {                                                           // dynamic CIA-setting (Galway/Rubicon workaround)
                            C64->FrameCycles = ((C64->IObankWR[0xDC04] + (C64->IObankWR[0xDC05] << 8)));  //<< 4) / C64->SampleClockRatio;
                        }
                        break;
                    case 0xDC08:
                        C64->IObankRD[0xDC08] = data;
                        break;  // refresh TOD-clock
                    case 0xDC09:
                        C64->IObankRD[0xDC09] = data;
                        break;                           // refresh TOD-clock
                    case 0xD012:                         // dynamic VIC IRQ-rasterline setting (Microprose Soccer V1 workaround)
                        if (C64->PrevRasterLine >= 0) {  // was $d012 set before? (or set only once?)
                            if (C64->IObankWR[0xD012] != C64->PrevRasterLine) {
                                int Tmp = C64->IObankWR[0xD012] - C64->PrevRasterLine;
                                if (Tmp < 0) Tmp += C64->VIC.RasterLines;
                                C64->FrameCycleCnt = C64->FrameCycles - Tmp * C64->VIC.RasterRowCycles;
                            }
                        }
                        C64->PrevRasterLine = C64->IObankWR[0xD012];
                        break;
                }
            }
        }
    }

    inline void addrModeImmediate(void) {
        ++PC;
        Addr = PC;
        Cycles = 2;
    }  // imm.
    inline void addrModeZeropage(void) {
        ++PC;
        Addr = rd(PC);
        Cycles = 3;
    }  // zp
    inline void addrModeAbsolute(void) {
        ++PC;
        Addr = rd(PC);
        ++PC;
        Addr += rd(PC) << 8;
        Cycles = 4;
    }  // abs
    inline void addrModeZeropageXindexed(void) {
        ++PC;
        Addr = (rd(PC) + X) & 0xFF;
        Cycles = 4;
    }  // zp,x (with zeropage-wraparound of 6502)
    inline void addrModeZeropageYindexed(void) {
        ++PC;
        Addr = (rd(PC) + Y) & 0xFF;
        Cycles = 4;
    }  // zp,y (with zeropage-wraparound of 6502)

    inline void addrModeXindexed(void) {  // abs,x (only STA is 5 cycles, others are 4 if page not crossed, RMW:7)
        ++PC;
        Addr = rd(PC) + X;
        ++PC;
        SamePage = (Addr <= 0xFF);
        Addr += rd(PC) << 8;
        Cycles = 5;
    }

    inline void addrModeYindexed(void) {  // abs,y (only STA is 5 cycles, others are 4 if page not crossed, RMW:7)
        ++PC;
        Addr = rd(PC) + Y;
        ++PC;
        SamePage = (Addr <= 0xFF);
        Addr += rd(PC) << 8;
        Cycles = 5;
    }

    inline void addrModeIndirectYindexed(void) {  // (zp),y (only STA is 6 cycles, others are 5 if page not crossed, RMW:8)
        ++PC;
        Addr = rd(rd(PC)) + Y;
        SamePage = (Addr <= 0xFF);
        Addr += rd((rd(PC) + 1) & 0xFF) << 8;
        Cycles = 6;
    }

    inline void addrModeXindexedIndirect(void) {  // (zp,x)
        ++PC;
        Addr = (rd(rd(PC) + X) & 0xFF) + ((rd(rd(PC) + X + 1) & 0xFF) << 8);
        Cycles = 6;
    }

    inline void clrC(void) { ST &= ~C; }  // clear Carry-flag
    inline void setC(unsigned char expr) {
        ST &= ~C;
        ST |= (expr != 0);
    }  // set Carry-flag if expression is not zero
    inline void clrNZC(void) { ST &= ~(N | Z | C); }       // clear flags
    inline void clrNVZC(void) { ST &= ~(N | V | Z | C); }  // clear flags
    inline void setNZbyA(void) {
        ST &= ~(N | Z);
        ST |= ((!A) << 1) | (A & N);
    }  // set Negative-flag and Zero-flag based on result in Accumulator
    inline void setNZbyT(void) {
        T &= 0xFF;
        ST &= ~(N | Z);
        ST |= ((!T) << 1) | (T & N);
    }
    inline void setNZbyX(void) {
        ST &= ~(N | Z);
        ST |= ((!X) << 1) | (X & N);
    }  // set Negative-flag and Zero-flag based on result in X-register
    inline void setNZbyY(void) {
        ST &= ~(N | Z);
        ST |= ((!Y) << 1) | (Y & N);
    }  // set Negative-flag and Zero-flag based on result in Y-register
    inline void setNZbyM(void) {
        ST &= ~(N | Z);
        ST |= ((!rd(Addr)) << 1) | (rd(Addr) & N);
    }  // set Negative-flag and Zero-flag based on result at Memory-Address
    inline void setNZCbyAdd(void) {
        ST &= ~(N | Z | C);
        ST |= (A & N) | (A > 255);
        A &= 0xFF;
        ST |= (!A) << 1;
    }  // after increase/addition
    inline void setVbyAdd(unsigned char M) {
        ST &= ~V;
        ST |= ((~(T ^ M)) & (T ^ A) & N) >> 1;
    }  // calculate V-flag from A and T (previous A) and input2 (Memory)
    inline void setNZCbySub(signed short* obj) {
        ST &= ~(N | Z | C);
        ST |= (*obj & N) | (*obj >= 0);
        *obj &= 0xFF;
        ST |= ((!(*obj)) << 1);
    }

    inline void push(unsigned char value) {
        C64->RAMbank[0x100 + SP] = value;
        --SP;
        SP &= 0xFF;
    }  // push a value to stack
    inline unsigned char pop(void) {
        ++SP;
        SP &= 0xFF;
        return C64->RAMbank[0x100 + SP];
    }  // pop a value from stack

    loadReg();
    PrevPC = PC;
    IR = rd(PC);
    Cycles = 2;
    SamePage = 0;  //'Cycles': ensure smallest 6510 runtime (for implied/register instructions)

    if (IR & 1) {  // nybble2:  1/5/9/D:accu.instructions, 3/7/B/F:illegal opcodes

        switch ((IR & 0x1F) >> 1) {  // value-forming to cause jump-table //PC wraparound not handled inside to save codespace
            case 0:
            case 1:
                addrModeXindexedIndirect();
                break;  //(zp,x)
            case 2:
            case 3:
                addrModeZeropage();
                break;
            case 4:
            case 5:
                addrModeImmediate();
                break;
            case 6:
            case 7:
                addrModeAbsolute();
                break;
            case 8:
            case 9:
                addrModeIndirectYindexed();
                break;  //(zp),y (5..6 cycles, 8 for R-M-W)
            case 0xA:
                addrModeZeropageXindexed();
                break;  // zp,x
            case 0xB:
                if ((IR & 0xC0) != 0x80)
                    addrModeZeropageXindexed();  // zp,x for illegal opcodes
                else
                    addrModeZeropageYindexed();  // zp,y for LAX/SAX illegal opcodes
                break;
            case 0xC:
            case 0xD:
                addrModeYindexed();
                break;
            case 0xE:
                addrModeXindexed();
                break;
            case 0xF:
                if ((IR & 0xC0) != 0x80)
                    addrModeXindexed();  // abs,x for illegal opcodes
                else
                    addrModeYindexed();  // abs,y for LAX/SAX illegal opcodes
                break;
        }
        Addr &= 0xFFFF;

        switch ((IR & 0xE0) >> 5) {  // value-forming to cause gapless case-values and faster jump-table creation from switch-case

            case 0:
                if ((IR & 0x1F) != 0xB) {  // ORA / SLO(ASO)=ASL+ORA
                    if ((IR & 3) == 3) {
                        clrNZC();
                        setC(rd(Addr) >= N);
                        wr(Addr, rd(Addr) << 1);
                        Cycles += 2;
                    }  // for SLO
                    else
                        Cycles -= SamePage;
                    A |= rd(Addr);
                    setNZbyA();  // ORA
                } else {
                    A &= rd(Addr);
                    setNZbyA();
                    setC(A >= N);
                }  // ANC (AND+Carry=bit7)
                break;

            case 1:
                if ((IR & 0x1F) != 0xB) {  // AND / RLA (ROL+AND)
                    if ((IR & 3) == 3) {   // for RLA
                        T = (rd(Addr) << 1) + (ST & C);
                        clrNZC();
                        setC(T > 255);
                        T &= 0xFF;
                        wr(Addr, T);
                        Cycles += 2;
                    } else
                        Cycles -= SamePage;
                    A &= rd(Addr);
                    setNZbyA();  // AND
                } else {
                    A &= rd(Addr);
                    setNZbyA();
                    setC(A >= N);
                }  // ANC (AND+Carry=bit7)
                break;

            case 2:
                if ((IR & 0x1F) != 0xB) {  // EOR / SRE(LSE)=LSR+EOR
                    if ((IR & 3) == 3) {
                        clrNZC();
                        setC(rd(Addr) & 1);
                        wr(Addr, rd(Addr) >> 1);
                        Cycles += 2;
                    }  // for SRE
                    else
                        Cycles -= SamePage;
                    A ^= rd(Addr);
                    setNZbyA();  // EOR
                } else {
                    A &= rd(Addr);
                    setC(A & 1);
                    A >>= 1;
                    A &= 0xFF;
                    setNZbyA();
                }  // ALR(ASR)=(AND+LSR)
                break;

            case 3:
                if ((IR & 0x1F) != 0xB) {  // RRA (ROR+ADC) / ADC
                    if ((IR & 3) == 3) {   // for RRA
                        T = (rd(Addr) >> 1) + ((ST & C) << 7);
                        clrNZC();
                        setC(T & 1);
                        wr(Addr, T);
                        Cycles += 2;
                    } else
                        Cycles -= SamePage;
                    T = A;
                    A += rd(Addr) + (ST & C);
                    setNZCbyAdd();
                    setVbyAdd(rd(Addr));  // ADC
                } else {                  // ARR (AND+ROR, bit0 not going to C, but C and bit7 get exchanged.)
                    A &= rd(Addr);
                    T += rd(Addr) + (ST & C);
                    clrNVZC();
                    setC(T > 255);
                    setVbyAdd(rd(Addr));  // V-flag set by intermediate ADC mechanism: (A&mem)+mem
                    T = A;
                    A = (A >> 1) + ((ST & C) << 7);
                    setC(T >= N);
                    setNZbyA();
                }
                break;

            case 4:
                if ((IR & 0x1F) == 0xB) {
                    A = X & rd(Addr);
                    setNZbyA();
                }  // XAA (TXA+AND), highly unstable on real 6502!
                else if ((IR & 0x1F) == 0x1B) {
                    SP = A & X;
                    wr(Addr, SP & ((Addr >> 8) + 1));
                }  // TAS(SHS) (SP=A&X, mem=S&H} - unstable on real 6502
                else {
                    wr2(Addr, A & (((IR & 3) == 3) ? X : 0xFF));
                }  // STA / SAX (at times same as AHX/SHX/SHY) (illegal)
                break;

            case 5:
                if ((IR & 0x1F) != 0x1B) {
                    A = rd(Addr);
                    if ((IR & 3) == 3) X = A;
                }  // LDA / LAX (illegal, used by my 1 rasterline player) (LAX #imm is unstable on C64)
                else {
                    A = X = SP = rd(Addr) & SP;
                }  // LAS(LAR)
                setNZbyA();
                Cycles -= SamePage;
                break;

            case 6:
                if ((IR & 0x1F) != 0xB) {  // CMP / DCP(DEC+CMP)
                    if ((IR & 3) == 3) {
                        wr(Addr, rd(Addr) - 1);
                        Cycles += 2;
                    }  // DCP
                    else
                        Cycles -= SamePage;
                    T = A - rd(Addr);
                } else {
                    X = T = (A & X) - rd(Addr);
                }  // SBX(AXS)  //SBX (AXS) (CMP+DEX at the same time)
                setNZCbySub(&T);
                break;

            case 7:
                if ((IR & 3) == 3 && (IR & 0x1F) != 0xB) {
                    wr(Addr, rd(Addr) + 1);
                    Cycles += 2;
                }  // ISC(ISB)=INC+SBC / SBC
                else
                    Cycles -= SamePage;
                T = A;
                A -= rd(Addr) + !(ST & C);
                setNZCbySub(&A);
                setVbyAdd(~rd(Addr));
                break;
        }
    }

    else if (IR & 2) {  // nybble2:  2:illegal/LDX, 6:A/X/INC/DEC, A:Accu-shift/reg.transfer/NOP, E:shift/X/INC/DEC

        switch (IR & 0x1F) {  // Addressing modes
            case 2:
                addrModeImmediate();
                break;
            case 6:
                addrModeZeropage();
                break;
            case 0xE:
                addrModeAbsolute();
                break;
            case 0x16:
                if ((IR & 0xC0) != 0x80)
                    addrModeZeropageXindexed();  // zp,x
                else
                    addrModeZeropageYindexed();  // zp,y
                break;
            case 0x1E:
                if ((IR & 0xC0) != 0x80)
                    addrModeXindexed();  // abs,x
                else
                    addrModeYindexed();  // abs,y
                break;
        }
        Addr &= 0xFFFF;

        switch ((IR & 0xE0) >> 5) {
            case 0:
                clrC();
            case 1:
                if ((IR & 0xF) == 0xA) {
                    A = (A << 1) + (ST & C);
                    setNZCbyAdd();
                }  // ASL/ROL (Accu)
                else {
                    T = (rd(Addr) << 1) + (ST & C);
                    setC(T > 255);
                    setNZbyT();
                    wr(Addr, T);
                    Cycles += 2;
                }  // RMW (Read-Write-Modify)
                break;

            case 2:
                clrC();
            case 3:
                if ((IR & 0xF) == 0xA) {
                    T = A;
                    A = (A >> 1) + ((ST & C) << 7);
                    setC(T & 1);
                    A &= 0xFF;
                    setNZbyA();
                }  // LSR/ROR (Accu)
                else {
                    T = (rd(Addr) >> 1) + ((ST & C) << 7);
                    setC(rd(Addr) & 1);
                    setNZbyT();
                    wr(Addr, T);
                    Cycles += 2;
                }  // memory (RMW)
                break;

            case 4:
                if (IR & 4) {
                    wr2(Addr, X);
                }  // STX
                else if (IR & 0x10)
                    SP = X;  // TXS
                else {
                    A = X;
                    setNZbyA();
                }  // TXA
                break;

            case 5:
                if ((IR & 0xF) != 0xA) {
                    X = rd(Addr);
                    Cycles -= SamePage;
                }  // LDX
                else if (IR & 0x10)
                    X = SP;  // TSX
                else
                    X = A;  // TAX
                setNZbyX();
                break;

            case 6:
                if (IR & 4) {
                    wr(Addr, rd(Addr) - 1);
                    setNZbyM();
                    Cycles += 2;
                }  // DEC
                else {
                    --X;
                    setNZbyX();
                }  // DEX
                break;

            case 7:
                if (IR & 4) {
                    wr(Addr, rd(Addr) + 1);
                    setNZbyM();
                    Cycles += 2;
                }  // INC/NOP
                break;
        }
    }

    else if ((IR & 0xC) == 8) {  // nybble2:  8:register/statusflag
        if (IR & 0x10) {
            if (IR == 0x98) {
                A = Y;
                setNZbyA();
            }  // TYA
            else {  // CLC/SEC/CLI/SEI/CLV/CLD/SED
                if (FlagSwitches[IR >> 5] & 0x20)
                    ST |= (FlagSwitches[IR >> 5] & 0xDF);
                else
                    ST &= ~(FlagSwitches[IR >> 5] & 0xDF);
            }
        } else {
            switch ((IR & 0xF0) >> 5) {
                case 0:
                    push(ST);
                    Cycles = 3;
                    break;  // PHP
                case 1:
                    ST = pop();
                    Cycles = 4;
                    break;  // PLP
                case 2:
                    push(A);
                    Cycles = 3;
                    break;  // PHA
                case 3:
                    A = pop();
                    setNZbyA();
                    Cycles = 4;
                    break;  // PLA
                case 4:
                    --Y;
                    setNZbyY();
                    break;  // DEY
                case 5:
                    Y = A;
                    setNZbyY();
                    break;  // TAY
                case 6:
                    ++Y;
                    setNZbyY();
                    break;  // INY
                case 7:
                    ++X;
                    setNZbyX();
                    break;  // INX
            }
        }
    }

    else {  // nybble2:  0: control/branch/Y/compare  4: Y/compare  C:Y/compare/JMP

        if ((IR & 0x1F) == 0x10) {  // BPL/BMI/BVC/BVS/BCC/BCS/BNE/BEQ  relative branch
            ++PC;
            T = rd(PC);
            if (T & 0x80) T -= 0x100;
            if (IR & 0x20) {
                if (ST & BranchFlags[IR >> 6]) {
                    PC += T;
                    Cycles = 3;
                }
            } else {
                if (!(ST & BranchFlags[IR >> 6])) {
                    PC += T;
                    Cycles = 3;
                }  // plus 1 cycle if page is crossed?
            }
        }

        else {                    // nybble2:  0:Y/control/Y/compare  4:Y/compare  C:Y/compare/JMP
            switch (IR & 0x1F) {  // Addressing modes
                case 0:
                    addrModeImmediate();
                    break;  // imm. (or abs.low for JSR/BRK)
                case 4:
                    addrModeZeropage();
                    break;
                case 0xC:
                    addrModeAbsolute();
                    break;
                case 0x14:
                    addrModeZeropageXindexed();
                    break;  // zp,x
                case 0x1C:
                    addrModeXindexed();
                    break;  // abs,x
            }
            Addr &= 0xFFFF;

            switch ((IR & 0xE0) >> 5) {
                case 0:
                    if (!(IR & 4)) {  // BRK / NOP-absolute/abs,x/zp/zp,x
                        push((PC + 2 - 1) >> 8);
                        push((PC + 2 - 1) & 0xFF);
                        push(ST | B);
                        ST |= I;  // BRK
                        PC = rd(0xFFFE) + (rd(0xFFFF) << 8) - 1;
                        Cycles = 7;
                    } else if (IR == 0x1C)
                        Cycles -= SamePage;  // NOP abs,x
                    break;

                case 1:
                    if (IR & 0xF) {  // BIT / NOP-abs,x/zp,x
                        if (!(IR & 0x10)) {
                            ST &= 0x3D;
                            ST |= (rd(Addr) & 0xC0) | ((!(A & rd(Addr))) << 1);
                        }  // BIT
                        else if (IR == 0x3C)
                            Cycles -= SamePage;  // NOP abs,x
                    } else {                     // JSR
                        push((PC + 2 - 1) >> 8);
                        push((PC + 2 - 1) & 0xFF);
                        PC = rd(Addr) + rd(Addr + 1) * 256 - 1;
                        Cycles = 6;
                    }
                    break;

                case 2:
                    if (IR & 0xF) {        // JMP / NOP-abs,x/zp/zp,x
                        if (IR == 0x4C) {  // JMP
                            PC = Addr - 1;
                            Cycles = 3;
                            rd(Addr + 1);  // a read from jump-address highbyte is used in some tunes with jmp DD0C (e.g. Wonderland_XIII_tune_1.sid)
                            // if (Addr==PrevPC) {storeReg(); C64->Returned=1; return 0xFF;} //turn self-jump mainloop (after init) into idle time
                        } else if (IR == 0x5C)
                            Cycles -= SamePage;  // NOP abs,x
                    } else {                     // RTI
                        ST = pop();
                        T = pop();
                        PC = (pop() << 8) + T - 1;
                        Cycles = 6;
                        if (C64->Returned && SP >= 0xFF) {
                            ++PC;
                            storeReg();
                            return 0xFE;
                        }
                    }
                    break;

                case 3:
                    if (IR & 0xF) {                                          // JMP() (indirect) / NOP-abs,x/zp/zp,x
                        if (IR == 0x6C) {                                    // JMP() (indirect)
                            PC = rd((Addr & 0xFF00) + ((Addr + 1) & 0xFF));  //(with highbyte-wraparound bug)
                            PC = (PC << 8) + rd(Addr) - 1;
                            Cycles = 5;
                        } else if (IR == 0x7C)
                            Cycles -= SamePage;  // NOP abs,x
                    } else {                     // RTS
                        if (SP >= 0xFF) {
                            storeReg();
                            C64->Returned = 1;
                            return 0xFF;
                        }  // Init returns, provide idle-time between IRQs
                        T = pop();
                        PC = (pop() << 8) + T;
                        Cycles = 6;
                    }
                    break;

                case 4:
                    if (IR & 4) {
                        wr2(Addr, Y);
                    }  // STY / NOP #imm
                    break;

                case 5:
                    Y = rd(Addr);
                    setNZbyY();
                    Cycles -= SamePage;  // LDY
                    break;

                case 6:
                    if (!(IR & 0x10)) {  // CPY / NOP abs,x/zp,x
                        T = Y - rd(Addr);
                        setNZCbySub(&T);  // CPY
                    } else if (IR == 0xDC)
                        Cycles -= SamePage;  // NOP abs,x
                    break;

                case 7:
                    if (!(IR & 0x10)) {  // CPX / NOP abs,x/zp,x
                        T = X - rd(Addr);
                        setNZCbySub(&T);  // CPX
                    } else if (IR == 0xFC)
                        Cycles -= SamePage;  // NOP abs,x
                    break;
            }
        }
    }

    ++PC;  // PC&=0xFFFF;

    storeReg();

    if (!C64->RealSIDmode) {  // substitute KERNAL IRQ-return in PSID (e.g. Microprose Soccer)
        if ((C64->RAMbank[1] & 3) > 1 && PrevPC < 0xE000 && (PC == 0xEA31 || PC == 0xEA81 || PC == 0xEA7E)) return 0xFE;
    }

    return Cycles;
}

// handle entering into IRQ and NMI interrupt
static inline char cRSID_handleCPUinterrupts(cRSID_C64instance* C64) {
    enum StatusFlagBitValues { B = 0x10, I = 0x04 };
    inline void push(unsigned char value) {
        C64->RAMbank[0x100 + C64->CPU.SP] = value;
        --C64->CPU.SP;
        C64->CPU.SP &= 0xFF;
    }  // push a value to stack

    if (C64->NMI > C64->CPU.PrevNMI) {  // if IRQ and NMI at the same time, NMI is serviced first
        push(C64->CPU.PC >> 8);
        push(C64->CPU.PC & 0xFF);
        push(C64->CPU.ST);
        C64->CPU.ST |= I;
        C64->CPU.PC = *cRSID_getMemReadPtr(C64, 0xFFFA) + (*cRSID_getMemReadPtr(C64, 0xFFFB) << 8);  // NMI-vector
        C64->CPU.PrevNMI = C64->NMI;
        return 1;
    } else if (C64->IRQ && !(C64->CPU.ST & I)) {
        push(C64->CPU.PC >> 8);
        push(C64->CPU.PC & 0xFF);
        push(C64->CPU.ST);
        C64->CPU.ST |= I;
        C64->CPU.PC = *cRSID_getMemReadPtr(C64, 0xFFFE) + (*cRSID_getMemReadPtr(C64, 0xFFFF) << 8);  // maskable IRQ-vector
        C64->CPU.PrevNMI = C64->NMI;
        return 1;
    }
    C64->CPU.PrevNMI = C64->NMI;  // prepare for NMI edge-detection

    return 0;
}
