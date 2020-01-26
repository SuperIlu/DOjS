/*
** THIS SOFTWARE IS SUBJECT TO COPYRIGHT PROTECTION AND IS OFFERED ONLY
** PURSUANT TO THE 3DFX GLIDE GENERAL PUBLIC LICENSE. THERE IS NO RIGHT
** TO USE THE GLIDE TRADEMARK WITHOUT PRIOR WRITTEN PERMISSION OF 3DFX
** INTERACTIVE, INC. A COPY OF THIS LICENSE MAY BE OBTAINED FROM THE 
** DISTRIBUTOR OR BY CONTACTING 3DFX INTERACTIVE INC(info@3dfx.com). 
** THIS PROGRAM IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND, EITHER 
** EXPRESSED OR IMPLIED. SEE THE 3DFX GLIDE GENERAL PUBLIC LICENSE FOR A
** FULL TEXT OF THE NON-WARRANTY PROVISIONS.  
** 
** USE, DUPLICATION OR DISCLOSURE BY THE GOVERNMENT IS SUBJECT TO
** RESTRICTIONS AS SET FORTH IN SUBDIVISION (C)(1)(II) OF THE RIGHTS IN
** TECHNICAL DATA AND COMPUTER SOFTWARE CLAUSE AT DFARS 252.227-7013,
** AND/OR IN SIMILAR OR SUCCESSOR CLAUSES IN THE FAR, DOD OR NASA FAR
** SUPPLEMENT. UNPUBLISHED RIGHTS RESERVED UNDER THE COPYRIGHT LAWS OF
** THE UNITED STATES.  
** 
** COPYRIGHT 3DFX INTERACTIVE, INC. 1999, ALL RIGHTS RESERVED
*/
#ifndef __FX64_H__
#define __FX64_H__

#include "3dfx.h"

/*
** first figure out what compiler/os we are using and what is supported
*/

/*
** WATCOMC
*/
#if defined(__WATCOMC__)
#if (__WATCOMC__ < 1100)
   #define FX_BIT64(n) ( FX_SHL64( i64_one, n ) )
#else
   typedef signed long long FxI64;
   typedef unsigned long long FxU64;
   #define NATIVE_64_SUPPORT 1
   #define FX_MASK64(n) (0xFFFFFFFFFFFFFFFFLL >> (64-(n)))
   #define FX_BIT64(n) (((FxI64)1) << (n))
#endif
/*
** GCC
*/
#elif defined(__GNUC__)
   typedef signed long long FxI64;
   typedef unsigned long long FxU64;
   #define NATIVE_64_SUPPORT 1
   #define FX_MASK64(n) (0xFFFFFFFFFFFFFFFFLL >> (64-(n)))
   #define FX_BIT64(n) (((FxI64)1) << (n))
/*
** Microsoft C
*/
#elif defined(_MSC_VER)
   typedef signed __int64 FxI64;
   typedef unsigned __int64 FxU64;

   #define FX_MASK64(n) (0xFFFFFFFFFFFFFFFF >> (64-(n)))
   #define FX_BIT64(n) (((FxI64)1) << (n))
   #define NATIVE_64_SUPPORT 1
#endif

/*
** The following code is for systems with native 64-bit data types
*/
#ifdef NATIVE_64_SUPPORT

#define FX_ADD64( a, b )	 ((a) + (b))
#define FX_AND64( a, b )	 ((a) & (b))
#define FX_CMP64( a, b )	 error
#define FX_COMP64( a )		 (~(a))
#define FX_CREATE64( a, b ) ( (((FxI64) (a)) << 32) | (b) )
#define FX_COPY32( a, b )	 a = (b)
#define FX_EQ064( a )		 ( (a) == 0 )
#define FX_LO64(a)		    ((FxI32)(a))
#define FX_HI64(a)		    ((FxI32)FX_SHR64(a,32))
#define FX_MUL32( a, b )    (( FxI64 )(  ( FxI64 ) (a) * ( FxI64 ) (b) ) )
#define FX_MUL64( a, b )	 ((a) * (b))
#define FX_NEG64( a )		 (-(a))
#define FX_OR64( a, b )		 ((a) | (b))
#define FX_SET64( a, b, c ) a = (c) | (((FxI64)(b))<<32)
#define FX_SHL64( a, n )	 ((a) << (n))             // n < 64
#define FX_SHR64( a, n )	 ((a) >> (n))             // n < 64
#define FX_SUB64( a, b ) 	 ((a) - (b))
#define FX_XOR64( a, b )	 ((a) ^ (b))
#define FX_FLOATTO64( a )	 ((FxI64)(a))
#define FX_64TOFLOAT( a )	 (float)(a)
#define FX_64TODBL( a )		 (double)(a)

