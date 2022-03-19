#include "assyntax.h"
/** TODO:
  * - insert PREFETCH instructions to avoid cache-misses !
  * - some more optimizations are possible...
  * - for 40-50% more performance in the SSE-functions, the
  *   data (trans-matrix, src_vert, dst_vert) needs to be 16byte aligned !
  */

   SEG_TEXT


#define OFFSET_MAT 4
#define OFFSET_SCALE 8
#define OFFSET_IN 12
#define OFFSET_LENGTHS 16
#define OFFSET_MASK 20
#define OFFSET_DEST 24

#define ARG_MAT         REGOFF(FRAME_OFFSET+OFFSET_MAT, ESP)
#define ARG_SCALE       REGOFF(FRAME_OFFSET+OFFSET_SCALE, ESP)
#define ARG_IN          REGOFF(FRAME_OFFSET+OFFSET_IN, ESP)
#define ARG_LENGTHS     REGOFF(FRAME_OFFSET+OFFSET_LENGTHS, ESP)
#define ARG_MASK        REGOFF(FRAME_OFFSET+OFFSET_MASK, ESP)
#define ARG_DEST        REGOFF(FRAME_OFFSET+OFFSET_DEST, ESP)


#define V3F_START       4
#define V3F_COUNT       8
#define V3F_STRIDE      12

#define MAT_INV         4

#define M(i)    REGOFF(i * 4, EDX)
#define S(i)	REGOFF(i * 4, ESI)
#define D(i)	REGOFF(i * 4, EDI)
#define STRIDE  REGOFF(12, ESI)

#undef FRAME_OFFSET

/*
 *	Offsets for norm_func arguments
 *
 * static void TAG(transform_rescale_normals_no_rot)( const GLmatrix *mat,
 *						   GLfloat scale,
 *						   const GLvector3f *in,
 *						   const GLfloat *lengths,
 *						   const GLubyte mask[],
 *						   GLvector3f *dest )
 */

ALIGNTEXT16
GLOBL GLNAME(gl_katmai_transform_rescale_normals_no_rot_raw)
GLNAME(gl_katmai_transform_rescale_normals_no_rot_raw):

#define FRAME_OFFSET 8
	PUSH_L  ( ESI )
	PUSH_L  ( EDI )

	MOV_L	( ARG_IN, ESI )				/* ptr to source GLvector3f */
	MOV_L	( ARG_DEST, EDI )			/* ptr to dest GLvector3f */

	MOV_L	( ARG_MAT, EDX )			/* ptr to matrix */
	MOV_L	( REGOFF(MAT_INV, EDX), EDX)		/* matrix->inv */

	MOV_L	( REGOFF(V3F_COUNT, ESI), ECX ) 	/* source count */

	TEST_L	( ECX, ECX )
	JZ( LLBL(K_G3TRNNRR_finish) )			/* count was zero; go to finish */

	MOV_L	( STRIDE, EAX )				/* stride */
	MOV_L	( ECX, REGOFF(V3F_COUNT, EDI) )		/* set dest-count */

	IMUL_L( CONST(12), ECX )			/* count *= 12 */
	MOV_L( REGOFF(V3F_START, ESI), ESI )		/* ptr to first source vertex */

	MOV_L( REGOFF(V3F_START, EDI), EDI )		/* ptr to first dest vertex */
	ADD_L( EDI, ECX ) 				/* count += dest ptr */

ALIGNTEXT32
	MOVSS	( M(0), XMM1 )				/* m0 */
	MOVSS	( M(5), XMM2 )				/* m5 */
	UNPCKLPS( XMM2, XMM1 )				/* m5 | m0 */
	MOVSS	( ARG_SCALE, XMM0 )			/* scale */
	SHUFPS	( CONST(0x0), XMM0, XMM0 )		/* scale | scale */
	MULPS	( XMM0, XMM1 )				/* m5*scale | m0*scale */
	MULSS	( M(10), XMM0 )				/* m10*scale */

