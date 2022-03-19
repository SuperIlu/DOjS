/* $Id: x86_xform_masked2.S,v 1.1.2.1 2000/10/22 23:10:51 gareth Exp $ */

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

#define FP_ONE		1065353216
#define FP_ZERO		0

#define SRC(i)		REGOFF(i * 4, ESI)
#define DST(i)		REGOFF(i * 4, EDI)
#define MAT(i)		REGOFF(i * 4, EDX)


ALIGNTEXT16
GLOBL GLNAME( gl_x86_transform_points2_general_masked )
GLNAME( gl_x86_transform_points2_general_masked ):

	PUSH_L( ESI )
	PUSH_L( EDI )

	PUSH_L( EBX )
	PUSH_L( EBP )

	MOV_L( ARG_SOURCE, ESI )
	MOV_L( ARG_DEST, EDI )

	MOV_L( ARG_MATRIX, EDX )
	MOV_L( ARG_CLIP, EBP )

	MOV_L( REGOFF(V4F_COUNT, ESI), ECX )
	MOV_B( ARG_FLAG, BL )

	TEST_L( ECX, ECX )
	JZ( LLBL( x86_p2_gm_done ) )

	MOV_L( REGOFF(V4F_STRIDE, ESI), EAX )
	OR_L( CONST(VEC_SIZE_4), REGOFF(V4F_FLAGS, EDI) )

	MOV_L( ECX, REGOFF(V4F_COUNT, EDI) )
	MOV_L( CONST(4), REGOFF(V4F_SIZE, EDI) )

	SHL_L( CONST(4), ECX )
	MOV_L( REGOFF(V4F_START, ESI), ESI )

	MOV_L( REGOFF(V4F_START, EDI), EDI )
	ADD_L( EDI, ECX )

ALIGNTEXT16
LLBL( x86_p2_gm_loop ):

	TEST_B( BL, REGIND(EBP) )
	JNZ( LLBL( x86_p2_gm_skip ) )

	FLD_S( SRC(0) )			/* F4 */
	FMUL_S( MAT(0) )
	FLD_S( SRC(0) )			/* F5 F4 */
	FMUL_S( MAT(1) )
	FLD_S( SRC(0) )			/* F6 F5 F4 */
	FMUL_S( MAT(2) )
	FLD_S( SRC(0) )			/* F7 F6 F5 F4 */
	FMUL_S( MAT(3) )

	FLD_S( SRC(1) )			/* F0 F7 F6 F5 F4 */
	FMUL_S( MAT(4) )
	FLD_S( SRC(1) )			/* F1 F0 F7 F6 F5 F4 */
	FMUL_S( MAT(5) )
	FLD_S( SRC(1) )			/* F2 F1 F0 F7 F6 F5 F4 */
	FMUL_S( MAT(6) )
	FLD_S( SRC(1) )			/* F3 F2 F1 F0 F7 F6 F5 F4 */
	FMUL_S( MAT(7) )

	FXCH( ST(3) )			/* F0 F2 F1 F3 F7 F6 F5 F4 */
	FADDP( ST(0), ST(7) )		/* F2 F1 F3 F7 F6 F5 F4 */
	FXCH( ST(1) )			/* F1 F2 F3 F7 F6 F5 F4 */
	FADDP( ST(0), ST(5) )		/* F2 F3 F7 F6 F5 F4 */
	FADDP( ST(0), ST(3) )		/* F3 F7 F6 F5 F4 */
	FADDP( ST(0), ST(1) )		/* F7 F6 F5 F4 */

	FXCH( ST(3) )			/* F4 F6 F5 F7 */
	FADD_S( MAT(12) )
	FXCH( ST(2) )			/* F5 F6 F4 F7 */
	FADD_S( MAT(13) )
	FXCH( ST(1) )			/* F6 F5 F4 F7 */
	FADD_S( MAT(14) )
	FXCH( ST(3) )			/* F7 F5 F4 F6 */
	FADD_S( MAT(15) )

	FXCH( ST(2) )			/* F4 F5 F7 F6 */
	FSTP_S( DST(0) )		/* F5 F7 F6 */
	FSTP_S( DST(1) )		/* F7 F6 */
	FXCH( ST(1) )			/* F6 F7 */
	FSTP_S( DST(2) )		/* F7 */
	FSTP_S( DST(3) )		/* */

