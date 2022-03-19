#include "gl_mangle.h"

#include "gl.h"
/*
 * Miscellaneous
 */

extern void mglClearIndex(CTX_ARG GLfloat c );

extern void mglClearColor(CTX_ARG GLclampf red,
			  GLclampf green,
			  GLclampf blue,
			  GLclampf alpha );

extern void mglClear(CTX_ARG GLbitfield mask );

extern void mglIndexMask(CTX_ARG GLuint mask );

extern void mglColorMask(CTX_ARG GLboolean red, GLboolean green,
			 GLboolean blue, GLboolean alpha );

extern void mglAlphaFunc(CTX_ARG GLenum func, GLclampf ref );

extern void mglBlendFunc(CTX_ARG GLenum sfactor, GLenum dfactor );

extern void mglLogicOp(CTX_ARG GLenum opcode );

extern void mglCullFace(CTX_ARG GLenum mode );

extern void mglFrontFace(CTX_ARG GLenum mode );

extern void mglPointSize(CTX_ARG GLfloat size );

extern void mglLineWidth(CTX_ARG GLfloat width );

extern void mglLineStipple(CTX_ARG GLint factor, GLushort pattern );

extern void mglPolygonMode(CTX_ARG GLenum face, GLenum mode );

extern void mglPolygonOffset(CTX_ARG GLfloat factor, GLfloat units );

extern void mglPolygonStipple(CTX_ARG const GLubyte *mask );

extern void mglGetPolygonStipple(CTX_ARG GLubyte *mask );

extern void mglEdgeFlag(CTX_ARG GLboolean flag );

extern void mglEdgeFlagv(CTX_ARG const GLboolean *flag );

extern void mglScissor(CTX_ARG GLint x, GLint y,
                                   GLsizei width, GLsizei height);

extern void mglClipPlane(CTX_ARG GLenum plane, const GLdouble *equation );

extern void mglGetClipPlane(CTX_ARG GLenum plane, GLdouble *equation );

extern void mglDrawBuffer(CTX_ARG GLenum mode );

extern void mglReadBuffer(CTX_ARG GLenum mode );

extern void mglEnable(CTX_ARG GLenum cap );

extern void mglDisable(CTX_ARG GLenum cap );

extern GLboolean mglIsEnabled(CTX_ARG GLenum cap );


extern void mglEnableClientState(CTX_ARG GLenum cap );  /* 1.1 */

extern void mglDisableClientState(CTX_ARG GLenum cap );  /* 1.1 */


extern void mglGetBooleanv(CTX_ARG GLenum pname, GLboolean *params );

extern void mglGetDoublev(CTX_ARG GLenum pname, GLdouble *params );

extern void mglGetFloatv(CTX_ARG GLenum pname, GLfloat *params );

extern void mglGetIntegerv(CTX_ARG GLenum pname, GLint *params );


extern void mglPushAttrib(CTX_ARG GLbitfield mask );

extern void mglPopAttrib(CTX_VOID );


extern void mglPushClientAttrib(CTX_ARG GLbitfield mask );  /* 1.1 */

extern void mglPopClientAttrib(CTX_VOID );  /* 1.1 */


extern GLint mglRenderMode(CTX_ARG GLenum mode );

extern GLenum mglGetError(CTX_VOID );

extern const GLubyte* mglGetString(CTX_ARG GLenum name );

extern void mglFinish(CTX_VOID );

extern void mglFlush(CTX_VOID );

extern void mglHint(CTX_ARG GLenum target, GLenum mode );



/*
 * Depth Buffer
 */

extern void mglClearDepth(CTX_ARG GLclampd depth );

extern void mglDepthFunc(CTX_ARG GLenum func );

extern void mglDepthMask(CTX_ARG GLboolean flag );

extern void mglDepthRange(CTX_ARG GLclampd near_val, GLclampd far_val );


/*
 * Accumulation Buffer
 */

extern void mglClearAccum(CTX_ARG GLfloat red, GLfloat green,
                                      GLfloat blue, GLfloat alpha );

extern void mglAccum(CTX_ARG GLenum op, GLfloat value );



/*
 * Transformation
 */

extern void mglMatrixMode(CTX_ARG GLenum mode );

extern void mglOrtho(CTX_ARG GLdouble left, GLdouble right,
                                 GLdouble bottom, GLdouble top,
                                 GLdouble near_val, GLdouble far_val );

extern void mglFrustum(CTX_ARG GLdouble left, GLdouble right,
                                   GLdouble bottom, GLdouble top,
                                   GLdouble near_val, GLdouble far_val );

extern void mglViewport(CTX_ARG GLint x, GLint y,
                                    GLsizei width, GLsizei height );

extern void mglPushMatrix(CTX_VOID );

extern void mglPopMatrix(CTX_VOID );

extern void mglLoadIdentity(CTX_VOID );

extern void mglLoadMatrixd(CTX_ARG const GLdouble *m );
extern void mglLoadMatrixf(CTX_ARG const GLfloat *m );

extern void mglMultMatrixd(CTX_ARG const GLdouble *m );
extern void mglMultMatrixf(CTX_ARG const GLfloat *m );

extern void mglRotated(CTX_ARG GLdouble angle,
                                   GLdouble x, GLdouble y, GLdouble z );
extern void mglRotatef(CTX_ARG GLfloat angle,
                                   GLfloat x, GLfloat y, GLfloat z );

extern void mglScaled(CTX_ARG GLdouble x, GLdouble y, GLdouble z );
extern void mglScalef(CTX_ARG GLfloat x, GLfloat y, GLfloat z );

extern void mglTranslated(CTX_ARG GLdouble x, GLdouble y, GLdouble z );
extern void mglTranslatef(CTX_ARG GLfloat x, GLfloat y, GLfloat z );



/*
 * Display Lists
 */

extern GLboolean mglIsList(CTX_ARG GLuint list );

extern void mglDeleteLists(CTX_ARG GLuint list, GLsizei range );

extern GLuint mglGenLists(CTX_ARG GLsizei range );

extern void mglNewList(CTX_ARG GLuint list, GLenum mode );

extern void mglEndList(CTX_VOID );

extern void mglCallList(CTX_ARG GLuint list );

extern void mglCallLists(CTX_ARG GLsizei n, GLenum type,
                                     const GLvoid *lists );

extern void mglListBase(CTX_ARG GLuint base );



/*
 * Drawing Functions
 */

extern void mglBegin(CTX_ARG GLenum mode );

extern void mglEnd(CTX_VOID );


