/**
 ** memfill.h ---- inline assembly memory fill macros
 **                Watcom special version
 **
 ** Copyright (c) 1998 Hartmut Schirmer
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

#if defined(USE_WATCOM386_ASM) && defined(__386__)

/* ------------------------------------------------------------------------ */

extern void *_GR_rowfill_b_set(void *P, GR_int8u  V, unsigned C);
extern void *_GR_rowfill_w_set(void *P, GR_int16u V, unsigned C);
extern void *_GR_rowfill_l_set(void *P, GR_int32u V, unsigned C);

#pragma aux _GR_rowfill_b_set =                                              \
	       "cld"                                                         \
	       "rep stosb"                                                   \
	       parm [edi]/*=P*/ [al]/*=V*/  [ecx]/*=C*/                      \
	       value [edi];

#pragma aux _GR_rowfill_w_set =                                              \
	       "cld"                                                         \
	       "rep stosw"                                                   \
	       parm [edi]/*=P*/ [ax]/*=V*/  [ecx]/*=C*/                      \
	       value [edi];

#pragma aux _GR_rowfill_l_set =                                              \
	       "cld"                                                         \
	       "rep stosd"                                                   \
	       parm [edi]/*=P*/ [eax]/*=V*/  [ecx]/*=C*/                     \
	       value [edi];

/* ------------------------------------------------------------------------ */

extern void *_GR_colfill_b_set(void *P, unsigned SKIP, GR_int8u  V, unsigned C);
extern void *_GR_colfill_w_set(void *P, unsigned SKIP, GR_int16u V, unsigned C);
extern void *_GR_colfill_l_set(void *P, unsigned SKIP, GR_int32u V, unsigned C);

#pragma aux _GR_colfill_b_set =                                              \
	       "inc     ecx"                                                 \
	       "shr     ecx,1"                                               \
	       "jnc     short odd"                                           \
	       "jmp     short lop1"                                          \
	       /* align to 16byte boundary here */                           \
	"lop1:  mov     byte ptr [edi],al"                                   \
	       "add     edi,ebx"                                             \
	"odd:   mov     byte ptr [edi],al"                                   \
	       "add     edi,ebx"                                             \
	       "dec     ecx"                                                 \
	       "jnz     short lop1"                                          \
	 parm [edi]/*=P*/ [ebx]/*=SKIP*/ [al]/*=V*/ [ecx]/*=C*/              \
	 value [edi];

#pragma aux _GR_colfill_w_set =                                              \
	       "inc     ecx"                                                 \
	       "shr     ecx,1"                                               \
	       "jnc     short odd"                                           \
	       "jmp     short lop1"                                          \
	       /* align to 16byte boundary here */                           \
	"lop1:  mov     word ptr [edi],ax"                                   \
	       "add     edi,ebx"                                             \
	"odd:   mov     word ptr [edi],ax"                                   \
	       "add     edi,ebx"                                             \
	       "dec     ecx"                                                 \
	       "jnz     short lop1"                                          \
	 parm [edi]/*=P*/ [ebx]/*=SKIP*/ [ax]/*=V*/ [ecx]/*=C*/              \
	 value [edi];

#pragma aux _GR_colfill_l_set =                                              \
	       "inc     ecx"                                                 \
	       "shr     ecx,1"                                               \
	       "jnc     short odd"                                           \
	       "jmp     short lop1"                                          \
	       /* align to 16byte boundary here */                           \
	"lop1:  mov     dword ptr [edi],eax"                                 \
	       "add     edi,ebx"                                             \
	"odd:   mov     dword ptr [edi],eax"                                 \
	       "add     edi,ebx"                                             \
	       "dec     ecx"                                                 \
	       "jnz     short lop1"                                          \
	 parm [edi]/*=P*/ [ebx]/*=SKIP*/ [eax]/*=V*/ [ecx]/*=C*/             \
	 value [edi];

/* ------------------------------------------------------------------------ */

