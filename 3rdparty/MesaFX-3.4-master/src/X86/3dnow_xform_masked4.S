/* $Id: 3dnow_xform_masked4.S,v 1.2.4.1 2000/10/22 23:10:51 gareth Exp $ */

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
GLOBL GLNAME( gl_3dnow_transform_points4_general_masked )
GLNAME( gl_3dnow_transform_points4_general_masked ):

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
    MOVD      ( REGOFF(16, ECX), MM7 )	/*                 | m10             */

    PSLLQ     ( CONST(32), MM7 )	/* m10             |                 */
    POR       ( MM7, MM0 )		/* m10             | m00             */

    MOVD      ( REGOFF(4, ECX), MM1 )	/*                 | m01             */
    MOVD      ( REGOFF(20, ECX), MM7 )	/*                 | m11             */

    PSLLQ     ( CONST(32), MM7 )	/* m11             |                 */
    POR       ( MM7, MM1 )		/* m11             | m01             */

    MOVD      ( REGOFF(32, ECX), MM2 )	/*                 | m20             */
    MOVD      ( REGOFF(48, ECX), MM7 )	/*                 | m30             */

    PSLLQ     ( CONST(32), MM7 )	/* m30             |                 */
    POR       ( MM7, MM2 )		/* m30             | m20             */

    MOVD      ( REGOFF(36, ECX), MM3 )	/*                 | m21             */
    MOVD      ( REGOFF(52, ECX), MM7 )	/*                 | m31             */

    PSLLQ     ( CONST(32), MM7 )	/* m31             |                 */
    POR       ( MM7, MM3 )		/* m31             | m21             */

    TEST_L    ( ESI, ESI )
    JZ        ( LLBL( G3TPGM_6 ) )

    PUSH_L    ( EBP )
    PUSH_L    ( EAX )
    PUSH_L    ( EDX )
    PUSH_L    ( ESI )

ALIGNTEXT32
LLBL( G3TPGM_2 ):

    TEST_B    ( BL, REGIND(EBP) )	/* mask [i] != clip flag ??          */
    JNZ       ( LLBL( G3TPGM_3 ) )	/* skip vertex                       */

    MOVQ      ( REGIND(EAX), MM4 )	/* x1              | x0              */
    MOVQ      ( MM4, MM5 )		/* x1              | x0              */

    MOVQ      ( REGOFF(8, EAX), MM6 )	/* x3              | x2              */
    PFMUL     ( MM0, MM4 )		/* x1*m10          | x0*m00          */

    MOVQ      ( MM6, MM7 )		/* x3              | x2              */
    PFMUL     ( MM1, MM5 )		/* x1*m11          | x0*m01          */

    PFMUL     ( MM2, MM6 )		/* x3*m30          | x2*m20          */
    PFACC     ( MM5, MM4 )		/* x0*m01+x1*m11   | x0*m00+x1*m10   */

    PFMUL     ( MM3, MM7 )		/* x3*m31          | x2*m21          */
    PFACC     ( MM7, MM6 )		/* x2*m21+x3*m31   | x2*m20+x3*m30   */

    PFADD     ( MM4, MM6 )		/* r1              | r0              */
    MOVQ      ( MM6, REGIND(EDX) )	/* write r0, r1                      */

LLBL( G3TPGM_3 ):

    ADD_L     ( EDI, EAX )		/* next vertex                       */
    ADD_L     ( CONST(16), EDX )	/* next r                            */

    INC_L     ( EBP )			/* next clipmask                     */
    DEC_L     ( ESI )			/* decrement vertex counter          */

    JNZ       ( LLBL( G3TPGM_2 ) )	/* cnt > 0 ? -> process next vertex  */

                                        /* and now the second stripe ...     */
    MOVD      ( REGOFF(8, ECX), MM0 )	/*                 | m02             */
    MOVD      ( REGOFF(24, ECX), MM7 )	/*                 | m12             */

    PSLLQ     ( CONST(32), MM7 )	/* m12             |                 */
    POR       ( MM7, MM0 )		/* m12             | m02             */

    MOVD      ( REGOFF(12, ECX), MM1 )	/*                 | m03             */
    MOVD      ( REGOFF(28, ECX), MM7 )	/*                 | m13             */

    PSLLQ     ( CONST(32), MM7 )	/* m13             |                 */
    POR       ( MM7, MM1 )		/* m13             | m03             */

    MOVD      ( REGOFF(40, ECX), MM2 )	/*                 | m22             */
    MOVD      ( REGOFF(56, ECX), MM7 )	/*                 | m32             */

    PSLLQ     ( CONST(32), MM7 )	/* m32             |                 */
    POR       ( MM7, MM2 )		/* m32             | m22             */

    MOVD      ( REGOFF(44, ECX), MM3 )	/*                 | m23             */
    MOVD      ( REGOFF(60, ECX), MM7 )	/*                 | m33             */

    PSLLQ     ( CONST(32), MM7 )	/* m33             |                 */
    POR       ( MM7, MM3 )		/* m33             | m23             */

    POP_L     ( ESI )			/* reset counter & pointers          */
    POP_L     ( EDX )
    POP_L     ( EAX )
    POP_L     ( EBP )

