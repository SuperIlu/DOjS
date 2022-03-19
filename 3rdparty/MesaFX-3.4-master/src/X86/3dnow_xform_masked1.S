/* $Id: 3dnow_xform_masked1.S,v 1.2.4.1 2000/10/22 23:10:51 gareth Exp $ */

/*
 * Mesa 3-D graphics library
 * Version:  3.4
 *
 * Copyright (C) 1999-2000  Brian Paul   All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * BRIAN PAUL BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
 * AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "assyntax.h"
#include "xform_args.h"

    SEG_TEXT

#define FRAME_OFFSET	16


ALIGNTEXT16
GLOBL GLNAME( gl_3dnow_transform_points1_general_masked )
GLNAME( gl_3dnow_transform_points1_general_masked ):

    PUSH_L    ( ESI )
    PUSH_L    ( EDI )
    PUSH_L    ( EBX )
    PUSH_L    ( EBP )

    MOV_L     ( ARG_DEST, ECX )
    MOV_L     ( ARG_MATRIX, ESI )
    MOV_L     ( ARG_SOURCE, EAX )
    MOV_L     ( CONST(4), REGOFF(V4F_SIZE, ECX) )
    OR_B      ( CONST(VEC_SIZE_4), REGOFF(V4F_FLAGS, ECX) )
    MOV_L     ( REGOFF(V4F_COUNT, EAX), EDX )
    MOV_L     ( EDX, REGOFF(V4F_COUNT, ECX) )

    MOV_L     ( REGOFF(V4F_START, ECX), EDX )
    MOV_L     ( ESI, ECX )
    MOV_L     ( REGOFF(V4F_COUNT, EAX), ESI )
    MOV_L     ( REGOFF(V4F_STRIDE, EAX), EDI )
    MOV_L     ( REGOFF(V4F_START, EAX), EAX )
    MOV_L     ( ARG_CLIP, EBP )
    MOV_B     ( ARG_FLAG, BL )

    MOVQ      ( REGIND(ECX), MM0 )	/* m01             | m00             */
    MOVQ      ( REGOFF(8, ECX), MM1 )	/* m03             | m02             */

    MOVQ      ( REGOFF(48, ECX), MM2 )	/* m31             | m30             */
    MOVQ      ( REGOFF(56, ECX), MM3 )	/* m33             | m32             */

    TEST_L    ( ESI, ESI )
    JZ        ( LLBL( G3TPGM_4 ) )

ALIGNTEXT16
LLBL( G3TPGM_2 ):

    TEST_B    ( BL, REGIND(EBP) )	/* mask [i] != clip flag ??          */
    JNZ       ( LLBL( G3TPGM_3 ) )	/* skip vertex                       */

    MOVD      ( REGIND(EAX), MM4 )	/*                 | x0              */
    PUNPCKLDQ ( MM4, MM4 )		/* x0              | x0              */

    MOVQ      ( MM4, MM5 )		/* x0              | x0              */
    PFMUL     ( MM0, MM4 )		/* x0*m01          | x0*m00          */

    PFMUL     ( MM1, MM5 )		/* x0*m03          | x0*m02          */
    PFADD     ( MM2, MM4 )		/* x0*m01+m31      | x0*m00+m30      */

    PFADD     ( MM3, MM5 )		/* x0*m03+m33      | x0*m02+m32      */
    MOVQ      ( MM4, REGIND(EDX) )	/* write r1, r0                      */

    MOVQ      ( MM5, REGOFF(8, EDX) )	/* write r3, r2                      */

LLBL( G3TPGM_3 ):

    ADD_L     ( EDI, EAX )		/* next vertex                       */

    INC_L     ( EBP )			/* next clipmask                     */
    ADD_L     ( CONST(16), EDX )	/* next r                            */

    DEC_L     ( ESI )			/* decrement vertex counter          */
    JNZ       ( LLBL( G3TPGM_2 ) )	/* cnt > 0 ? -> process next vertex  */

LLBL( G3TPGM_4 ):

    FEMMS
    POP_L     ( EBP )
    POP_L     ( EBX )
    POP_L     ( EDI )
    POP_L     ( ESI )
    RET