extern void *_GR_colfill_b_xor(void *P, unsigned SKIP, GR_int8u  V, unsigned C);
extern void *_GR_colfill_w_xor(void *P, unsigned SKIP, GR_int16u V, unsigned C);
extern void *_GR_colfill_l_xor(void *P, unsigned SKIP, GR_int32u V, unsigned C);

#pragma aux _GR_colfill_b_xor =                                              \
	       "inc     ecx"                                                 \
	       "shr     ecx,1"                                               \
	       "jnc     short odd"                                           \
	       "jmp     short lop1"                                          \
	       /* align to 16byte boundary here */                           \
	"lop1:  xor     byte ptr [edi],al"                                   \
	       "add     edi,ebx"                                             \
	"odd:   xor     byte ptr [edi],al"                                   \
	       "add     edi,ebx"                                             \
	       "dec     ecx"                                                 \
	       "jnz     short lop1"                                          \
	 parm [edi]/*=P*/ [ebx]/*=SKIP*/ [al]/*=V*/ [ecx]/*=C*/              \
	 value [edi];

#pragma aux _GR_colfill_w_xor =                                              \
	       "inc     ecx"                                                 \
	       "shr     ecx,1"                                               \
	       "jnc     short odd"                                           \
	       "jmp     short lop1"                                          \
	       /* align to 16byte boundary here */                           \
	"lop1:  xor     word ptr [edi],ax"                                   \
	       "add     edi,ebx"                                             \
	"odd:   xor     word ptr [edi],ax"                                   \
	       "add     edi,ebx"                                             \
	       "dec     ecx"                                                 \
	       "jnz     short lop1"                                          \
	 parm [edi]/*=P*/ [ebx]/*=SKIP*/ [ax]/*=V*/ [ecx]/*=C*/              \
	 value [edi];

#pragma aux _GR_colfill_l_xor =                                              \
	       "inc     ecx"                                                 \
	       "shr     ecx,1"                                               \
	       "jnc     short odd"                                           \
	       "jmp     short lop1"                                          \
	       /* align to 16byte boundary here */                           \
	"lop1:  xor     dword ptr [edi],eax"                                 \
	       "add     edi,ebx"                                             \
	"odd:   xor     dword ptr [edi],eax"                                 \
	       "add     edi,ebx"                                             \
	       "dec     ecx"                                                 \
	       "jnz     short lop1"                                          \
	 parm [edi]/*=P*/ [ebx]/*=SKIP*/ [eax]/*=V*/ [ecx]/*=C*/             \
	 value [edi];

/* ------------------------------------------------------------------------ */

extern void *_GR_colfill_b_or(void *P, unsigned SKIP, GR_int8u  V, unsigned C);
extern void *_GR_colfill_w_or(void *P, unsigned SKIP, GR_int16u V, unsigned C);
extern void *_GR_colfill_l_or(void *P, unsigned SKIP, GR_int32u V, unsigned C);

#pragma aux _GR_colfill_b_or  =                                              \
	       "inc     ecx"                                                 \
	       "shr     ecx,1"                                               \
	       "jnc     short odd"                                           \
	       "jmp     short lop1"                                          \
	       /* align to 16byte boundary here */                           \
	"lop1:  or      byte ptr [edi],al"                                   \
	       "add     edi,ebx"                                             \
	"odd:   or      byte ptr [edi],al"                                   \
	       "add     edi,ebx"                                             \
	       "dec     ecx"                                                 \
	       "jnz     short lop1"                                          \
	 parm [edi]/*=P*/ [ebx]/*=SKIP*/ [al]/*=V*/ [ecx]/*=C*/              \
	 value [edi];