extern void mglVertex2d(CTX_ARG GLdouble x, GLdouble y );
extern void mglVertex2f(CTX_ARG GLfloat x, GLfloat y );
extern void mglVertex2i(CTX_ARG GLint x, GLint y );
extern void mglVertex2s(CTX_ARG GLshort x, GLshort y );

extern void mglVertex3d(CTX_ARG GLdouble x, GLdouble y, GLdouble z );
extern void mglVertex3f(CTX_ARG GLfloat x, GLfloat y, GLfloat z );
extern void mglVertex3i(CTX_ARG GLint x, GLint y, GLint z );
extern void mglVertex3s(CTX_ARG GLshort x, GLshort y, GLshort z );

extern void mglVertex4d(CTX_ARG GLdouble x, GLdouble y, GLdouble z, GLdouble w );
extern void mglVertex4f(CTX_ARG GLfloat x, GLfloat y, GLfloat z, GLfloat w );
extern void mglVertex4i(CTX_ARG GLint x, GLint y, GLint z, GLint w );
extern void mglVertex4s(CTX_ARG GLshort x, GLshort y, GLshort z, GLshort w );

extern void mglVertex2dv(CTX_ARG const GLdouble *v );
extern void mglVertex2fv(CTX_ARG const GLfloat *v );
extern void mglVertex2iv(CTX_ARG const GLint *v );
extern void mglVertex2sv(CTX_ARG const GLshort *v );

extern void mglVertex3dv(CTX_ARG const GLdouble *v );
extern void mglVertex3fv(CTX_ARG const GLfloat *v );
extern void mglVertex3iv(CTX_ARG const GLint *v );
extern void mglVertex3sv(CTX_ARG const GLshort *v );

extern void mglVertex4dv(CTX_ARG const GLdouble *v );
extern void mglVertex4fv(CTX_ARG const GLfloat *v );
extern void mglVertex4iv(CTX_ARG const GLint *v );
extern void mglVertex4sv(CTX_ARG const GLshort *v );


extern void mglNormal3b(CTX_ARG GLbyte nx, GLbyte ny, GLbyte nz );
extern void mglNormal3d(CTX_ARG GLdouble nx, GLdouble ny, GLdouble nz );
extern void mglNormal3f(CTX_ARG GLfloat nx, GLfloat ny, GLfloat nz );
extern void mglNormal3i(CTX_ARG GLint nx, GLint ny, GLint nz );
extern void mglNormal3s(CTX_ARG GLshort nx, GLshort ny, GLshort nz );

extern void mglNormal3bv(CTX_ARG const GLbyte *v );
extern void mglNormal3dv(CTX_ARG const GLdouble *v );
extern void mglNormal3fv(CTX_ARG const GLfloat *v );
extern void mglNormal3iv(CTX_ARG const GLint *v );
extern void mglNormal3sv(CTX_ARG const GLshort *v );


extern void mglIndexd(CTX_ARG GLdouble c );
extern void mglIndexf(CTX_ARG GLfloat c );
extern void mglIndexi(CTX_ARG GLint c );
extern void mglIndexs(CTX_ARG GLshort c );
extern void mglIndexub(CTX_ARG GLubyte c );  /* 1.1 */

extern void mglIndexdv(CTX_ARG const GLdouble *c );
extern void mglIndexfv(CTX_ARG const GLfloat *c );
extern void mglIndexiv(CTX_ARG const GLint *c );
extern void mglIndexsv(CTX_ARG const GLshort *c );
extern void mglIndexubv(CTX_ARG const GLubyte *c );  /* 1.1 */

extern void mglColor3b(CTX_ARG GLbyte red, GLbyte green, GLbyte blue );
extern void mglColor3d(CTX_ARG GLdouble red, GLdouble green, GLdouble blue );
extern void mglColor3f(CTX_ARG GLfloat red, GLfloat green, GLfloat blue );
extern void mglColor3i(CTX_ARG GLint red, GLint green, GLint blue );
extern void mglColor3s(CTX_ARG GLshort red, GLshort green, GLshort blue );
extern void mglColor3ub(CTX_ARG GLubyte red, GLubyte green, GLubyte blue );
extern void mglColor3ui(CTX_ARG GLuint red, GLuint green, GLuint blue );
extern void mglColor3us(CTX_ARG GLushort red, GLushort green, GLushort blue );

extern void mglColor4b(CTX_ARG GLbyte red, GLbyte green,
                                   GLbyte blue, GLbyte alpha );
extern void mglColor4d(CTX_ARG GLdouble red, GLdouble green,
                                   GLdouble blue, GLdouble alpha );
extern void mglColor4f(CTX_ARG GLfloat red, GLfloat green,
                                   GLfloat blue, GLfloat alpha );
extern void mglColor4i(CTX_ARG GLint red, GLint green,
                                   GLint blue, GLint alpha );
extern void mglColor4s(CTX_ARG GLshort red, GLshort green,
                                   GLshort blue, GLshort alpha );
extern void mglColor4ub(CTX_ARG GLubyte red, GLubyte green,
                                    GLubyte blue, GLubyte alpha );
extern void mglColor4ui(CTX_ARG GLuint red, GLuint green,
                                    GLuint blue, GLuint alpha );
extern void mglColor4us(CTX_ARG GLushort red, GLushort green,
                                    GLushort blue, GLushort alpha );


extern void mglColor3bv(CTX_ARG const GLbyte *v );
extern void mglColor3dv(CTX_ARG const GLdouble *v );
extern void mglColor3fv(CTX_ARG const GLfloat *v );
extern void mglColor3iv(CTX_ARG const GLint *v );
extern void mglColor3sv(CTX_ARG const GLshort *v );
extern void mglColor3ubv(CTX_ARG const GLubyte *v );
extern void mglColor3uiv(CTX_ARG const GLuint *v );
extern void mglColor3usv(CTX_ARG const GLushort *v );

extern void mglColor4bv(CTX_ARG const GLbyte *v );
extern void mglColor4dv(CTX_ARG const GLdouble *v );
extern void mglColor4fv(CTX_ARG const GLfloat *v );
extern void mglColor4iv(CTX_ARG const GLint *v );
extern void mglColor4sv(CTX_ARG const GLshort *v );
extern void mglColor4ubv(CTX_ARG const GLubyte *v );
extern void mglColor4uiv(CTX_ARG const GLuint *v );
extern void mglColor4usv(CTX_ARG const GLushort *v );