ALIGNTEXT16
GLOBL GLNAME( gl_3dnow_transform_points1_perspective_masked )
GLNAME( gl_3dnow_transform_points1_perspective_masked ):

    PUSH_L    ( ESI )
    PUSH_L    ( EDI )
    PUSH_L    ( EBX )
    PUSH_L    ( EBP )

    MOV_L     ( ARG_DEST, ECX )
    MOV_L     ( ARG_MATRIX, ESI )
    MOV_L     ( ARG_SOURCE, EAX )
    MOV_L     ( CONST(4), REGOFF(V4F_SIZE, ECX) )
    OR_B      ( CONST(VEC_SIZE_4), REGOFF(V4F_FLAGS, ECX) )
    MOV_L     ( REGOFF(V4F_COUNT, EAX), EDX )
    MOV_L     ( EDX, REGOFF(V4F_COUNT, ECX) )

    MOV_L     ( REGOFF(V4F_START, ECX), EDX )
    MOV_L     ( ESI, ECX )
    MOV_L     ( REGOFF(V4F_COUNT, EAX), ESI )
    MOV_L     ( REGOFF(V4F_STRIDE, EAX), EDI )
    MOV_L     ( REGOFF(V4F_START, EAX), EAX )
    MOV_L     ( ARG_CLIP, EBP )
    MOV_B     ( ARG_FLAG, BL )

    MOVD      ( REGIND(ECX), MM0 )	/*                 | m00             */
    MOVD      ( REGOFF(56, ECX), MM3 )	/*                 | m32             */

    TEST_L    ( ESI, ESI )
    JZ        ( LLBL( G3TPPM_4 ) )

ALIGNTEXT16
LLBL( G3TPPM_2 ):

    TEST_B    ( BL, REGIND(EBP) )	/* mask [i] != clip flag ??          */
    JNZ       ( LLBL( G3TPPM_3 ) )	/* skip vertex                       */

    MOVD      ( REGIND(EAX), MM4 )	/* 0               | x0              */
    PFMUL     ( MM0, MM4 )		/* 0               | x0*m00          */

    MOVQ      ( MM4, REGIND(EDX) )	/* write r1, r0                      */
    MOVQ      ( MM3, REGOFF(8, EDX) )	/* write r2  (=m32), r3 (=0)         */

LLBL( G3TPPM_3 ):

    ADD_L     ( EDI, EAX )		/* next vertex                       */
    ADD_L     ( CONST(16), EDX )	/* next r                            */

    INC_L     ( EBP )			/* next clipmask                     */
    DEC_L     ( ESI )			/* decrement vertex counter          */

    JNZ       ( LLBL( G3TPPM_2 ) )	/* cnt > 0 ? -> process next vertex  */

LLBL( G3TPPM_4 ):

    FEMMS
    POP_L     ( EBP )
    POP_L     ( EBX )
    POP_L     ( EDI )
    POP_L     ( ESI )
    RET




ALIGNTEXT16
GLOBL GLNAME( gl_3dnow_transform_points1_3d_masked )
GLNAME( gl_3dnow_transform_points1_3d_masked ):

    PUSH_L    ( ESI )
    PUSH_L    ( EDI )
    PUSH_L    ( EBX )
    PUSH_L    ( EBP )

    MOV_L     ( ARG_DEST, ECX )
    MOV_L     ( ARG_MATRIX, ESI )
    MOV_L     ( ARG_SOURCE, EAX )
    MOV_L     ( CONST(3), REGOFF(V4F_SIZE, ECX) )
    OR_B      ( CONST(VEC_SIZE_3), REGOFF(V4F_FLAGS, ECX) )
    MOV_L     ( REGOFF(V4F_COUNT, EAX), EDX )
    MOV_L     ( EDX, REGOFF(V4F_COUNT, ECX) )

    MOV_L     ( REGOFF(V4F_START, ECX), EDX )
    MOV_L     ( ESI, ECX )
    MOV_L     ( REGOFF(V4F_COUNT, EAX), ESI )
    MOV_L     ( REGOFF(V4F_STRIDE, EAX), EDI )
    MOV_L     ( REGOFF(V4F_START, EAX), EAX )
    MOV_L     ( ARG_CLIP, EBP )
    MOV_B     ( ARG_FLAG, BL )

    MOVQ      ( REGIND(ECX), MM0 )	/* m01             | m00             */
    MOVD      ( REGOFF(8, ECX), MM1 )	/*                 | m02             */

    MOVQ      ( REGOFF(48, ECX), MM2 )	/* m31             | m30             */
    MOVD      ( REGOFF(56, ECX), MM3 )	/*                 | m32             */

    TEST_L    ( ESI, ESI )
    JZ        ( LLBL( G3TP3M_4 ) )

