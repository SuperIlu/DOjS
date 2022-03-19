#include "assyntax.h"

   SEG_TEXT

#define MAT_SY 20
#define MAT_SZ 40
#define MAT_TX 48
#define MAT_TY 52
#define MAT_TZ 56


/*
 * void gl_v16_3dnow_general_xform( GLfloat *dest,
 *		        	    const GLfloat *m,
 *				    const GLfloat *src,
 *                                  GLuint src_stride,
 *				    GLuint count )
 *
 * These tranformation functions could disappear if the standard ones
 * took an output stride.
 */
GLOBL GLNAME( gl_v16_3dnow_general_xform )
GLNAME( gl_v16_3dnow_general_xform ):

    PUSH_L    ( EDI )
    PUSH_L    ( ESI )

    MOV_L     ( REGOFF(12, ESP), EAX )		/* dest                      */
    MOV_L     ( REGOFF(16, ESP), ESI )		/* mat                       */
    MOV_L     ( REGOFF(20, ESP), EDX )		/* src                       */
    MOV_L     ( REGOFF(24, ESP), EDI )		/* src_stride                */
    MOV_L     ( REGOFF(28, ESP), ECX )		/* count                     */

    FEMMS

    MOVQ      ( REGOFF(MAT_TX, ESI), MM7 )	/* ty          | tx          */
    MOVQ      ( REGOFF(MAT_TZ, ESI), MM3 )	/* tw          | tz          */

ALIGNTEXT32
LLBL( v16_3dnow_general_loop ):

    PREFETCHW ( REGOFF(128, EAX) )		/* write alloc 2 verts ahead */
    PREFETCH  ( REGOFF(32, EDX) )		/* prefetch next cache line  */

    MOVQ      ( REGIND(EDX),  MM0 )		/* x1          | x0          */
    MOVD      ( REGOFF(8, EDX), MM1 )		/*             | x2          */
    MOVQ      ( REGIND(ESI),  MM4 )		/* m1          | m0          */
    PUNPCKHDQ ( MM0, MM2 )			/* x1          |             */
    MOVQ      ( REGOFF(16, ESI), MM5 )		/* m5          | m4          */
    PUNPCKLDQ ( MM0, MM0 )			/* x0          | x0          */
    MOVQ      ( REGOFF(32, ESI), MM6 )		/* m9          | m8          */
    PFMUL     ( MM0, MM4 )			/* x0*m1       | x0*m0       */
    PUNPCKHDQ ( MM2, MM2 )			/* x1          | x1          */
    PFMUL     ( MM2, MM5 )			/* x1*m5       | x1*m4       */
    PUNPCKLDQ ( MM1, MM1 )			/* x2          | x2          */
    PFMUL     ( REGOFF(8, ESI), MM0 )		/* x0*m3       | x0*m2       */
    PFMUL     ( REGOFF(24, ESI), MM2 )		/* x1*m7       | x1*m6       */
    PFMUL     ( MM1, MM6 )			/* x2*m9       | x2*m8       */
    PFADD     ( MM4, MM5 )			/* x0*m1+x1*m5 | x0*m0+x1*m4 */
    PFMUL     ( REGOFF(40, ESI), MM1 )		/* x2*m11      | x2*m10      */
    PFADD     ( MM0, MM2 )			/* x0*m3+x1*m7 | x0*m2+x1*m6 */
    PFADD     ( MM5, MM6 )
    PFADD     ( MM1, MM2 )
    PFADD     ( MM7, MM6 )			/* r1          | r0          */
    PFADD     ( MM3, MM2 )			/* r3          | r2          */
    ADD_L     ( EDI, EDX )			/* next input vertex         */
    MOVQ      ( MM6, REGIND(EAX) )
    MOVQ      ( MM2, REGOFF(8, EAX) )
    ADD_L     ( CONST(64), EAX )		/* next output vertex        */
    DEC_L     ( ECX )
    JNE       ( LLBL(v16_3dnow_general_loop) )

    FEMMS

    POP_L     ( ESI )
    POP_L     ( EDI )
    RET