#pragma aux _GR_colfill_w_or  =                                              \
	       "inc     ecx"                                                 \
	       "shr     ecx,1"                                               \
	       "jnc     short odd"                                           \
	       "jmp     short lop1"                                          \
	       /* align to 16byte boundary here */                           \
	"lop1:  or      word ptr [edi],ax"                                   \
	       "add     edi,ebx"                                             \
	"odd:   or      word ptr [edi],ax"                                   \
	       "add     edi,ebx"                                             \
	       "dec     ecx"                                                 \
	       "jnz     short lop1"                                          \
	 parm [edi]/*=P*/ [ebx]/*=SKIP*/ [ax]/*=V*/ [ecx]/*=C*/              \
	 value [edi];

#pragma aux _GR_colfill_l_or  =                                              \
	       "inc     ecx"                                                 \
	       "shr     ecx,1"                                               \
	       "jnc     short odd"                                           \
	       "jmp     short lop1"                                          \
	       /* align to 16byte boundary here */                           \
	"lop1:  or      dword ptr [edi],eax"                                 \
	       "add     edi,ebx"                                             \
	"odd:   or      dword ptr [edi],eax"                                 \
	       "add     edi,ebx"                                             \
	       "dec     ecx"                                                 \
	       "jnz     short lop1"                                          \
	 parm [edi]/*=P*/ [ebx]/*=SKIP*/ [eax]/*=V*/ [ecx]/*=C*/             \
	 value [edi];

/* ------------------------------------------------------------------------ */

extern void *_GR_colfill_b_and(void *P, unsigned SKIP, GR_int8u  V, unsigned C);
extern void *_GR_colfill_w_and(void *P, unsigned SKIP, GR_int16u V, unsigned C);
extern void *_GR_colfill_l_and(void *P, unsigned SKIP, GR_int32u V, unsigned C);

#pragma aux _GR_colfill_b_and =                                              \
	       "inc     ecx"                                                 \
	       "shr     ecx,1"                                               \
	       "jnc     short odd"                                           \
	       "jmp     short lop1"                                          \
	       /* align to 16byte boundary here */                           \
	"lop1:  and     byte ptr [edi],al"                                   \
	       "add     edi,ebx"                                             \
	"odd:   and     byte ptr [edi],al"                                   \
	       "add     edi,ebx"                                             \
	       "dec     ecx"                                                 \
	       "jnz     short lop1"                                          \
	 parm [edi]/*=P*/ [ebx]/*=SKIP*/ [al]/*=V*/ [ecx]/*=C*/              \
	 value [edi];

#pragma aux _GR_colfill_w_and =                                              \
	       "inc     ecx"                                                 \
	       "shr     ecx,1"                                               \
	       "jnc     short odd"                                           \
	       "jmp     short lop1"                                          \
	       /* align to 16byte boundary here */                           \
	"lop1:  and     word ptr [edi],ax"                                   \
	       "add     edi,ebx"                                             \
	"odd:   and     word ptr [edi],ax"                                   \
	       "add     edi,ebx"                                             \
	       "dec     ecx"                                                 \
	       "jnz     short lop1"                                          \
	 parm [edi]/*=P*/ [ebx]/*=SKIP*/ [ax]/*=V*/ [ecx]/*=C*/              \
	 value [edi];

#pragma aux _GR_colfill_l_and =                                              \
	       "inc     ecx"                                                 \
	       "shr     ecx,1"                                               \
	       "jnc     short odd"                                           \
	       "jmp     short lop1"                                          \
	       /* align to 16byte boundary here */                           \
	"lop1:  and     dword ptr [edi],eax"                                 \
	       "add     edi,ebx"                                             \
	"odd:   and     dword ptr [edi],eax"                                 \
	       "add     edi,ebx"                                             \
	       "dec     ecx"                                                 \
	       "jnz     short lop1"                                          \
	 parm [edi]/*=P*/ [ebx]/*=SKIP*/ [eax]/*=V*/ [ecx]/*=C*/             \
	 value [edi];

/* ------------------------------------------------------------------------ */