ALIGNTEXT16
LLBL( G3TP3M_2 ):

    TEST_B    ( BL, REGIND(EBP) )	/* mask [i] != clip flag ??          */
    JNZ       ( LLBL( G3TP3M_3 ) )	/* skip vertex                       */

    MOVD      ( REGIND(EAX), MM4 )	/*                 | x0              */
    PUNPCKLDQ ( MM4, MM4 )		/* x0              | x0              */

    MOVQ      ( MM4, MM5 )		/*                 | x0              */
    PFMUL     ( MM0, MM4 )		/* x0*m01          | x0*m00          */

    PFMUL     ( MM1, MM5 )		/*                 | x0*m02          */
    PFADD     ( MM2, MM4 )		/* x0*m01+m31      | x0*m00+m30      */

    PFADD     ( MM3, MM5 )		/*                 | x0*m02+m32      */
    MOVQ      ( MM4, REGIND(EDX) )	/* write r1, r0                      */

    MOVD      ( MM5, REGOFF(8, EDX) )	/* write r2                          */

LLBL( G3TP3M_3 ):

    ADD_L     ( EDI, EAX )		/* next vertex                       */
    ADD_L     ( CONST(16), EDX )	/* next r                            */

    INC_L     ( EBP )			/* next clipmask                     */
    DEC_L     ( ESI )			/* decrement vertex counter          */
    JNZ       ( LLBL( G3TP3M_2 ) )	/* cnt > 0 ? -> process next vertex  */

LLBL( G3TP3M_4 ):

    FEMMS
    POP_L     ( EBP )
    POP_L     ( EBX )
    POP_L     ( EDI )
    POP_L     ( ESI )
    RET




ALIGNTEXT16
GLOBL GLNAME( gl_3dnow_transform_points1_3d_no_rot_masked )
GLNAME( gl_3dnow_transform_points1_3d_no_rot_masked ):

    PUSH_L    ( ESI )
    PUSH_L    ( EDI )
    PUSH_L    ( EBX )
    PUSH_L    ( EBP )

    MOV_L     ( ARG_DEST, ECX )
    MOV_L     ( ARG_MATRIX, ESI )
    MOV_L     ( ARG_SOURCE, EAX )
    MOV_L     ( CONST(3), REGOFF(V4F_SIZE, ECX) )
    OR_B      ( CONST(VEC_SIZE_3), REGOFF(V4F_FLAGS, ECX) )
    MOV_L     ( REGOFF(V4F_COUNT, EAX), EDX )
    MOV_L     ( EDX, REGOFF(V4F_COUNT, ECX) )

    MOV_L     ( REGOFF(V4F_START, ECX), EDX )
    MOV_L     ( ESI, ECX )
    MOV_L     ( REGOFF(V4F_COUNT, EAX), ESI )
    MOV_L     ( REGOFF(V4F_STRIDE, EAX), EDI )
    MOV_L     ( REGOFF(V4F_START, EAX), EAX )
    MOV_L     ( ARG_CLIP, EBP )
    MOV_B     ( ARG_FLAG, BL )

    MOVD      ( REGIND(ECX), MM0 )	/*                 | m00             */
    MOVQ      ( REGOFF(48, ECX), MM2 )	/* m31             | m30             */

    MOVD      ( REGOFF(56, ECX), MM3 )	/*                 | m32             */

    TEST_L    ( ESI, ESI )
    JZ        ( LLBL( G3TP3NRM_4 ) )

ALIGNTEXT16
LLBL( G3TP3NRM_2 ):

    TEST_B    ( BL, REGIND(EBP) )	/* mask [i] != clip flag ??          */
    JNZ       ( LLBL( G3TP3NRM_3))	/* skip vertex                       */

    MOVD      ( REGIND(EAX), MM4 )	/*                 | x0              */
    PFMUL     ( MM0, MM4 )		/*                 | x0*m00          */

    PFADD     ( MM2, MM4 )		/* m31             | x0*m00+m30      */
    MOVQ      ( MM4, REGIND(EDX) )	/* write r1, r0                      */

    MOVD      ( MM3, REGOFF(8, EDX) )	/* write r2                          */

