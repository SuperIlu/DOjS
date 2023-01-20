#ifndef __KEY_H
#define __KEY_H

#ifndef DWORD
#define DWORD unsigned long
#endif

extern DWORD       KeyGetKey    (void);
extern void        KeyUngetKey  (DWORD key);
extern int         KeyDriver    (const char* file);
extern const char *KeyTranslate (DWORD key);
extern const char *KeyName      (DWORD key);
extern int         KeyFilterPad (int on);
extern int         KeyGetFlag   (int flg);
extern void        KeySetFlag   (int flg, int val);

#define Key_ESC         0x1b          /* ASCII codes */
#define Key_ENTER       '\r'
#define Key_TAB         '\t'
#define Key_BACKSPC     '\b'
#define Key_NL          '\n'
#define Key_LFEED       '\n'
#define Key_FFEED       '\f'

#define Key_F1          0x13B         /* Function keys */
#define Key_F2          0x13C
#define Key_F3          0x13D
#define Key_F4          0x13E
#define Key_F5          0x13F
#define Key_F6          0x140
#define Key_F7          0x141
#define Key_F8          0x142
#define Key_F9          0x143
#define Key_F10         0x144
#define Key_F11         0x185
#define Key_F12         0x186

#define Key_CF1         0x15E         /* Ctrl-Function keys */
#define Key_CF2         0x15F
#define Key_CF3         0x160
#define Key_CF4         0x161
#define Key_CF5         0x162
#define Key_CF6         0x163
#define Key_CF7         0x164
#define Key_CF8         0x165
#define Key_CF9         0x166
#define Key_CF10        0x167
#define Key_CF11        0x189
#define Key_CF12        0x18A

#define Key_SF1         0x154         /* Shift-Function keys */
#define Key_SF2         0x155
#define Key_SF3         0x156
#define Key_SF4         0x157
#define Key_SF5         0x158
#define Key_SF6         0x159
#define Key_SF7         0x15A
#define Key_SF8         0x15B
#define Key_SF9         0x15C
#define Key_SF10        0x15D
#define Key_SF11        0x187
#define Key_SF12        0x188

#define Key_AF1         0x168         /* Alt-Function keys */
#define Key_AF2         0x169
#define Key_AF3         0x16A
#define Key_AF4         0x16B
#define Key_AF5         0x16C
#define Key_AF6         0x16D
#define Key_AF7         0x16E
#define Key_AF8         0x16F
#define Key_AF9         0x170
#define Key_AF10        0x171
#define Key_AF11        0x18B
#define Key_AF12        0x18C

#define Key_INS         0x152         /* Numeric pad keys */
#define Key_DEL         0x153
#define Key_HOME        0x147
#define Key_END         0x14F
#define Key_PGUP        0x149
#define Key_PGDN        0x151
#define Key_UPARROW     0x148
#define Key_DNARROW     0x150
#define Key_LTARROW     0x14B
#define Key_RTARROW     0x14D
#define Key_PADMIDDLE   0x14C

#define Key_CEND        0x175         /* Ctrl-Numeric pad keys */
#define Key_CDNARROW    0x191
#define Key_CPGDN       0x176
#define Key_CLTARROW    0x173
#define Key_CPADMIDDLE  0x18F
#define Key_CRTARROW    0x174
#define Key_CHOME       0x177
#define Key_CUPARROW    0x18D
#define Key_CPGUP       0x184
#define Key_CINS        0x192
#define Key_CDEL        0x193

#define Key_CPEND       0x275         /* Ctrl-Cursor pad keys */
#define Key_CPDNARROW   0x291
#define Key_CPPGDN      0x276
#define Key_CPLTARROW   0x273
#define Key_CPRTARROW   0x274
#define Key_CPHOME      0x277
#define Key_CPUPARROW   0x28D
#define Key_CPPGUP      0x284
#define Key_CPINS       0x292
#define Key_CPDEL       0x293

#define Key_PADEQ       0x03D         /* Numeric pad grey keys */
#define Key_PADASTERISK 0x22A
#define Key_PADPLUS     0x22B
#define Key_PADMINUS    0x22D
#define Key_PADSLASH    0x22F
#define Key_PADENTER    0x20D

