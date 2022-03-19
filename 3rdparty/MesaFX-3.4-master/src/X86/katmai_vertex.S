#include "assyntax.h"

	SEG_TEXT

#define MAT_SY 20
#define MAT_SZ 40
#define MAT_TX 48
#define MAT_TY 52
#define MAT_TZ 56


/*
 * void gl_v16_katmai_general_xform( GLfloat *dest,
 *		        	     const GLfloat *m,
 *				     const GLfloat *src,
 *                                   GLuint src_stride,
 *				     GLuint count )
 *
 */

ALIGNTEXT16
GLOBL GLNAME( gl_v16_katmai_general_xform )
GLNAME( gl_v16_katmai_general_xform ):

    PUSH_L( EDI )
    PUSH_L( ESI )

    MOV_L( REGOFF(12, ESP), EAX )		/* destination                       */
    MOV_L( REGOFF(16, ESP), ESI )		/* matrix                            */
    MOV_L( REGOFF(20, ESP), EDX )		/* source                            */
    MOV_L( REGOFF(24, ESP), EDI )		/* src_stride                        */
    MOV_L( REGOFF(28, ESP), ECX )		/* count                             */

    MOVAPS( REGOFF(0, ESI), XMM4 )		/* m3     | m2     | m1     | m0     */
    MOVAPS( REGOFF(16, ESI), XMM5 )		/* m7     | m6     | m5     | m4     */
    MOVAPS( REGOFF(32, ESI), XMM6 )		/* m11    | m10    | m9     | m8     */
    MOVAPS( REGOFF(48, ESI), XMM7 )		/* m15    | m14    | m13    | m12    */

ALIGNTEXT32
LLBL(v16_katmai_general_loop):

    MOVSS( REGOFF(0, EDX), XMM0 )		/*        |        |        | x0     */
    SHUFPS( CONST(0x0), XMM0, XMM0 )		/* x0     | x0     | x0     | x0     */
    MULPS( XMM4, XMM0 )				/* x0*m3  | x0*m2  | x0*m1  | x0*m0  */

    MOVSS( REGOFF(4, EDX), XMM1 )		/*        |        |        | x1     */
    SHUFPS( CONST(0x0), XMM1, XMM1 )		/* x1     | x1     | x1     | x1     */
    MULPS( XMM5, XMM1 )				/* x1*m7  | x1*m6  | x1*m5  | x1*m4  */

    MOVSS( REGOFF(8, EDX), XMM2 )		/*        |        |        | x2     */
    SHUFPS( CONST(0x0), XMM2, XMM2 )		/* x2     | x2     | x2     | x2     */
    MULPS( XMM6, XMM2 )				/* x2*m11 | x2*m10 | x2*m9  | x2*m8  */

    ADDPS( XMM1, XMM0 )
    ADDPS( XMM2, XMM0 )
    ADDPS( XMM7, XMM0 )				/* r3     | r2     | r1     | r1     */

    MOVAPS( XMM0, REGOFF(0, EAX) )

    ADD_L( CONST(64), EAX )			/* next output vertex                */
    ADD_L( EDI, EDX )				/* next input vertex                 */

    DEC_L( ECX )
    JNE( LLBL(v16_katmai_general_loop) )

    POP_L( ESI )
    POP_L( EDI )

    RET


/* void gl_katmai_project_vertices(GLfloat *first,
 *	              		   GLfloat *last,
 *	              		   const GLfloat *m,
 *	              		   GLuint stride )
 */


ALIGNTEXT16
GLOBL GLNAME( gl_katmai_project_vertices )
GLNAME( gl_katmai_project_vertices ):

	PUSH_L( EBP )

	MOV_L( REGOFF(8, ESP), ECX )	/* first_vert */
	MOV_L( REGOFF(12, ESP), EDX )	/* last_vert */
	MOV_L( REGOFF(16, ESP), EBP )	/* matrix */
	MOV_L( REGOFF(20, ESP), EAX )	/* stride */


ALIGNTEXT32
	MOVAPS( REGOFF(MAT_TX, EBP), XMM0 )	/* (x44), x43, x42, x41 => xmm0 */
	MOVSS( REGOFF(0, EBP), XMM1 )		/* -, -, -, x11 => xmm1 */
	UNPCKLPS( REGOFF(MAT_SY, EBP), XMM1 )	/* -, -, x22, x11 => xmm1 */
	SHUFPS( CONST(0x44), REGOFF(MAT_SZ, EBP), XMM1 )
						/* -, x33, x22, x11 => xmm1 */

	SUB_L( ECX, EDX )			/* last -= first */

