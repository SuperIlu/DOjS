/**
 ** grxkeys.h ---- platform independent key definitions
 **
 ** Copyright (c) 1997 Hartmut Schirmer
 **
 ** This file is part of the GRX graphics library.
 **
 ** The GRX graphics library is free software; you can redistribute it
 ** and/or modify it under some conditions; see the "copying.grx" file
 ** for details.
 **
 ** This library is distributed in the hope that it will be useful,
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 **
 **/

#ifndef __GRKEYS_H_INCLUDED__
#define __GRKEYS_H_INCLUDED__

/*
** NOTES - some keys may not be available under all systems
**       - key values will be differ on different systems
*/

#ifndef __GRX20_H_INCLUDED__
#include <grx20.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* all keycodes should fit into 16 bit unsigned */
typedef unsigned short GrKeyType;

/* no key available */
#define GrKey_NoKey                0x0000

/* key typed but code outside 1..GrKey_LastDefinedKeycode */
#define GrKey_OutsideValidRange    0x0100

/* standard ASCII key codes */
#define GrKey_Control_A            0x0001
#define GrKey_Control_B            0x0002
#define GrKey_Control_C            0x0003
#define GrKey_Control_D            0x0004
#define GrKey_Control_E            0x0005
#define GrKey_Control_F            0x0006
#define GrKey_Control_G            0x0007
#define GrKey_Control_H            0x0008
#define GrKey_Control_I            0x0009
#define GrKey_Control_J            0x000a
#define GrKey_Control_K            0x000b
#define GrKey_Control_L            0x000c
#define GrKey_Control_M            0x000d
#define GrKey_Control_N            0x000e
#define GrKey_Control_O            0x000f
#define GrKey_Control_P            0x0010
#define GrKey_Control_Q            0x0011
#define GrKey_Control_R            0x0012
#define GrKey_Control_S            0x0013
#define GrKey_Control_T            0x0014
#define GrKey_Control_U            0x0015
#define GrKey_Control_V            0x0016
#define GrKey_Control_W            0x0017
#define GrKey_Control_X            0x0018
#define GrKey_Control_Y            0x0019
#define GrKey_Control_Z            0x001a
#define GrKey_Control_LBracket     0x001b
#define GrKey_Control_BackSlash    0x001c
#define GrKey_Control_RBracket     0x001d
#define GrKey_Control_Caret        0x001e
#define GrKey_Control_Underscore   0x001f
#define GrKey_Space                0x0020
#define GrKey_ExclamationPoint     0x0021
#define GrKey_DoubleQuote          0x0022
#define GrKey_Hash                 0x0023
#define GrKey_Dollar               0x0024
#define GrKey_Percent              0x0025
#define GrKey_Ampersand            0x0026
#define GrKey_Quote                0x0027
#define GrKey_LParen               0x0028
#define GrKey_RParen               0x0029
#define GrKey_Star                 0x002a
#define GrKey_Plus                 0x002b
#define GrKey_Comma                0x002c
#define GrKey_Dash                 0x002d
#define GrKey_Period               0x002e
#define GrKey_Slash                0x002f
#define GrKey_0                    0x0030
#define GrKey_1                    0x0031
#define GrKey_2                    0x0032
#define GrKey_3                    0x0033
#define GrKey_4                    0x0034
#define GrKey_5                    0x0035
#define GrKey_6                    0x0036
#define GrKey_7                    0x0037
#define GrKey_8                    0x0038
#define GrKey_9                    0x0039
#define GrKey_Colon                0x003a
#define GrKey_SemiColon            0x003b
#define GrKey_LAngle               0x003c
#define GrKey_Equals               0x003d
#define GrKey_RAngle               0x003e
#define GrKey_QuestionMark         0x003f
#define GrKey_At                   0x0040
#define GrKey_A                    0x0041
#define GrKey_B                    0x0042
#define GrKey_C                    0x0043
#define GrKey_D                    0x0044
#define GrKey_E                    0x0045
#define GrKey_F                    0x0046
#define GrKey_G                    0x0047
#define GrKey_H                    0x0048
#define GrKey_I                    0x0049
#define GrKey_J                    0x004a
#define GrKey_K                    0x004b
#define GrKey_L                    0x004c
#define GrKey_M                    0x004d
#define GrKey_N                    0x004e
#define GrKey_O                    0x004f
#define GrKey_P                    0x0050
#define GrKey_Q                    0x0051
#define GrKey_R                    0x0052
#define GrKey_S                    0x0053
#define GrKey_T                    0x0054
#define GrKey_U                    0x0055
#define GrKey_V                    0x0056
#define GrKey_W                    0x0057
#define GrKey_X                    0x0058
#define GrKey_Y                    0x0059
#define GrKey_Z                    0x005a
#define GrKey_LBracket             0x005b
#define GrKey_BackSlash            0x005c
#define GrKey_RBracket             0x005d
#define GrKey_Caret                0x005e
#define GrKey_UnderScore           0x005f
#define GrKey_BackQuote            0x0060
#define GrKey_a                    0x0061
#define GrKey_b                    0x0062
#define GrKey_c                    0x0063
#define GrKey_d                    0x0064
#define GrKey_e                    0x0065
#define GrKey_f                    0x0066
#define GrKey_g                    0x0067
#define GrKey_h                    0x0068
#define GrKey_i                    0x0069
#define GrKey_j                    0x006a
#define GrKey_k                    0x006b
#define GrKey_l                    0x006c
#define GrKey_m                    0x006d
#define GrKey_n                    0x006e
#define GrKey_o                    0x006f
#define GrKey_p                    0x0070
#define GrKey_q                    0x0071
#define GrKey_r                    0x0072
#define GrKey_s                    0x0073
#define GrKey_t                    0x0074
#define GrKey_u                    0x0075
#define GrKey_v                    0x0076
#define GrKey_w                    0x0077
#define GrKey_x                    0x0078
#define GrKey_y                    0x0079
#define GrKey_z                    0x007a
#define GrKey_LBrace               0x007b
#define GrKey_Pipe                 0x007c
#define GrKey_RBrace               0x007d
#define GrKey_Tilde                0x007e
#define GrKey_Control_Backspace    0x007f