#define colfill_b(P,S,V,C)       do (P)=_GR_colfill_b_set(P,S,V,C); while (0)
#define colfill_w(P,S,V,C)       do (P)=_GR_colfill_w_set(P,S,V,C); while (0)
#define colfill_l(P,S,V,C)       do (P)=_GR_colfill_l_set(P,S,V,C); while (0)

#define colfill_b_xor(P,S,V,C)   do (P)=_GR_colfill_b_xor(P,S,V,C); while (0)
#define colfill_w_xor(P,S,V,C)   do (P)=_GR_colfill_w_xor(P,S,V,C); while (0)
#define colfill_l_xor(P,S,V,C)   do (P)=_GR_colfill_l_xor(P,S,V,C); while (0)

#define colfill_b_or(P,S,V,C)    do (P)=_GR_colfill_b_or(P,S,V,C);  while (0)
#define colfill_w_or(P,S,V,C)    do (P)=_GR_colfill_w_or(P,S,V,C);  while (0)
#define colfill_l_or(P,S,V,C)    do (P)=_GR_colfill_l_or(P,S,V,C);  while (0)

#define colfill_b_and(P,S,V,C)   do (P)=_GR_colfill_b_and(P,S,V,C); while (0)
#define colfill_w_and(P,S,V,C)   do (P)=_GR_colfill_w_and(P,S,V,C); while (0)
#define colfill_l_and(P,S,V,C)   do (P)=_GR_colfill_l_and(P,S,V,C); while (0)

/* ------------------------------------------------------------------------ */

#define rowfill_b(P,V,C)         do (P)=_GR_rowfill_b_set(P,V,C); while (0)
#define rowfill_w(P,V,C)         do (P)=_GR_rowfill_w_set(P,V,C); while (0)
#define rowfill_l(P,V,C)         do (P)=_GR_rowfill_l_set(P,V,C); while (0)

#define rowfill_b_xor(P,V,C)     colfill_b_xor(P,1,V,C)
#define rowfill_w_xor(P,V,C)     colfill_w_xor(P,2,V,C)
#define rowfill_l_xor(P,V,C)     colfill_l_xor(P,4,V,C)

#define rowfill_b_or(P,V,C)      colfill_b_or(P,1,V,C)
#define rowfill_w_or(P,V,C)      colfill_w_or(P,2,V,C)
#define rowfill_l_or(P,V,C)      colfill_l_or(P,4,V,C)

#define rowfill_b_and(P,V,C)     colfill_b_and(P,1,V,C)
#define rowfill_w_and(P,V,C)     colfill_w_and(P,2,V,C)
#define rowfill_l_and(P,V,C)     colfill_l_and(P,4,V,C)

/* ------------------------------------------------------------------------ */

#define colfill_b_f              colfill_b
#define colfill_w_f              colfill_w
#define colfill_l_f              colfill_l

#define colfill_b_f_xor          colfill_b_xor
#define colfill_w_f_xor          colfill_w_xor
#define colfill_l_f_xor          colfill_l_xor

#define colfill_b_f_or           colfill_b_or
#define colfill_w_f_or           colfill_w_or
#define colfill_l_f_or           colfill_l_or

#define colfill_b_f_and          colfill_b_and
#define colfill_w_f_and          colfill_w_and
#define colfill_l_f_and          colfill_l_and

/* ------------------------------------------------------------------------ */

#define rowfill_b_f              rowfill_b
#define rowfill_w_f              rowfill_w
#define rowfill_l_f              rowfill_l

#define rowfill_b_f_xor          rowfill_b_xor
#define rowfill_w_f_xor          rowfill_w_xor
#define rowfill_l_f_xor          rowfill_l_xor

#define rowfill_b_f_or           rowfill_b_or
#define rowfill_w_f_or           rowfill_w_or
#define rowfill_l_f_or           rowfill_l_or

#define rowfill_b_f_and          rowfill_b_and
#define rowfill_w_f_and          rowfill_w_and
#define rowfill_l_f_and          rowfill_l_and

#endif /* USE_WATCOM386_ASM && __386__ */
