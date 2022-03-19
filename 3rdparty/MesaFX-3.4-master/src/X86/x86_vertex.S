#include "assyntax.h"


	SEG_TEXT

/*#define MAT_SX 0*/        /*  accessed by REGIND !! */
#define MAT_SY 20
#define MAT_SZ 40
#define MAT_TX 48
#define MAT_TY 52
#define MAT_TZ 56


#define FP_ONE		1065353216
#define FP_ZERO		0

#define S(i)		REGOFF(i * 4, ESI)
#define D(i)		REGOFF(i * 4, EDI)
#define M(i)		REGOFF(i * 4, EDX)



/*
 * void gl_v16_x86_general_xform (	GLfloat *dest,
 *				const GLfloat *m,
 *				const GLfloat *src,
 *                              GLuint src_stride,
 *				GLuint count )
 */


/* This is nothing more glamorous than an objdump of one of Josh's
 * routines hacked to match the above.
 */
ALIGNTEXT16
GLOBL GLNAME( gl_v16_x86_general_xform )
GLNAME( gl_v16_x86_general_xform ):


	PUSH_L ( EDI )
	PUSH_L ( ESI )

    	MOV_L     ( REGOFF(12, ESP), EAX ) /* dest */
    	MOV_L     ( REGOFF(16, ESP), ESI ) /* mat */
    	MOV_L     ( REGOFF(20, ESP), EDX ) /* src */
    	MOV_L     ( REGOFF(24, ESP), EDI ) /* src_stride */
    	MOV_L     ( REGOFF(28, ESP), ECX ) /* count */


LLBL(v16x86_loop):
        FLD_S   ( REGOFF( 0x0, EDX ) )
        FMUL_S  ( REGOFF( 0x0, ESI ) )
        FLD_S   ( REGOFF( 0x0, EDX ) )
        FMUL_S  ( REGOFF( 0x4, ESI ) )
        FLD_S   ( REGOFF( 0x0, EDX ) )
        FMUL_S  ( REGOFF( 0x8, ESI ) )
        FLD_S   ( REGOFF( 0x0, EDX ) )
        FMUL_S  ( REGOFF( 0xc, ESI ) )
        FLD_S   ( REGOFF( 0x4, EDX ) )
        FMUL_S  ( REGOFF( 0x10, ESI ) )
        FLD_S   ( REGOFF( 0x4, EDX ) )
        FMUL_S  ( REGOFF( 0x14, ESI ) )
        FLD_S   ( REGOFF( 0x4, EDX ) )
        FMUL_S  ( REGOFF( 0x18, ESI ) )
        FLD_S   ( REGOFF( 0x4, EDX ) )
        FMUL_S  ( REGOFF( 0x1c, ESI ) )
        FXCH    ( ST(3) )
        FADDP   ( ST(0),ST(7) )
        FXCH    ( ST(1) )
        FADDP   ( ST(0),ST(5) )
        FADDP   ( ST(0),ST(3) )
        FADDP   ( ST(0),ST(1) )
        FLD_S   ( REGOFF( 0x8, EDX ) )
        FMUL_S  ( REGOFF( 0x20, ESI ) )
        FLD_S   ( REGOFF( 0x8, EDX ) )
        FMUL_S  ( REGOFF( 0x24, ESI ) )
        FLD_S   ( REGOFF( 0x8, EDX ) )
        FMUL_S  ( REGOFF( 0x28, ESI ) )
        FLD_S   ( REGOFF( 0x8, EDX ) )
        FMUL_S  ( REGOFF( 0x2c, ESI ) )
        FXCH    ( ST(3) )
        FADDP   ( ST(0),ST(7) )
        FXCH    ( ST(1) )
        FADDP   ( ST(0),ST(5) )
        FADDP   ( ST(0),ST(3) )
        FADDP   ( ST(0),ST(1) )
        FXCH    ( ST(3) )
        FADD_S  ( REGOFF( 0x30, ESI ) )
        FXCH    ( ST(2) )
        FADD_S  ( REGOFF( 0x34, ESI ) )
        FXCH    ( ST(1) )
        FADD_S  ( REGOFF( 0x38, ESI ) )
        FXCH    ( ST(3) )
        FADD_S  ( REGOFF( 0x3c, ESI ) )
        FXCH    ( ST(2) )
        FSTP_S  ( REGOFF( 0x0, EAX ) )
        FSTP_S  ( REGOFF( 0x4, EAX ) )
        FXCH    ( ST(1) )
        FSTP_S  ( REGOFF( 0x8, EAX ) )
        FSTP_S  ( REGOFF( 0xc, EAX ) )
        ADD_L   ( CONST(64), EAX )
        ADD_L   ( EDI, EDX )
        DEC_L   ( ECX )
        JNE     ( LLBL(v16x86_loop) )

	POP_L ( ESI )
	POP_L ( EDI )
	RET


