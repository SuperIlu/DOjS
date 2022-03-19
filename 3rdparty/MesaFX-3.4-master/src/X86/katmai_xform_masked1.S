#include "assyntax.h"
/** TODO:
  * - insert PREFETCH instructions to avoid cache-misses !
  * - some more optimizations are possible...
  * - for 40-50% more performance in the SSE-functions, the
  *   data (trans-matrix, src_vert, dst_vert) needs to be 16byte aligned !
  */

   SEG_TEXT


#define S(i) 	REGOFF(i * 4, ESI)
#define D(i) 	REGOFF(i * 4, EDI)
#define M(i) 	REGOFF(i * 4, EDX)

/*
 * 	Offsets into GLvector4f
 */
#define V4F_DATA 	0
#define V4F_START 	4
#define V4F_COUNT 	8
#define V4F_STRIDE 	12
#define V4F_SIZE 	16
#define V4F_FLAGS 	20

#define VEC_SIZE_1   1
#define VEC_SIZE_2   3
#define VEC_SIZE_3   7
#define VEC_SIZE_4   15

/*
 *	Offsets for transform_func arguments
 *
 *	typedef void (*transform_func)( GLvector4f *to_vec,
 *	                                const GLfloat m[16],
 *	                                const GLvector4f *from_vec,
 *	                                const GLubyte *clipmask,
 *	                                const GLubyte flag );
 */
#define OFFSET_DEST 4
#define OFFSET_MATRIX 8
#define OFFSET_SOURCE 12
#define OFFSET_CLIP 16
#define OFFSET_FLAG 20

#define ARG_DEST 	REGOFF(FRAME_OFFSET+OFFSET_DEST, ESP)
#define ARG_MATRIX 	REGOFF(FRAME_OFFSET+OFFSET_MATRIX, ESP)
#define ARG_SOURCE 	REGOFF(FRAME_OFFSET+OFFSET_SOURCE, ESP)
#define ARG_CLIP 	REGOFF(FRAME_OFFSET+OFFSET_CLIP, ESP)
#define ARG_FLAG 	REGOFF(FRAME_OFFSET+OFFSET_FLAG, ESP)


ALIGNTEXT4
GLOBL GLNAME(gl_katmai_transform_points1_general_masked)
GLNAME( gl_katmai_transform_points1_general_masked ):

#define FRAME_OFFSET 16
    PUSH_L    ( ESI )
    PUSH_L    ( EDI )
    PUSH_L    ( EBX )
    PUSH_L    ( EBP )

    MOV_L( ARG_SOURCE, ESI )			/* ptr to source GLvector4f */
    MOV_L( ARG_DEST, EDI )			/* ptr to dest GLvector4f */

    MOV_L( ARG_MATRIX, EDX ) 			/* ptr to matrix */
    MOV_L( ARG_CLIP, EBP )			/* ptr to clip mask array */

    MOV_L( REGOFF(V4F_COUNT, ESI), ECX ) 	/* source count */
    MOV_B( ARG_FLAG, BL )			/* clip mask flags */

    TEST_L( ECX, ECX )				/* count was zero ? */
    JZ( LLBL(K_GTP1GM_finish) )			/* yes -> nothing to do. */

    MOV_L( REGOFF(V4F_STRIDE, ESI), EAX ) 	/* stride */
    OR_L( CONST(VEC_SIZE_4), REGOFF(V4F_FLAGS, EDI) ) 	/* set dest flags */

    MOV_L( ECX, REGOFF(V4F_COUNT, EDI) ) 	/* set dest count */
    MOV_L( CONST(4), REGOFF(V4F_SIZE, EDI) ) 	/* set dest size */

    SHL_L( CONST(4), ECX ) 			/* count *= 16 */
    MOV_L( REGOFF(V4F_START, ESI), ESI ) 	/* ptr to first source vertex */

    MOV_L( REGOFF(V4F_START, EDI), EDI ) 	/* ptr to first dest vertex */
    ADD_L( EDI, ECX ) 				/* count += dest ptr */