extern void mglTexCoord1d(CTX_ARG GLdouble s );
extern void mglTexCoord1f(CTX_ARG GLfloat s );
extern void mglTexCoord1i(CTX_ARG GLint s );
extern void mglTexCoord1s(CTX_ARG GLshort s );

extern void mglTexCoord2d(CTX_ARG GLdouble s, GLdouble t );
extern void mglTexCoord2f(CTX_ARG GLfloat s, GLfloat t );
extern void mglTexCoord2i(CTX_ARG GLint s, GLint t );
extern void mglTexCoord2s(CTX_ARG GLshort s, GLshort t );

extern void mglTexCoord3d(CTX_ARG GLdouble s, GLdouble t, GLdouble r );
extern void mglTexCoord3f(CTX_ARG GLfloat s, GLfloat t, GLfloat r );
extern void mglTexCoord3i(CTX_ARG GLint s, GLint t, GLint r );
extern void mglTexCoord3s(CTX_ARG GLshort s, GLshort t, GLshort r );

extern void mglTexCoord4d(CTX_ARG GLdouble s, GLdouble t, GLdouble r, GLdouble q );
extern void mglTexCoord4f(CTX_ARG GLfloat s, GLfloat t, GLfloat r, GLfloat q );
extern void mglTexCoord4i(CTX_ARG GLint s, GLint t, GLint r, GLint q );
extern void mglTexCoord4s(CTX_ARG GLshort s, GLshort t, GLshort r, GLshort q );

extern void mglTexCoord1dv(CTX_ARG const GLdouble *v );
extern void mglTexCoord1fv(CTX_ARG const GLfloat *v );
extern void mglTexCoord1iv(CTX_ARG const GLint *v );
extern void mglTexCoord1sv(CTX_ARG const GLshort *v );

extern void mglTexCoord2dv(CTX_ARG const GLdouble *v );
extern void mglTexCoord2fv(CTX_ARG const GLfloat *v );
extern void mglTexCoord2iv(CTX_ARG const GLint *v );
extern void mglTexCoord2sv(CTX_ARG const GLshort *v );

extern void mglTexCoord3dv(CTX_ARG const GLdouble *v );
extern void mglTexCoord3fv(CTX_ARG const GLfloat *v );
extern void mglTexCoord3iv(CTX_ARG const GLint *v );
extern void mglTexCoord3sv(CTX_ARG const GLshort *v );

extern void mglTexCoord4dv(CTX_ARG const GLdouble *v );
extern void mglTexCoord4fv(CTX_ARG const GLfloat *v );
extern void mglTexCoord4iv(CTX_ARG const GLint *v );
extern void mglTexCoord4sv(CTX_ARG const GLshort *v );


extern void mglRasterPos2d(CTX_ARG GLdouble x, GLdouble y );
extern void mglRasterPos2f(CTX_ARG GLfloat x, GLfloat y );
extern void mglRasterPos2i(CTX_ARG GLint x, GLint y );
extern void mglRasterPos2s(CTX_ARG GLshort x, GLshort y );

extern void mglRasterPos3d(CTX_ARG GLdouble x, GLdouble y, GLdouble z );
extern void mglRasterPos3f(CTX_ARG GLfloat x, GLfloat y, GLfloat z );
extern void mglRasterPos3i(CTX_ARG GLint x, GLint y, GLint z );
extern void mglRasterPos3s(CTX_ARG GLshort x, GLshort y, GLshort z );

extern void mglRasterPos4d(CTX_ARG GLdouble x, GLdouble y, GLdouble z, GLdouble w );
extern void mglRasterPos4f(CTX_ARG GLfloat x, GLfloat y, GLfloat z, GLfloat w );
extern void mglRasterPos4i(CTX_ARG GLint x, GLint y, GLint z, GLint w );
extern void mglRasterPos4s(CTX_ARG GLshort x, GLshort y, GLshort z, GLshort w );

extern void mglRasterPos2dv(CTX_ARG const GLdouble *v );
extern void mglRasterPos2fv(CTX_ARG const GLfloat *v );
extern void mglRasterPos2iv(CTX_ARG const GLint *v );
extern void mglRasterPos2sv(CTX_ARG const GLshort *v );

extern void mglRasterPos3dv(CTX_ARG const GLdouble *v );
extern void mglRasterPos3fv(CTX_ARG const GLfloat *v );
extern void mglRasterPos3iv(CTX_ARG const GLint *v );
extern void mglRasterPos3sv(CTX_ARG const GLshort *v );

extern void mglRasterPos4dv(CTX_ARG const GLdouble *v );
extern void mglRasterPos4fv(CTX_ARG const GLfloat *v );
extern void mglRasterPos4iv(CTX_ARG const GLint *v );
extern void mglRasterPos4sv(CTX_ARG const GLshort *v );


extern void mglRectd(CTX_ARG GLdouble x1, GLdouble y1, GLdouble x2, GLdouble y2 );
extern void mglRectf(CTX_ARG GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2 );
extern void mglRecti(CTX_ARG GLint x1, GLint y1, GLint x2, GLint y2 );
extern void mglRects(CTX_ARG GLshort x1, GLshort y1, GLshort x2, GLshort y2 );


extern void mglRectdv(CTX_ARG const GLdouble *v1, const GLdouble *v2 );
extern void mglRectfv(CTX_ARG const GLfloat *v1, const GLfloat *v2 );
extern void mglRectiv(CTX_ARG const GLint *v1, const GLint *v2 );
extern void mglRectsv(CTX_ARG const GLshort *v1, const GLshort *v2 );



/*
 * Vertex Arrays  (1.1)
 */

extern void mglVertexPointer(CTX_ARG GLint size, GLenum type,
                                       GLsizei stride, const GLvoid *ptr );

extern void mglNormalPointer(CTX_ARG GLenum type, GLsizei stride,
                                       const GLvoid *ptr );

extern void mglColorPointer(CTX_ARG GLint size, GLenum type,
                                      GLsizei stride, const GLvoid *ptr );

extern void mglIndexPointer(CTX_ARG GLenum type, GLsizei stride,
                                      const GLvoid *ptr );

extern void mglTexCoordPointer(CTX_ARG GLint size, GLenum type,
                                         GLsizei stride, const GLvoid *ptr );

extern void mglEdgeFlagPointer(CTX_ARG GLsizei stride, const GLvoid *ptr );

extern void mglGetPointerv(CTX_ARG GLenum pname, void **params );

extern void mglArrayElement(CTX_ARG GLint i );