/*
 * Table for clip test.
 *
 * 	bit6 = S(3) < 0
 * 	bit5 = S(2) < 0
 * 	bit4 = abs(S(2)) > abs(S(3))
 * 	bit3 = S(1) < 0
 * 	bit2 = abs(S(1)) > abs(S(3))
 * 	bit1 = S(0) < 0
 * 	bit0 = abs(S(0)) > abs(S(3))
 */


	SEG_DATA

clip_table:
	D_BYTE 0, 1, 0, 2, 4, 5, 4, 6
	D_BYTE 0, 1, 0, 2, 8, 9, 8, 10
	D_BYTE 32, 33, 32, 34, 36, 37, 36, 38
	D_BYTE 32, 33, 32, 34, 40, 41, 40, 42
	D_BYTE 0, 1, 0, 2, 4, 5, 4, 6
	D_BYTE 0, 1, 0, 2, 8, 9, 8, 10
	D_BYTE 16, 17, 16, 18, 20, 21, 20, 22
	D_BYTE 16, 17, 16, 18, 24, 25, 24, 26
	D_BYTE 63, 61, 63, 62, 55, 53, 55, 54
	D_BYTE 63, 61, 63, 62, 59, 57, 59, 58
	D_BYTE 47, 45, 47, 46, 39, 37, 39, 38
	D_BYTE 47, 45, 47, 46, 43, 41, 43, 42
	D_BYTE 63, 61, 63, 62, 55, 53, 55, 54
	D_BYTE 63, 61, 63, 62, 59, 57, 59, 58
	D_BYTE 31, 29, 31, 30, 23, 21, 23, 22
	D_BYTE 31, 29, 31, 30, 27, 25, 27, 26

	SEG_TEXT


/*
########################################
##
## gl_v16_x86_cliptest_points4
##
## Performs cliptesting equivalent to that done by cliptest_v16()
## in vertices.c
##
## This is a hacked version of the original above.
##
########################################
*/



#define OFFSET_V16_SOURCE	4
#define OFFSET_V16_LAST		8
#define OFFSET_V16_OR		12
#define OFFSET_V16_AND		16
#define OFFSET_V16_MASK		20

#define ARG_V16_SOURCE	REGOFF(V16_FRAME_OFFSET+OFFSET_V16_SOURCE, ESP)
#define ARG_V16_LAST	REGOFF(V16_FRAME_OFFSET+OFFSET_V16_LAST, ESP)
#define ARG_V16_OR	REGOFF(V16_FRAME_OFFSET+OFFSET_V16_OR, ESP)
#define ARG_V16_AND	REGOFF(V16_FRAME_OFFSET+OFFSET_V16_AND, ESP)
#define ARG_V16_MASK	REGOFF(V16_FRAME_OFFSET+OFFSET_V16_MASK, ESP)


#if defined(__ELF__) && defined(__PIC__) && !defined(ELFPIC)
#define ELFPIC
#endif

	GLOBL GLNAME(gl_v16_x86_cliptest_points4)
	ALIGNTEXT4

GLNAME(gl_v16_x86_cliptest_points4):
#ifdef ELFPIC
#define V16_FRAME_OFFSET 20
#else
#define V16_FRAME_OFFSET 16
#endif
	PUSH_L( ESI )
	PUSH_L( EDI )
	PUSH_L( EBP )
	PUSH_L( EBX )