ALIGNTEXT32
    MOVAPS( M(0), XMM0 )			/* m3  | m2  | m1  | m0  */
    MOVAPS( M(12), XMM1 )			/* m15 | m14 | m13 | m12 */

ALIGNTEXT32
LLBL(K_GTP1GM_top):
    TEST_B( BL, REGIND(EBP) )
    JNZ( LLBL(K_GTP1GM_skip) )

    MOVSS( S(0), XMM2 )				/* ox */
    SHUFPS( CONST(0x0), XMM2, XMM2 )		/* ox | ox | ox | ox */
    MULPS( XMM0, XMM2 )				/* ox*m3 | ox*m2 | ox*m1 | ox*m0 */
    ADDPS( XMM1, XMM2 )				/* + | + | + | + */
    MOVAPS( XMM2, D(0) )

LLBL(K_GTP1GM_skip):
    INC_L     ( EBP )
    ADD_L     ( CONST(16), EDI )
    ADD_L     ( EAX, ESI )
    CMP_L     ( ECX, EDI )
    JNE       ( LLBL(K_GTP1GM_top) )

LLBL(K_GTP1GM_finish):
    POP_L     ( EBP )
    POP_L     ( EBX )
    POP_L     ( EDI )
    POP_L     ( ESI )
    RET
#undef FRAME_OFFSET



ALIGNTEXT4
GLOBL GLNAME(gl_katmai_transform_points1_identity_masked)
GLNAME( gl_katmai_transform_points1_identity_masked ):

#define FRAME_OFFSET 16
    PUSH_L    ( ESI )
    PUSH_L    ( EDI )
    PUSH_L    ( EBX )
    PUSH_L    ( EBP )

    MOV_L( ARG_SOURCE, ESI )			/* ptr to source GLvector4f */
    MOV_L( ARG_DEST, EDI )			/* ptr to dest GLvector4f */

    MOV_L( REGOFF(V4F_COUNT, ESI), ECX ) 	/* source count */

    TEST_L( ECX, ECX )
    JZ( LLBL(K_GTP1IM_finish) ) 		/* count was zero; go to finish */

    MOV_L( ARG_CLIP, EBP )			/* ptr to clip mask array */
    MOV_B( ARG_FLAG, BL )			/* clip mask flags */

    MOV_L( REGOFF(V4F_STRIDE, ESI), EAX ) 	/* stride */
    OR_L( CONST(VEC_SIZE_1), REGOFF(V4F_FLAGS, EDI) ) 	/* set dest flags */

    MOV_L( ECX, REGOFF(V4F_COUNT, EDI) ) 	/* set dest count */
    MOV_L( CONST(1), REGOFF(V4F_SIZE, EDI) ) 	/* set dest size */

    SHL_L( CONST(4), ECX ) 			/* count *= 16 */
    MOV_L( REGOFF(V4F_START, ESI), ESI ) 	/* ptr to first source vertex */

    MOV_L( REGOFF(V4F_START, EDI), EDI ) 	/* ptr to first dest vertex */
    ADD_L( EDI, ECX ) 				/* count += dest ptr */

    CMP_L( ESI, EDI )
    JE( LLBL(K_GTP1IM_finish) )

ALIGNTEXT32
LLBL(K_GTP1IM_top):
    TEST_B( BL, REGIND(EBP) )
    JNZ( LLBL(K_GTP1IM_skip) )

    MOV_L( S(0), EDX )
    MOV_L( EDX, D(0) )

LLBL(K_GTP1IM_skip):
    INC_L     ( EBP )
    ADD_L     ( CONST(16), EDI )
    ADD_L     ( EAX, ESI )
    CMP_L     ( ECX, EDI )
    JNE       ( LLBL(K_GTP1IM_top) )

LLBL(K_GTP1IM_finish):
    POP_L     ( EBP )
    POP_L     ( EBX )
    POP_L     ( EDI )
    POP_L     ( ESI )
    RET
#undef FRAME_OFFSET