LLBL( G3TP3NRM_3 ):

    ADD_L     ( EDI, EAX )		/* next vertex                       */
    ADD_L     ( CONST(16), EDX )	/* next r                            */

    INC_L     ( EBP )			/* next clipmask                     */
    DEC_L     ( ESI )			/* decrement vertex counter          */
    JNZ       ( LLBL( G3TP3NRM_2 ) )	/* cnt > 0 ? -> process next vertex  */

LLBL( G3TP3NRM_4 ):

    FEMMS
    POP_L     ( EBP )
    POP_L     ( EBX )
    POP_L     ( EDI )
    POP_L     ( ESI )
    RET




ALIGNTEXT16
GLOBL GLNAME( gl_3dnow_transform_points1_2d_masked )
GLNAME( gl_3dnow_transform_points1_2d_masked ):

    PUSH_L    ( ESI )
    PUSH_L    ( EDI )
    PUSH_L    ( EBX )
    PUSH_L    ( EBP )

    MOV_L     ( ARG_DEST, ECX )
    MOV_L     ( ARG_MATRIX, ESI )
    MOV_L     ( ARG_SOURCE, EAX )
    MOV_L     ( CONST(2), REGOFF(V4F_SIZE, ECX) )
    OR_B      ( CONST(VEC_SIZE_2), REGOFF(V4F_FLAGS, ECX) )
    MOV_L     ( REGOFF(V4F_COUNT, EAX), EDX )
    MOV_L     ( EDX, REGOFF(V4F_COUNT, ECX) )

    MOV_L     ( REGOFF(V4F_START, ECX), EDX )
    MOV_L     ( ESI, ECX )
    MOV_L     ( REGOFF(V4F_COUNT, EAX), ESI )
    MOV_L     ( REGOFF(V4F_STRIDE, EAX), EDI )
    MOV_L     ( REGOFF(V4F_START, EAX), EAX )
    MOV_L     ( ARG_CLIP, EBP )
    MOV_B     ( ARG_FLAG, BL )

    MOVQ      ( REGIND(ECX), MM0 )	/* m01             | m00             */
    MOVQ      ( REGOFF(48, ECX), MM2 )	/* m31             | m30             */

    TEST_L    ( ESI, ESI )
    JZ        ( LLBL( G3TP2M_4 ) )

ALIGNTEXT16
LLBL( G3TP2M_2 ):

    TEST_B    ( BL, REGIND(EBP) )	/* mask [i] != clip flag ??          */
    JNZ       ( LLBL( G3TP2M_3 ) )	/* skip vertex                       */

    MOVD      ( REGIND(EAX), MM4 )	/*                 | x0              */
    PUNPCKLDQ ( MM4, MM4 )		/* x0              | x0              */

    PFMUL     ( MM0, MM4 )		/* x0*m01          | x0*m00          */
    PFADD     ( MM2, MM4 )		/* x0*m01+m31      | x0*m00+m30      */

    MOVQ      ( MM4, REGIND(EDX) )	/* write r1, r0                      */

LLBL( G3TP2M_3 ):

    ADD_L     ( EDI, EAX )		/* next vertex                       */

    ADD_L     ( CONST(16), EDX )	/* next r                            */
    INC_L     ( EBP )			/* next clipmask                     */

    DEC_L     ( ESI )			/* decrement vertex counter          */
    JNZ       ( LLBL( G3TP2M_2 ) )	/* cnt > 0 ? -> process next vertex  */

LLBL( G3TP2M_4 ):

    FEMMS
    POP_L     ( EBP )
    POP_L     ( EBX )
    POP_L     ( EDI )
    POP_L     ( ESI )
    RET




ALIGNTEXT16
GLOBL GLNAME(gl_3dnow_transform_points1_2d_no_rot_masked)
GLNAME( gl_3dnow_transform_points1_2d_no_rot_masked ):

    PUSH_L    ( ESI )
    PUSH_L    ( EDI )
    PUSH_L    ( EBX )
    PUSH_L    ( EBP )

    MOV_L     ( ARG_DEST, ECX )
    MOV_L     ( ARG_MATRIX, ESI )
    MOV_L     ( ARG_SOURCE, EAX )
    MOV_L     ( CONST(2), REGOFF(V4F_SIZE, ECX) )
    OR_B      ( CONST(VEC_SIZE_2), REGOFF(V4F_FLAGS, ECX) )
    MOV_L     ( REGOFF(V4F_COUNT, EAX), EDX )
    MOV_L     ( EDX, REGOFF(V4F_COUNT, ECX) )

    MOV_L     ( REGOFF(V4F_START, ECX), EDX )
    MOV_L     ( ESI, ECX )
    MOV_L     ( REGOFF(V4F_COUNT, EAX), ESI )
    MOV_L     ( REGOFF(V4F_STRIDE, EAX), EDI )
    MOV_L     ( REGOFF(V4F_START, EAX), EAX )
    MOV_L     ( ARG_CLIP, EBP )
    MOV_B     ( ARG_FLAG, BL )

    MOVD      ( REGIND(ECX), MM0 )	/*                 | m00             */
    MOVQ      ( REGOFF(48, ECX), MM2 )	/* m31             | m30             */

    TEST_L    ( ESI, ESI )
    JZ        ( LLBL( G3TP2NRM_4 ) )

