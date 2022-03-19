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
GLOBL GLNAME(gl_katmai_transform_points4_general_masked)
GLNAME( gl_katmai_transform_points4_general_masked ):

#define FRAME_OFFSET 16
	PUSH_L    ( ESI )
	PUSH_L    ( EDI )
	PUSH_L    ( EBX )
	PUSH_L    ( EBP )

	MOV_L( ARG_SOURCE, ESI )			/* ptr to source GLvector4f */
	MOV_L( ARG_DEST, EDI )				/* ptr to dest GLvector4f */

	MOV_L( ARG_MATRIX, EDX ) 			/* ptr to matrix */
	MOV_L( ARG_CLIP, EBP )				/* ptr to clip mask array */

	MOV_L( REGOFF(V4F_COUNT, ESI), ECX )		/* source count */
	MOV_B( ARG_FLAG, BL )				/* clip mask flags */

	TEST_L( ECX, ECX )				/* count was zero ? */
	JZ( LLBL(K_GTP4GM_finish) )			/* yes -> nothing to do. */

	MOV_L( REGOFF(V4F_STRIDE, ESI), EAX )		/* stride */
	OR_L( CONST(VEC_SIZE_4), REGOFF(V4F_FLAGS, EDI) ) 	/* set dest flags */

	MOV_L( ECX, REGOFF(V4F_COUNT, EDI) )		/* set dest count */
	MOV_L( CONST(4), REGOFF(V4F_SIZE, EDI) ) 	/* set dest size */

	SHL_L( CONST(4), ECX )				/* count *= 16 */
	MOV_L( REGOFF(V4F_START, ESI), ESI )		/* ptr to first source vertex */

	MOV_L( REGOFF(V4F_START, EDI), EDI )		/* ptr to first dest vertex */
	ADD_L( EDI, ECX ) 				/* count += dest ptr */

ALIGNTEXT32
	MOVAPS( M(0), XMM0 )				/* m3  | m2  | m1  | m0 */
	MOVAPS( M(4), XMM1 )				/* m7  | m6  | m5  | m4 */
	MOVAPS( M(8), XMM2 )				/* m11 | m10 | m9  | m8 */
	MOVAPS( M(12), XMM3 )				/* m15 | m14 | m13 | m12 */

ALIGNTEXT32
LLBL(K_GTP4GM_top):
	TEST_B( BL, REGIND(EBP) )
	JNZ( LLBL(K_GTP4GM_skip) )

	MOVSS( S(0), XMM4 )				/* ox */
	SHUFPS( CONST(0x0), XMM4, XMM4 )		/* ox | ox | ox | ox */
	MULPS( XMM0, XMM4 )				/* ox*m3 | ox*m2 | ox*m1 | ox*m0 */

	MOVSS( S(1), XMM5 )				/* oy */
	SHUFPS( CONST(0x0), XMM5, XMM5 )		/* oy | oy | oy | oy */
	MULPS( XMM1, XMM5 )				/* oy*m7 | oy*m6 | oy*m5 | oy*m4 */

	MOVSS( S(2), XMM6 )				/* oz */
	SHUFPS( CONST(0x0), XMM6, XMM6 )		/* oz | oz | oz | oz */
	MULPS( XMM2, XMM6 )				/* oz*m11 | oz*m10 | oz*m9 | oz*m8 */

	MOVSS( S(3), XMM7 )				/* ow */
	SHUFPS( CONST(0x0), XMM7, XMM7 )		/* ow | ow | ow | ow */
	MULPS( XMM3, XMM7 )				/* ow*m15 | ow*m14 | ow*m13 | ow*m12 */

	ADDPS( XMM5, XMM4 )				/* ox*m3+oy*m7 | ... */
	ADDPS( XMM6, XMM4 )				/* ox*m3+oy*m7+oz*m11 | ... */
	ADDPS( XMM7, XMM4 )				/* ox*m3+oy*m7+oz*m11+ow*m15 | ... */
	MOVAPS( XMM4, D(0) )				/* ->D(3) | ->D(2) | ->D(1) | ->D(0) */

LLBL(K_GTP4GM_skip):
	INC_L     ( EBP )
	ADD_L     ( CONST(16), EDI )
	ADD_L     ( EAX, ESI )
	CMP_L     ( ECX, EDI )
	JNE       ( LLBL(K_GTP4GM_top) )

LLBL(K_GTP4GM_finish):
	POP_L     ( EBP )
	POP_L     ( EBX )
	POP_L     ( EDI )
	POP_L     ( ESI )
	RET
#undef FRAME_OFFSET