#ifdef ELFPIC
	/* store pointer to clip_table on stack */
	CALL( LLBL(v16_ctp4_get_eip) )
	ADD_L( CONST(_GLOBAL_OFFSET_TABLE_), EBX )
	MOV_L( REGOFF(clip_table@GOT, EBX), EBX )
	PUSH_L( EBX )
	JMP( LLBL(v16_ctp4_clip_table_ready) )

LLBL(v16_ctp4_get_eip):
	/* store eip in ebx */
	MOV_L( REGIND(ESP), EBX )
	RET

LLBL(v16_ctp4_clip_table_ready):
#endif

	MOV_L( ARG_V16_SOURCE, ESI )            /* ptr to first source vertex */
	MOV_L( ARG_V16_LAST, EDX ) 	        /* ptr to last source vertex */
	MOV_L( ARG_V16_OR, EBX )
	MOV_L( ARG_V16_AND, EBP )
	MOV_L( ARG_V16_MASK, EDI )

	CMP_L( EDX, ESI )

	MOV_B( REGIND(EBX), AL )
	MOV_B( REGIND(EBP), AH )

	JZ( LLBL(v16_ctp4_finish) )

	ALIGNTEXT4ifNOP
LLBL(v16_ctp4_top):
#if 0
	FLD1 	/* F0 */
	FDIV_S( S(3) )
#endif

	MOV_L( S(3), EBP )
	MOV_L( S(2), EBX )

	XOR_L( ECX, ECX )
	ADD_L( EBP, EBP )	/* ebp = abs(S(3))*2 ; carry = sign of S(3) */

	ADC_L( ECX, ECX )
	ADD_L( EBX, EBX )	/* ebx = abs(S(2))*2 ; carry = sign of S(2) */

	ADC_L( ECX, ECX )
	CMP_L( EBX, EBP )	/* carry = abs(S(2))*2 > abs(S(3))*2 */

	ADC_L( ECX, ECX )
	MOV_L( S(1), EBX )

	ADD_L( EBX, EBX )	/* ebx = abs(S(1))*2 ; carry = sign of S(1) */

	ADC_L( ECX, ECX )
	CMP_L( EBX, EBP )	/* carry = abs(S(1))*2 > abs(S(3))*2 */

	ADC_L( ECX, ECX )
	MOV_L( S(0), EBX )

	ADD_L( EBX, EBX )	/* ebx = abs(S(0))*2 ; carry = sign of S(0) */

	ADC_L( ECX, ECX )
	CMP_L( EBX, EBP )	/* carry = abs(S(0))*2 > abs(S(3))*2 */

	ADC_L( ECX, ECX )

#ifdef ELFPIC
	MOV_L( REGIND(ESP), EBP ) 	/* clip_table */

	MOV_B( REGBI(EBP, ECX), CL )
#else
	MOV_B( REGOFF(clip_table,ECX), CL )
#endif

	OR_B( CL, AL )
	AND_B( CL, AH )

	MOV_B( CL, REGIND(EDI) )     /* save clipmask */
	INC_L( EDI )                 /* next clipmask */

#if 0
	FSTP_S( S(8)   ) 	/* */ 	/* GR_VERTEX_OOW_OFFSET */
#endif

	ADD_L( CONST(64), ESI )      /* next fxVertex  */

	CMP_L( EDX, ESI )            /* finished? */
	JNZ( LLBL(v16_ctp4_top) )

	MOV_L( ARG_V16_OR, ECX )
	MOV_L( ARG_V16_AND, EDX )

	MOV_B( AL, REGIND(ECX) )
	MOV_B( AH, REGIND(EDX) )

LLBL(v16_ctp4_finish):

#ifdef ELFPIC
	POP_L( ESI ) 	/* discard ptr to clip_table */
#endif
	POP_L( EBX )
	POP_L( EBP )
	POP_L( EDI )
	POP_L( ESI )

	RET