ALIGNTEXT16
LLBL( G3TP2NRM_2 ):

    TEST_B    ( BL, REGIND(EBP) )	/* mask [i] != clip flag ??          */
    JNZ       ( LLBL( G3TP2NRM_3 ) )	/* skip vertex                       */

    MOVD      ( REGIND(EAX), MM4 )	/*                 | x0              */
    PFMUL     ( MM0, MM4 )		/*                 | x0*m00          */

    PFADD     ( MM2, MM4 )		/* m31             | x0*m00+m30      */
    MOVQ      ( MM4, REGIND(EDX) )	/* write r1, r0                      */

LLBL( G3TP2NRM_3 ):

    ADD_L     ( CONST(16), EDX )	/* next r                            */
    ADD_L     ( EDI, EAX )		/* next vertex                       */

    INC_L     ( EBP )			/* next clipmask                     */
    DEC_L     ( ESI )			/* decrement vertex counter          */

    JNZ       ( LLBL( G3TP2NRM_2 ) )	/* cnt > 0 ? -> process next vertex  */

LLBL( G3TP2NRM_4 ):

    FEMMS
    POP_L     ( EBP )
    POP_L     ( EBX )
    POP_L     ( EDI )
    POP_L     ( ESI )
    RET




ALIGNTEXT16
GLOBL GLNAME( gl_3dnow_transform_points1_identity_masked )
GLNAME( gl_3dnow_transform_points1_identity_masked ):

    PUSH_L    ( ESI )
    PUSH_L    ( EDI )
    PUSH_L    ( EBX )
    PUSH_L    ( EBP )

    MOV_L     ( ARG_DEST, ECX )
    MOV_L     ( ARG_MATRIX, ESI )
    MOV_L     ( ARG_SOURCE, EAX )
    MOV_L     ( CONST(1), REGOFF(V4F_SIZE, ECX) )
    OR_B      ( CONST(VEC_SIZE_1), REGOFF(V4F_FLAGS, ECX) )
    MOV_L     ( REGOFF(V4F_COUNT, EAX), EDX )
    MOV_L     ( EDX, REGOFF(V4F_COUNT, ECX) )

    MOV_L     ( REGOFF(V4F_START, ECX), EDX )
    MOV_L     ( ESI, ECX )
    MOV_L     ( REGOFF(V4F_COUNT, EAX), ESI )
    MOV_L     ( REGOFF(V4F_STRIDE, EAX), EDI )
    MOV_L     ( REGOFF(V4F_START, EAX), EAX )
    MOV_L     ( ARG_CLIP, EBP )
    MOV_B     ( ARG_FLAG, BL )

ALIGNTEXT16
LLBL( G3TPIM_2 ):

    TEST_B    ( BL, REGIND(EBP) )	/* mask [i] != clip flag ??          */
    JNZ       ( LLBL( G3TPIM_3 ) )	/* skip vertex                       */

    MOVD      ( REGIND(EAX), MM0 )	/*                 | x0              */
    MOVD      ( MM0, REGIND(EDX) )	/*                 | r0              */

LLBL( G3TPIM_3 ):

    ADD_L     ( EDI, EAX )		/* next vertex                       */
    ADD_L     ( CONST(16), EDX )	/* next r                            */

    INC_L     ( EBP )			/* next clipmask                     */
    DEC_L     ( ESI )			/* decrement vertex counter          */

    JNZ       ( LLBL( G3TPIM_2 ) )	/* cnt > 0 ? -> process next vertex  */

LLBL( G3TPIM_4 ):

    FEMMS
    POP_L     ( EBP )
    POP_L     ( EBX )
    POP_L     ( EDI )
    POP_L     ( ESI )
    RET