/* extended key codes as defined in DJGPP */
#define GrKey_Alt_Escape           0x0101
#define GrKey_Control_At           0x0103
#define GrKey_Alt_Backspace        0x010e
#define GrKey_BackTab              0x010f
#define GrKey_Alt_Q                0x0110
#define GrKey_Alt_W                0x0111
#define GrKey_Alt_E                0x0112
#define GrKey_Alt_R                0x0113
#define GrKey_Alt_T                0x0114
#define GrKey_Alt_Y                0x0115
#define GrKey_Alt_U                0x0116
#define GrKey_Alt_I                0x0117
#define GrKey_Alt_O                0x0118
#define GrKey_Alt_P                0x0119
#define GrKey_Alt_LBracket         0x011a
#define GrKey_Alt_RBracket         0x011b
#define GrKey_Alt_Return           0x011c
#define GrKey_Alt_A                0x011e
#define GrKey_Alt_S                0x011f
#define GrKey_Alt_D                0x0120
#define GrKey_Alt_F                0x0121
#define GrKey_Alt_G                0x0122
#define GrKey_Alt_H                0x0123
#define GrKey_Alt_J                0x0124
#define GrKey_Alt_K                0x0125
#define GrKey_Alt_L                0x0126
#define GrKey_Alt_Semicolon        0x0127
#define GrKey_Alt_Quote            0x0128
#define GrKey_Alt_Backquote        0x0129
#define GrKey_Alt_Backslash        0x012b
#define GrKey_Alt_Z                0x012c
#define GrKey_Alt_X                0x012d
#define GrKey_Alt_C                0x012e
#define GrKey_Alt_V                0x012f
#define GrKey_Alt_B                0x0130
#define GrKey_Alt_N                0x0131
#define GrKey_Alt_M                0x0132
#define GrKey_Alt_Comma            0x0133
#define GrKey_Alt_Period           0x0134
#define GrKey_Alt_Slash            0x0135
#define GrKey_Alt_KPStar           0x0137
#define GrKey_F1                   0x013b
#define GrKey_F2                   0x013c
#define GrKey_F3                   0x013d
#define GrKey_F4                   0x013e
#define GrKey_F5                   0x013f
#define GrKey_F6                   0x0140
#define GrKey_F7                   0x0141
#define GrKey_F8                   0x0142
#define GrKey_F9                   0x0143
#define GrKey_F10                  0x0144
#define GrKey_Home                 0x0147
#define GrKey_Up                   0x0148
#define GrKey_PageUp               0x0149
#define GrKey_Alt_KPMinus          0x014a
#define GrKey_Left                 0x014b
#define GrKey_Center               0x014c
#define GrKey_Right                0x014d
#define GrKey_Alt_KPPlus           0x014e
#define GrKey_End                  0x014f
#define GrKey_Down                 0x0150
#define GrKey_PageDown             0x0151
#define GrKey_Insert               0x0152
#define GrKey_Delete               0x0153
#define GrKey_Shift_F1             0x0154
#define GrKey_Shift_F2             0x0155
#define GrKey_Shift_F3             0x0156
#define GrKey_Shift_F4             0x0157
#define GrKey_Shift_F5             0x0158
#define GrKey_Shift_F6             0x0159
#define GrKey_Shift_F7             0x015a
#define GrKey_Shift_F8             0x015b
#define GrKey_Shift_F9             0x015c
#define GrKey_Shift_F10            0x015d
#define GrKey_Control_F1           0x015e
#define GrKey_Control_F2           0x015f
#define GrKey_Control_F3           0x0160
#define GrKey_Control_F4           0x0161
#define GrKey_Control_F5           0x0162
#define GrKey_Control_F6           0x0163
#define GrKey_Control_F7           0x0164
#define GrKey_Control_F8           0x0165
#define GrKey_Control_F9           0x0166
#define GrKey_Control_F10          0x0167
#define GrKey_Alt_F1               0x0168
#define GrKey_Alt_F2               0x0169
#define GrKey_Alt_F3               0x016a
#define GrKey_Alt_F4               0x016b
#define GrKey_Alt_F5               0x016c
#define GrKey_Alt_F6               0x016d
#define GrKey_Alt_F7               0x016e
#define GrKey_Alt_F8               0x016f
#define GrKey_Alt_F9               0x0170
#define GrKey_Alt_F10              0x0171
#define GrKey_Control_Print        0x0172
#define GrKey_Control_Left         0x0173
#define GrKey_Control_Right        0x0174
#define GrKey_Control_End          0x0175
#define GrKey_Control_PageDown     0x0176
#define GrKey_Control_Home         0x0177
#define GrKey_Alt_1                0x0178
#define GrKey_Alt_2                0x0179
#define GrKey_Alt_3                0x017a
#define GrKey_Alt_4                0x017b
#define GrKey_Alt_5                0x017c
#define GrKey_Alt_6                0x017d
#define GrKey_Alt_7                0x017e
#define GrKey_Alt_8                0x017f
#define GrKey_Alt_9                0x0180
#define GrKey_Alt_0                0x0181
#define GrKey_Alt_Dash             0x0182
#define GrKey_Alt_Equals           0x0183
#define GrKey_Control_PageUp       0x0184
#define GrKey_F11                  0x0185
#define GrKey_F12                  0x0186
#define GrKey_Shift_F11            0x0187
#define GrKey_Shift_F12            0x0188
#define GrKey_Control_F11          0x0189
#define GrKey_Control_F12          0x018a
#define GrKey_Alt_F11              0x018b
#define GrKey_Alt_F12              0x018c
#define GrKey_Control_Up           0x018d
#define GrKey_Control_KPDash       0x018e
#define GrKey_Control_Center       0x018f
#define GrKey_Control_KPPlus       0x0190
#define GrKey_Control_Down         0x0191
#define GrKey_Control_Insert       0x0192
#define GrKey_Control_Delete       0x0193
#define GrKey_Control_Tab          0x0194
#define GrKey_Control_KPSlash      0x0195
#define GrKey_Control_KPStar       0x0196
#define GrKey_Alt_KPSlash          0x01a4
#define GrKey_Alt_Tab              0x01a5
#define GrKey_Alt_Enter            0x01a6