ALIGNTEXT32
LLBL(K_G3TRNNRR_top):
	MOVLPS	( S(0), XMM2 )				/* uy | ux */
	MULPS	( XMM1, XMM2 )				/* uy*m5*scale | ux*m0*scale */
	MOVLPS	( XMM2, D(0) )				/* ->D(1) | D(0) */

	MOVSS	( S(2), XMM2 )				/* uz */
	MULSS	( XMM0, XMM2 )				/* uz*m10*scale */
	MOVSS	( XMM2, D(2) )				/* ->D(2) */

LLBL(K_G3TRNNRR_skip):
	ADD_L	( CONST(12), EDI )
	ADD_L	( EAX, ESI )
	CMP_L	( ECX, EDI )
	JNE	( LLBL(K_G3TRNNRR_top) )

LLBL(K_G3TRNNRR_finish):
	POP_L	( EDI )
	POP_L	( ESI )
	RET
#undef FRAME_OFFSET



ALIGNTEXT16
GLOBL GLNAME(gl_katmai_transform_rescale_normals_raw)
GLNAME(gl_katmai_transform_rescale_normals_raw):

#define FRAME_OFFSET 8
	PUSH_L  ( ESI )
	PUSH_L  ( EDI )

	MOV_L	( ARG_IN, ESI )				/* ptr to source GLvector3f */
	MOV_L	( ARG_DEST, EDI )			/* ptr to dest GLvector3f */

	MOV_L	( ARG_MAT, EDX )			/* ptr to matrix */
	MOV_L	( REGOFF(MAT_INV, EDX), EDX)		/* matrix->inv */

	MOV_L	( REGOFF(V3F_COUNT, ESI), ECX ) 	/* source count */

	TEST_L	( ECX, ECX )
	JZ( LLBL(K_G3TRNR_finish) )			/* count was zero; go to finish */

	MOV_L	( STRIDE, EAX )				/* stride */
	MOV_L	( ECX, REGOFF(V3F_COUNT, EDI) )		/* set dest-count */

	IMUL_L( CONST(12), ECX )			/* count *= 12 */
	MOV_L( REGOFF(V3F_START, ESI), ESI )		/* ptr to first source vertex */

	MOV_L( REGOFF(V3F_START, EDI), EDI )		/* ptr to first dest vertex */
	ADD_L( EDI, ECX ) 				/* count += dest ptr */

ALIGNTEXT32
	MOVSS	( M(0), XMM0 )				/* m0 */
	MOVSS	( M(4), XMM1 )				/* m4 */
	UNPCKLPS( XMM1, XMM0 )				/* m4 | m0 */

	MOVSS	( ARG_SCALE, XMM4 )			/* scale */
	SHUFPS	( CONST(0x0), XMM4, XMM4 )		/* scale | scale */

	MULPS	( XMM4, XMM0 )				/* m4*scale | m0*scale */
	MOVSS	( M(1), XMM1 )				/* m1 */
	MOVSS	( M(5), XMM2 )				/* m5 */
	UNPCKLPS( XMM2, XMM1 )				/* m5 | m1 */
	MULPS	( XMM4, XMM1 )				/* m5*scale | m1*scale */
	MOVSS	( M(2), XMM2 )				/* m2 */
	MOVSS	( M(6), XMM3 )				/* m6 */
	UNPCKLPS( XMM3, XMM2 )				/* m6 | m2 */
	MULPS	( XMM4, XMM2 )				/* m6*scale | m2*scale */

	MOVSS	( M(8), XMM6 )				/* m8 */
	MULSS	( ARG_SCALE, XMM6 )			/* m8*scale */
	MOVSS	( M(9), XMM7 )				/* m9 */
	MULSS	( ARG_SCALE, XMM7 )			/* m9*scale */