/*
** The following code is for Watcom C
*/
#else

typedef struct
{
  FxI32 hi;
  FxU32 lo;
} FxI64;

typedef struct
{
  FxU32 hi;
  FxU32 lo;
} FxU64;

extern FxI64 i64_one;
extern FxI64 i64_zero;

#define FX_ADD64( a, b )  ( __FX_ADD64( (a).hi, (a).lo, (b).hi, (b).lo ) )
#define FX_AND64( a, b )  ( __FX_AND64( a, b ) )
#define FX_CMP64( a, b )  ( __FX_CMP64( a, b ) )
#define FX_COMP64( a )	  ( __FX_COMP64( a ) )
#define FX_CREATE64( a, b ) ( __FX_CREATE64( a, b ) )
#define FX_COPY32( a, b ) ( a = __FX_32TO64( (b) ) )
#define FX_EQ064( a )		( ( (a).lo == 0 ) && ( (a).hi == 0 ) )
#define FX_HI64( a )      ( (a).hi )
#define FX_LO64( a )      ( (a).lo )
#define FX_MASK64( n )    ( __FX_MASK64( n ) )
#define FX_MUL32( a, b )  ( __FX_MUL32( (a), (b) ) )
#define FX_MUL64( a, b )  ( __FX_MUL64( (a), (b) ) )
#define FX_NEG64( a )     ( __FX_NEG64( (a).hi, (a).lo ) )
#define FX_OR64( a, b )   ( __FX_OR64( a, b ) )
#define FX_SET64( a, b, c ) do {(a).hi = b; (a).lo = c;} while(0)
#define FX_SHL64( a, n )  ( __FX_SHL64( (a).hi, (a).lo, n ) )
#define FX_SHR64( a, n )  ( __FX_SHR64( (a).hi, (a).lo, n ) )
#define FX_SUB64( a, b )  ( __FX_SUB64( (a).hi, (a).lo, (b).hi, (b).lo ) )
#define FX_FLOATTO64( a ) ( __FX_FLOATTO64((a)) )
#define FX_64TOFLOAT( a )   ( __FX_64TOFLOAT((a)) )

/*
** The following functions are implemented as inline assembly
*/
FxI64 __FX_ADD64( FxI32 a_hi, FxU32 a_lo, FxI32 b_hi, FxU32 b_lo );
FxI64 __FX_32TO64( FxI32 value );
FxI64 __FX_MASK64( int n );
FxI64 __FX_MUL32( FxI32 a, FxI32 b );
FxI64 __FX_NEG64( FxI32 hi, FxU32 lo );
FxI64 __FX_SHL64( FxI32 hi, FxU32 lo, int n );
FxI64 __FX_SHR64( FxI32 hi, FxU32 lo, int n );
FxI64 __FX_SUB64( FxI32 a_hi, FxU32 a_lo, FxI32 b_hi, FxU32 b_lo );

/*
** The following functions are implemented as C functions
*/
FxI64 __FX_AND64( FxI64 a, FxI64 b );
int   __FX_CMP64( FxI64 a, FxI64 b );
FxI64 __FX_COMP64( FxI64 a );
FxI64 __FX_CREATE64( FxI32 a, FxU32 b );
FxI64 __FX_FLOATTO64( float a );
FxI64 __FX_MUL64( FxI64 a, FxI64 b );
FxI64 __FX_OR64( FxI64 a, FxI64 b );
float __FX_64TOFLOAT( FxI64 );