ALIGNTEXT32
LLBL( G3TPGM_4 ):

    TEST_B    ( BL, REGIND(EBP) )	/* mask [i] != clip flag ??          */
    JNZ       ( LLBL( G3TPGM_5 ) )	/* skip vertex                       */

    MOVQ      ( REGIND(EAX), MM4 )	/* x1              | x0              */
    MOVQ      ( MM4, MM5 )		/* x1              | x0              */

    MOVQ      ( REGOFF(8, EAX), MM6 )	/* x3              | x2              */
    PFMUL     ( MM0, MM4 )		/* x1*m12          | x0*m02          */

    MOVQ      ( MM6, MM7 )		/* x3              | x2              */
    PFMUL     ( MM1, MM5 )		/* x1*m13          | x0*m03          */

    PFMUL     ( MM2, MM6 )		/* x3*m32          | x2*m22          */
    PFACC     ( MM5, MM4 )		/* x0*m03+x1*m13   | x0*m02+x1*m12   */

    PFMUL     ( MM3, MM7 )		/* x3*m33          | x2*m23          */
    PFACC     ( MM7, MM6 )		/* x2*m23+x3*m33   | x2*m22+x3*m32   */

    PFADD     ( MM4, MM6 )		/* r3              | r2              */
    MOVQ      ( MM6, REGOFF(8, EDX) )	/* write r2, r3                      */

LLBL( G3TPGM_5 ):

    ADD_L     ( EDI, EAX )		/* next vertex                       */
    ADD_L     ( CONST(16), EDX )	/* next r                            */

    INC_L     ( EBP )			/* next clipmask                     */
    DEC_L     ( ESI )			/* decrement vertex counter          */

    JNZ       ( LLBL( G3TPGM_4 ) )	/* cnt > 0 ? -> process next vertex  */

LLBL( G3TPGM_6 ):

    FEMMS
    POP_L     ( EBP )
    POP_L     ( EBX )
    POP_L     ( EDI )
    POP_L     ( ESI )
    RET




ALIGNTEXT16
GLOBL GLNAME( gl_3dnow_transform_points4_perspective_masked )
GLNAME( gl_3dnow_transform_points4_perspective_masked ):

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
    MOVD      ( REGOFF(20, ECX), MM7 )	/*                 | m11             */

    PSLLQ     ( CONST(32), MM7 )	/* m11             |                 */
    POR       ( MM7, MM0 )		/* m11             | m00             */

    MOVD      ( REGOFF(40, ECX), MM1 )	/*                 | m22             */
    MOVD      ( REGOFF(56, ECX), MM7 )	/*                 | m32             */

    PSLLQ     ( CONST(32), MM7 )	/* m32             |                 */
    POR       ( MM7, MM1 )		/* m32             | m22             */

    MOVQ      ( REGOFF(32, ECX), MM2 )	/* m21             | m20             */
    PUNPCKLDQ ( MM7, MM7 )		/* 0               | 0               */

    TEST_L    ( ESI, ESI )
    JZ        ( LLBL( G3TPPM_4 ) )