/* some additional codes not in DJGPP */
#define GrKey_Alt_LAngle           0x01b0
#define GrKey_Alt_RAngle           0x01b1
#define GrKey_Alt_At               0x01b2
#define GrKey_Alt_LBrace           0x01b3
#define GrKey_Alt_Pipe             0x01b4
#define GrKey_Alt_RBrace           0x01b5
#define GrKey_Print                0x01b6
#define GrKey_Shift_Insert         0x01b7
#define GrKey_Shift_Home           0x01b8
#define GrKey_Shift_End            0x01b9
#define GrKey_Shift_PageUp         0x01ba
#define GrKey_Shift_PageDown       0x01bb
#define GrKey_Alt_Up               0x01bc
#define GrKey_Alt_Left             0x01bd
#define GrKey_Alt_Center           0x01be
#define GrKey_Alt_Right            0x01c0
#define GrKey_Alt_Down             0x01c1
#define GrKey_Alt_Insert           0x01c2
#define GrKey_Alt_Delete           0x01c3
#define GrKey_Alt_Home             0x01c4
#define GrKey_Alt_End              0x01c5
#define GrKey_Alt_PageUp           0x01c6
#define GrKey_Alt_PageDown         0x01c7
#define GrKey_Shift_Up             0x01c8
#define GrKey_Shift_Down           0x01c9
#define GrKey_Shift_Right          0x01ca
#define GrKey_Shift_Left           0x01cb

/* this may be usefull for table allocation ... */
#define GrKey_LastDefinedKeycode   GrKey_Shift_Left

/* some well known synomyms */
#define GrKey_BackSpace            GrKey_Control_H
#define GrKey_Tab                  GrKey_Control_I
#define GrKey_LineFeed             GrKey_Control_J
#define GrKey_Escape               GrKey_Control_LBracket
#define GrKey_Return               GrKey_Control_M

/*
** new functions to replace the old style
**   kbhit / getch / getkey / getxkey / getkbstat
** keyboard interface
*/
extern int GrKeyPressed(void);
extern GrKeyType GrKeyRead(void);
extern int GrKeyStat(void);

/* some compatibility interface here ?? */
/* eg., #define kbhit() GrKeyPressed() ?? */

#ifdef __cplusplus
}
#endif

#endif /* whole file */