extern void mglDrawArrays(CTX_ARG GLenum mode, GLint first, GLsizei count );

extern void mglDrawElements(CTX_ARG GLenum mode, GLsizei count,
                                      GLenum type, const GLvoid *indices );

extern void mglInterleavedArrays(CTX_ARG GLenum format, GLsizei stride,
                                           const GLvoid *pointer );


/*
 * Lighting
 */

extern void mglShadeModel(CTX_ARG GLenum mode );

extern void mglLightf(CTX_ARG GLenum light, GLenum pname, GLfloat param );
extern void mglLighti(CTX_ARG GLenum light, GLenum pname, GLint param );
extern void mglLightfv(CTX_ARG GLenum light, GLenum pname,
                                 const GLfloat *params );
extern void mglLightiv(CTX_ARG GLenum light, GLenum pname,
                                 const GLint *params );

extern void mglGetLightfv(CTX_ARG GLenum light, GLenum pname,
                                    GLfloat *params );
extern void mglGetLightiv(CTX_ARG GLenum light, GLenum pname,
                                    GLint *params );

extern void mglLightModelf(CTX_ARG GLenum pname, GLfloat param );
extern void mglLightModeli(CTX_ARG GLenum pname, GLint param );
extern void mglLightModelfv(CTX_ARG GLenum pname, const GLfloat *params );
extern void mglLightModeliv(CTX_ARG GLenum pname, const GLint *params );

extern void mglMaterialf(CTX_ARG GLenum face, GLenum pname, GLfloat param );
extern void mglMateriali(CTX_ARG GLenum face, GLenum pname, GLint param );
extern void mglMaterialfv(CTX_ARG GLenum face, GLenum pname, const GLfloat *params );
extern void mglMaterialiv(CTX_ARG GLenum face, GLenum pname, const GLint *params );

extern void mglGetMaterialfv(CTX_ARG GLenum face, GLenum pname, GLfloat *params );
extern void mglGetMaterialiv(CTX_ARG GLenum face, GLenum pname, GLint *params );

extern void mglColorMaterial(CTX_ARG GLenum face, GLenum mode );




/*
 * Raster functions
 */

extern void mglPixelZoom(CTX_ARG GLfloat xfactor, GLfloat yfactor );

extern void mglPixelStoref(CTX_ARG GLenum pname, GLfloat param );
extern void mglPixelStorei(CTX_ARG GLenum pname, GLint param );

extern void mglPixelTransferf(CTX_ARG GLenum pname, GLfloat param );
extern void mglPixelTransferi(CTX_ARG GLenum pname, GLint param );

extern void mglPixelMapfv(CTX_ARG GLenum map, GLint mapsize,
                                    const GLfloat *values );
extern void mglPixelMapuiv(CTX_ARG GLenum map, GLint mapsize,
                                     const GLuint *values );
extern void mglPixelMapusv(CTX_ARG GLenum map, GLint mapsize,
                                     const GLushort *values );

extern void mglGetPixelMapfv(CTX_ARG GLenum map, GLfloat *values );
extern void mglGetPixelMapuiv(CTX_ARG GLenum map, GLuint *values );
extern void mglGetPixelMapusv(CTX_ARG GLenum map, GLushort *values );

extern void mglBitmap(CTX_ARG GLsizei width, GLsizei height,
                                GLfloat xorig, GLfloat yorig,
                                GLfloat xmove, GLfloat ymove,
                                const GLubyte *bitmap );

extern void mglReadPixels(CTX_ARG GLint x, GLint y,
                                    GLsizei width, GLsizei height,
                                    GLenum format, GLenum type,
                                    GLvoid *pixels );

extern void mglDrawPixels(CTX_ARG GLsizei width, GLsizei height,
                                    GLenum format, GLenum type,
                                    const GLvoid *pixels );

extern void mglCopyPixels(CTX_ARG GLint x, GLint y,
                                    GLsizei width, GLsizei height,
                                    GLenum type );



/*
 * Stenciling
 */

extern void mglStencilFunc(CTX_ARG GLenum func, GLint ref, GLuint mask );

extern void mglStencilMask(CTX_ARG GLuint mask );

extern void mglStencilOp(CTX_ARG GLenum fail, GLenum zfail, GLenum zpass );

extern void mglClearStencil(CTX_ARG GLint s );



/*
 * Texture mapping
 */

extern void mglTexGend(CTX_ARG GLenum coord, GLenum pname, GLdouble param );
extern void mglTexGenf(CTX_ARG GLenum coord, GLenum pname, GLfloat param );
extern void mglTexGeni(CTX_ARG GLenum coord, GLenum pname, GLint param );

extern void mglTexGendv(CTX_ARG GLenum coord, GLenum pname, const GLdouble *params );
extern void mglTexGenfv(CTX_ARG GLenum coord, GLenum pname, const GLfloat *params );
extern void mglTexGeniv(CTX_ARG GLenum coord, GLenum pname, const GLint *params );

extern void mglGetTexGendv(CTX_ARG GLenum coord, GLenum pname, GLdouble *params );
extern void mglGetTexGenfv(CTX_ARG GLenum coord, GLenum pname, GLfloat *params );
extern void mglGetTexGeniv(CTX_ARG GLenum coord, GLenum pname, GLint *params );


extern void mglTexEnvf(CTX_ARG GLenum target, GLenum pname, GLfloat param );
extern void mglTexEnvi(CTX_ARG GLenum target, GLenum pname, GLint param );

extern void mglTexEnvfv(CTX_ARG GLenum target, GLenum pname, const GLfloat *params );
extern void mglTexEnviv(CTX_ARG GLenum target, GLenum pname, const GLint *params );

extern void mglGetTexEnvfv(CTX_ARG GLenum target, GLenum pname, GLfloat *params );
extern void mglGetTexEnviv(CTX_ARG GLenum target, GLenum pname, GLint *params );


extern void mglTexParameterf(CTX_ARG GLenum target, GLenum pname, GLfloat param );
extern void mglTexParameteri(CTX_ARG GLenum target, GLenum pname, GLint param );

extern void mglTexParameterfv(CTX_ARG GLenum target, GLenum pname,
                                          const GLfloat *params );
extern void mglTexParameteriv(CTX_ARG GLenum target, GLenum pname,
                                          const GLint *params );

extern void mglGetTexParameterfv(CTX_ARG GLenum target,
                                           GLenum pname, GLfloat *params);
extern void mglGetTexParameteriv(CTX_ARG GLenum target,
                                           GLenum pname, GLint *params );