ALIGNTEXT4
GLOBL GLNAME(gl_katmai_transform_points1_3d_no_rot_masked)
GLNAME(gl_katmai_transform_points1_3d_no_rot_masked):

#define FRAME_OFFSET 16
    PUSH_L( ESI )
    PUSH_L( EDI )
    PUSH_L( EBX )
    PUSH_L( EBP )

    MOV_L( ARG_SOURCE, ESI )			/* ptr to source GLvector4f */
    MOV_L( ARG_DEST, EDI )			/* ptr to dest GLvector4f */

    MOV_L( ARG_MATRIX, EDX ) 			/* ptr to matrix */
    MOV_L( ARG_CLIP, EBP )			/* ptr to clip mask array */

    MOV_L( REGOFF(V4F_COUNT, ESI), ECX ) 	/* source count */
    MOV_B( ARG_FLAG, BL )			/* clip mask flags */

    TEST_L( ECX, ECX )
    JZ( LLBL(K_GTP13DNRM_finish) ) 		/* count was zero; go to finish */

    MOV_L( REGOFF(V4F_STRIDE, ESI), EAX ) 	/* stride */
    OR_L( CONST(VEC_SIZE_3), REGOFF(V4F_FLAGS, EDI) ) 	/* set dest flags */

    MOV_L( ECX, REGOFF(V4F_COUNT, EDI) ) 	/* set dest count */
    MOV_L( CONST(3), REGOFF(V4F_SIZE, EDI) ) 	/* set dest size */

    SHL_L( CONST(4), ECX ) 			/* count *= 16 */
    MOV_L( REGOFF(V4F_START, ESI), ESI ) 	/* ptr to first source vertex */

    MOV_L( REGOFF(V4F_START, EDI), EDI ) 	/* ptr to first dest vertex */
    ADD_L( EDI, ECX ) 				/* count += dest ptr */


ALIGNTEXT32
    MOVSS( M(0), XMM0 )				/* m0 */
    MOVSS( M(12), XMM1 )			/* m12 */
    MOVSS( M(13), XMM2 )			/* m13 */
    MOVSS( M(14), XMM3 )			/* m14 */

ALIGNTEXT32
LLBL(K_GTP13DNRM_top):
    TEST_B( BL, REGIND(EBP) )
    JNZ( LLBL(K_GTP13DNRM_skip) )

    MOVSS( S(0), XMM4 )				/* ox */
    MULSS( XMM0, XMM4 )				/* ox*m0 */
    ADDSS( XMM1, XMM4 )				/* ox*m0+m12 */
    MOVSS( XMM4, D(0) )

    MOVSS( XMM2, D(1) )
    MOVSS( XMM3, D(2) )

LLBL(K_GTP13DNRM_skip):
    INC_L    ( EBP )
    ADD_L    ( CONST(16), EDI )
    ADD_L    ( EAX, ESI )
    CMP_L    ( ECX, EDI )
    JNE      ( LLBL(K_GTP13DNRM_top) )

LLBL(K_GTP13DNRM_finish):
    POP_L    ( EBP )
    POP_L    ( EBX )
    POP_L    ( EDI )
    POP_L    ( ESI )
    RET
#undef FRAME_OFFSET


ALIGNTEXT4
GLOBL GLNAME(gl_katmai_transform_points1_perspective_masked)
GLNAME(gl_katmai_transform_points1_perspective_masked):