ALIGNTEXT32
LLBL( G3TPPM_2 ):

    TEST_B    ( BL, REGIND(EBP) )	/* mask [i] != clip flag ??          */
    JNZ       ( LLBL( G3TPPM_3 ) )	/* skip vertex                       */

    MOVQ      ( REGIND(EAX), MM4 )	/* x1              | x0              */
    MOVQ      ( REGOFF(8, EAX), MM5 )	/* x3              | x2              */

    PFMUL     ( MM0, MM4 )		/* x1*m11          | x0*m00          */
    MOVQ      ( MM5, MM6 )		/* x3              | x2              */

    PUNPCKLDQ ( MM5, MM5 )		/* x2              | x2              */
    PFMUL     ( MM1, MM6 )		/* x3*m32          | x2*m22          */

    PFMUL     ( MM2, MM5 )		/* x2*m21          | x2*m20          */
    PFADD     ( MM4, MM5 )		/* x1*m11+x2*m21   | x0*m00+x2*m20   */

    MOVQ      ( MM5, REGIND(EDX) )	/* write r0, r1                      */
    MOVD      ( REGOFF(8, EAX), MM5 )	/*                 | x2              */

    PFSUBR    ( MM7, MM5 )		/*                 | -x2             */
    PFACC     ( MM5, MM6 )		/* -x2             | x2*m22+x3*m32   */

    MOVQ      ( MM6, REGOFF(8, EDX) )	/* write r2, r3                      */

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
GLOBL GLNAME(gl_3dnow_transform_points4_3d_masked)
GLNAME( gl_3dnow_transform_points4_3d_masked ):

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
    MOVD      ( REGOFF(16, ECX), MM7 )	/*                 | m10             */

    PSLLQ     ( CONST(32), MM7 )	/* m10             |                 */
    POR       ( MM7, MM0 )		/* m10             | m00             */

    MOVD      ( REGOFF(4, ECX), MM1 )	/*                 | m01             */
    MOVD      ( REGOFF(20, ECX), MM7 )	/*                 | m11             */

    PSLLQ     ( CONST(32), MM7 )	/* m11             |                 */
    POR       ( MM7, MM1 )		/* m11             | m01             */

    MOVD      ( REGOFF(32, ECX), MM2 )	/*                 | m20             */
    MOVD      ( REGOFF(48, ECX), MM7 )	/*                 | m30             */

    PSLLQ     ( CONST(32), MM7 )	/* m30             |                 */
    POR       ( MM7, MM2 )		/* m30             | m20             */

    MOVD      ( REGOFF(36, ECX), MM3 )	/*                 | m21             */
    MOVD      ( REGOFF(52, ECX), MM7 )	/*                 | m31             */

    PSLLQ     ( CONST(32), MM7 )	/* m31             |                 */
    POR       ( MM7, MM3 )		/* m31             | m21             */

    TEST_L    ( ESI, ESI )
    JZ        ( LLBL( G3TP3M_6 ) )

    PUSH_L    ( EBP )
    PUSH_L    ( EAX )
    PUSH_L    ( EDX )
    PUSH_L    ( ESI )

ALIGNTEXT32
LLBL( G3TP3M_2 ):

    TEST_B    ( BL, REGIND(EBP) )	/* mask [i] != clip flag ??          */
    JNZ       ( LLBL( G3TP3M_3 ) )	/* skip vertex                       */

    MOVQ      ( REGIND(EAX), MM4 )	/* x1              | x0              */
    MOVQ      ( MM4, MM5 )		/* x1              | x0              */

    PFMUL     ( MM0, MM4 )		/* x1*m10          | x0*m00          */
    MOVQ      ( REGOFF(8, EAX), MM6 )	/* x3              | x2              */

    PFMUL     ( MM1, MM5 )		/* x1*m11          | x0*m01          */
    MOVQ      ( MM6, MM7 )		/* x3              | x2              */

    PFMUL     ( MM2, MM6 )		/* x3*m30          | x2*m20          */
    PFACC     ( MM5, MM4 )		/* x0*m01+x1*m11   | x0*m00+x1*m10   */

    PFMUL     ( MM3, MM7 )		/* x3*m31          | x2*m21          */
    PFACC     ( MM7, MM6 )		/* x2*m21+x3*m31   | x2*m20+x3*m30   */

    PFADD     ( MM4, MM6 )		/* r1              | r0              */
    MOVQ      ( MM6, REGIND(EDX) )	/* write r0, r1                      */

LLBL( G3TP3M_3 ):

    ADD_L     ( EDI, EAX )		/* next vertex                       */
    ADD_L     ( CONST(16), EDX )	/* next r                            */

    INC_L     ( EBP )			/* next clipmask                     */
    DEC_L     ( ESI )			/* decrement vertex counter          */

    JNZ       ( LLBL( G3TP3M_2 ) )	/* cnt > 0 ? -> process next vertex  */

                                        /* and now the second stripe ...     */
    MOVD      ( REGOFF(8, ECX), MM0 )	/*                 | m02             */
    MOVD      ( REGOFF(24, ECX), MM7 )	/*                 | m12             */

    PSLLQ     ( CONST(32), MM7 )	/* m12             |                 */
    POR       ( MM7, MM0 )		/* m12             | m02             */

    MOVD      ( REGOFF(40, ECX), MM2 )	/*                 | m22             */
    MOVD      ( REGOFF(56, ECX), MM7 )	/*                 | m32             */

    PSLLQ     ( CONST(32), MM7 )	/* m32             |                 */
    POR       ( MM7, MM2 )		/* m32             | m22             */

    POP_L     ( ESI )			/* reset counter & pointers          */
    POP_L     ( EDX )
    POP_L     ( EAX )
    POP_L     ( EBP )