LLBL( x86_p2_gm_skip ):

	INC_L( EBP )
	ADD_L( CONST(16), EDI )
	ADD_L( EAX, ESI )
	CMP_L( ECX, EDI )
	JNE( LLBL( x86_p2_gm_loop ) )

LLBL( x86_p2_gm_done ):

	POP_L( EBP )
	POP_L( EBX )
	POP_L( EDI )
	POP_L( ESI )
	RET




ALIGNTEXT16
GLOBL GLNAME( gl_x86_transform_points2_perspective_masked )
GLNAME( gl_x86_transform_points2_perspective_masked ):

	PUSH_L( ESI )
	PUSH_L( EDI )

	PUSH_L( EBX )
	PUSH_L( EBP )

	MOV_L( ARG_SOURCE, ESI )
	MOV_L( ARG_DEST, EDI )

	MOV_L( ARG_MATRIX, EDX )
	MOV_L( ARG_CLIP, EBP )

	MOV_L( REGOFF(V4F_COUNT, ESI), ECX )
	MOV_B( ARG_FLAG, BL )

	TEST_L( ECX, ECX )
	JZ( LLBL( x86_p2_pm_done ) )

	MOV_L( REGOFF(V4F_STRIDE, ESI), EAX )
	OR_L( CONST(VEC_SIZE_4), REGOFF(V4F_FLAGS, EDI) )

	MOV_L( ECX, REGOFF(V4F_COUNT, EDI) )
	MOV_L( CONST(4), REGOFF(V4F_SIZE, EDI) )

	SHL_L( CONST(4), ECX )
	MOV_L( REGOFF(V4F_START, ESI), ESI )

	MOV_L( REGOFF(V4F_START, EDI), EDI )
	ADD_L( EDI, ECX )

	/* need eax; put stride in ARG_SOURCE */
	MOV_L( EAX, ARG_SOURCE )

	MOV_L( MAT(14), EAX )

ALIGNTEXT16
LLBL( x86_p2_pm_loop ):

	TEST_B( BL, REGIND(EBP) )
	JNZ( LLBL( x86_p2_pm_skip ) )

	FLD_S( SRC(0) )			/* F4 */
	FMUL_S( MAT(0) )

	FLD_S( SRC(1) )			/* F1 F4 */
	FMUL_S( MAT(5) )

	FXCH( ST(1) )			/* F4 F1 */
	FSTP_S( DST(0)   )		/* F1 */
	FSTP_S( DST(1)   )		/* */
	MOV_L( EAX, DST(2) )
	MOV_L( CONST(FP_ZERO), DST(3) )

LLBL( x86_p2_pm_skip ):

	INC_L( EBP )
	ADD_L( CONST(16), EDI )
	ADD_L( ARG_SOURCE, ESI )
	CMP_L( ECX, EDI )
	JNE( LLBL( x86_p2_pm_loop ) )

LLBL( x86_p2_pm_done ):

	POP_L( EBP )
	POP_L( EBX )
	POP_L( EDI )
	POP_L( ESI )
	RET




ALIGNTEXT16
GLOBL GLNAME( gl_x86_transform_points2_3d_masked )
GLNAME( gl_x86_transform_points2_3d_masked ):

	PUSH_L( ESI )
	PUSH_L( EDI )

	PUSH_L( EBX )
	PUSH_L( EBP )

	MOV_L( ARG_SOURCE, ESI )
	MOV_L( ARG_DEST, EDI )

	MOV_L( ARG_MATRIX, EDX )
	MOV_L( ARG_CLIP, EBP )

	MOV_L( REGOFF(V4F_COUNT, ESI), ECX )
	MOV_B( ARG_FLAG, BL )

	TEST_L( ECX, ECX )
	JZ( LLBL( x86_p2_3dm_done ) )

	MOV_L( REGOFF(V4F_STRIDE, ESI), EAX )
	OR_L( CONST(VEC_SIZE_3), REGOFF(V4F_FLAGS, EDI) )

	MOV_L( ECX, REGOFF(V4F_COUNT, EDI) )
	MOV_L( CONST(3), REGOFF(V4F_SIZE, EDI) )

	SHL_L( CONST(4), ECX )
	MOV_L( REGOFF(V4F_START, ESI), ESI )

	MOV_L( REGOFF(V4F_START, EDI), EDI )
	ADD_L( EDI, ECX )