extern void mglGetTexLevelParameterfv(CTX_ARG GLenum target, GLint level,
                                                GLenum pname, GLfloat *params );
extern void mglGetTexLevelParameteriv(CTX_ARG GLenum target, GLint level,
                                                GLenum pname, GLint *params );


extern void mglTexImage1D(CTX_ARG GLenum target, GLint level,
                                    GLint internalFormat,
                                    GLsizei width, GLint border,
                                    GLenum format, GLenum type,
                                    const GLvoid *pixels );

extern void mglTexImage2D(CTX_ARG GLenum target, GLint level,
                                    GLint internalFormat,
                                    GLsizei width, GLsizei height,
                                    GLint border, GLenum format, GLenum type,
                                    const GLvoid *pixels );

extern void mglGetTexImage(CTX_ARG GLenum target, GLint level,
                                     GLenum format, GLenum type,
                                     GLvoid *pixels );



/* 1.1 functions */

extern void mglGenTextures(CTX_ARG GLsizei n, GLuint *textures );

extern void mglDeleteTextures(CTX_ARG GLsizei n, const GLuint *textures);

extern void mglBindTexture(CTX_ARG GLenum target, GLuint texture );

extern void mglPrioritizeTextures(CTX_ARG GLsizei n,
                                            const GLuint *textures,
                                            const GLclampf *priorities );

extern GLboolean mglAreTexturesResident(CTX_ARG GLsizei n,
                                                  const GLuint *textures,
                                                  GLboolean *residences );

extern GLboolean mglIsTexture(CTX_ARG GLuint texture );


extern void mglTexSubImage1D(CTX_ARG GLenum target, GLint level,
                                       GLint xoffset,
                                       GLsizei width, GLenum format,
                                       GLenum type, const GLvoid *pixels );


extern void mglTexSubImage2D(CTX_ARG GLenum target, GLint level,
                                       GLint xoffset, GLint yoffset,
                                       GLsizei width, GLsizei height,
                                       GLenum format, GLenum type,
                                       const GLvoid *pixels );


extern void mglCopyTexImage1D(CTX_ARG GLenum target, GLint level,
                                        GLenum internalformat,
                                        GLint x, GLint y,
                                        GLsizei width, GLint border );


extern void mglCopyTexImage2D(CTX_ARG GLenum target, GLint level,
                                        GLenum internalformat,
                                        GLint x, GLint y,
                                        GLsizei width, GLsizei height,
                                        GLint border );


extern void mglCopyTexSubImage1D(CTX_ARG GLenum target, GLint level,
                                           GLint xoffset, GLint x, GLint y,
                                           GLsizei width );


extern void mglCopyTexSubImage2D(CTX_ARG GLenum target, GLint level,
                                           GLint xoffset, GLint yoffset,
                                           GLint x, GLint y,
                                           GLsizei width, GLsizei height );




/*
 * Evaluators
 */

extern void mglMap1d(CTX_ARG GLenum target, GLdouble u1, GLdouble u2,
                               GLint stride,
                               GLint order, const GLdouble *points );
extern void mglMap1f(CTX_ARG GLenum target, GLfloat u1, GLfloat u2,
                               GLint stride,
                               GLint order, const GLfloat *points );

extern void mglMap2d(CTX_ARG GLenum target,
		     GLdouble u1, GLdouble u2, GLint ustride, GLint uorder,
		     GLdouble v1, GLdouble v2, GLint vstride, GLint vorder,
		     const GLdouble *points );
extern void mglMap2f(CTX_ARG GLenum target,
		     GLfloat u1, GLfloat u2, GLint ustride, GLint uorder,
		     GLfloat v1, GLfloat v2, GLint vstride, GLint vorder,
		     const GLfloat *points );

extern void mglGetMapdv(CTX_ARG GLenum target, GLenum query, GLdouble *v );
extern void mglGetMapfv(CTX_ARG GLenum target, GLenum query, GLfloat *v );
extern void mglGetMapiv(CTX_ARG GLenum target, GLenum query, GLint *v );

extern void mglEvalCoord1d(CTX_ARG GLdouble u );
extern void mglEvalCoord1f(CTX_ARG GLfloat u );

extern void mglEvalCoord1dv(CTX_ARG const GLdouble *u );
extern void mglEvalCoord1fv(CTX_ARG const GLfloat *u );

extern void mglEvalCoord2d(CTX_ARG GLdouble u, GLdouble v );
extern void mglEvalCoord2f(CTX_ARG GLfloat u, GLfloat v );

extern void mglEvalCoord2dv(CTX_ARG const GLdouble *u );
extern void mglEvalCoord2fv(CTX_ARG const GLfloat *u );

extern void mglMapGrid1d(CTX_ARG GLint un, GLdouble u1, GLdouble u2 );
extern void mglMapGrid1f(CTX_ARG GLint un, GLfloat u1, GLfloat u2 );

extern void mglMapGrid2d(CTX_ARG GLint un, GLdouble u1, GLdouble u2,
                                   GLint vn, GLdouble v1, GLdouble v2 );
extern void mglMapGrid2f(CTX_ARG GLint un, GLfloat u1, GLfloat u2,
                                   GLint vn, GLfloat v1, GLfloat v2 );

extern void mglEvalPoint1(CTX_ARG GLint i );

extern void mglEvalPoint2(CTX_ARG GLint i, GLint j );

extern void mglEvalMesh1(CTX_ARG GLenum mode, GLint i1, GLint i2 );

extern void mglEvalMesh2(CTX_ARG GLenum mode, GLint i1, GLint i2, GLint j1, GLint j2 );



/*
 * Fog
 */

extern void mglFogf(CTX_ARG GLenum pname, GLfloat param );

extern void mglFogi(CTX_ARG GLenum pname, GLint param );

extern void mglFogfv(CTX_ARG GLenum pname, const GLfloat *params );

extern void mglFogiv(CTX_ARG GLenum pname, const GLint *params );



/*
 * Selection and Feedback
 */

extern void mglFeedbackBuffer(CTX_ARG GLsizei size, GLenum type, GLfloat *buffer );

extern void mglPassThrough(CTX_ARG GLfloat token );

extern void mglSelectBuffer(CTX_ARG GLsizei size, GLuint *buffer );

extern void mglInitNames(CTX_VOID );

extern void mglLoadName(CTX_ARG GLuint name );

extern void mglPushName(CTX_ARG GLuint name );

extern void mglPopName(CTX_VOID );



/*
 * 1.0 Extensions
 */