ALIGNTEXT32
LLBL(K_G3TRNR_top):
	MOVSS	( S(0), XMM3 )				/* ux */
	SHUFPS	( CONST(0x0), XMM3, XMM3 )		/* ux | ux */
	MULPS	( XMM0, XMM3 )				/* ux*m4 | ux*m0 */
	MOVSS	( S(1), XMM4 )				/* uy */
	SHUFPS	( CONST(0x0), XMM4, XMM4 )		/* uy | uy */
	MULPS	( XMM1, XMM4 )				/* uy*m5 | uy*m1 */
	MOVSS	( S(2), XMM5 )				/* uz */
	SHUFPS	( CONST(0x0), XMM5, XMM5 )		/* uz | uz */
	MULPS	( XMM2, XMM5 )				/* uz*m6 | uz*m2 */

	ADDPS	( XMM4, XMM3 )
	ADDPS	( XMM5, XMM3 )
	MOVLPS	( XMM3, D(0) )

	MOVSS	( M(10), XMM3 )				/* m10 */
	MULSS	( ARG_SCALE, XMM3 )			/* m10*scale */
	MULSS	( S(2), XMM3 )				/* m10*scale*uz */
	MOVSS	( S(1), XMM4 )				/* uy */
	MULSS	( XMM7, XMM4 )				/* uy*m9*scale */
	MOVSS	( S(0), XMM5 )				/* ux */
	MULSS	( XMM6, XMM5 )				/* ux*m8*scale */

	ADDSS	( XMM4, XMM3 )
	ADDSS	( XMM5, XMM3 )
	MOVSS	( XMM3, D(2) )

LLBL(K_G3TRNR_skip):
	ADD_L	( CONST(12), EDI )
	ADD_L	( EAX, ESI )
	CMP_L	( ECX, EDI )
	JNE	( LLBL(K_G3TRNR_top) )

LLBL(K_G3TRNR_finish):
	POP_L	( EDI )
	POP_L	( ESI )
	RET
#undef FRAME_OFFSET


ALIGNTEXT16
GLOBL GLNAME(gl_katmai_transform_normals_no_rot_raw)
GLNAME(gl_katmai_transform_normals_no_rot_raw):

#define FRAME_OFFSET 8
	PUSH_L  ( ESI )
	PUSH_L  ( EDI )

	MOV_L	( ARG_IN, ESI )				/* ptr to source GLvector3f */
	MOV_L	( ARG_DEST, EDI )			/* ptr to dest GLvector3f */

	MOV_L	( ARG_MAT, EDX )			/* ptr to matrix */
	MOV_L	( REGOFF(MAT_INV, EDX), EDX)		/* matrix->inv */

	MOV_L	( REGOFF(V3F_COUNT, ESI), ECX ) 	/* source count */

	TEST_L	( ECX, ECX )
	JZ( LLBL(K_G3TNNRR_finish) )			/* count was zero; go to finish */

	MOV_L	( STRIDE, EAX )				/* stride */
	MOV_L	( ECX, REGOFF(V3F_COUNT, EDI) )		/* set dest-count */

	IMUL_L( CONST(12), ECX )			/* count *= 12 */
	MOV_L( REGOFF(V3F_START, ESI), ESI )		/* ptr to first source vertex */

	MOV_L( REGOFF(V3F_START, EDI), EDI )		/* ptr to first dest vertex */
	ADD_L( EDI, ECX ) 				/* count += dest ptr */

ALIGNTEXT32
	MOVSS( M(0), XMM0 )				/* m0 */
	MOVSS( M(5), XMM1 )				/* m5 */
	UNPCKLPS( XMM1, XMM0 )				/* m5 | m0 */
	MOVSS( M(10), XMM1 )				/* m10 */

ALIGNTEXT32
LLBL(K_G3TNNRR_top):
	MOVLPS( S(0), XMM2 )				/* uy | ux */
	MULPS( XMM0, XMM2 )				/* uy*m5 | ux*m0 */
	MOVLPS( XMM2, D(0) )

	MOVSS( S(2), XMM2 )				/* uz */
	MULSS( XMM1, XMM2 )				/* uz*m10 */
	MOVSS( XMM2, D(2) )

LLBL(K_G3TNNRR_skip):
	ADD_L	( CONST(12), EDI )
	ADD_L	( EAX, ESI )
	CMP_L	( ECX, EDI )
	JNE	( LLBL(K_G3TNNRR_top) )

LLBL(K_G3TNNRR_finish):
	POP_L	( EDI )
	POP_L	( ESI )
	RET
#undef FRAME_OFFSET