ALIGNTEXT16
LLBL( x86_p2_3dm_loop ):

	TEST_B( BL, REGIND(EBP) )
	JNZ( LLBL( x86_p2_3dm_skip ) )

	FLD_S( SRC(0) )			/* F4 */
	FMUL_S( MAT(0) )
	FLD_S( SRC(0) )			/* F5 F4 */
	FMUL_S( MAT(1) )
	FLD_S( SRC(0) )			/* F6 F5 F4 */
	FMUL_S( MAT(2) )

	FLD_S( SRC(1) )			/* F0 F6 F5 F4 */
	FMUL_S( MAT(4) )
	FLD_S( SRC(1) )			/* F1 F0 F6 F5 F4 */
	FMUL_S( MAT(5) )
	FLD_S( SRC(1) )			/* F2 F1 F0 F6 F5 F4 */
	FMUL_S( MAT(6) )

	FXCH( ST(2) )			/* F0 F1 F2 F6 F5 F4 */
	FADDP( ST(0), ST(5) )		/* F1 F2 F6 F5 F4 */
	FADDP( ST(0), ST(3) )		/* F2 F6 F5 F4 */
	FADDP( ST(0), ST(1) )		/* F6 F5 F4 */

	FXCH( ST(2) )			/* F4 F5 F6 */
	FADD_S( MAT(12) )
	FXCH( ST(1) )			/* F5 F4 F6 */
	FADD_S( MAT(13) )
	FXCH( ST(2) )			/* F6 F4 F5 */
	FADD_S( MAT(14) )

	FXCH( ST(1) )			/* F4 F6 F5 */
	FSTP_S( DST(0) )		/* F6 F5 */
	FXCH( ST(1) )			/* F5 F6 */
	FSTP_S( DST(1) )		/* F6 */
	FSTP_S( DST(2) )		/* */

LLBL( x86_p2_3dm_skip ):

	INC_L( EBP )
	ADD_L( CONST(16), EDI )
	ADD_L( EAX, ESI )
	CMP_L( ECX, EDI )
	JNE( LLBL( x86_p2_3dm_loop ) )

LLBL( x86_p2_3dm_done ):

	POP_L( EBP )
	POP_L( EBX )
	POP_L( EDI )
	POP_L( ESI )
	RET




ALIGNTEXT16
GLOBL GLNAME( gl_x86_transform_points2_3d_no_rot_masked )
GLNAME( gl_x86_transform_points2_3d_no_rot_masked ):

	PUSH_L( ESI )
	PUSH_L( EDI )

	PUSH_L( EBX )
	PUSH_L( EBP )

	MOV_L( ARG_SOURCE, ESI )
	MOV_L( ARG_DEST, EDI )

	MOV_L( ARG_MATRIX, EDX )
	MOV_L( ARG_CLIP, EBP )

	MOV_L( REGOFF(V4F_COUNT, ESI), ECX )
	MOV_B( ARG_FLAG, BL )

	TEST_L( ECX, ECX )
	JZ( LLBL( x86_p2_3dnrm_done ) )

	MOV_L( REGOFF(V4F_STRIDE, ESI), EAX )
	OR_L( CONST(VEC_SIZE_3), REGOFF(V4F_FLAGS, EDI) )

	MOV_L( ECX, REGOFF(V4F_COUNT, EDI) )
	MOV_L( CONST(3), REGOFF(V4F_SIZE, EDI) )

	SHL_L( CONST(4), ECX )
	MOV_L( REGOFF(V4F_START, ESI), ESI )

	MOV_L( REGOFF(V4F_START, EDI), EDI )
	ADD_L( EDI, ECX )

	/* need eax; put stride in ARG_SOURCE */
	MOV_L( EAX, ARG_SOURCE )

	MOV_L( MAT(14), EAX )

ALIGNTEXT16
LLBL( x86_p2_3dnrm_loop ):

	TEST_B( BL, REGIND(EBP) )
	JNZ( LLBL( x86_p2_3dnrm_skip ) )

	FLD_S( SRC(0) )			/* F4 */
	FMUL_S( MAT(0) )

	FLD_S( SRC(1) )			/* F1 F4 */
	FMUL_S( MAT(5) )

	FXCH( ST(1) )			/* F4 F1 */
	FADD_S( MAT(12) )
	FLD_S( MAT(13) )		/* F5 F4 F1 */
	FXCH( ST(2) )			/* F1 F4 F5 */
	FADDP( ST(0), ST(2) )		/* F4 F5 */

	FSTP_S( DST(0)   )		/* F5 */
	FSTP_S( DST(1)   )		/* */
	MOV_L( EAX, DST(2) )