/* GL_EXT_blend_minmax */
extern void mglBlendEquationEXT(CTX_ARG GLenum mode );



/* GL_EXT_blend_color */
extern void mglBlendColorEXT(CTX_ARG GLclampf red, GLclampf green,
                                       GLclampf blue, GLclampf alpha );



/* GL_EXT_polygon_offset */
extern void mglPolygonOffsetEXT(CTX_ARG GLfloat factor, GLfloat bias );



/* GL_EXT_vertex_array */

extern void mglVertexPointerEXT(CTX_ARG GLint size, GLenum type,
                                          GLsizei stride,
                                          GLsizei count, const GLvoid *ptr );

extern void mglNormalPointerEXT(CTX_ARG GLenum type, GLsizei stride,
                                          GLsizei count, const GLvoid *ptr );

extern void mglColorPointerEXT(CTX_ARG GLint size, GLenum type,
                                         GLsizei stride,
                                         GLsizei count, const GLvoid *ptr );

extern void mglIndexPointerEXT(CTX_ARG GLenum type, GLsizei stride,
                                         GLsizei count, const GLvoid *ptr );

extern void mglTexCoordPointerEXT(CTX_ARG GLint size, GLenum type,
                                            GLsizei stride, GLsizei count,
                                            const GLvoid *ptr );

extern void mglEdgeFlagPointerEXT(CTX_ARG GLsizei stride, GLsizei count,
                                            const GLboolean *ptr );

extern void mglGetPointervEXT(CTX_ARG GLenum pname, void **params );

extern void mglArrayElementEXT(CTX_ARG GLint i );

extern void mglDrawArraysEXT(CTX_ARG GLenum mode, GLint first,
                                       GLsizei count );



/* GL_EXT_texture_object */

extern void mglGenTexturesEXT(CTX_ARG GLsizei n, GLuint *textures );

extern void mglDeleteTexturesEXT(CTX_ARG GLsizei n, const GLuint *textures);

extern void mglBindTextureEXT(CTX_ARG GLenum target, GLuint texture );

extern void mglPrioritizeTexturesEXT(CTX_ARG GLsizei n,
                                               const GLuint *textures,
                                               const GLclampf *priorities );

extern GLboolean mglAreTexturesResidentEXT(CTX_ARG GLsizei n,
                                                     const GLuint *textures,
                                                     GLboolean *residences );

extern GLboolean mglIsTextureEXT(CTX_ARG GLuint texture );



/* GL_EXT_texture3D */

extern void mglTexImage3DEXT(CTX_ARG GLenum target, GLint level,
                                       GLenum internalFormat,
                                       GLsizei width, GLsizei height,
                                       GLsizei depth, GLint border,
                                       GLenum format, GLenum type,
                                       const GLvoid *pixels );

extern void mglTexSubImage3DEXT(CTX_ARG GLenum target, GLint level,
                                          GLint xoffset, GLint yoffset,
                                          GLint zoffset, GLsizei width,
                                          GLsizei height, GLsizei depth,
                                          GLenum format,
                                          GLenum type, const GLvoid *pixels);

extern void mglCopyTexSubImage3DEXT(CTX_ARG GLenum target, GLint level,
                                              GLint xoffset, GLint yoffset,
                                              GLint zoffset, GLint x,
                                              GLint y, GLsizei width,
                                              GLsizei height );



/* GL_EXT_color_table */

extern void mglColorTableEXT(CTX_ARG GLenum target, GLenum internalformat,
                                       GLsizei width, GLenum format,
                                       GLenum type, const GLvoid *table );

extern void mglColorSubTableEXT(CTX_ARG GLenum target,
                                          GLsizei start, GLsizei count,
                                          GLenum format, GLenum type,
                                          const GLvoid *data );

extern void mglGetColorTableEXT(CTX_ARG GLenum target, GLenum format,
                                          GLenum type, GLvoid *table );

extern void mglGetColorTableParameterfvEXT(CTX_ARG GLenum target,
                                                     GLenum pname,
                                                     GLfloat *params );

extern void mglGetColorTableParameterivEXT(CTX_ARG GLenum target,
                                                     GLenum pname,
                                                     GLint *params );


/* GL_SGIS_multitexture */

extern void mglMultiTexCoord1dSGIS(CTX_ARG GLenum target, GLdouble s);
extern void mglMultiTexCoord1dvSGIS(CTX_ARG GLenum target, const GLdouble *v);
extern void mglMultiTexCoord1fSGIS(CTX_ARG GLenum target, GLfloat s);
extern void mglMultiTexCoord1fvSGIS(CTX_ARG GLenum target, const GLfloat *v);
extern void mglMultiTexCoord1iSGIS(CTX_ARG GLenum target, GLint s);
extern void mglMultiTexCoord1ivSGIS(CTX_ARG GLenum target, const GLint *v);
extern void mglMultiTexCoord1sSGIS(CTX_ARG GLenum target, GLshort s);
extern void mglMultiTexCoord1svSGIS(CTX_ARG GLenum target, const GLshort *v);
extern void mglMultiTexCoord2dSGIS(CTX_ARG GLenum target, GLdouble s, GLdouble t);
extern void mglMultiTexCoord2dvSGIS(CTX_ARG GLenum target, const GLdouble *v);
extern void mglMultiTexCoord2fSGIS(CTX_ARG GLenum target, GLfloat s, GLfloat t);
extern void mglMultiTexCoord2fvSGIS(CTX_ARG GLenum target, const GLfloat *v);
extern void mglMultiTexCoord2iSGIS(CTX_ARG GLenum target, GLint s, GLint t);
extern void mglMultiTexCoord2ivSGIS(CTX_ARG GLenum target, const GLint *v);
extern void mglMultiTexCoord2sSGIS(CTX_ARG GLenum target, GLshort s, GLshort t);
extern void mglMultiTexCoord2svSGIS(CTX_ARG GLenum target, const GLshort *v);
extern void mglMultiTexCoord3dSGIS(CTX_ARG GLenum target, GLdouble s, GLdouble t, GLdouble r);
extern void mglMultiTexCoord3dvSGIS(CTX_ARG GLenum target, const GLdouble *v);
extern void mglMultiTexCoord3fSGIS(CTX_ARG GLenum target, GLfloat s, GLfloat t, GLfloat r);
extern void mglMultiTexCoord3fvSGIS(CTX_ARG GLenum target, const GLfloat *v);
extern void mglMultiTexCoord3iSGIS(CTX_ARG GLenum target, GLint s, GLint t, GLint r);
extern void mglMultiTexCoord3ivSGIS(CTX_ARG GLenum target, const GLint *v);
extern void mglMultiTexCoord3sSGIS(CTX_ARG GLenum target, GLshort s, GLshort t, GLshort r);
extern void mglMultiTexCoord3svSGIS(CTX_ARG GLenum target, const GLshort *v);
extern void mglMultiTexCoord4dSGIS(CTX_ARG GLenum target, GLdouble s, GLdouble t, GLdouble r, GLdouble q);
extern void mglMultiTexCoord4dvSGIS(CTX_ARG GLenum target, const GLdouble *v);
extern void mglMultiTexCoord4fSGIS(CTX_ARG GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q);
extern void mglMultiTexCoord4fvSGIS(CTX_ARG GLenum target, const GLfloat *v);
extern void mglMultiTexCoord4iSGIS(CTX_ARG GLenum target, GLint s, GLint t, GLint r, GLint q);
extern void mglMultiTexCoord4ivSGIS(CTX_ARG GLenum target, const GLint *v);
extern void mglMultiTexCoord4sSGIS(CTX_ARG GLenum target, GLshort s, GLshort t, GLshort r, GLshort q);
extern void mglMultiTexCoord4svSGIS(CTX_ARG GLenum target, const GLshort *v);