ALIGNTEXT4
GLOBL GLNAME(gl_katmai_transform_points4_identity_masked)
GLNAME( gl_katmai_transform_points4_identity_masked ):

#define FRAME_OFFSET 16
	PUSH_L    ( ESI )
	PUSH_L    ( EDI )
	PUSH_L    ( EBX )
	PUSH_L    ( EBP )

	MOV_L( ARG_SOURCE, ESI )			/* ptr to source GLvector4f */
	MOV_L( ARG_DEST, EDI )				/* ptr to dest GLvector4f */

	MOV_L( REGOFF(V4F_COUNT, ESI), ECX )		/* source count */

	TEST_L( ECX, ECX )
	JZ( LLBL(K_GTP4IM_finish) )			/* count was zero; go to finish */

	MOV_L( ARG_CLIP, EBP )				/* ptr to clip mask array */
	MOV_B( ARG_FLAG, BL )				/* clip mask flags */

	MOV_L( REGOFF(V4F_STRIDE, ESI), EAX )		/* stride */
	OR_L( CONST(VEC_SIZE_4), REGOFF(V4F_FLAGS, EDI) ) 	/* set dest flags */

	MOV_L( ECX, REGOFF(V4F_COUNT, EDI) )		/* set dest count */
	MOV_L( CONST(4), REGOFF(V4F_SIZE, EDI) ) 	/* set dest size */

	SHL_L( CONST(4), ECX )				/* count *= 16 */
	MOV_L( REGOFF(V4F_START, ESI), ESI )		/* ptr to first source vertex */

	MOV_L( REGOFF(V4F_START, EDI), EDI )		/* ptr to first dest vertex */
	ADD_L( EDI, ECX ) 				/* count += dest ptr */

	MOV_L( EAX, ARG_SOURCE )

	CMP_L( ESI, EDI )
	JE( LLBL(K_GTP4IM_finish) )

ALIGNTEXT16
LLBL(K_GTP4IM_top):
	TEST_B( BL, REGIND(EBP) )
	JNZ( LLBL(K_GTP4IM_skip) )

	MOV_L( S(0), EAX )
	MOV_L( S(1), EDX )
	MOV_L( EAX, D(0) )
	MOV_L( EDX, D(1) )

	MOV_L( S(2), EAX )
	MOV_L( S(3), EDX )
	MOV_L( EAX, D(2) )
	MOV_L( EDX, D(3) )

LLBL(K_GTP4IM_skip):
	INC_L     ( EBP )
	ADD_L     ( CONST(16), EDI )
	ADD_L     ( ARG_SOURCE, ESI )
	CMP_L     ( ECX, EDI )
	JNE       ( LLBL(K_GTP4IM_top) )

LLBL(K_GTP4IM_finish):
	POP_L     ( EBP )
	POP_L     ( EBX )
	POP_L     ( EDI )
	POP_L     ( ESI )
	RET
#undef FRAME_OFFSET


ALIGNTEXT4
GLOBL GLNAME(gl_katmai_transform_points4_3d_no_rot_masked)
GLNAME(gl_katmai_transform_points4_3d_no_rot_masked):

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

	TEST_L( ECX, ECX )
	JZ( LLBL(K_GTP43DNRM_finish) )			/* count was zero; go to finish */

	MOV_L( REGOFF(V4F_STRIDE, ESI), EAX )		/* stride */
	OR_L( CONST(VEC_SIZE_4), REGOFF(V4F_FLAGS, EDI) ) 	/* set dest flags */

	MOV_L( ECX, REGOFF(V4F_COUNT, EDI) )		/* set dest count */
	MOV_L( CONST(4), REGOFF(V4F_SIZE, EDI) )	/* set dest size */

	SHL_L( CONST(4), ECX )				/* count *= 16 */
	MOV_L( REGOFF(V4F_START, ESI), ESI )		/* ptr to first source vertex */

	MOV_L( REGOFF(V4F_START, EDI), EDI )		/* ptr to first dest vertex */
	ADD_L( EDI, ECX ) 				/* count += dest ptr */


ALIGNTEXT32
	MOVSS( M(0), XMM0 )			/* m0 */
	MOVSS( M(5), XMM1 )			/* m5 */
	UNPCKLPS( XMM1, XMM0 )			/* m5 | m0 */
	MOVSS( M(10), XMM1 )			/* m10 */
	MOVLPS( M(12), XMM2 )			/* m13 | m12 */
	MOVSS( M(14), XMM3 )			/* m14 */