#define FRAME_OFFSET 16
    PUSH_L   ( ESI )
    PUSH_L   ( EDI )
    PUSH_L   ( EBX )
    PUSH_L   ( EBP )

    MOV_L( ARG_SOURCE, ESI )			/* ptr to source GLvector4f */
    MOV_L( ARG_DEST, EDI )			/* ptr to dest GLvector4f */

    MOV_L( ARG_MATRIX, EDX ) 			/* ptr to matrix */
    MOV_L( ARG_CLIP, EBP )			/* ptr to clip mask array */
    MOV_B( ARG_FLAG, BL )			/* clip mask flags */
    MOV_L( REGOFF(V4F_COUNT, ESI), ECX ) 	/* source count */

    TEST_L( ECX, ECX)
    JZ( LLBL(K_GTP13PM_finish) )		/* count was zero; go to finish */

    MOV_L( REGOFF(V4F_STRIDE, ESI), EAX ) 	/* stride */
    OR_L( CONST(VEC_SIZE_4), REGOFF(V4F_FLAGS, EDI) ) 	/* set dest flags */

    MOV_L( ECX, REGOFF(V4F_COUNT, EDI) ) 	/* set dest count */
    MOV_L( CONST(4), REGOFF(V4F_SIZE, EDI) ) 	/* set dest size */

    SHL_L( CONST(4), ECX ) 			/* count *= 16 */
    MOV_L( REGOFF(V4F_START, ESI), ESI ) 	/* ptr to first source vertex */

    MOV_L( REGOFF(V4F_START, EDI), EDI ) 	/* ptr to first dest vertex */
    ADD_L( EDI, ECX ) 				/* count += dest ptr */

ALIGNTEXT32
    XORPS( XMM0, XMM0 )				/* 0 | 0 | 0 | 0 */
    MOVSS( M(0), XMM1 )				/* m0 */
    MOVSS( M(14), XMM2 )			/* m14 */

ALIGNTEXT32
LLBL(K_GTP13PM_top):
    TEST_B   ( BL, REGIND(EBP) )
    JNZ      ( LLBL(K_GTP13PM_skip) )

    MOVSS( S(0), XMM3 )				/* ox */
    MULSS( XMM1, XMM3 )				/* ox*m0 */
    MOVSS( XMM3, D(0) )				/* ox*m0->D(0) */
    MOVSS( XMM2, D(2) )				/* m14->D(2) */

    MOVSS( XMM0, D(1) )
    MOVSS( XMM0, D(3) )

LLBL(K_GTP13PM_skip):
    INC_L( EBP )
    ADD_L( CONST(16), EDI )
    ADD_L( EAX, ESI )
    CMP_L( ECX, EDI )
    JNE( LLBL(K_GTP13PM_top) )

LLBL(K_GTP13PM_finish):
    POP_L    ( EBP )
    POP_L    ( EBX )
    POP_L    ( EDI )
    POP_L    ( ESI )
    RET
#undef FRAME_OFFSET


ALIGNTEXT4
GLOBL GLNAME(gl_katmai_transform_points1_2d_masked)
GLNAME(gl_katmai_transform_points1_2d_masked):

#define FRAME_OFFSET 16
    PUSH_L( ESI )
    PUSH_L( EDI )
    PUSH_L( EBX )
    PUSH_L( EBP )

    MOV_L( ARG_SOURCE, ESI )			/* ptr to source GLvector4f */
    MOV_L( ARG_DEST, EDI )			/* ptr to dest GLvector4f */

    MOV_L( ARG_MATRIX, EDX ) 			/* ptr to matrix */
    MOV_L( ARG_CLIP, EBP )			/* ptr to clip mask array */
    MOV_L( REGOFF(V4F_COUNT, ESI), ECX ) 	/* source count */
    MOV_B( ARG_FLAG, BL )			/* clip mask flags */

    TEST_L( ECX, ECX)
    JZ( LLBL(K_GTP13P2DM_finish) ) 		/* count was zero; go to finish */

    MOV_L( REGOFF(V4F_STRIDE, ESI), EAX ) 	/* stride */
    OR_L( CONST(VEC_SIZE_2), REGOFF(V4F_FLAGS, EDI) ) 	/* set dest flags */

    MOV_L( ECX, REGOFF(V4F_COUNT, EDI) ) 	/* set dest count */
    MOV_L( CONST(2), REGOFF(V4F_SIZE, EDI) ) 	/* set dest size */

    SHL_L( CONST(4), ECX ) 			/* count *= 16 */
    MOV_L( REGOFF(V4F_START, ESI), ESI ) 	/* ptr to first source vertex */

    MOV_L( REGOFF(V4F_START, EDI), EDI ) 	/* ptr to first dest vertex */
    ADD_L( EDI, ECX ) 				/* count += dest ptr */