/* Do viewport map and perspective projection.  Args should look like:
 *
 * gl_3dnow_project_vertices( float *first_vertex,
 *                            const float *last_vertex,
 *                            float *matrix,
 *                            GLuint stride )
 *
 * This routine assumes a sane vertex layout with x,y,z,w as
 * the first four elements, to be projected in clip-space, to
 * x/w,y/w,z/w,1/w, and then transformed according to the matrix to
 * device space.  The device coordinates will overwrite the clip
 * coordinates as the first four elements of the vertex.
 *
 * If projection is required for other elements, such as texcoords,
 * you will have to code a specialized version of this routine.  See
 * FX/X86 for examples.
 *
 * These routines are simplified versions of the FX code written by
 * Holger.
 */

GLOBL GLNAME( gl_3dnow_project_vertices )
GLNAME( gl_3dnow_project_vertices ):

    PUSH_L    ( EBP )
    FEMMS
    PREFETCH  ( REGOFF(8, ESP) )		/* fetch the first vertex    */

    MOV_L     ( REGOFF(8, ESP), ECX )		/* first_vert                */
    MOV_L     ( REGOFF(12, ESP), EDX )		/* last_vert                 */
    MOV_L     ( REGOFF(16, ESP), EBP )		/* matrix                    */
    MOV_L     ( REGOFF(20, ESP), EAX )		/* stride                    */

    MOVD      ( REGOFF(MAT_TX, EBP), MM6 )	/*             | tx          */
    PUNPCKLDQ ( REGOFF(MAT_TY, EBP), MM6 )	/* ty          | tx          */
    MOVD      ( REGIND(EBP), MM5 )
    PUNPCKLDQ ( REGOFF(MAT_SY, EBP), MM5 )	/* vsy         | vsx         */
    MOVD      ( REGOFF(MAT_SZ, EBP), MM1 )	/*             | vsz         */
    SUB_L     ( ECX, EDX )			/* last -= first             */

ALIGNTEXT32
LLBL( v16_3dnow_pv_loop_start ):

    PREFETCH  ( REGOFF(64, ECX) )		/* fetch one/two verts ahead */
    MOVD      ( REGOFF(12, ECX), MM0 )		/*             | f[3]        */
    PFRCP     ( MM0, MM0 )			/* oow = 1/f[3]              */
    MOVD      ( REGOFF(12, ECX), MM7 )		/*             | f[3]        */
    PFRCPIT1  ( MM0, MM7 )
    PFRCPIT2  ( MM0, MM7 )			/* oow         | oow         */
    PUNPCKLDQ ( MM7, MM7 )
    MOVQ      ( REGIND(ECX), MM2 )		/* f[1]        | f[0]        */
    PFMUL     ( MM7, MM2 )			/* f[1] * oow  | f[0] * oow  */
    MOVD      ( REGOFF(8, ECX), MM3 )		/*             | f[2]        */
    PFMUL     ( MM7, MM3 )			/*             | f[2] * oow  */
    MOVD      ( REGOFF(MAT_TZ, EBP), MM0 )	/*             | vtz         */
    PFMUL     ( MM1, MM3 )			/*             | f[2] *= vsz */
    PFADD     ( MM0, MM3 )			/*             | f[2] += vtz */
    PFMUL     ( MM5, MM2 )			/* f[1] *= vsy | f[0] *= vsx */
    PFADD     ( MM6, MM2 )			/* f[1] += vty | f[0] += vtx */
    PUNPCKLDQ ( MM7, MM3 )			/* f[3] = oow  | f[2]        */
    MOVQ      ( MM2, REGOFF(0, ECX) )
    MOVQ      ( MM3, REGOFF(8, ECX) )
    ADD_L     ( EAX, ECX )			/* f += stride               */
    SUB_L     ( EAX, EDX )
    JA        ( LLBL(v16_3dnow_pv_loop_start) )

    FEMMS
    POP_L     ( EBP )
    RET