LLBL( x86_p2_3dnrm_skip ):

	INC_L( EBP )
	ADD_L( CONST(16), EDI )
	ADD_L( ARG_SOURCE, ESI )
	CMP_L( ECX, EDI )
	JNE( LLBL( x86_p2_3dnrm_loop ) )

LLBL( x86_p2_3dnrm_done ):

	POP_L( EBP )
	POP_L( EBX )
	POP_L( EDI )
	POP_L( ESI )
	RET




ALIGNTEXT16
GLOBL GLNAME( gl_x86_transform_points2_2d_masked )
GLNAME( gl_x86_transform_points2_2d_masked ):

	PUSH_L( ESI )
	PUSH_L( EDI )

	PUSH_L( EBX )
	PUSH_L( EBP )

	MOV_L( ARG_SOURCE, ESI )
	MOV_L( ARG_DEST, EDI )

	MOV_L( ARG_MATRIX, EDX )
	MOV_L( ARG_CLIP, EBP )

	MOV_L( REGOFF(V4F_COUNT, ESI), ECX )
	MOV_B( ARG_FLAG, BL )

	TEST_L( ECX, ECX )
	JZ( LLBL( x86_p2_2dm_done ) )

	MOV_L( REGOFF(V4F_STRIDE, ESI), EAX )
	OR_L( CONST(VEC_SIZE_2), REGOFF(V4F_FLAGS, EDI) )

	MOV_L( ECX, REGOFF(V4F_COUNT, EDI) )
	MOV_L( CONST(2), REGOFF(V4F_SIZE, EDI) )

	SHL_L( CONST(4), ECX )
	MOV_L( REGOFF(V4F_START, ESI), ESI )

	MOV_L( REGOFF(V4F_START, EDI), EDI )
	ADD_L( EDI, ECX )

ALIGNTEXT16
LLBL( x86_p2_2dm_loop ):

	TEST_B( BL, REGIND(EBP) )
	JNZ( LLBL( x86_p2_2dm_skip ) )

	FLD_S( SRC(0) )			/* F4 */
	FMUL_S( MAT(0) )
	FLD_S( SRC(0) )			/* F5 F4 */
	FMUL_S( MAT(1) )

	FLD_S( SRC(1) )			/* F0 F5 F4 */
	FMUL_S( MAT(4) )
	FLD_S( SRC(1) )			/* F1 F0 F5 F4 */
	FMUL_S( MAT(5) )

	FXCH( ST(1) )			/* F0 F1 F5 F4 */
	FADDP( ST(0), ST(3) )		/* F1 F5 F4 */
	FADDP( ST(0), ST(1) )		/* F5 F4 */

	FXCH( ST(1) )			/* F4 F5 */
	FADD_S( MAT(12) )
	FXCH( ST(1) )			/* F5 F4 */
	FADD_S( MAT(13) )

	FXCH( ST(1) )			/* F4 F5 */
	FSTP_S( DST(0) )		/* F5 */
	FSTP_S( DST(1) )		/* */

LLBL( x86_p2_2dm_skip ):

	INC_L( EBP )
	ADD_L( CONST(16), EDI )
	ADD_L( EAX, ESI )
	CMP_L( ECX, EDI )
	JNE( LLBL( x86_p2_2dm_loop ) )

LLBL( x86_p2_2dm_done ):

	POP_L( EBP )
	POP_L( EBX )
	POP_L( EDI )
	POP_L( ESI )
	RET