#pragma aux __FX_32TO64 = \
   "cdq" \
   "mov [esi], edx" \
   "mov [esi+4], eax" \
   parm [eax] \
   modify [edx] \
   value struct struct;

#pragma aux __FX_ADD64 = \
   "add  eax, ecx" \
   "adc  edx, ebx" \
   "mov  [esi], edx" \
   "mov  [esi+4], eax" \
   parm [edx] [eax] [ebx] [ecx] \
   value struct struct;

#pragma aux __FX_MUL32 = \
   "imul  edx" \
   "mov   [esi], edx" \
   "mov   [esi+4], eax" \
   parm [edx] [eax] \
   value struct struct;

#pragma aux __FX_SUB64 = \
   "sub  eax, ecx" \
   "sbb  edx, ebx" \
   "mov  [esi], edx" \
   "mov  [esi+4], eax" \
   parm [edx] [eax] [ebx] [ecx] \
   value struct struct;

#pragma aux __FX_SHL64 = \
   "cmp  cl, 32" \
   "jl   SHL_LT32" \
   "sub  cl, 32" \
   "mov  edx, eax " \
   "mov  eax, 0 " \
   "shl  edx, cl" \
   "jmp SHL_DONE" \
   "SHL_LT32:" \
   "shld edx, eax, cl" \
   "sal  eax, cl" \
   "SHL_DONE:" \
   "mov  [esi], edx" \
   "mov  [esi+4], eax" \
   parm [edx] [eax] [cx] \
   value struct struct;

#pragma aux __FX_SHR64 = \
   "cmp  cl, 32" \
   "jl   SHR_LT32" \
   "sub  cl, 32 " \
   "mov  eax, edx" \
   "test edx, 080000000h" \
   "jz  SHR_GE32_POSITIVE" \
   "SHR_GE32_NEGATIVE:" \
   "mov  edx, 0FFFFFFFFh" \
   "or   eax, 080000000h" \
   "sar  eax, cl" \
   "jmp  SHR_DONE" \
   "SHR_GE32_POSITIVE:" \
   "mov  edx, 0" \
   "shr  eax, cl" \
   "jmp  SHR_DONE" \
   "SHR_LT32: " \
   "shrd eax, edx, cl" \
   "sar  edx, cl" \
   "SHR_DONE: " \
   "mov  [esi], edx" \
   "mov  [esi+4], eax" \
   parm [edx] [eax] [cx] \
   value struct struct;

#pragma aux __FX_NEG64 = \
   "neg eax" \
   "adc edx, 0" \
   "neg edx" \
   "mov [esi], edx" \
   "mov [esi+4], eax" \
   parm [edx] [eax] \
   value struct struct;

#pragma aux __FX_MASK64 = \
   "mov edx, 64" \
   "sub edx, ecx" \
   "mov ecx, edx" \
   "mov edx, 0ffffffffh" \
   "mov eax, 0ffffffffh" \
   "cmp  cl, 32" \
   "jl   M64_LT32" \
   "sub  cl, 32 " \
   "mov  eax, edx" \
   "mov  edx, 0" \
   "shr  eax, cl" \
   "jmp  M64_DONE" \
   "M64_LT32: " \
   "shrd eax, edx, cl" \
   "shr  edx, cl" \
   "M64_DONE: " \
   "mov  [esi], edx" \
   "mov  [esi+4], eax" \
   modify [edx eax] \
   parm [ecx] \
   value struct struct;

#endif /* __WATCOMC__ */

/*
** The following is shared between NATIVE64 and Watcom
*/

// a | -(a & (1 << n))  branchless, but may not be good implement for PC
// for constant n, does &, -, |
#define FX_SGNEXT64( a, n ) (FX_OR64((a), FX_NEG64(FX_AND64((a), FX_SHL64(FX_CREATE64(0,1), (n))))))

#endif /* __FX64_H__ */