/*
 * gl_3dnow_project_clipped_vertices( float *first_vertex,
 *                                    const float *last_vertex,
 *                                    float *matrix,
 *                                    GLuint stride,
 *                                    const GLubyte *clip_mask )
 */
GLOBL GLNAME( gl_3dnow_project_clipped_vertices )
GLNAME( gl_3dnow_project_clipped_vertices ):

    PUSH_L    ( EBP )
    PUSH_L    ( ESI )

    FEMMS

    PREFETCH  ( REGOFF(12, ESP) )		/* fetch the first vertex    */

    MOV_L     ( REGOFF(12, ESP), ECX )		/* first_vert                */
    MOV_L     ( REGOFF(16, ESP), EDX )		/* last_vert                 */
    MOV_L     ( REGOFF(20, ESP), EBP )		/* matrix                    */
    MOV_L     ( REGOFF(24, ESP), EAX )		/* stride                    */
    MOV_L     ( REGOFF(28, ESP), ESI )		/* clip_mask                 */


    MOVD      ( REGOFF(MAT_TX, EBP), MM6 )	/*             | tx          */
    PUNPCKLDQ ( REGOFF(MAT_TY, EBP), MM6 )	/* ty          | tx          */
    MOVD      ( REGIND(EBP), MM5 )
    PUNPCKLDQ ( REGOFF(MAT_SY, EBP), MM5 )	/* vsy         | vsx         */
    MOVD      ( REGOFF(MAT_SZ, EBP), MM1 )	/*             | vsz         */

ALIGNTEXT32
LLBL( v16_3dnow_pcv_loop_start ):

    CMP_B     ( CONST(0), REGIND(ESI) )
    JNE       ( LLBL(v16_3dnow_pcv_skip) )

    MOVD      ( REGOFF(12, ECX), MM0)		/*             | f[3]        */
    PFRCP     ( MM0, MM0 )			/* oow = 1/f[3]              */
    MOVD      ( REGOFF(12, ECX), MM7)		/*             | f[3]        */
    PFRCPIT1  ( MM0, MM7 )
    PFRCPIT2  ( MM0, MM7 )			/* oow         | oow         */
    PUNPCKLDQ ( MM7, MM7 )
    MOVQ      ( REGIND(ECX), MM2 )		/* f[1]        | f[0]        */
    PFMUL     ( MM7, MM2 )			/* f[1] * oow  | f[0] * oow  */
    MOVD      ( REGOFF(8, ECX), MM3 )		/*             | f[2]        */
    PFMUL     ( MM7, MM3 )			/*             | f[2] * oow  */
    MOVD      ( REGOFF(MAT_TZ, EBP), MM0 )	/*             | vtz         */
    PFMUL     ( MM1, MM3 )			/*             | f[2] *= vsz */
    PFADD     ( MM0, MM3 )			/*             | f[2] += vtz */
    PFMUL     ( MM5, MM2 )			/* f[1] *= vsy | f[0] *= vsx */
    PFADD     ( MM6, MM2 )			/* f[1] += vty | f[0] += vtx */
    PUNPCKLDQ ( MM7, MM3 )			/* f[3] = oow  | f[2]        */
    MOVQ      ( MM2, REGOFF(0, ECX) )
    MOVQ      ( MM3, REGOFF(8, ECX) )

LLBL( v16_3dnow_pcv_skip ):

    ADD_L     ( EAX, ECX )			/* f += stride               */
    INC_L     ( ESI )				/* next clip_mask            */

    CMP_L     ( ECX, EDX )
    JNE       ( LLBL(v16_3dnow_pcv_loop_start) )

    FEMMS

    POP_L     ( ESI )
    POP_L     ( EBP )
    RET