ALIGNTEXT32
LLBL( G3TP3M_4 ):

    TEST_B    ( BL, REGIND(EBP) )	/* mask [i] != clip flag ??          */
    JNZ       ( LLBL( G3TP3M_5 ) )	/* skip vertex                       */

    MOVQ      ( REGIND(EAX), MM4 )	/* x1              | x0              */
    MOVQ      ( REGOFF(8, EAX), MM6 )	/* x3              | x2              */

    PFMUL     ( MM0, MM4 )		/* x1*m12          | x0*m02          */
    MOVD      ( REGOFF(12, EAX), MM7 )	/*                 | x3              */

    PFMUL     ( MM2, MM6 )		/* x3*m32          | x2*m22          */
    PFACC     ( MM4, MM6 )		/* x0*m03+x1*m13   | x0*m02+x1*m12   */

    PFACC     ( MM7, MM6 )		/* x3              | x2*m22+x3*m32   */
    MOVQ      ( MM6, REGOFF(8, EDX) )	/* write r2, r3                      */

LLBL( G3TP3M_5 ):

    ADD_L     ( EDI, EAX )		/* next vertex                       */
    ADD_L     ( CONST(16), EDX )	/* next r                            */

    INC_L     ( EBP )			/* next clipmask                     */
    DEC_L     ( ESI )			/* decrement vertex counter          */

    JNZ       ( LLBL( G3TP3M_4 ) )	/* cnt > 0 ? -> process next vertex  */

LLBL( G3TP3M_6 ):

    FEMMS
    POP_L     ( EBP )
    POP_L     ( EBX )
    POP_L     ( EDI )
    POP_L     ( ESI )
    RET




ALIGNTEXT16
GLOBL GLNAME( gl_3dnow_transform_points4_3d_no_rot_masked )
GLNAME( gl_3dnow_transform_points4_3d_no_rot_masked ):

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
    MOVD      ( REGOFF(20, ECX), MM7 )	/*                 | m11             */

    PSLLQ     ( CONST(32), MM7 )	/* m11             |                 */
    POR       ( MM7, MM0 )		/* m11             | m00             */

    MOVD      ( REGOFF(40, ECX), MM2 )	/*                 | m22             */
    MOVD      ( REGOFF(56, ECX), MM3 )	/*                 | m32             */

    PSLLQ     ( CONST(32), MM3 )	/* m32             |                 */
    POR       ( MM3, MM2 )		/* m32             | m22             */

    MOVQ      ( REGOFF(48, ECX), MM1 )	/* m31             | m30             */

    TEST_L    ( ESI, ESI )
    JZ        ( LLBL( G3TP3NRM_4 ) )

ALIGNTEXT32
LLBL( G3TP3NRM_2 ):

    TEST_B    ( BL, REGIND(EBP) )	/* mask [i] != clip flag ??          */
    JNZ       ( LLBL( G3TP3NRM_3 ) )	/* skip vertex                       */

    MOVQ      ( REGIND(EAX), MM4 )	/* x1              | x0              */
    MOVQ      ( REGOFF(8, EAX), MM5 )	/* x3              | x2              */

    MOVQ      ( MM5, MM6 )		/* x3              | x2              */
    PFMUL     ( MM0, MM4 )		/* x1*m11          | x0*m00          */

    MOVD      ( REGOFF(12, EAX), MM7 )	/*                 | x3              */
    PFMUL     ( MM2, MM5 )		/* x3*m32          | x2*m22          */

    PUNPCKHDQ ( MM6, MM6 )		/* x3              | x3              */
    PFACC     ( MM7, MM5 )		/* x3              | x2*m22+x3*m32   */

    PFMUL     ( MM1, MM6 )		/* x3*m31          | x3*m30          */
    PFADD     ( MM6, MM4 )		/* x1*m11+x3*m31   | x0*m00+x3*m30   */

    MOVQ      ( MM4, REGIND(EDX) )	/* write r0, r1                      */
    MOVQ      ( MM5, REGOFF(8, EDX) )	/* write r2, r3                      */

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
GLOBL GLNAME( gl_3dnow_transform_points4_2d_masked )
GLNAME( gl_3dnow_transform_points4_2d_masked ):

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
    MOVD      ( REGOFF(16, ECX), MM7 )	/*                 | m10             */

    PSLLQ     ( CONST(32), MM7 )	/* m10             |                 */
    POR       ( MM7, MM0 )		/* m10             | m00             */

    MOVD      ( REGOFF(4, ECX), MM1 )	/*                 | m01             */
    MOVD      ( REGOFF(20, ECX), MM7 )	/*                 | m11             */

    PSLLQ     ( CONST(32), MM7 )	/* m11             |                 */
    POR       ( MM7, MM1 )		/* m11             | m01             */

    MOVQ      ( REGOFF(48, ECX), MM2 )	/* m31             | m30             */

    TEST_L    ( ESI, ESI )
    JZ        ( LLBL( G3TP2M_4 ) )