#define Key_PINS        0x252         /* Cursor pad keys */
#define Key_PDEL        0x253
#define Key_PHOME       0x247
#define Key_PEND        0x24F
#define Key_PPGUP       0x249
#define Key_PPGDN       0x251
#define Key_PUPARROW    0x248
#define Key_PDNARROW    0x250
#define Key_PLTARROW    0x24B
#define Key_PRTARROW    0x24D

#define Key_ALTPSLASH   0x1A4         /* Alt-numeric pad grey keys */
#define Key_ALTPASTRSK  0x137
#define Key_ALTPMINUS   0x14A
#define Key_ALTPPLUS    0x14E
#define Key_ALTPEQUALS  0x183
#define Key_ALTPENTER   0x1A6

#define Key_ALTBACKSPC  0x10E         /* Special PC keyboard keys */
#define Key_CTRLBACKSPC 0x07F
#define Key_SHIFTTAB    0x10F
#define Key_CTRLTAB     0x194
#define Key_ALTESC      0x101

#define Key_ALT1        0x178         /* Alt keys - number row */
#define Key_ALT2        0x179
#define Key_ALT3        0x17A
#define Key_ALT4        0x17B
#define Key_ALT5        0x17C
#define Key_ALT6        0x17D
#define Key_ALT7        0x17E
#define Key_ALT8        0x17F
#define Key_ALT9        0x180
#define Key_ALT0        0x181
#define Key_ALTMINUS    0x182
#define Key_ALTEQUALS   0x183

#define Key_ALTQ        0x110         /* Alt keys - top alpha row */
#define Key_ALTW        0x111
#define Key_ALTE        0x112
#define Key_ALTR        0x113
#define Key_ALTT        0x114
#define Key_ALTY        0x115
#define Key_ALTU        0x116
#define Key_ALTI        0x117
#define Key_ALTO        0x118
#define Key_ALTP        0x119
#define Key_ALTLBRACE   0x11A
#define Key_ALTRBRACE   0x11B

#define Key_ALTA        0x11E         /* Alt keys - mid alpha row */
#define Key_ALTS        0x11F
#define Key_ALTD        0x120
#define Key_ALTF        0x121
#define Key_ALTG        0x122
#define Key_ALTH        0x123
#define Key_ALTJ        0x124
#define Key_ALTK        0x125
#define Key_ALTL        0x126
#define Key_ALTCOLON    0x127
#define Key_ALTQUOTE    0x128
#define Key_ALTENTER    0x11C

#define Key_ALTZ        0x12C         /* Alt keys - lower alpha row */
#define Key_ALTX        0x12D
#define Key_ALTC        0x12E
#define Key_ALTV        0x12F
#define Key_ALTB        0x130
#define Key_ALTN        0x131
#define Key_ALTM        0x132
#define Key_ALTCOMMA    0x133
#define Key_ALTPERIOD   0x134
#define Key_ALTSLASH    0x135
#define Key_ALTBSLASH   0x12B
#define Key_ALTTILDE    0x129

#define Key_CTRL_A      0x001
#define Key_CTRL_B      0x002
#define Key_CTRL_C      0x003
#define Key_CTRL_D      0x004
#define Key_CTRL_E      0x005
#define Key_CTRL_F      0x006
#define Key_CTRL_G      0x007
#define Key_CTRL_H      0x008
#define Key_CTRL_I      0x009
#define Key_CTRL_J      0x00A
#define Key_CTRL_K      0x00B
#define Key_CTRL_L      0x00C
#define Key_CTRL_M      0x00D
#define Key_CTRL_N      0x00E
#define Key_CTRL_O      0x00F
#define Key_CTRL_P      0x010
#define Key_CTRL_Q      0x011
#define Key_CTRL_R      0x012
#define Key_CTRL_S      0x013
#define Key_CTRL_T      0x014
#define Key_CTRL_U      0x015
#define Key_CTRL_V      0x016
#define Key_CTRL_W      0x017
#define Key_CTRL_X      0x018
#define Key_CTRL_Y      0x019
#define Key_CTRL_Z      0x01A

#endif