extern void mglMultiTexCoordPointerSGIS(CTX_ARG GLenum target, GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);

extern void mglSelectTextureSGIS(CTX_ARG GLenum target);

extern void mglSelectTextureCoordSetSGIS(CTX_ARG GLenum target);


/* GL_ARB_multitexture */

extern void mglActiveTextureARB(CTX_ARG GLenum texture);
extern void mglClientActiveTextureARB(CTX_ARG GLenum texture);
extern void mglMultiTexCoord1dARB(CTX_ARG GLenum target, GLdouble s);
extern void mglMultiTexCoord1dvARB(CTX_ARG GLenum target, const GLdouble *v);
extern void mglMultiTexCoord1fARB(CTX_ARG GLenum target, GLfloat s);
extern void mglMultiTexCoord1fvARB(CTX_ARG GLenum target, const GLfloat *v);
extern void mglMultiTexCoord1iARB(CTX_ARG GLenum target, GLint s);
extern void mglMultiTexCoord1ivARB(CTX_ARG GLenum target, const GLint *v);
extern void mglMultiTexCoord1sARB(CTX_ARG GLenum target, GLshort s);
extern void mglMultiTexCoord1svARB(CTX_ARG GLenum target, const GLshort *v);
extern void mglMultiTexCoord2dARB(CTX_ARG GLenum target, GLdouble s, GLdouble t);
extern void mglMultiTexCoord2dvARB(CTX_ARG GLenum target, const GLdouble *v);
extern void mglMultiTexCoord2fARB(CTX_ARG GLenum target, GLfloat s, GLfloat t);
extern void mglMultiTexCoord2fvARB(CTX_ARG GLenum target, const GLfloat *v);
extern void mglMultiTexCoord2iARB(CTX_ARG GLenum target, GLint s, GLint t);
extern void mglMultiTexCoord2ivARB(CTX_ARG GLenum target, const GLint *v);
extern void mglMultiTexCoord2sARB(CTX_ARG GLenum target, GLshort s, GLshort t);
extern void mglMultiTexCoord2svARB(CTX_ARG GLenum target, const GLshort *v);
extern void mglMultiTexCoord3dARB(CTX_ARG GLenum target, GLdouble s, GLdouble t, GLdouble r);
extern void mglMultiTexCoord3dvARB(CTX_ARG GLenum target, const GLdouble *v);
extern void mglMultiTexCoord3fARB(CTX_ARG GLenum target, GLfloat s, GLfloat t, GLfloat r);
extern void mglMultiTexCoord3fvARB(CTX_ARG GLenum target, const GLfloat *v);
extern void mglMultiTexCoord3iARB(CTX_ARG GLenum target, GLint s, GLint t, GLint r);
extern void mglMultiTexCoord3ivARB(CTX_ARG GLenum target, const GLint *v);
extern void mglMultiTexCoord3sARB(CTX_ARG GLenum target, GLshort s, GLshort t, GLshort r);
extern void mglMultiTexCoord3svARB(CTX_ARG GLenum target, const GLshort *v);
extern void mglMultiTexCoord4dARB(CTX_ARG GLenum target, GLdouble s, GLdouble t, GLdouble r, GLdouble q);
extern void mglMultiTexCoord4dvARB(CTX_ARG GLenum target, const GLdouble *v);
extern void mglMultiTexCoord4fARB(CTX_ARG GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q);
extern void mglMultiTexCoord4fvARB(CTX_ARG GLenum target, const GLfloat *v);
extern void mglMultiTexCoord4iARB(CTX_ARG GLenum target, GLint s, GLint t, GLint r, GLint q);
extern void mglMultiTexCoord4ivARB(CTX_ARG GLenum target, const GLint *v);
extern void mglMultiTexCoord4sARB(CTX_ARG GLenum target, GLshort s, GLshort t, GLshort r, GLshort q);
extern void mglMultiTexCoord4svARB(CTX_ARG GLenum target, const GLshort *v);



/* GL_EXT_point_parameters */
extern void mglPointParameterfEXT(CTX_ARG GLenum pname, GLfloat param );
extern void mglPointParameterfvEXT(CTX_ARG GLenum pname,
                                               const GLfloat *params );



/* GL_INGR_blend_func_separate */
extern void mglBlendFuncSeparateINGR(CTX_ARG GLenum sfactorRGB,
                                               GLenum dfactorRGB,
                                               GLenum sfactorAlpha, 
                                               GLenum dfactorAlpha );



/* GL_MESA_window_pos */

extern void mglWindowPos2iMESA(CTX_ARG GLint x, GLint y );
extern void mglWindowPos2sMESA(CTX_ARG GLshort x, GLshort y );
extern void mglWindowPos2fMESA(CTX_ARG GLfloat x, GLfloat y );
extern void mglWindowPos2dMESA(CTX_ARG GLdouble x, GLdouble y );

extern void mglWindowPos2ivMESA(CTX_ARG const GLint *p );
extern void mglWindowPos2svMESA(CTX_ARG const GLshort *p );
extern void mglWindowPos2fvMESA(CTX_ARG const GLfloat *p );
extern void mglWindowPos2dvMESA(CTX_ARG const GLdouble *p );