ALIGNTEXT32
LLBL( G3TP2M_2 ):

    TEST_B    ( BL, REGIND(EBP) )	/* mask [i] != clip flag ??          */
    JNZ       ( LLBL( G3TP2M_3 ) )	/* skip vertex                       */

    MOVQ      ( REGIND(EAX), MM3 )	/* x1              | x0              */
    MOVQ      ( MM3, MM4 )		/* x1              | x0              */

    MOVQ      ( REGOFF(8, EAX), MM5 )	/* x3              | x2              */
    PFMUL     ( MM0, MM3 )		/* x1*m10          | x0*m00          */

    MOVQ      ( MM5, MM6 )		/* x3              | x2              */
    PFMUL     ( MM1, MM4 )		/* x1*m11          | x0*m01          */

    PUNPCKHDQ ( MM6, MM6 )		/* x3              | x3              */
    PFACC     ( MM4, MM3 )		/* x0*m01+x1*m11   | x0*m00+x1*m10   */

    PFMUL     ( MM2, MM6 )		/* x3*m31          | x3*m30          */
    PFADD     ( MM6, MM3 )		/* r1              | r0              */

    MOVQ      ( MM3, REGIND(EDX) )	/* write r0, r1                      */
    MOVQ      ( MM5, REGOFF(8, EDX) )	/* write r2, r3                      */

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
GLOBL GLNAME( gl_3dnow_transform_points4_2d_no_rot_masked )
GLNAME( gl_3dnow_transform_points4_2d_no_rot_masked ):

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
    MOVD      ( REGOFF(20, ECX), MM7 )	/*                 | m11             */

    PSLLQ     ( CONST(32), MM7 )	/* m11             |                 */
    POR       ( MM7, MM0 )		/* m11             | m00             */

    MOVQ      ( REGOFF(48, ECX), MM1 )	/* m31             | m30             */

    TEST_L    ( ESI, ESI )
    JZ        ( LLBL( G3TP2NRM_4 ) )

ALIGNTEXT32
LLBL( G3TP2NRM_2 ):

    TEST_B    ( BL, REGIND(EBP) )	/* mask [i] != clip flag ??          */
    JNZ       ( LLBL( G3TP2NRM_3 ) )	/* skip vertex                       */

    MOVQ      ( REGIND(EAX), MM4 )	/* x1              | x0              */
    MOVQ      ( REGOFF(8, EAX), MM5 )	/* x3              | x2              */

    MOVQ      ( MM5, MM6 )		/* x3              | x2              */
    PFMUL     ( MM0, MM4 )		/* x1*m11          | x0*m00          */

    PUNPCKHDQ ( MM6, MM6 )		/* x3              | x2              */
    PFMUL     ( MM1, MM6 )		/* x3*m31          | x3*m30          */

    PFADD     ( MM6, MM4 )		/* x1*m11+x3*m31   | x0*m00+x3*m30   */
    MOVQ      ( MM4, REGIND(EDX) )	/* write r0, r1                      */

    MOVQ      ( MM5, REGOFF(8, EDX) )	/* write r2, r3                      */

LLBL( G3TP2NRM_3 ):

    ADD_L     ( EDI, EAX )		/* next vertex                       */
    ADD_L     ( CONST(16), EDX )	/* next r                            */

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
GLOBL GLNAME( gl_3dnow_transform_points4_identity_masked )
GLNAME( gl_3dnow_transform_points4_identity_masked ):

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

    TEST_L    ( ESI, ESI )
    JZ        ( LLBL( G3TPIM_4 ) )

ALIGNTEXT32
LLBL( G3TPIM_2 ):

    TEST_B    ( BL, REGIND(EBP) )	/* mask [i] != clip flag ??          */
    JNZ       ( LLBL( G3TPIM_3 ) )	/* skip vertex                       */

    MOVQ      ( REGIND(EAX), MM0 )	/* x1              | x0              */
    MOVQ      ( MM0, REGIND(EDX) )	/* r1              | r0              */

    MOVQ      ( REGOFF(8, EAX), MM1 )	/* x3              | x2              */
    MOVQ      ( MM1, REGOFF(8, EDX) )	/* r3              | r2              */

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