LLBL(v16_katmai_pv_loop_start):
	MOVAPS( REGOFF(0, ECX), XMM3 )		/* f[3], f[2], f[1], f[0] */

	MOVSS( REGOFF(12, ECX), XMM2 )		/* -, -, -, f[3] */
	SHUFPS( CONST(0x0), XMM2, XMM2 )	/* f[3], f[3], f[3], f[3] */
	RCPPS( XMM2, XMM2 )			/* 1/f[3], 1/f[3], 1/f[3], 1/f[3] */

	MULPS( XMM2, XMM1 )			/* -, x33*1/f[3]... */
	MULPS( XMM3, XMM1 )			/* -, x33*1/f[3]*f[2]... */
	ADDPS( XMM0, XMM1 )			/* -, x33*1/f[3]*f[2]+x43... */

	MOVAPS( XMM1, REGOFF(0, ECX) )		/* back to f */
	MOVSS( XMM2, REGOFF(12, ECX) )		/* 1/f[3] into f[3] ! */


	ADD_L( EAX, ECX )			/* f += stride */
	SUB_L( EAX, EDX )			/* (last-first)-stride */
	JA ( LLBL(v16_katmai_pv_loop_start) )

	POP_L( EBP )
	RET



/* void gl_katmai_project_clipped_vertices(GLfloat *first,
 *	              		   	   GLfloat *last,
 *	              		   	   const GLfloat *m,
 *	              		   	   GLuint stride,
 *					   const GLubyte *clipmask );
 */


ALIGNTEXT16
GLOBL GLNAME( gl_katmai_project_clipped_vertices )
GLNAME( gl_katmai_project_clipped_vertices ):

	PUSH_L( EBP )
	PUSH_L( ESI )

	MOV_L( REGOFF(12, ESP), ECX )	/* first_vert */
	MOV_L( REGOFF(16, ESP), EDX )	/* last_vert */
	MOV_L( REGOFF(20, ESP), EBP )	/* matrix */
	MOV_L( REGOFF(24, ESP), EAX )	/* stride */
	MOV_L( REGOFF(28, ESP), ESI )	/* clip_mask */



ALIGNTEXT32

	MOVAPS( REGOFF(MAT_TX, EBP), XMM0 )	/* (x44), x43, x42, x41 => xmm0 */
	MOVSS( REGOFF(0, EBP), XMM1 )		/* -, -, -, x11 => xmm1 */
	UNPCKLPS( REGOFF(MAT_SY, EBP), XMM1 )	/* -, -, x22, x11 => xmm1 */
	SHUFPS( CONST(0x44), REGOFF(MAT_SZ, EBP), XMM1 )
						/* -, x33, x22, x11 => xmm1 */

LLBL(v16_katmai_pcv_loop_start):
	CMP_B ( CONST(0), REGIND(ESI) )		/* clip_mask == 0 ? */
	JNE( LLBL(v16_katmai_pcv_skip) )	/* no -> skip ! */

	MOVAPS( REGOFF(0, ECX), XMM3 )		/* f[3], f[2], f[1], f[0] */

	MOVSS( REGOFF(12, ECX), XMM2 )		/* -, -, -, f[3] */
	SHUFPS( CONST(0x0), XMM2, XMM2 )	/* f[3], f[3], f[3], f[3] */
	RCPPS( XMM2, XMM2 )			/* 1/f[3], 1/f[3], 1/f[3], 1/f[3] */

	MULPS( XMM2, XMM1 )			/* -, x33*1/f[3]... */
	MULPS( XMM3, XMM1 )			/* -, x33*1/f[3]*f[2]... */
	ADDPS( XMM0, XMM1 )			/* -, x33*1/f[3]*f[2]+x43... */

	MOVAPS( XMM1, REGOFF(0, ECX) )		/* back to f */
	MOVSS( XMM2, REGOFF(12, ECX) )		/* 1/f[3] into f[3] ! */

LLBL(v16_katmai_pcv_skip):
	ADD_L( EAX, ECX )			/* f += stride */
	INC_L( ESI )				/* nect ClipMask */

	CMP_L( ECX, EDX )			/* p_first_vertex == p_last_vertex */
	JNE( LLBL(v16_katmai_pcv_loop_start) )
						/* no -> go on with next vertex */

	POP_L( ESI )
	POP_L( EBP )
	RET