extern void mglWindowPos3iMESA(CTX_ARG GLint x, GLint y, GLint z );
extern void mglWindowPos3sMESA(CTX_ARG GLshort x, GLshort y, GLshort z );
extern void mglWindowPos3fMESA(CTX_ARG GLfloat x, GLfloat y, GLfloat z );
extern void mglWindowPos3dMESA(CTX_ARG GLdouble x, GLdouble y, GLdouble z );

extern void mglWindowPos3ivMESA(CTX_ARG const GLint *p );
extern void mglWindowPos3svMESA(CTX_ARG const GLshort *p );
extern void mglWindowPos3fvMESA(CTX_ARG const GLfloat *p );
extern void mglWindowPos3dvMESA(CTX_ARG const GLdouble *p );

extern void mglWindowPos4iMESA(CTX_ARG GLint x, GLint y, GLint z, GLint w );
extern void mglWindowPos4sMESA(CTX_ARG GLshort x, GLshort y, GLshort z, GLshort w );
extern void mglWindowPos4fMESA(CTX_ARG GLfloat x, GLfloat y, GLfloat z, GLfloat w );
extern void mglWindowPos4dMESA(CTX_ARG GLdouble x, GLdouble y, GLdouble z, GLdouble w);

extern void mglWindowPos4ivMESA(CTX_ARG const GLint *p );
extern void mglWindowPos4svMESA(CTX_ARG const GLshort *p );
extern void mglWindowPos4fvMESA(CTX_ARG const GLfloat *p );
extern void mglWindowPos4dvMESA(CTX_ARG const GLdouble *p );


/* GL_MESA_resize_buffers */

extern void mglResizeBuffersMESA(CTX_VOID );


/* 1.2 functions */
extern void mglDrawRangeElements(CTX_ARG GLenum mode, GLuint start,
	GLuint end, GLsizei count, GLenum type, const GLvoid *indices );

extern void mglTexImage3D(CTX_ARG GLenum target, GLint level,
                                      GLint internalFormat,
                                      GLsizei width, GLsizei height,
                                      GLsizei depth, GLint border,
                                      GLenum format, GLenum type,
                                      const GLvoid *pixels );

extern void mglTexSubImage3D(CTX_ARG GLenum target, GLint level,
                                         GLint xoffset, GLint yoffset,
                                         GLint zoffset, GLsizei width,
                                         GLsizei height, GLsizei depth,
                                         GLenum format,
                                         GLenum type, const GLvoid *pixels);

extern void mglCopyTexSubImage3D(CTX_ARG GLenum target, GLint level,
                                             GLint xoffset, GLint yoffset,
                                             GLint zoffset, GLint x,
                                             GLint y, GLsizei width,
                                             GLsizei height );


/* 1.2 imaging extension functions */

extern void mglBlendEquation(CTX_ARG GLenum mode );

extern void mglBlendColor(CTX_ARG GLclampf red, GLclampf green,
                                    GLclampf blue, GLclampf alpha );

extern void mglHistogram(CTX_ARG GLenum target, GLsizei width,
				   GLenum internalformat, GLboolean sink );

extern void mglResetHistogram(CTX_ARG GLenum target );

extern void mglGetHistogram(CTX_ARG GLenum target, GLboolean reset,
				      GLenum format, GLenum type,
				      GLvoid *values );

extern void mglGetHistogramParameterfv(CTX_ARG GLenum target, GLenum pname,
						 GLfloat *params );

extern void mglGetHistogramParameteriv(CTX_ARG GLenum target, GLenum pname,
						 GLint *params );

extern void mglMinmax(CTX_ARG GLenum target, GLenum internalformat,
				GLboolean sink );

extern void mglResetMinmax(CTX_ARG GLenum target );

extern void mglGetMinMax(CTX_ARG GLenum target, GLboolean reset,
                                   GLenum format, GLenum types,
                                   GLvoid *values );

extern void mglGetMinmaxParameterfv(CTX_ARG GLenum target, GLenum pname,
					      GLfloat *params );

extern void mglGetMinmaxParameteriv(CTX_ARG GLenum target, GLenum pname,
					      GLint *params );

extern void mglConvolutionFilter1D(CTX_ARG GLenum target,
	GLenum internalformat, GLsizei width, GLenum format, GLenum type,
	const GLvoid *image );

extern void mglConvolutionFilter2D(CTX_ARG GLenum target,
	GLenum internalformat, GLsizei width, GLsizei height, GLenum format,
	GLenum type, const GLvoid *image );

extern void mglConvolutionParameterf(CTX_ARG GLenum target, GLenum pname,
	GLfloat params );

extern void mglConvolutionParameterfv(CTX_ARG GLenum target, GLenum pname,
	const GLfloat *params );

extern void mglConvolutionParameteri(CTX_ARG GLenum target, GLenum pname,
	GLint params );

extern void mglConvolutionParameteriv(CTX_ARG GLenum target, GLenum pname,
	const GLint *params );

extern void mglCopyConvolutionFilter1D(CTX_ARG GLenum target,
	GLenum internalformat, GLint x, GLint y, GLsizei width );

extern void mglCopyConvolutionFilter2D(CTX_ARG GLenum target,
	GLenum internalformat, GLint x, GLint y, GLsizei width,
	GLsizei height);

extern void mglGetConvolutionFilter(CTX_ARG GLenum target, GLenum format,
	GLenum type, GLvoid *image );

extern void mglGetConvolutionParameterfv(CTX_ARG GLenum target, GLenum pname,
	GLfloat *params );

extern void mglGetConvolutionParameteriv(CTX_ARG GLenum target, GLenum pname,
	GLint *params );

extern void mglSeparableFilter2D(CTX_ARG GLenum target,
	GLenum internalformat, GLsizei width, GLsizei height, GLenum format,
	GLenum type, const GLvoid *row, const GLvoid *column );

extern void mglGetSeparableFilter(CTX_ARG GLenum target, GLenum format,
	GLenum type, GLvoid *row, GLvoid *column, GLvoid *span );

extern void mglCopyColorSubTable(CTX_ARG GLenum target, GLsizei start,
	GLint x, GLint y, GLsizei width );

extern void mglCopyColorTable(CTX_ARG GLenum target, GLenum internalformat,
	GLint x, GLint y, GLsizei width );



/* GL_EXT_compiled_vertex_array */
extern void mglLockArraysEXT(CTX_ARG GLint first, GLsizei count );
extern void mglUnlockArraysEXT(CTX_VOID );