ALIGNTEXT32
LLBL(K_GTP43DNRM_top):
	TEST_B( BL, REGIND(EBP) )
	JNZ( LLBL(K_GTP43DNRM_skip) )

	MOVLPS( S(0), XMM4 )			/* oy | ox */
	MULPS( XMM0, XMM4 )			/* oy*m5 | ox*m0 */
	MOVSS( S(3), XMM5 )			/* ow */
	SHUFPS( CONST(0x0), XMM5, XMM5 )	/* ow | ow | ow | ow */
	MULPS( XMM2, XMM5 )			/* ow*m13 | ow*m12 */
	ADDPS( XMM5, XMM4 )			/* oy*m5+ow*m13 | ox*m0+ow*m12 */
	MOVLPS( XMM4, D(0) )			/* ->D(1) | ->D(0) */

	MOVSS( S(2), XMM4 )			/* oz */
	MULSS( XMM1, XMM4 )			/* oz*m10 */
	MOVSS( S(3), XMM5 )			/* ow */
	MOVSS( XMM5, D(3) )			/* ->D(3) */
	MULSS( XMM3, XMM5 )			/* ow*m14 */
	ADDSS( XMM5, XMM4 )			/* ow*m14+oz*m10 */
	MOVSS( XMM4, D(2) )			/* ->D(2) */

LLBL(K_GTP43DNRM_skip):
	INC_L    ( EBP )
	ADD_L    ( CONST(16), EDI )
	ADD_L    ( EAX, ESI )
	CMP_L    ( ECX, EDI )
	JNE      ( LLBL(K_GTP43DNRM_top) )

LLBL(K_GTP43DNRM_finish):
	POP_L    ( EBP )
	POP_L    ( EBX )
	POP_L    ( EDI )
	POP_L    ( ESI )
	RET
#undef FRAME_OFFSET


ALIGNTEXT4
GLOBL GLNAME(gl_katmai_transform_points4_3d_masked)
GLNAME(gl_katmai_transform_points4_3d_masked):

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
	JZ( LLBL(K_GTP43P3DM_finish) )		/* count was zero; go to finish */

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
	MOVAPS( M(4), XMM1 )			/* m7  | m6  | m5  |  m4 */
	MOVAPS( M(8), XMM2 )			/* m11 | m10 | m9  |  m8 */
	MOVAPS( M(12), XMM3 )			/* m15 | m14 | m13 | m12 */

ALIGNTEXT32
LLBL(K_GTP43P3DM_top):
	TEST_B( BL, REGIND(EBP) )
	JNZ( LLBL(K_GTP43P3DM_skip) )

	MOVSS( S(0), XMM4 )				/* ox */
	SHUFPS( CONST(0x0), XMM4, XMM4 )		/* ox | ox | ox | ox */
	MULPS( XMM0, XMM4 )				/* ox*m3 | ox*m2 | ox*m1 | ox*m0 */

	MOVSS( S(1), XMM5 )				/* oy */
	SHUFPS( CONST(0x0), XMM5, XMM5 )		/* oy | oy | oy | oy */
	MULPS( XMM1, XMM5 )				/* oy*m7 | oy*m6 | oy*m5 | oy*m4 */

	MOVSS( S(2), XMM6 )				/* oz */
	SHUFPS( CONST(0x0), XMM6, XMM6 )		/* oz | oz | oz | oz */
	MULPS( XMM2, XMM6 )				/* oz*m11 | oz*m10 | oz*m9 | oz*m8 */

	MOVSS( S(3), XMM7 )				/* ow */
	SHUFPS( CONST(0x0), XMM7, XMM7 )		/* ow | ow | ow | ow */
	MULPS( XMM3, XMM7 )				/* ow*m15 | ow*m14 | ow*m13 | ow*m12 */

	ADDPS( XMM5, XMM4 )				/* ox*m3+oy*m7 | ... */
	ADDPS( XMM6, XMM4 )				/* ox*m3+oy*m7+oz*m11 | ... */
	ADDPS( XMM7, XMM4 )				/* ox*m3+oy*m7+oz*m11+ow*m15 | ... */
	MOVAPS( XMM4, D(0) )				/* ->D(3) | ->D(2) | ->D(1) | ->D(0) */

	MOVSS( S(3), XMM4 )				/* ow */
	MOVSS( XMM4, D(3) )				/* ->D(3) */

LLBL(K_GTP43P3DM_skip):
	INC_L( EBP )
	ADD_L( CONST(16), EDI )
	ADD_L( EAX, ESI )
	CMP_L( ECX, EDI )
	JNE( LLBL(K_GTP43P3DM_top) )

LLBL(K_GTP43P3DM_finish):
	POP_L( EBP )
	POP_L( EBX )
	POP_L( EDI )
	POP_L( ESI )
	RET
#undef FRAME_OFFSET