ALIGNTEXT32
    MOVLPS( M(0), XMM0 )			/* m1  | m0  */
    MOVLPS( M(12), XMM1 )			/* m13 | m12 */

ALIGNTEXT32
LLBL(K_GTP13P2DM_top):
    TEST_B( BL, REGIND(EBP) )
    JNZ( LLBL(K_GTP13P2DM_skip) )

    MOVSS( S(0), XMM2 )				/* ox */
    SHUFPS( CONST(0x0), XMM2, XMM2 )		/* ox | ox | ox | ox */
    MULPS( XMM0, XMM2 )				/* - | - | ox*m1 | ox*m0 */
    ADDPS( XMM1, XMM2 )				/* - | - | ox*m1+m13 | ox*m0+m12 */
    MOVLPS( XMM2, D(0) )

LLBL(K_GTP13P2DM_skip):
    INC_L    ( EBP )
    ADD_L    ( CONST(16), EDI )
    ADD_L    ( EAX, ESI )
    CMP_L    ( ECX, EDI )
    JNE      ( LLBL(K_GTP13P2DM_top) )

LLBL(K_GTP13P2DM_finish):
    POP_L    ( EBP )
    POP_L    ( EBX )
    POP_L    ( EDI )
    POP_L    ( ESI )
    RET
#undef FRAME_OFFSET


ALIGNTEXT4
GLOBL GLNAME(gl_katmai_transform_points1_2d_no_rot_masked)
GLNAME(gl_katmai_transform_points1_2d_no_rot_masked):

#define FRAME_OFFSET 16
	PUSH_L( ESI )
	PUSH_L( EDI )
	PUSH_L( EBX )
	PUSH_L( EBP )

	MOV_L( ARG_SOURCE, ESI )			/* ptr to source GLvector4f */
	MOV_L( ARG_DEST, EDI )				/* ptr to dest GLvector4f */

	MOV_L( ARG_MATRIX, EDX )			/* ptr to matrix */
	MOV_L( ARG_CLIP, EBP )				/* ptr to clip mask array */

	MOV_L( REGOFF(V4F_COUNT, ESI), ECX )		/* source count */
	MOV_B( ARG_FLAG, BL )				/* clip mask flags */

	TEST_L( ECX, ECX)
	JZ( LLBL(K_GTP13P2DNRM_finish) )		/* count was zero; go to finish */

	MOV_L( REGOFF(V4F_STRIDE, ESI), EAX )		/* stride */
	OR_L( CONST(VEC_SIZE_2), REGOFF(V4F_FLAGS, EDI) ) 	/* set dest flags */

	MOV_L( ECX, REGOFF(V4F_COUNT, EDI) )		/* set dest count */
	MOV_L( CONST(2), REGOFF(V4F_SIZE, EDI) ) 	/* set dest size */

	SHL_L( CONST(4), ECX )				/* count *= 16 */
	MOV_L( REGOFF(V4F_START, ESI), ESI )		/* ptr to first source vertex */

	MOV_L( REGOFF(V4F_START, EDI), EDI )		/* ptr to first dest vertex */
	ADD_L( EDI, ECX )				/* count += dest ptr */

ALIGNTEXT32
	MOVSS( M(0), XMM0 )			/* m0 */
	MOVSS( M(12), XMM1 )			/* m12 */
	MOVSS( M(13), XMM2 )			/* m13 */

ALIGNTEXT32
LLBL(K_GTP13P2DNRM_top):
	TEST_B( BL, REGIND(EBP) )
	JNZ( LLBL(K_GTP13P2DNRM_skip) )

	MOVSS( S(0), XMM3 )			/* ox */
	MULSS( XMM0, XMM3 )			/* ox*m0 */
	ADDSS( XMM1, XMM3 )			/* ox*m0+m12 */
	MOVSS( XMM3, D(0) )
	MOVSS( XMM2, D(1) )