ALIGNTEXT16
GLOBL GLNAME( gl_x86_transform_points2_2d_no_rot_masked )
GLNAME( gl_x86_transform_points2_2d_no_rot_masked ):

	PUSH_L( ESI )
	PUSH_L( EDI )

	PUSH_L( EBX )
	PUSH_L( EBP )

	MOV_L( ARG_SOURCE, ESI )
	MOV_L( ARG_DEST, EDI )

	MOV_L( ARG_MATRIX, EDX )
	MOV_L( ARG_CLIP, EBP )

	MOV_L( REGOFF(V4F_COUNT, ESI), ECX )
	MOV_B( ARG_FLAG, BL )

	TEST_L( ECX, ECX )
	JZ( LLBL( x86_p2_2dnrm_done ) )

	MOV_L( REGOFF(V4F_STRIDE, ESI), EAX )
	OR_L( CONST(VEC_SIZE_2), REGOFF(V4F_FLAGS, EDI) )

	MOV_L( ECX, REGOFF(V4F_COUNT, EDI) )
	MOV_L( CONST(2), REGOFF(V4F_SIZE, EDI) )

	SHL_L( CONST(4), ECX )
	MOV_L( REGOFF(V4F_START, ESI), ESI )

	MOV_L( REGOFF(V4F_START, EDI), EDI )
	ADD_L( EDI, ECX )

ALIGNTEXT16
LLBL( x86_p2_2dnrm_loop ):

	TEST_B( BL, REGIND(EBP) )
	JNZ( LLBL( x86_p2_2dnrm_skip ) )

	FLD_S( SRC(0) )			/* F4 */
	FMUL_S( MAT(0) )

	FLD_S( SRC(1) )			/* F1 F4 */
	FMUL_S( MAT(5) )

	FXCH( ST(1) )			/* F4 F1 */
	FADD_S( MAT(12) )
	FLD_S( MAT(13) )		/* F5 F4 F1 */
	FXCH( ST(2) )			/* F1 F4 F5 */
	FADDP( ST(0), ST(2) )		/* F4 F5 */

	FSTP_S( DST(0) )		/* F5 */
	FSTP_S( DST(1) )		/* */

LLBL( x86_p2_2dnrm_skip ):

	INC_L( EBP )
	ADD_L( CONST(16), EDI )
	ADD_L( EAX, ESI )
	CMP_L( ECX, EDI )
	JNE( LLBL( x86_p2_2dnrm_loop ) )

LLBL( x86_p2_2dnrm_done ):

	POP_L( EBP )
	POP_L( EBX )
	POP_L( EDI )
	POP_L( ESI )
	RET




ALIGNTEXT16
GLOBL GLNAME( gl_x86_transform_points2_identity_masked )
GLNAME( gl_x86_transform_points2_identity_masked ):

	PUSH_L( ESI )
	PUSH_L( EDI )

	PUSH_L( EBX )
	PUSH_L( EBP )

	MOV_L( ARG_SOURCE, ESI )
	MOV_L( ARG_DEST, EDI )

	MOV_L( ARG_MATRIX, EDX )
	MOV_L( ARG_CLIP, EBP )

	MOV_L( REGOFF(V4F_COUNT, ESI), ECX )
	MOV_B( ARG_FLAG, BL )

	TEST_L( ECX, ECX )
	JZ( LLBL( x86_p2_im_done ) )

	MOV_L( REGOFF(V4F_STRIDE, ESI), EAX )
	OR_L( CONST(VEC_SIZE_2), REGOFF(V4F_FLAGS, EDI) )

	MOV_L( ECX, REGOFF(V4F_COUNT, EDI) )
	MOV_L( CONST(2), REGOFF(V4F_SIZE, EDI) )

	SHL_L( CONST(4), ECX )
	MOV_L( REGOFF(V4F_START, ESI), ESI )

	MOV_L( REGOFF(V4F_START, EDI), EDI )
	ADD_L( EDI, ECX )

	/* need eax; put stride in ARG_SOURCE */
	MOV_L( EAX, ARG_SOURCE )

	CMP_L( ESI, EDI )
	JE( LLBL( x86_p2_im_done ) )

ALIGNTEXT16
LLBL( x86_p2_im_loop ):

	TEST_B( BL, REGIND(EBP) )
	JNZ( LLBL( x86_p2_im_skip ) )

	MOV_L( SRC(0), EAX )
	MOV_L( SRC(1), EDX )

	MOV_L( EAX, DST(0) )
	MOV_L( EDX, DST(1) )

LLBL( x86_p2_im_skip ):

	INC_L( EBP )
	ADD_L( CONST(16), EDI )
	ADD_L( ARG_SOURCE, ESI )
	CMP_L( ECX, EDI )
	JNE( LLBL( x86_p2_im_loop ) )

LLBL( x86_p2_im_done ):

	POP_L( EBP )
	POP_L( EBX )
	POP_L( EDI )
	POP_L( ESI )
	RET