LLBL(K_GTP13P2DNRM_skip):
	INC_L( EBP )
	ADD_L( CONST(16), EDI )
	ADD_L( EAX, ESI )
	CMP_L( ECX, EDI )
	JNE( LLBL(K_GTP13P2DNRM_top) )

LLBL(K_GTP13P2DNRM_finish):
	POP_L( EBP )
	POP_L( EBX )
	POP_L( EDI )
	POP_L( ESI )
	RET
#undef FRAME_OFFSET


ALIGNTEXT4
GLOBL GLNAME(gl_katmai_transform_points1_3d_masked)
GLNAME(gl_katmai_transform_points1_3d_masked):

#define FRAME_OFFSET 16
	PUSH_L( ESI )
	PUSH_L( EDI )
	PUSH_L( EBX )
	PUSH_L( EBP )

	MOV_L( ARG_SOURCE, ESI )		/* ptr to source GLvector4f */
	MOV_L( ARG_DEST, EDI )			/* ptr to dest GLvector4f */

	MOV_L( ARG_MATRIX, EDX ) 		/* ptr to matrix */
	MOV_L( ARG_CLIP, EBP ) 			/* ptr to clip mask array */

	MOV_L( REGOFF(V4F_COUNT, ESI), ECX ) 	/* source count */
	MOV_B( ARG_FLAG, BL ) 			/* clip mask flags */

	TEST_L( ECX, ECX)
	JZ( LLBL(K_GTP13P3DM_finish) )		/* count was zero; go to finish */

	MOV_L( REGOFF(V4F_STRIDE, ESI), EAX ) 	/* stride */
	OR_L( CONST(VEC_SIZE_3), REGOFF(V4F_FLAGS, EDI) ) 	/* set dest flags */

	MOV_L( ECX, REGOFF(V4F_COUNT, EDI) ) 	/* set dest count */
	MOV_L( CONST(3), REGOFF(V4F_SIZE, EDI) ) 	/* set dest size */

	SHL_L( CONST(4), ECX ) 			/* count *= 16 */
	MOV_L( REGOFF(V4F_START, ESI), ESI ) 	/* ptr to first source vertex */

	MOV_L( REGOFF(V4F_START, EDI), EDI ) 	/* ptr to first dest vertex */
	ADD_L( EDI, ECX ) 			/* count += dest ptr */


ALIGNTEXT32
	MOVAPS( M(0), XMM0 )			/* m3  | m2  | m1  |  m0 */
	MOVAPS( M(12), XMM1 )			/* m15 | m14 | m13 | m12 */

ALIGNTEXT32
LLBL(K_GTP13P3DM_top):
	TEST_B( BL, REGIND(EBP) )
	JNZ( LLBL(K_GTP13P3DM_skip) )

	MOVSS( S(0), XMM2 )			/* ox */
	SHUFPS( CONST(0x0), XMM2, XMM2 )	/* ox | ox | ox | ox */
	MULPS( XMM0, XMM2 )			/* ox*m3 | ox*m2 | ox*m1 | ox*m0 */
	ADDPS( XMM1, XMM2 )			/* +m15  | +m14  | +m13  | +m12  */
	MOVLPS( XMM2, D(0) )			/*   -   |   -   | ->D(1)| ->D(0)*/
	UNPCKHPS( XMM2, XMM2 )			/* ox*m3+m15 | ox*m3+m15 | ox*m2+m14 | ox*m2+m14 */
	MOVSS( XMM2, D(2) )			/* ->D(2) */

LLBL(K_GTP13P3DM_skip):
	INC_L( EBP )
	ADD_L( CONST(16), EDI )
	ADD_L( EAX, ESI )
	CMP_L( ECX, EDI )
	JNE( LLBL(K_GTP13P3DM_top) )

LLBL(K_GTP13P3DM_finish):
	POP_L( EBP )
	POP_L( EBX )
	POP_L( EDI )
	POP_L( ESI )
	RET
#undef FRAME_OFFSET
