/**
 * OpenGL mapping functions.
 * Please note that only some functions have been tested by me, most weren't.
 * 
 * For a detailed OpenGL 1.1 documentation please refer to other sources.
 * 
 * **Note: noise module must be loaded by calling LoadLibrary("ogl") before using!**
 * 
 * @example
// The following functions are missing from the implementation (help is much appreciated)

// Vertex Arrays  (1.1)
GLAPI void GLAPIENTRY glVertexPointer(GLint size, GLenum type, GLsizei stride, const GLvoid * ptr);
GLAPI void GLAPIENTRY glNormalPointer(GLenum type, GLsizei stride, const GLvoid * ptr);
GLAPI void GLAPIENTRY glColorPointer(GLint size, GLenum type, GLsizei stride, const GLvoid * ptr);
GLAPI void GLAPIENTRY glIndexPointer(GLenum type, GLsizei stride, const GLvoid * ptr);
GLAPI void GLAPIENTRY glTexCoordPointer(GLint size, GLenum type, GLsizei stride, const GLvoid * ptr);
GLAPI void GLAPIENTRY glEdgeFlagPointer(GLsizei stride, const GLvoid * ptr);
GLAPI void GLAPIENTRY glGetPointerv(GLenum pname, void ** params);
GLAPI void GLAPIENTRY glArrayElement(GLint i);
GLAPI void GLAPIENTRY glDrawArrays(GLenum mode, GLint first, GLsizei count);
GLAPI void GLAPIENTRY glDrawElements(GLenum mode, GLsizei count, GLenum type, const GLvoid * indices);
GLAPI void GLAPIENTRY glInterleavedArrays(GLenum format, GLsizei stride, const GLvoid * pointer);

// Texture mapping
// 1.1 functions
GLAPI void GLAPIENTRY glPrioritizeTextures(GLsizei n, const GLuint * textures, const GLclampf * priorities);
GLAPI GLboolean GLAPIENTRY glAreTexturesResident(GLsizei n, const GLuint * textures, GLboolean * residences);
GLAPI void GLAPIENTRY glTexSubImage1D(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const GLvoid * pixels);
GLAPI void GLAPIENTRY glTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid * pixels);
GLAPI void GLAPIENTRY glCopyTexImage1D(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLint border);
GLAPI void GLAPIENTRY glCopyTexImage2D(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border);
GLAPI void GLAPIENTRY glCopyTexSubImage1D(GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width);
GLAPI void GLAPIENTRY glCopyTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);

// Evaluators
GLAPI void GLAPIENTRY glMap1d(GLenum target, GLdouble u1, GLdouble u2, GLint stride, GLint order, const GLdouble * points);
GLAPI void GLAPIENTRY glMap1f(GLenum target, GLfloat u1, GLfloat u2, GLint stride, GLint order, const GLfloat * points);
GLAPI void GLAPIENTRY glMap2d(GLenum target, GLdouble u1, GLdouble u2, GLint ustride, GLint uorder, GLdouble v1, GLdouble v2, GLint vstride, GLint vorder, const GLdouble* points); GLAPI void GLAPIENTRY glMap2f(GLenum target, GLfloat u1, GLfloat u2, GLint ustride, GLint uorder, GLfloat v1, GLfloat v2, GLint vstride, GLint vorder, const GLfloat* points);
GLAPI void GLAPIENTRY glGetMapdv(GLenum target, GLenum query, GLdouble * v);
GLAPI void GLAPIENTRY glGetMapfv(GLenum target, GLenum query, GLfloat * v);
GLAPI void GLAPIENTRY glGetMapiv(GLenum target, GLenum query, GLint * v);
GLAPI void GLAPIENTRY glEvalCoord1d(GLdouble u);
GLAPI void GLAPIENTRY glEvalCoord1f(GLfloat u);
GLAPI void GLAPIENTRY glEvalCoord1dv(const GLdouble * u);
GLAPI void GLAPIENTRY glEvalCoord1fv(const GLfloat * u);
GLAPI void GLAPIENTRY glEvalCoord2d(GLdouble u, GLdouble v);
GLAPI void GLAPIENTRY glEvalCoord2f(GLfloat u, GLfloat v);
GLAPI void GLAPIENTRY glEvalCoord2dv(const GLdouble * u);
GLAPI void GLAPIENTRY glEvalCoord2fv(const GLfloat * u);
GLAPI void GLAPIENTRY glMapGrid1d(GLint un, GLdouble u1, GLdouble u2);
GLAPI void GLAPIENTRY glMapGrid1f(GLint un, GLfloat u1, GLfloat u2);
GLAPI void GLAPIENTRY glMapGrid2d(GLint un, GLdouble u1, GLdouble u2, GLint vn, GLdouble v1, GLdouble v2);
GLAPI void GLAPIENTRY glMapGrid2f(GLint un, GLfloat u1, GLfloat u2, GLint vn, GLfloat v1, GLfloat v2);
GLAPI void GLAPIENTRY glEvalPoint1(GLint i);
GLAPI void GLAPIENTRY glEvalPoint2(GLint i, GLint j);
GLAPI void GLAPIENTRY glEvalMesh1(GLenum mode, GLint i1, GLint i2);
GLAPI void GLAPIENTRY glEvalMesh2(GLenum mode, GLint i1, GLint i2, GLint j1, GLint j2);

// Selection and Feedback
GLAPI void GLAPIENTRY glFeedbackBuffer(GLsizei size, GLenum type, GLfloat * buffer);
GLAPI void GLAPIENTRY glPassThrough(GLfloat token);
GLAPI void GLAPIENTRY glSelectBuffer(GLsizei size, GLuint * buffer);
GLAPI void GLAPIENTRY glInitNames(void);
GLAPI void GLAPIENTRY glLoadName(GLuint name);
GLAPI void GLAPIENTRY glPushName(GLuint name);
GLAPI void GLAPIENTRY glPopName(void);

// 1.2 functions
GLAPI void GLAPIENTRY glDrawRangeElements(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid * indices);
GLAPI void GLAPIENTRY glTexImage3D(GLenum target, GLint level, GLint internalFormat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid * pixels);
GLAPI void GLAPIENTRY glTexSubImage3D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid * pixels);
GLAPI void GLAPIENTRY glCopyTexSubImage3D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height);

// 1.2 imaging extension functions
GLAPI void GLAPIENTRY glColorTable(GLenum target, GLenum internalformat, GLsizei width, GLenum format, GLenum type, const GLvoid * table);
GLAPI void GLAPIENTRY glColorSubTable(GLenum target, GLsizei start, GLsizei count, GLenum format, GLenum type, const GLvoid * data);
GLAPI void GLAPIENTRY glColorTableParameteriv(GLenum target, GLenum pname, const GLint * params);
GLAPI void GLAPIENTRY glColorTableParameterfv(GLenum target, GLenum pname, const GLfloat * params);
GLAPI void GLAPIENTRY glCopyColorSubTable(GLenum target, GLsizei start, GLint x, GLint y, GLsizei width);
GLAPI void GLAPIENTRY glCopyColorTable(GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width);
GLAPI void GLAPIENTRY glGetColorTable(GLenum target, GLenum format, GLenum type, GLvoid * table);
GLAPI void GLAPIENTRY glGetColorTableParameterfv(GLenum target, GLenum pname, GLfloat * params);
GLAPI void GLAPIENTRY glGetColorTableParameteriv(GLenum target, GLenum pname, GLint * params);
GLAPI void GLAPIENTRY glBlendEquation(GLenum mode);
GLAPI void GLAPIENTRY glBlendColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
GLAPI void GLAPIENTRY glHistogram(GLenum target, GLsizei width, GLenum internalformat, GLboolean sink);
GLAPI void GLAPIENTRY glResetHistogram(GLenum target);
GLAPI void GLAPIENTRY glGetHistogram(GLenum target, GLboolean reset, GLenum format, GLenum type, GLvoid * values);
GLAPI void GLAPIENTRY glGetHistogramParameterfv(GLenum target, GLenum pname, GLfloat * params);
GLAPI void GLAPIENTRY glGetHistogramParameteriv(GLenum target, GLenum pname, GLint * params);
GLAPI void GLAPIENTRY glMinmax(GLenum target, GLenum internalformat, GLboolean sink);
GLAPI void GLAPIENTRY glResetMinmax(GLenum target);
GLAPI void GLAPIENTRY glGetMinmax(GLenum target, GLboolean reset, GLenum format, GLenum types, GLvoid * values);
GLAPI void GLAPIENTRY glGetMinmaxParameterfv(GLenum target, GLenum pname, GLfloat * params);
GLAPI void GLAPIENTRY glGetMinmaxParameteriv(GLenum target, GLenum pname, GLint * params);
GLAPI void GLAPIENTRY glConvolutionFilter1D(GLenum target, GLenum internalformat, GLsizei width, GLenum format, GLenum type, const GLvoid * image);
GLAPI void GLAPIENTRY glConvolutionFilter2D(GLenum target, GLenum internalformat, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid * image);
GLAPI void GLAPIENTRY glConvolutionParameterf(GLenum target, GLenum pname, GLfloat params);
GLAPI void GLAPIENTRY glConvolutionParameterfv(GLenum target, GLenum pname, const GLfloat * params);
GLAPI void GLAPIENTRY glConvolutionParameteri(GLenum target, GLenum pname, GLint params);
GLAPI void GLAPIENTRY glConvolutionParameteriv(GLenum target, GLenum pname, const GLint * params);
GLAPI void GLAPIENTRY glCopyConvolutionFilter1D(GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width);
GLAPI void GLAPIENTRY glCopyConvolutionFilter2D(GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height);
GLAPI void GLAPIENTRY glGetConvolutionFilter(GLenum target, GLenum format, GLenum type, GLvoid * image);
GLAPI void GLAPIENTRY glGetConvolutionParameterfv(GLenum target, GLenum pname, GLfloat * params);
GLAPI void GLAPIENTRY glGetConvolutionParameteriv(GLenum target, GLenum pname, GLint * params);
GLAPI void GLAPIENTRY glSeparableFilter2D(GLenum target, GLenum internalformat, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid * row, const GLvoid * column);
GLAPI void GLAPIENTRY glGetSeparableFilter(GLenum target, GLenum format, GLenum type, GLvoid * row, GLvoid * column, GLvoid * span);
 * @see {@link https://docs.microsoft.com/en-us/windows/win32/opengl/opengl|OpenGL documentation at Microsoft}
 * @see LoadLibrary()
 * 
 * @module ogl
 */

/**
 * @var {*} OGL_WIDTH screen width
 */
/**
 * @var {*} OGL_HEIGHT screen height
 */

/**
 * OpenGL constants
 * @typedef {Object} GL
 * @property {*} 1PASS_EXT
 * @property {*} 1PASS_SGIS
 * @property {*} 2_BYTES
 * @property {*} 2D
 * @property {*} 2PASS_0_EXT
 * @property {*} 2PASS_0_SGIS
 * @property {*} 2PASS_1_EXT
 * @property {*} 2PASS_1_SGIS
 * @property {*} 3_BYTES
 * @property {*} 3D_COLOR_TEXTURE
 * @property {*} 3D_COLOR
 * @property {*} 3D
 * @property {*} 4_BYTES
 * @property {*} 422_AVERAGE_EXT
 * @property {*} 422_EXT
 * @property {*} 422_REV_AVERAGE_EXT
 * @property {*} 422_REV_EXT
 * @property {*} 4D_COLOR_TEXTURE
 * @property {*} 4PASS_0_EXT
 * @property {*} 4PASS_0_SGIS
 * @property {*} 4PASS_1_EXT
 * @property {*} 4PASS_1_SGIS
 * @property {*} 4PASS_2_EXT
 * @property {*} 4PASS_2_SGIS
 * @property {*} 4PASS_3_EXT
 * @property {*} 4PASS_3_SGIS
 * @property {*} ABGR_EXT
 * @property {*} ACCUM_ALPHA_BITS
 * @property {*} ACCUM_BLUE_BITS
 * @property {*} ACCUM_BUFFER_BIT
 * @property {*} ACCUM_CLEAR_VALUE
 * @property {*} ACCUM_GREEN_BITS
 * @property {*} ACCUM_RED_BITS
 * @property {*} ACCUM
 * @property {*} ACTIVE_TEXTURE_ARB
 * @property {*} ACTIVE_VERTEX_UNITS_ARB
 * @property {*} ADD_SIGNED_EXT
 * @property {*} ADD
 * @property {*} ALIASED_LINE_WIDTH_RANGE
 * @property {*} ALIASED_POINT_SIZE_RANGE
 * @property {*} ALL_ATTRIB_BITS
 * @property {*} ALL_CLIENT_ATTRIB_BITS
 * @property {*} ALLOW_DRAW_FRG_HINT_PGI
 * @property {*} ALLOW_DRAW_MEM_HINT_PGI
 * @property {*} ALLOW_DRAW_OBJ_HINT_PGI
 * @property {*} ALLOW_DRAW_WIN_HINT_PGI
 * @property {*} ALPHA_BIAS
 * @property {*} ALPHA_BITS
 * @property {*} ALPHA_MAX_CLAMP_INGR
 * @property {*} ALPHA_MAX_SGIX
 * @property {*} ALPHA_MIN_CLAMP_INGR
 * @property {*} ALPHA_MIN_SGIX
 * @property {*} ALPHA_SCALE
 * @property {*} ALPHA_TEST_FUNC
 * @property {*} ALPHA_TEST_REF
 * @property {*} ALPHA_TEST
 * @property {*} ALPHA
 * @property {*} ALPHA12_EXT
 * @property {*} ALPHA12
 * @property {*} ALPHA16_EXT
 * @property {*} ALPHA16
 * @property {*} ALPHA4_EXT
 * @property {*} ALPHA4
 * @property {*} ALPHA8_EXT
 * @property {*} ALPHA8
 * @property {*} ALWAYS_FAST_HINT_PGI
 * @property {*} ALWAYS_SOFT_HINT_PGI
 * @property {*} ALWAYS
 * @property {*} AMBIENT_AND_DIFFUSE
 * @property {*} AMBIENT
 * @property {*} AND_INVERTED
 * @property {*} AND_REVERSE
 * @property {*} AND
 * @property {*} ARRAY_ELEMENT_LOCK_COUNT_EXT
 * @property {*} ARRAY_ELEMENT_LOCK_FIRST_EXT
 * @property {*} ASYNC_DRAW_PIXELS_SGIX
 * @property {*} ASYNC_HISTOGRAM_SGIX
 * @property {*} ASYNC_MARKER_SGIX
 * @property {*} ASYNC_READ_PIXELS_SGIX
 * @property {*} ASYNC_TEX_IMAGE_SGIX
 * @property {*} ATTENUATION_EXT
 * @property {*} ATTRIB_STACK_DEPTH
 * @property {*} AUTO_NORMAL
 * @property {*} AUX_BUFFERS
 * @property {*} AUX0
 * @property {*} AUX1
 * @property {*} AUX2
 * @property {*} AUX3
 * @property {*} AVERAGE_EXT
 * @property {*} AVERAGE_HP
 * @property {*} BACK_LEFT
 * @property {*} BACK_NORMALS_HINT_PGI
 * @property {*} BACK_RIGHT
 * @property {*} BACK
 * @property {*} BGR_EXT
 * @property {*} BGR
 * @property {*} BGRA_EXT
 * @property {*} BGRA
 * @property {*} BIAS_BY_NEGATIVE_ONE_HALF_NV
 * @property {*} BINORMAL_ARRAY_EXT
 * @property {*} BINORMAL_ARRAY_POINTER_EXT
 * @property {*} BINORMAL_ARRAY_STRIDE_EXT
 * @property {*} BINORMAL_ARRAY_TYPE_EXT
 * @property {*} BITMAP_TOKEN
 * @property {*} BITMAP
 * @property {*} BLEND_COLOR_EXT
 * @property {*} BLEND_COLOR
 * @property {*} BLEND_DST_ALPHA_EXT
 * @property {*} BLEND_DST_RGB_EXT
 * @property {*} BLEND_DST
 * @property {*} BLEND_EQUATION_EXT
 * @property {*} BLEND_EQUATION
 * @property {*} BLEND_SRC_ALPHA_EXT
 * @property {*} BLEND_SRC_RGB_EXT
 * @property {*} BLEND_SRC
 * @property {*} BLEND
 * @property {*} BLUE_BIAS
 * @property {*} BLUE_BITS
 * @property {*} BLUE_MAX_CLAMP_INGR
 * @property {*} BLUE_MIN_CLAMP_INGR
 * @property {*} BLUE_SCALE
 * @property {*} BLUE
 * @property {*} BYTE
 * @property {*} C3F_V3F
 * @property {*} C4F_N3F_V3F
 * @property {*} C4UB_V2F
 * @property {*} C4UB_V3F
 * @property {*} CALLIGRAPHIC_FRAGMENT_SGIX
 * @property {*} CCW
 * @property {*} CLAMP_TO_BORDER_SGIS
 * @property {*} CLAMP_TO_EDGE_SGIS
 * @property {*} CLAMP_TO_EDGE
 * @property {*} CLAMP
 * @property {*} CLEAR
 * @property {*} CLIENT_ACTIVE_TEXTURE_ARB
 * @property {*} CLIENT_ATTRIB_STACK_DEPTH
 * @property {*} CLIENT_PIXEL_STORE_BIT
 * @property {*} CLIENT_VERTEX_ARRAY_BIT
 * @property {*} CLIP_FAR_HINT_PGI
 * @property {*} CLIP_NEAR_HINT_PGI
 * @property {*} CLIP_PLANE0
 * @property {*} CLIP_PLANE1
 * @property {*} CLIP_PLANE2
 * @property {*} CLIP_PLANE3
 * @property {*} CLIP_PLANE4
 * @property {*} CLIP_PLANE5
 * @property {*} CLIP_VOLUME_CLIPPING_HINT_EXT
 * @property {*} CMYK_EXT
 * @property {*} CMYKA_EXT
 * @property {*} COEFF
 * @property {*} COLOR_ARRAY_COUNT_EXT
 * @property {*} COLOR_ARRAY_EXT
 * @property {*} COLOR_ARRAY_LIST_IBM
 * @property {*} COLOR_ARRAY_LIST_STRIDE_IBM
 * @property {*} COLOR_ARRAY_PARALLEL_POINTERS_INTEL
 * @property {*} COLOR_ARRAY_POINTER_EXT
 * @property {*} COLOR_ARRAY_POINTER
 * @property {*} COLOR_ARRAY_SIZE_EXT
 * @property {*} COLOR_ARRAY_SIZE
 * @property {*} COLOR_ARRAY_STRIDE_EXT
 * @property {*} COLOR_ARRAY_STRIDE
 * @property {*} COLOR_ARRAY_TYPE_EXT
 * @property {*} COLOR_ARRAY_TYPE
 * @property {*} COLOR_ARRAY
 * @property {*} COLOR_BUFFER_BIT
 * @property {*} COLOR_CLEAR_VALUE
 * @property {*} COLOR_INDEX
 * @property {*} COLOR_INDEX1_EXT
 * @property {*} COLOR_INDEX12_EXT
 * @property {*} COLOR_INDEX16_EXT
 * @property {*} COLOR_INDEX2_EXT
 * @property {*} COLOR_INDEX4_EXT
 * @property {*} COLOR_INDEX8_EXT
 * @property {*} COLOR_INDEXES
 * @property {*} COLOR_LOGIC_OP
 * @property {*} COLOR_MATERIAL_FACE
 * @property {*} COLOR_MATERIAL_PARAMETER
 * @property {*} COLOR_MATERIAL
 * @property {*} COLOR_MATRIX_SGI
 * @property {*} COLOR_MATRIX_STACK_DEPTH_SGI
 * @property {*} COLOR_MATRIX_STACK_DEPTH
 * @property {*} COLOR_MATRIX
 * @property {*} COLOR_SUM_CLAMP_NV
 * @property {*} COLOR_SUM_EXT
 * @property {*} COLOR_TABLE_ALPHA_SIZE_EXT
 * @property {*} COLOR_TABLE_ALPHA_SIZE_SGI
 * @property {*} COLOR_TABLE_ALPHA_SIZE
 * @property {*} COLOR_TABLE_BIAS_SGI
 * @property {*} COLOR_TABLE_BIAS
 * @property {*} COLOR_TABLE_BLUE_SIZE_EXT
 * @property {*} COLOR_TABLE_BLUE_SIZE_SGI
 * @property {*} COLOR_TABLE_BLUE_SIZE
 * @property {*} COLOR_TABLE_FORMAT_EXT
 * @property {*} COLOR_TABLE_FORMAT_SGI
 * @property {*} COLOR_TABLE_FORMAT
 * @property {*} COLOR_TABLE_GREEN_SIZE_EXT
 * @property {*} COLOR_TABLE_GREEN_SIZE_SGI
 * @property {*} COLOR_TABLE_GREEN_SIZE
 * @property {*} COLOR_TABLE_INTENSITY_SIZE_EXT
 * @property {*} COLOR_TABLE_INTENSITY_SIZE_SGI
 * @property {*} COLOR_TABLE_INTENSITY_SIZE
 * @property {*} COLOR_TABLE_LUMINANCE_SIZE_EXT
 * @property {*} COLOR_TABLE_LUMINANCE_SIZE_SGI
 * @property {*} COLOR_TABLE_LUMINANCE_SIZE
 * @property {*} COLOR_TABLE_RED_SIZE_EXT
 * @property {*} COLOR_TABLE_RED_SIZE_SGI
 * @property {*} COLOR_TABLE_RED_SIZE
 * @property {*} COLOR_TABLE_SCALE_SGI
 * @property {*} COLOR_TABLE_SCALE
 * @property {*} COLOR_TABLE_SGI
 * @property {*} COLOR_TABLE_WIDTH_EXT
 * @property {*} COLOR_TABLE_WIDTH_SGI
 * @property {*} COLOR_TABLE_WIDTH
 * @property {*} COLOR_TABLE
 * @property {*} COLOR_WRITEMASK
 * @property {*} COLOR
 * @property {*} COLOR3_BIT_PGI
 * @property {*} COLOR4_BIT_PGI
 * @property {*} COMBINE_ALPHA_EXT
 * @property {*} COMBINE_EXT
 * @property {*} COMBINE_RGB_EXT
 * @property {*} COMBINE4_NV
 * @property {*} COMBINER_AB_DOT_PRODUCT_NV
 * @property {*} COMBINER_AB_OUTPUT_NV
 * @property {*} COMBINER_BIAS_NV
 * @property {*} COMBINER_CD_DOT_PRODUCT_NV
 * @property {*} COMBINER_CD_OUTPUT_NV
 * @property {*} COMBINER_COMPONENT_USAGE_NV
 * @property {*} COMBINER_INPUT_NV
 * @property {*} COMBINER_MAPPING_NV
 * @property {*} COMBINER_MUX_SUM_NV
 * @property {*} COMBINER_SCALE_NV
 * @property {*} COMBINER_SUM_OUTPUT_NV
 * @property {*} COMBINER0_NV
 * @property {*} COMBINER1_NV
 * @property {*} COMBINER2_NV
 * @property {*} COMBINER3_NV
 * @property {*} COMBINER4_NV
 * @property {*} COMBINER5_NV
 * @property {*} COMBINER6_NV
 * @property {*} COMBINER7_NV
 * @property {*} COMPILE_AND_EXECUTE
 * @property {*} COMPILE
 * @property {*} COMPRESSED_ALPHA_ARB
 * @property {*} COMPRESSED_INTENSITY_ARB
 * @property {*} COMPRESSED_LUMINANCE_ALPHA_ARB
 * @property {*} COMPRESSED_LUMINANCE_ARB
 * @property {*} COMPRESSED_RGB_ARB
 * @property {*} COMPRESSED_RGB_FXT1_3DFX
 * @property {*} COMPRESSED_RGB_S3TC_DXT1_EXT
 * @property {*} COMPRESSED_RGBA_ARB
 * @property {*} COMPRESSED_RGBA_FXT1_3DFX
 * @property {*} COMPRESSED_RGBA_S3TC_DXT1_EXT
 * @property {*} COMPRESSED_RGBA_S3TC_DXT3_EXT
 * @property {*} COMPRESSED_RGBA_S3TC_DXT5_EXT
 * @property {*} COMPRESSED_TEXTURE_FORMATS_ARB
 * @property {*} CONSERVE_MEMORY_HINT_PGI
 * @property {*} CONSTANT_ALPHA_EXT
 * @property {*} CONSTANT_ALPHA
 * @property {*} CONSTANT_ATTENUATION
 * @property {*} CONSTANT_BORDER_HP
 * @property {*} CONSTANT_BORDER
 * @property {*} CONSTANT_COLOR_EXT
 * @property {*} CONSTANT_COLOR
 * @property {*} CONSTANT_COLOR0_NV
 * @property {*} CONSTANT_COLOR1_NV
 * @property {*} CONSTANT_EXT
 * @property {*} CONVOLUTION_1D_EXT
 * @property {*} CONVOLUTION_1D
 * @property {*} CONVOLUTION_2D_EXT
 * @property {*} CONVOLUTION_2D
 * @property {*} CONVOLUTION_BORDER_COLOR_HP
 * @property {*} CONVOLUTION_BORDER_COLOR
 * @property {*} CONVOLUTION_BORDER_MODE_EXT
 * @property {*} CONVOLUTION_BORDER_MODE
 * @property {*} CONVOLUTION_FILTER_BIAS_EXT
 * @property {*} CONVOLUTION_FILTER_BIAS
 * @property {*} CONVOLUTION_FILTER_SCALE_EXT
 * @property {*} CONVOLUTION_FILTER_SCALE
 * @property {*} CONVOLUTION_FORMAT_EXT
 * @property {*} CONVOLUTION_FORMAT
 * @property {*} CONVOLUTION_HEIGHT_EXT
 * @property {*} CONVOLUTION_HEIGHT
 * @property {*} CONVOLUTION_HINT_SGIX
 * @property {*} CONVOLUTION_WIDTH_EXT
 * @property {*} CONVOLUTION_WIDTH
 * @property {*} COPY_INVERTED
 * @property {*} COPY_PIXEL_TOKEN
 * @property {*} COPY
 * @property {*} CUBIC_EXT
 * @property {*} CUBIC_HP
 * @property {*} CULL_FACE_MODE
 * @property {*} CULL_FACE
 * @property {*} CULL_VERTEX_EXT
 * @property {*} CULL_VERTEX_EYE_POSITION_EXT
 * @property {*} CULL_VERTEX_IBM
 * @property {*} CULL_VERTEX_OBJECT_POSITION_EXT
 * @property {*} CURRENT_BINORMAL_EXT
 * @property {*} CURRENT_BIT
 * @property {*} CURRENT_COLOR
 * @property {*} CURRENT_FOG_COORDINATE_EXT
 * @property {*} CURRENT_INDEX
 * @property {*} CURRENT_NORMAL
 * @property {*} CURRENT_RASTER_COLOR
 * @property {*} CURRENT_RASTER_DISTANCE
 * @property {*} CURRENT_RASTER_INDEX
 * @property {*} CURRENT_RASTER_NORMAL_SGIX
 * @property {*} CURRENT_RASTER_POSITION_VALID
 * @property {*} CURRENT_RASTER_POSITION
 * @property {*} CURRENT_RASTER_TEXTURE_COORDS
 * @property {*} CURRENT_SECONDARY_COLOR_EXT
 * @property {*} CURRENT_TANGENT_EXT
 * @property {*} CURRENT_TEXTURE_COORDS
 * @property {*} CURRENT_VERTEX_WEIGHT_EXT
 * @property {*} CURRENT_WEIGHT_ARB
 * @property {*} CW
 * @property {*} DECAL
 * @property {*} DECR_WRAP_EXT
 * @property {*} DECR
 * @property {*} DEFORMATIONS_MASK_SGIX
 * @property {*} DEPTH_BIAS
 * @property {*} DEPTH_BITS
 * @property {*} DEPTH_BUFFER_BIT
 * @property {*} DEPTH_CLEAR_VALUE
 * @property {*} DEPTH_COMPONENT
 * @property {*} DEPTH_COMPONENT16_SGIX
 * @property {*} DEPTH_COMPONENT24_SGIX
 * @property {*} DEPTH_COMPONENT32_SGIX
 * @property {*} DEPTH_FUNC
 * @property {*} DEPTH_PASS_INSTRUMENT_COUNTERS_SGIX
 * @property {*} DEPTH_PASS_INSTRUMENT_MAX_SGIX
 * @property {*} DEPTH_PASS_INSTRUMENT_SGIX
 * @property {*} DEPTH_RANGE
 * @property {*} DEPTH_SCALE
 * @property {*} DEPTH_TEST
 * @property {*} DEPTH_WRITEMASK
 * @property {*} DEPTH
 * @property {*} DETAIL_TEXTURE_2D_BINDING_SGIS
 * @property {*} DETAIL_TEXTURE_2D_SGIS
 * @property {*} DETAIL_TEXTURE_FUNC_POINTS_SGIS
 * @property {*} DETAIL_TEXTURE_LEVEL_SGIS
 * @property {*} DETAIL_TEXTURE_MODE_SGIS
 * @property {*} DIFFUSE
 * @property {*} DISCARD_NV
 * @property {*} DISTANCE_ATTENUATION_EXT
 * @property {*} DISTANCE_ATTENUATION_SGIS
 * @property {*} DITHER
 * @property {*} DOMAIN
 * @property {*} DONT_CARE
 * @property {*} DOT3_RGB_EXT
 * @property {*} DOT3_RGBA_EXT
 * @property {*} DOUBLE
 * @property {*} DOUBLEBUFFER
 * @property {*} DRAW_BUFFER
 * @property {*} DRAW_PIXEL_TOKEN
 * @property {*} DST_ALPHA
 * @property {*} DST_COLOR
 * @property {*} DUAL_ALPHA12_SGIS
 * @property {*} DUAL_ALPHA16_SGIS
 * @property {*} DUAL_ALPHA4_SGIS
 * @property {*} DUAL_ALPHA8_SGIS
 * @property {*} DUAL_INTENSITY12_SGIS
 * @property {*} DUAL_INTENSITY16_SGIS
 * @property {*} DUAL_INTENSITY4_SGIS
 * @property {*} DUAL_INTENSITY8_SGIS
 * @property {*} DUAL_LUMINANCE_ALPHA4_SGIS
 * @property {*} DUAL_LUMINANCE_ALPHA8_SGIS
 * @property {*} DUAL_LUMINANCE12_SGIS
 * @property {*} DUAL_LUMINANCE16_SGIS
 * @property {*} DUAL_LUMINANCE4_SGIS
 * @property {*} DUAL_LUMINANCE8_SGIS
 * @property {*} DUAL_TEXTURE_SELECT_SGIS
 * @property {*} E_TIMES_F_NV
 * @property {*} EDGE_FLAG_ARRAY_COUNT_EXT
 * @property {*} EDGE_FLAG_ARRAY_EXT
 * @property {*} EDGE_FLAG_ARRAY_LIST_IBM
 * @property {*} EDGE_FLAG_ARRAY_LIST_STRIDE_IBM
 * @property {*} EDGE_FLAG_ARRAY_POINTER_EXT
 * @property {*} EDGE_FLAG_ARRAY_POINTER
 * @property {*} EDGE_FLAG_ARRAY_STRIDE_EXT
 * @property {*} EDGE_FLAG_ARRAY_STRIDE
 * @property {*} EDGE_FLAG_ARRAY
 * @property {*} EDGE_FLAG
 * @property {*} EDGEFLAG_BIT_PGI
 * @property {*} EMBOSS_CONSTANT_NV
 * @property {*} EMBOSS_LIGHT_NV
 * @property {*} EMBOSS_MAP_NV
 * @property {*} EMISSION
 * @property {*} ENABLE_BIT
 * @property {*} EQUAL
 * @property {*} EQUIV
 * @property {*} EVAL_BIT
 * @property {*} EXP
 * @property {*} EXP2
 * @property {*} EXPAND_NEGATE_NV
 * @property {*} EXPAND_NORMAL_NV
 * @property {*} EXTENSIONS
 * @property {*} EYE_DISTANCE_TO_LINE_SGIS
 * @property {*} EYE_DISTANCE_TO_POINT_SGIS
 * @property {*} EYE_LINE_SGIS
 * @property {*} EYE_LINEAR
 * @property {*} EYE_PLANE_ABSOLUTE_NV
 * @property {*} EYE_PLANE
 * @property {*} EYE_POINT_SGIS
 * @property {*} EYE_RADIAL_NV
 * @property {*} FALSE
 * @property {*} FASTEST
 * @property {*} FEEDBACK_BUFFER_POINTER
 * @property {*} FEEDBACK_BUFFER_SIZE
 * @property {*} FEEDBACK_BUFFER_TYPE
 * @property {*} FEEDBACK
 * @property {*} FILL
 * @property {*} FILTER4_SGIS
 * @property {*} FLAT
 * @property {*} FLOAT
 * @property {*} FOG_BIT
 * @property {*} FOG_COLOR
 * @property {*} FOG_COORDINATE_ARRAY_EXT
 * @property {*} FOG_COORDINATE_ARRAY_LIST_IBM
 * @property {*} FOG_COORDINATE_ARRAY_LIST_STRIDE_IBM
 * @property {*} FOG_COORDINATE_ARRAY_POINTER_EXT
 * @property {*} FOG_COORDINATE_ARRAY_STRIDE_EXT
 * @property {*} FOG_COORDINATE_ARRAY_TYPE_EXT
 * @property {*} FOG_COORDINATE_EXT
 * @property {*} FOG_COORDINATE_SOURCE_EXT
 * @property {*} FOG_DENSITY
 * @property {*} FOG_DISTANCE_MODE_NV
 * @property {*} FOG_END
 * @property {*} FOG_FUNC_POINTS_SGIS
 * @property {*} FOG_FUNC_SGIS
 * @property {*} FOG_HINT
 * @property {*} FOG_INDEX
 * @property {*} FOG_MODE
 * @property {*} FOG_OFFSET_SGIX
 * @property {*} FOG_OFFSET_VALUE_SGIX
 * @property {*} FOG_SCALE_SGIX
 * @property {*} FOG_SCALE_VALUE_SGIX
 * @property {*} FOG_SPECULAR_TEXTURE_WIN
 * @property {*} FOG_START
 * @property {*} FOG
 * @property {*} FRAGMENT_COLOR_EXT
 * @property {*} FRAGMENT_COLOR_MATERIAL_FACE_SGIX
 * @property {*} FRAGMENT_COLOR_MATERIAL_PARAMETER_SGIX
 * @property {*} FRAGMENT_COLOR_MATERIAL_SGIX
 * @property {*} FRAGMENT_DEPTH_EXT
 * @property {*} FRAGMENT_LIGHT_MODEL_AMBIENT_SGIX
 * @property {*} FRAGMENT_LIGHT_MODEL_LOCAL_VIEWER_SGIX
 * @property {*} FRAGMENT_LIGHT_MODEL_NORMAL_INTERPOLATION_SGIX
 * @property {*} FRAGMENT_LIGHT_MODEL_TWO_SIDE_SGIX
 * @property {*} FRAGMENT_LIGHT0_SGIX
 * @property {*} FRAGMENT_LIGHT1_SGIX
 * @property {*} FRAGMENT_LIGHT2_SGIX
 * @property {*} FRAGMENT_LIGHT3_SGIX
 * @property {*} FRAGMENT_LIGHT4_SGIX
 * @property {*} FRAGMENT_LIGHT5_SGIX
 * @property {*} FRAGMENT_LIGHT6_SGIX
 * @property {*} FRAGMENT_LIGHT7_SGIX
 * @property {*} FRAGMENT_LIGHTING_SGIX
 * @property {*} FRAGMENT_MATERIAL_EXT
 * @property {*} FRAGMENT_NORMAL_EXT
 * @property {*} FRAMEZOOM_FACTOR_SGIX
 * @property {*} FRAMEZOOM_SGIX
 * @property {*} FRONT_AND_BACK
 * @property {*} FRONT_FACE
 * @property {*} FRONT_LEFT
 * @property {*} FRONT_RIGHT
 * @property {*} FRONT
 * @property {*} FULL_STIPPLE_HINT_PGI
 * @property {*} FUNC_ADD_EXT
 * @property {*} FUNC_ADD
 * @property {*} FUNC_REVERSE_SUBTRACT_EXT
 * @property {*} FUNC_REVERSE_SUBTRACT
 * @property {*} FUNC_SUBTRACT_EXT
 * @property {*} FUNC_SUBTRACT
 * @property {*} GENERATE_MIPMAP_HINT_SGIS
 * @property {*} GENERATE_MIPMAP_SGIS
 * @property {*} GEOMETRY_DEFORMATION_BIT_SGIX
 * @property {*} GEOMETRY_DEFORMATION_SGIX
 * @property {*} GEQUAL
 * @property {*} GLOBAL_ALPHA_FACTOR_SUN
 * @property {*} GLOBAL_ALPHA_SUN
 * @property {*} GREATER
 * @property {*} GREEN_BIAS
 * @property {*} GREEN_BITS
 * @property {*} GREEN_MAX_CLAMP_INGR
 * @property {*} GREEN_MIN_CLAMP_INGR
 * @property {*} GREEN_SCALE
 * @property {*} GREEN
 * @property {*} HALF_BIAS_NEGATE_NV
 * @property {*} HALF_BIAS_NORMAL_NV
 * @property {*} HINT_BIT
 * @property {*} HISTOGRAM_ALPHA_SIZE_EXT
 * @property {*} HISTOGRAM_ALPHA_SIZE
 * @property {*} HISTOGRAM_BLUE_SIZE_EXT
 * @property {*} HISTOGRAM_BLUE_SIZE
 * @property {*} HISTOGRAM_EXT
 * @property {*} HISTOGRAM_FORMAT_EXT
 * @property {*} HISTOGRAM_FORMAT
 * @property {*} HISTOGRAM_GREEN_SIZE_EXT
 * @property {*} HISTOGRAM_GREEN_SIZE
 * @property {*} HISTOGRAM_LUMINANCE_SIZE_EXT
 * @property {*} HISTOGRAM_LUMINANCE_SIZE
 * @property {*} HISTOGRAM_RED_SIZE_EXT
 * @property {*} HISTOGRAM_RED_SIZE
 * @property {*} HISTOGRAM_SINK_EXT
 * @property {*} HISTOGRAM_SINK
 * @property {*} HISTOGRAM_WIDTH_EXT
 * @property {*} HISTOGRAM_WIDTH
 * @property {*} HISTOGRAM
 * @property {*} IGNORE_BORDER_HP
 * @property {*} IMAGE_CUBIC_WEIGHT_HP
 * @property {*} IMAGE_MAG_FILTER_HP
 * @property {*} IMAGE_MIN_FILTER_HP
 * @property {*} IMAGE_ROTATE_ANGLE_HP
 * @property {*} IMAGE_ROTATE_ORIGIN_X_HP
 * @property {*} IMAGE_ROTATE_ORIGIN_Y_HP
 * @property {*} IMAGE_SCALE_X_HP
 * @property {*} IMAGE_SCALE_Y_HP
 * @property {*} IMAGE_TRANSFORM_2D_HP
 * @property {*} IMAGE_TRANSLATE_X_HP
 * @property {*} IMAGE_TRANSLATE_Y_HP
 * @property {*} INCR_WRAP_EXT
 * @property {*} INCR
 * @property {*} INDEX_ARRAY_COUNT_EXT
 * @property {*} INDEX_ARRAY_EXT
 * @property {*} INDEX_ARRAY_LIST_IBM
 * @property {*} INDEX_ARRAY_LIST_STRIDE_IBM
 * @property {*} INDEX_ARRAY_POINTER_EXT
 * @property {*} INDEX_ARRAY_POINTER
 * @property {*} INDEX_ARRAY_STRIDE_EXT
 * @property {*} INDEX_ARRAY_STRIDE
 * @property {*} INDEX_ARRAY_TYPE_EXT
 * @property {*} INDEX_ARRAY_TYPE
 * @property {*} INDEX_ARRAY
 * @property {*} INDEX_BIT_PGI
 * @property {*} INDEX_BITS
 * @property {*} INDEX_CLEAR_VALUE
 * @property {*} INDEX_LOGIC_OP
 * @property {*} INDEX_MATERIAL_EXT
 * @property {*} INDEX_MATERIAL_FACE_EXT
 * @property {*} INDEX_MATERIAL_PARAMETER_EXT
 * @property {*} INDEX_MODE
 * @property {*} INDEX_OFFSET
 * @property {*} INDEX_SHIFT
 * @property {*} INDEX_TEST_EXT
 * @property {*} INDEX_TEST_FUNC_EXT
 * @property {*} INDEX_TEST_REF_EXT
 * @property {*} INDEX_WRITEMASK
 * @property {*} INSTRUMENT_BUFFER_POINTER_SGIX
 * @property {*} INSTRUMENT_MEASUREMENTS_SGIX
 * @property {*} INT
 * @property {*} INTENSITY_EXT
 * @property {*} INTENSITY
 * @property {*} INTENSITY12_EXT
 * @property {*} INTENSITY12
 * @property {*} INTENSITY16_EXT
 * @property {*} INTENSITY16
 * @property {*} INTENSITY4_EXT
 * @property {*} INTENSITY4
 * @property {*} INTENSITY8_EXT
 * @property {*} INTENSITY8
 * @property {*} INTERLACE_READ_INGR
 * @property {*} INTERLACE_SGIX
 * @property {*} INTERPOLATE_EXT
 * @property {*} INVALID_ENUM
 * @property {*} INVALID_OPERATION
 * @property {*} INVALID_VALUE
 * @property {*} INVERT
 * @property {*} INVERTED_SCREEN_W_REND
 * @property {*} IR_INSTRUMENT1_SGIX
 * @property {*} IUI_N3F_V2F_EXT
 * @property {*} IUI_N3F_V3F_EXT
 * @property {*} IUI_V2F_EXT
 * @property {*} IUI_V3F_EXT
 * @property {*} KEEP
 * @property {*} LEFT
 * @property {*} LEQUAL
 * @property {*} LESS
 * @property {*} LIGHT_ENV_MODE_SGIX
 * @property {*} LIGHT_MODEL_AMBIENT
 * @property {*} LIGHT_MODEL_COLOR_CONTROL_EXT
 * @property {*} LIGHT_MODEL_COLOR_CONTROL
 * @property {*} LIGHT_MODEL_LOCAL_VIEWER
 * @property {*} LIGHT_MODEL_SPECULAR_VECTOR_APPLE
 * @property {*} LIGHT_MODEL_TWO_SIDE
 * @property {*} LIGHT0
 * @property {*} LIGHT1
 * @property {*} LIGHT2
 * @property {*} LIGHT3
 * @property {*} LIGHT4
 * @property {*} LIGHT5
 * @property {*} LIGHT6
 * @property {*} LIGHT7
 * @property {*} LIGHTING_BIT
 * @property {*} LIGHTING
 * @property {*} LINE_BIT
 * @property {*} LINE_LOOP
 * @property {*} LINE_RESET_TOKEN
 * @property {*} LINE_SMOOTH_HINT
 * @property {*} LINE_SMOOTH
 * @property {*} LINE_STIPPLE_PATTERN
 * @property {*} LINE_STIPPLE_REPEAT
 * @property {*} LINE_STIPPLE
 * @property {*} LINE_STRIP
 * @property {*} LINE_TOKEN
 * @property {*} LINE_WIDTH_GRANULARITY
 * @property {*} LINE_WIDTH_RANGE
 * @property {*} LINE_WIDTH
 * @property {*} LINE
 * @property {*} LINEAR_ATTENUATION
 * @property {*} LINEAR_CLIPMAP_LINEAR_SGIX
 * @property {*} LINEAR_CLIPMAP_NEAREST_SGIX
 * @property {*} LINEAR_DETAIL_ALPHA_SGIS
 * @property {*} LINEAR_DETAIL_COLOR_SGIS
 * @property {*} LINEAR_DETAIL_SGIS
 * @property {*} LINEAR_MIPMAP_LINEAR
 * @property {*} LINEAR_MIPMAP_NEAREST
 * @property {*} LINEAR_SHARPEN_ALPHA_SGIS
 * @property {*} LINEAR_SHARPEN_COLOR_SGIS
 * @property {*} LINEAR_SHARPEN_SGIS
 * @property {*} LINEAR
 * @property {*} LINES
 * @property {*} LIST_BASE
 * @property {*} LIST_BIT
 * @property {*} LIST_INDEX
 * @property {*} LIST_MODE
 * @property {*} LIST_PRIORITY_SGIX
 * @property {*} LOAD
 * @property {*} LOGIC_OP_MODE
 * @property {*} LOGIC_OP
 * @property {*} LUMINANCE_ALPHA
 * @property {*} LUMINANCE
 * @property {*} LUMINANCE12_ALPHA12_EXT
 * @property {*} LUMINANCE12_ALPHA12
 * @property {*} LUMINANCE12_ALPHA4_EXT
 * @property {*} LUMINANCE12_ALPHA4
 * @property {*} LUMINANCE12_EXT
 * @property {*} LUMINANCE12
 * @property {*} LUMINANCE16_ALPHA16_EXT
 * @property {*} LUMINANCE16_ALPHA16
 * @property {*} LUMINANCE16_EXT
 * @property {*} LUMINANCE16
 * @property {*} LUMINANCE4_ALPHA4_EXT
 * @property {*} LUMINANCE4_ALPHA4
 * @property {*} LUMINANCE4_EXT
 * @property {*} LUMINANCE4
 * @property {*} LUMINANCE6_ALPHA2_EXT
 * @property {*} LUMINANCE6_ALPHA2
 * @property {*} LUMINANCE8_ALPHA8_EXT
 * @property {*} LUMINANCE8_ALPHA8
 * @property {*} LUMINANCE8_EXT
 * @property {*} LUMINANCE8
 * @property {*} MAP_COLOR
 * @property {*} MAP_STENCIL
 * @property {*} MAP1_BINORMAL_EXT
 * @property {*} MAP1_COLOR_4
 * @property {*} MAP1_GRID_DOMAIN
 * @property {*} MAP1_GRID_SEGMENTS
 * @property {*} MAP1_INDEX
 * @property {*} MAP1_NORMAL
 * @property {*} MAP1_TANGENT_EXT
 * @property {*} MAP1_TEXTURE_COORD_1
 * @property {*} MAP1_TEXTURE_COORD_2
 * @property {*} MAP1_TEXTURE_COORD_3
 * @property {*} MAP1_TEXTURE_COORD_4
 * @property {*} MAP1_VERTEX_3
 * @property {*} MAP1_VERTEX_4
 * @property {*} MAP2_BINORMAL_EXT
 * @property {*} MAP2_COLOR_4
 * @property {*} MAP2_GRID_DOMAIN
 * @property {*} MAP2_GRID_SEGMENTS
 * @property {*} MAP2_INDEX
 * @property {*} MAP2_NORMAL
 * @property {*} MAP2_TANGENT_EXT
 * @property {*} MAP2_TEXTURE_COORD_1
 * @property {*} MAP2_TEXTURE_COORD_2
 * @property {*} MAP2_TEXTURE_COORD_3
 * @property {*} MAP2_TEXTURE_COORD_4
 * @property {*} MAP2_VERTEX_3
 * @property {*} MAP2_VERTEX_4
 * @property {*} MAT_AMBIENT_AND_DIFFUSE_BIT_PGI
 * @property {*} MAT_AMBIENT_BIT_PGI
 * @property {*} MAT_COLOR_INDEXES_BIT_PGI
 * @property {*} MAT_DIFFUSE_BIT_PGI
 * @property {*} MAT_EMISSION_BIT_PGI
 * @property {*} MAT_SHININESS_BIT_PGI
 * @property {*} MAT_SPECULAR_BIT_PGI
 * @property {*} MATERIAL_SIDE_HINT_PGI
 * @property {*} MATRIX_MODE
 * @property {*} MAX_3D_TEXTURE_SIZE_EXT
 * @property {*} MAX_3D_TEXTURE_SIZE
 * @property {*} MAX_3D_TEXTURE_SIZE
 * @property {*} MAX_4D_TEXTURE_SIZE_SGIS
 * @property {*} MAX_ACTIVE_LIGHTS_SGIX
 * @property {*} MAX_ASYNC_DRAW_PIXELS_SGIX
 * @property {*} MAX_ASYNC_HISTOGRAM_SGIX
 * @property {*} MAX_ASYNC_READ_PIXELS_SGIX
 * @property {*} MAX_ASYNC_TEX_IMAGE_SGIX
 * @property {*} MAX_ATTRIB_STACK_DEPTH
 * @property {*} MAX_CLIENT_ATTRIB_STACK_DEPTH
 * @property {*} MAX_CLIP_PLANES
 * @property {*} MAX_CLIPMAP_DEPTH_SGIX
 * @property {*} MAX_CLIPMAP_VIRTUAL_DEPTH_SGIX
 * @property {*} MAX_COLOR_MATRIX_STACK_DEPTH_SGI
 * @property {*} MAX_COLOR_MATRIX_STACK_DEPTH
 * @property {*} MAX_CONVOLUTION_HEIGHT_EXT
 * @property {*} MAX_CONVOLUTION_HEIGHT
 * @property {*} MAX_CONVOLUTION_WIDTH_EXT
 * @property {*} MAX_CONVOLUTION_WIDTH
 * @property {*} MAX_CUBE_MAP_TEXTURE_SIZE_ARB
 * @property {*} MAX_CUBE_MAP_TEXTURE_SIZE_EXT
 * @property {*} MAX_DEFORMATION_ORDER_SGIX
 * @property {*} MAX_ELEMENTS_INDICES_EXT
 * @property {*} MAX_ELEMENTS_INDICES
 * @property {*} MAX_ELEMENTS_VERTICES_EXT
 * @property {*} MAX_ELEMENTS_VERTICES
 * @property {*} MAX_EVAL_ORDER
 * @property {*} MAX_EXT
 * @property {*} MAX_FOG_FUNC_POINTS_SGIS
 * @property {*} MAX_FRAGMENT_LIGHTS_SGIX
 * @property {*} MAX_FRAMEZOOM_FACTOR_SGIX
 * @property {*} MAX_GENERAL_COMBINERS_NV
 * @property {*} MAX_LIGHTS
 * @property {*} MAX_LIST_NESTING
 * @property {*} MAX_MODELVIEW_STACK_DEPTH
 * @property {*} MAX_NAME_STACK_DEPTH
 * @property {*} MAX_PIXEL_MAP_TABLE
 * @property {*} MAX_PIXEL_TRANSFORM_2D_STACK_DEPTH_EXT
 * @property {*} MAX_PROJECTION_STACK_DEPTH
 * @property {*} MAX_SHININESS_NV
 * @property {*} MAX_SPOT_EXPONENT_NV
 * @property {*} MAX_TEXTURE_LOD_BIAS_EXT
 * @property {*} MAX_TEXTURE_MAX_ANISOTROPY_EXT
 * @property {*} MAX_TEXTURE_SIZE
 * @property {*} MAX_TEXTURE_STACK_DEPTH
 * @property {*} MAX_TEXTURE_UNITS_ARB
 * @property {*} MAX_VERTEX_ARRAY_RANGE_ELEMENT_NV
 * @property {*} MAX_VERTEX_HINT_PGI
 * @property {*} MAX_VERTEX_UNITS_ARB
 * @property {*} MAX_VIEWPORT_DIMS
 * @property {*} MAX
 * @property {*} MIN_EXT
 * @property {*} MIN
 * @property {*} MINMAX_EXT
 * @property {*} MINMAX_FORMAT_EXT
 * @property {*} MINMAX_FORMAT
 * @property {*} MINMAX_SINK_EXT
 * @property {*} MINMAX_SINK
 * @property {*} MINMAX
 * @property {*} MODELVIEW_MATRIX
 * @property {*} MODELVIEW_MATRIX1_EXT
 * @property {*} MODELVIEW_STACK_DEPTH
 * @property {*} MODELVIEW
 * @property {*} MODELVIEW0_ARB
 * @property {*} MODELVIEW0_EXT
 * @property {*} MODELVIEW0_MATRIX_EXT
 * @property {*} MODELVIEW0_STACK_DEPTH_EXT
 * @property {*} MODELVIEW1_ARB
 * @property {*} MODELVIEW1_EXT
 * @property {*} MODELVIEW1_STACK_DEPTH_EXT
 * @property {*} MODELVIEW10_ARB
 * @property {*} MODELVIEW11_ARB
 * @property {*} MODELVIEW12_ARB
 * @property {*} MODELVIEW13_ARB
 * @property {*} MODELVIEW14_ARB
 * @property {*} MODELVIEW15_ARB
 * @property {*} MODELVIEW16_ARB
 * @property {*} MODELVIEW17_ARB
 * @property {*} MODELVIEW18_ARB
 * @property {*} MODELVIEW19_ARB
 * @property {*} MODELVIEW2_ARB
 * @property {*} MODELVIEW20_ARB
 * @property {*} MODELVIEW21_ARB
 * @property {*} MODELVIEW22_ARB
 * @property {*} MODELVIEW23_ARB
 * @property {*} MODELVIEW24_ARB
 * @property {*} MODELVIEW25_ARB
 * @property {*} MODELVIEW26_ARB
 * @property {*} MODELVIEW27_ARB
 * @property {*} MODELVIEW28_ARB
 * @property {*} MODELVIEW29_ARB
 * @property {*} MODELVIEW3_ARB
 * @property {*} MODELVIEW30_ARB
 * @property {*} MODELVIEW31_ARB
 * @property {*} MODELVIEW4_ARB
 * @property {*} MODELVIEW5_ARB
 * @property {*} MODELVIEW6_ARB
 * @property {*} MODELVIEW7_ARB
 * @property {*} MODELVIEW8_ARB
 * @property {*} MODELVIEW9_ARB
 * @property {*} MODULATE
 * @property {*} MULT
 * @property {*} MULTISAMPLE_3DFX
 * @property {*} MULTISAMPLE_ARB
 * @property {*} MULTISAMPLE_BIT_3DFX
 * @property {*} MULTISAMPLE_BIT_ARB
 * @property {*} MULTISAMPLE_EXT
 * @property {*} MULTISAMPLE_SGIS
 * @property {*} N3F_V3F
 * @property {*} NAME_STACK_DEPTH
 * @property {*} NAND
 * @property {*} NATIVE_GRAPHICS_BEGIN_HINT_PGI
 * @property {*} NATIVE_GRAPHICS_END_HINT_PGI
 * @property {*} NATIVE_GRAPHICS_HANDLE_PGI
 * @property {*} NEAREST_CLIPMAP_LINEAR_SGIX
 * @property {*} NEAREST_CLIPMAP_NEAREST_SGIX
 * @property {*} NEAREST_MIPMAP_LINEAR
 * @property {*} NEAREST_MIPMAP_NEAREST
 * @property {*} NEAREST
 * @property {*} NEVER
 * @property {*} NICEST
 * @property {*} NO_ERROR
 * @property {*} NONE
 * @property {*} NOOP
 * @property {*} NOR
 * @property {*} NORMAL_ARRAY_COUNT_EXT
 * @property {*} NORMAL_ARRAY_EXT
 * @property {*} NORMAL_ARRAY_LIST_IBM
 * @property {*} NORMAL_ARRAY_LIST_STRIDE_IBM
 * @property {*} NORMAL_ARRAY_PARALLEL_POINTERS_INTEL
 * @property {*} NORMAL_ARRAY_POINTER_EXT
 * @property {*} NORMAL_ARRAY_POINTER
 * @property {*} NORMAL_ARRAY_STRIDE_EXT
 * @property {*} NORMAL_ARRAY_STRIDE
 * @property {*} NORMAL_ARRAY_TYPE_EXT
 * @property {*} NORMAL_ARRAY_TYPE
 * @property {*} NORMAL_ARRAY
 * @property {*} NORMAL_BIT_PGI
 * @property {*} NORMAL_MAP_ARB
 * @property {*} NORMAL_MAP_EXT
 * @property {*} NORMAL_MAP_NV
 * @property {*} NORMALIZE
 * @property {*} NOTEQUAL
 * @property {*} NUM_COMPRESSED_TEXTURE_FORMATS_ARB
 * @property {*} NUM_GENERAL_COMBINERS_NV
 * @property {*} OBJECT_DISTANCE_TO_LINE_SGIS
 * @property {*} OBJECT_DISTANCE_TO_POINT_SGIS
 * @property {*} OBJECT_LINE_SGIS
 * @property {*} OBJECT_LINEAR
 * @property {*} OBJECT_PLANE
 * @property {*} OBJECT_POINT_SGIS
 * @property {*} OCCLUSION_TEST_HP
 * @property {*} OCCLUSION_TEST_RESULT_HP
 * @property {*} ONE_MINUS_CONSTANT_ALPHA_EXT
 * @property {*} ONE_MINUS_CONSTANT_ALPHA
 * @property {*} ONE_MINUS_CONSTANT_COLOR_EXT
 * @property {*} ONE_MINUS_CONSTANT_COLOR
 * @property {*} ONE_MINUS_DST_ALPHA
 * @property {*} ONE_MINUS_DST_COLOR
 * @property {*} ONE_MINUS_SRC_ALPHA
 * @property {*} ONE_MINUS_SRC_COLOR
 * @property {*} ONE
 * @property {*} OPERAND0_ALPHA_EXT
 * @property {*} OPERAND0_RGB_EXT
 * @property {*} OPERAND1_ALPHA_EXT
 * @property {*} OPERAND1_RGB_EXT
 * @property {*} OPERAND2_ALPHA_EXT
 * @property {*} OPERAND2_RGB_EXT
 * @property {*} OPERAND3_ALPHA_EXT
 * @property {*} OPERAND3_ALPHA_NV
 * @property {*} OPERAND3_RGB_EXT
 * @property {*} OPERAND3_RGB_NV
 * @property {*} OPERAND4_ALPHA_EXT
 * @property {*} OPERAND4_RGB_EXT
 * @property {*} OPERAND5_ALPHA_EXT
 * @property {*} OPERAND5_RGB_EXT
 * @property {*} OPERAND6_ALPHA_EXT
 * @property {*} OPERAND6_RGB_EXT
 * @property {*} OPERAND7_ALPHA_EXT
 * @property {*} OPERAND7_RGB_EXT
 * @property {*} OR_INVERTED
 * @property {*} OR_REVERSE
 * @property {*} OR
 * @property {*} ORDER
 * @property {*} OUT_OF_MEMORY
 * @property {*} PACK_ALIGNMENT
 * @property {*} PACK_CMYK_HINT_EXT
 * @property {*} PACK_IMAGE_DEPTH_SGIS
 * @property {*} PACK_IMAGE_HEIGHT_EXT
 * @property {*} PACK_IMAGE_HEIGHT
 * @property {*} PACK_IMAGE_HEIGHT
 * @property {*} PACK_LSB_FIRST
 * @property {*} PACK_RESAMPLE_SGIX
 * @property {*} PACK_ROW_LENGTH
 * @property {*} PACK_SKIP_IMAGES_EXT
 * @property {*} PACK_SKIP_IMAGES
 * @property {*} PACK_SKIP_IMAGES
 * @property {*} PACK_SKIP_PIXELS
 * @property {*} PACK_SKIP_ROWS
 * @property {*} PACK_SKIP_VOLUMES_SGIS
 * @property {*} PACK_SUBSAMPLE_RATE_SGIX
 * @property {*} PACK_SWAP_BYTES
 * @property {*} PARALLEL_ARRAYS_INTEL
 * @property {*} PASS_THROUGH_TOKEN
 * @property {*} PERSPECTIVE_CORRECTION_HINT
 * @property {*} PERTURB_EXT
 * @property {*} PHONG_HINT_WIN
 * @property {*} PHONG_WIN
 * @property {*} PIXEL_CUBIC_WEIGHT_EXT
 * @property {*} PIXEL_FRAGMENT_ALPHA_SOURCE_SGIS
 * @property {*} PIXEL_FRAGMENT_RGB_SOURCE_SGIS
 * @property {*} PIXEL_GROUP_COLOR_SGIS
 * @property {*} PIXEL_MAG_FILTER_EXT
 * @property {*} PIXEL_MAP_A_TO_A_SIZE
 * @property {*} PIXEL_MAP_A_TO_A
 * @property {*} PIXEL_MAP_B_TO_B_SIZE
 * @property {*} PIXEL_MAP_B_TO_B
 * @property {*} PIXEL_MAP_G_TO_G_SIZE
 * @property {*} PIXEL_MAP_G_TO_G
 * @property {*} PIXEL_MAP_I_TO_A_SIZE
 * @property {*} PIXEL_MAP_I_TO_A
 * @property {*} PIXEL_MAP_I_TO_B_SIZE
 * @property {*} PIXEL_MAP_I_TO_B
 * @property {*} PIXEL_MAP_I_TO_G_SIZE
 * @property {*} PIXEL_MAP_I_TO_G
 * @property {*} PIXEL_MAP_I_TO_I_SIZE
 * @property {*} PIXEL_MAP_I_TO_I
 * @property {*} PIXEL_MAP_I_TO_R_SIZE
 * @property {*} PIXEL_MAP_I_TO_R
 * @property {*} PIXEL_MAP_R_TO_R_SIZE
 * @property {*} PIXEL_MAP_R_TO_R
 * @property {*} PIXEL_MAP_S_TO_S_SIZE
 * @property {*} PIXEL_MAP_S_TO_S
 * @property {*} PIXEL_MIN_FILTER_EXT
 * @property {*} PIXEL_MODE_BIT
 * @property {*} PIXEL_SUBSAMPLE_2424_SGIX
 * @property {*} PIXEL_SUBSAMPLE_4242_SGIX
 * @property {*} PIXEL_SUBSAMPLE_4444_SGIX
 * @property {*} PIXEL_TEX_GEN_MODE_SGIX
 * @property {*} PIXEL_TEX_GEN_SGIX
 * @property {*} PIXEL_TEXTURE_SGIS
 * @property {*} PIXEL_TILE_BEST_ALIGNMENT_SGIX
 * @property {*} PIXEL_TILE_CACHE_INCREMENT_SGIX
 * @property {*} PIXEL_TILE_CACHE_SIZE_SGIX
 * @property {*} PIXEL_TILE_GRID_DEPTH_SGIX
 * @property {*} PIXEL_TILE_GRID_HEIGHT_SGIX
 * @property {*} PIXEL_TILE_GRID_WIDTH_SGIX
 * @property {*} PIXEL_TILE_HEIGHT_SGIX
 * @property {*} PIXEL_TILE_WIDTH_SGIX
 * @property {*} PIXEL_TRANSFORM_2D_EXT
 * @property {*} PIXEL_TRANSFORM_2D_MATRIX_EXT
 * @property {*} PIXEL_TRANSFORM_2D_STACK_DEPTH_EXT
 * @property {*} POINT_BIT
 * @property {*} POINT_FADE_THRESHOLD_SIZE_EXT
 * @property {*} POINT_FADE_THRESHOLD_SIZE_SGIS
 * @property {*} POINT_SIZE_GRANULARITY
 * @property {*} POINT_SIZE_MAX_EXT
 * @property {*} POINT_SIZE_MAX_SGIS
 * @property {*} POINT_SIZE_MIN_EXT
 * @property {*} POINT_SIZE_MIN_SGIS
 * @property {*} POINT_SIZE_RANGE
 * @property {*} POINT_SIZE
 * @property {*} POINT_SMOOTH_HINT
 * @property {*} POINT_SMOOTH
 * @property {*} POINT_TOKEN
 * @property {*} POINT
 * @property {*} POINTS
 * @property {*} POLYGON_BIT
 * @property {*} POLYGON_MODE
 * @property {*} POLYGON_OFFSET_BIAS_EXT
 * @property {*} POLYGON_OFFSET_EXT
 * @property {*} POLYGON_OFFSET_FACTOR_EXT
 * @property {*} POLYGON_OFFSET_FACTOR
 * @property {*} POLYGON_OFFSET_FILL
 * @property {*} POLYGON_OFFSET_LINE
 * @property {*} POLYGON_OFFSET_POINT
 * @property {*} POLYGON_OFFSET_UNITS
 * @property {*} POLYGON_SMOOTH_HINT
 * @property {*} POLYGON_SMOOTH
 * @property {*} POLYGON_STIPPLE_BIT
 * @property {*} POLYGON_STIPPLE
 * @property {*} POLYGON_TOKEN
 * @property {*} POLYGON
 * @property {*} POSITION
 * @property {*} POST_COLOR_MATRIX_ALPHA_BIAS_SGI
 * @property {*} POST_COLOR_MATRIX_ALPHA_BIAS
 * @property {*} POST_COLOR_MATRIX_ALPHA_SCALE_SGI
 * @property {*} POST_COLOR_MATRIX_ALPHA_SCALE
 * @property {*} POST_COLOR_MATRIX_BLUE_BIAS_SGI
 * @property {*} POST_COLOR_MATRIX_BLUE_BIAS
 * @property {*} POST_COLOR_MATRIX_BLUE_SCALE_SGI
 * @property {*} POST_COLOR_MATRIX_BLUE_SCALE
 * @property {*} POST_COLOR_MATRIX_COLOR_TABLE_SGI
 * @property {*} POST_COLOR_MATRIX_COLOR_TABLE
 * @property {*} POST_COLOR_MATRIX_GREEN_BIAS_SGI
 * @property {*} POST_COLOR_MATRIX_GREEN_BIAS
 * @property {*} POST_COLOR_MATRIX_GREEN_SCALE_SGI
 * @property {*} POST_COLOR_MATRIX_GREEN_SCALE
 * @property {*} POST_COLOR_MATRIX_RED_BIAS_SGI
 * @property {*} POST_COLOR_MATRIX_RED_BIAS
 * @property {*} POST_COLOR_MATRIX_RED_SCALE_SGI
 * @property {*} POST_COLOR_MATRIX_RED_SCALE
 * @property {*} POST_CONVOLUTION_ALPHA_BIAS_EXT
 * @property {*} POST_CONVOLUTION_ALPHA_BIAS
 * @property {*} POST_CONVOLUTION_ALPHA_SCALE_EXT
 * @property {*} POST_CONVOLUTION_ALPHA_SCALE
 * @property {*} POST_CONVOLUTION_BLUE_BIAS_EXT
 * @property {*} POST_CONVOLUTION_BLUE_BIAS
 * @property {*} POST_CONVOLUTION_BLUE_SCALE_EXT
 * @property {*} POST_CONVOLUTION_BLUE_SCALE
 * @property {*} POST_CONVOLUTION_COLOR_TABLE_SGI
 * @property {*} POST_CONVOLUTION_COLOR_TABLE
 * @property {*} POST_CONVOLUTION_GREEN_BIAS_EXT
 * @property {*} POST_CONVOLUTION_GREEN_BIAS
 * @property {*} POST_CONVOLUTION_GREEN_SCALE_EXT
 * @property {*} POST_CONVOLUTION_GREEN_SCALE
 * @property {*} POST_CONVOLUTION_RED_BIAS_EXT
 * @property {*} POST_CONVOLUTION_RED_BIAS
 * @property {*} POST_CONVOLUTION_RED_SCALE_EXT
 * @property {*} POST_CONVOLUTION_RED_SCALE
 * @property {*} POST_IMAGE_TRANSFORM_COLOR_TABLE_HP
 * @property {*} POST_TEXTURE_FILTER_BIAS_RANGE_SGIX
 * @property {*} POST_TEXTURE_FILTER_BIAS_SGIX
 * @property {*} POST_TEXTURE_FILTER_SCALE_RANGE_SGIX
 * @property {*} POST_TEXTURE_FILTER_SCALE_SGIX
 * @property {*} PREFER_DOUBLEBUFFER_HINT_PGI
 * @property {*} PREVIOUS_EXT
 * @property {*} PRIMARY_COLOR_EXT
 * @property {*} PRIMARY_COLOR_NV
 * @property {*} PROJECTION_MATRIX
 * @property {*} PROJECTION_STACK_DEPTH
 * @property {*} PROJECTION
 * @property {*} PROXY_COLOR_TABLE_SGI
 * @property {*} PROXY_COLOR_TABLE
 * @property {*} PROXY_HISTOGRAM_EXT
 * @property {*} PROXY_HISTOGRAM
 * @property {*} PROXY_POST_COLOR_MATRIX_COLOR_TABLE_SGI
 * @property {*} PROXY_POST_COLOR_MATRIX_COLOR_TABLE
 * @property {*} PROXY_POST_CONVOLUTION_COLOR_TABLE_SGI
 * @property {*} PROXY_POST_CONVOLUTION_COLOR_TABLE
 * @property {*} PROXY_POST_IMAGE_TRANSFORM_COLOR_TABLE_HP
 * @property {*} PROXY_TEXTURE_1D_EXT
 * @property {*} PROXY_TEXTURE_1D
 * @property {*} PROXY_TEXTURE_2D_EXT
 * @property {*} PROXY_TEXTURE_2D
 * @property {*} PROXY_TEXTURE_3D_EXT
 * @property {*} PROXY_TEXTURE_3D
 * @property {*} PROXY_TEXTURE_3D
 * @property {*} PROXY_TEXTURE_4D_SGIS
 * @property {*} PROXY_TEXTURE_COLOR_TABLE_SGI
 * @property {*} PROXY_TEXTURE_CUBE_MAP_ARB
 * @property {*} PROXY_TEXTURE_CUBE_MAP_EXT
 * @property {*} Q
 * @property {*} QUAD_ALPHA4_SGIS
 * @property {*} QUAD_ALPHA8_SGIS
 * @property {*} QUAD_INTENSITY4_SGIS
 * @property {*} QUAD_INTENSITY8_SGIS
 * @property {*} QUAD_LUMINANCE4_SGIS
 * @property {*} QUAD_LUMINANCE8_SGIS
 * @property {*} QUAD_STRIP
 * @property {*} QUAD_TEXTURE_SELECT_SGIS
 * @property {*} QUADRATIC_ATTENUATION
 * @property {*} QUADS
 * @property {*} R
 * @property {*} R1UI_C3F_V3F_SUN
 * @property {*} R1UI_C4F_N3F_V3F_SUN
 * @property {*} R1UI_C4UB_V3F_SUN
 * @property {*} R1UI_N3F_V3F_SUN
 * @property {*} R1UI_T2F_C4F_N3F_V3F_SUN
 * @property {*} R1UI_T2F_N3F_V3F_SUN
 * @property {*} R1UI_T2F_V3F_SUN
 * @property {*} R1UI_V3F_SUN
 * @property {*} R3_G3_B2
 * @property {*} RASTER_POSITION_UNCLIPPED_IBM
 * @property {*} READ_BUFFER
 * @property {*} RECLAIM_MEMORY_HINT_PGI
 * @property {*} RED_BIAS
 * @property {*} RED_BITS
 * @property {*} RED_MAX_CLAMP_INGR
 * @property {*} RED_MIN_CLAMP_INGR
 * @property {*} RED_SCALE
 * @property {*} RED
 * @property {*} REDUCE_EXT
 * @property {*} REDUCE
 * @property {*} REFERENCE_PLANE_EQUATION_SGIX
 * @property {*} REFERENCE_PLANE_SGIX
 * @property {*} REFLECTION_MAP_ARB
 * @property {*} REFLECTION_MAP_EXT
 * @property {*} REFLECTION_MAP_NV
 * @property {*} REGISTER_COMBINERS_NV
 * @property {*} RENDER_MODE
 * @property {*} RENDER
 * @property {*} RENDERER
 * @property {*} REPEAT
 * @property {*} REPLACE_EXT
 * @property {*} REPLACE_MIDDLE_SUN
 * @property {*} REPLACE_OLDEST_SUN
 * @property {*} REPLACE
 * @property {*} REPLACEMENT_CODE_ARRAY_POINTER_SUN
 * @property {*} REPLACEMENT_CODE_ARRAY_STRIDE_SUN
 * @property {*} REPLACEMENT_CODE_ARRAY_SUN
 * @property {*} REPLACEMENT_CODE_ARRAY_TYPE_SUN
 * @property {*} REPLACEMENT_CODE_SUN
 * @property {*} REPLICATE_BORDER_HP
 * @property {*} REPLICATE_BORDER
 * @property {*} RESAMPLE_DECIMATE_SGIX
 * @property {*} RESAMPLE_REPLICATE_SGIX
 * @property {*} RESAMPLE_ZERO_FILL_SGIX
 * @property {*} RESCALE_NORMAL_EXT
 * @property {*} RESCALE_NORMAL
 * @property {*} RESTART_SUN
 * @property {*} RETURN
 * @property {*} RGB_SCALE_EXT
 * @property {*} RGB
 * @property {*} RGB10_A2_EXT
 * @property {*} RGB10_A2
 * @property {*} RGB10_EXT
 * @property {*} RGB10
 * @property {*} RGB12_EXT
 * @property {*} RGB12
 * @property {*} RGB16_EXT
 * @property {*} RGB16
 * @property {*} RGB2_EXT
 * @property {*} RGB4_EXT
 * @property {*} RGB4
 * @property {*} RGB5_A1_EXT
 * @property {*} RGB5_A1
 * @property {*} RGB5_EXT
 * @property {*} RGB5
 * @property {*} RGB8_EXT
 * @property {*} RGB8
 * @property {*} RGBA_MODE
 * @property {*} RGBA
 * @property {*} RGBA12_EXT
 * @property {*} RGBA12
 * @property {*} RGBA16_EXT
 * @property {*} RGBA16
 * @property {*} RGBA2_EXT
 * @property {*} RGBA2
 * @property {*} RGBA4_EXT
 * @property {*} RGBA4
 * @property {*} RGBA8_EXT
 * @property {*} RGBA8
 * @property {*} RIGHT
 * @property {*} S
 * @property {*} SAMPLE_ALPHA_TO_COVERAGE_ARB
 * @property {*} SAMPLE_ALPHA_TO_MASK_EXT
 * @property {*} SAMPLE_ALPHA_TO_MASK_SGIS
 * @property {*} SAMPLE_ALPHA_TO_ONE_ARB
 * @property {*} SAMPLE_ALPHA_TO_ONE_EXT
 * @property {*} SAMPLE_ALPHA_TO_ONE_SGIS
 * @property {*} SAMPLE_BUFFERS_3DFX
 * @property {*} SAMPLE_BUFFERS_ARB
 * @property {*} SAMPLE_BUFFERS_EXT
 * @property {*} SAMPLE_BUFFERS_SGIS
 * @property {*} SAMPLE_COVERAGE_ARB
 * @property {*} SAMPLE_COVERAGE_INVERT_ARB
 * @property {*} SAMPLE_COVERAGE_VALUE_ARB
 * @property {*} SAMPLE_MASK_EXT
 * @property {*} SAMPLE_MASK_INVERT_EXT
 * @property {*} SAMPLE_MASK_INVERT_SGIS
 * @property {*} SAMPLE_MASK_SGIS
 * @property {*} SAMPLE_MASK_VALUE_EXT
 * @property {*} SAMPLE_MASK_VALUE_SGIS
 * @property {*} SAMPLE_PATTERN_EXT
 * @property {*} SAMPLE_PATTERN_SGIS
 * @property {*} SAMPLES_3DFX
 * @property {*} SAMPLES_ARB
 * @property {*} SAMPLES_EXT
 * @property {*} SAMPLES_SGIS
 * @property {*} SCALE_BY_FOUR_NV
 * @property {*} SCALE_BY_ONE_HALF_NV
 * @property {*} SCALE_BY_TWO_NV
 * @property {*} SCISSOR_BIT
 * @property {*} SCISSOR_BOX
 * @property {*} SCISSOR_TEST
 * @property {*} SCREEN_COORDINATES_REND
 * @property {*} SECONDARY_COLOR_ARRAY_EXT
 * @property {*} SECONDARY_COLOR_ARRAY_LIST_IBM
 * @property {*} SECONDARY_COLOR_ARRAY_LIST_STRIDE_IBM
 * @property {*} SECONDARY_COLOR_ARRAY_POINTER_EXT
 * @property {*} SECONDARY_COLOR_ARRAY_SIZE_EXT
 * @property {*} SECONDARY_COLOR_ARRAY_STRIDE_EXT
 * @property {*} SECONDARY_COLOR_ARRAY_TYPE_EXT
 * @property {*} SECONDARY_COLOR_NV
 * @property {*} SELECT
 * @property {*} SELECTION_BUFFER_POINTER
 * @property {*} SELECTION_BUFFER_SIZE
 * @property {*} SEPARABLE_2D_EXT
 * @property {*} SEPARABLE_2D
 * @property {*} SEPARATE_SPECULAR_COLOR_EXT
 * @property {*} SEPARATE_SPECULAR_COLOR
 * @property {*} SET
 * @property {*} SHADE_MODEL
 * @property {*} SHADOW_AMBIENT_SGIX
 * @property {*} SHADOW_ATTENUATION_EXT
 * @property {*} SHARED_TEXTURE_PALETTE_EXT
 * @property {*} SHARPEN_TEXTURE_FUNC_POINTS_SGIS
 * @property {*} SHININESS
 * @property {*} SHORT
 * @property {*} SIGNED_IDENTITY_NV
 * @property {*} SIGNED_NEGATE_NV
 * @property {*} SINGLE_COLOR_EXT
 * @property {*} SINGLE_COLOR
 * @property {*} SMOOTH_LINE_WIDTH_GRANULARITY
 * @property {*} SMOOTH_LINE_WIDTH_RANGE
 * @property {*} SMOOTH_POINT_SIZE_GRANULARITY
 * @property {*} SMOOTH_POINT_SIZE_RANGE
 * @property {*} SMOOTH
 * @property {*} SOURCE0_ALPHA_EXT
 * @property {*} SOURCE0_RGB_EXT
 * @property {*} SOURCE1_ALPHA_EXT
 * @property {*} SOURCE1_RGB_EXT
 * @property {*} SOURCE2_ALPHA_EXT
 * @property {*} SOURCE2_RGB_EXT
 * @property {*} SOURCE3_ALPHA_EXT
 * @property {*} SOURCE3_ALPHA_NV
 * @property {*} SOURCE3_RGB_EXT
 * @property {*} SOURCE3_RGB_NV
 * @property {*} SOURCE4_ALPHA_EXT
 * @property {*} SOURCE4_RGB_EXT
 * @property {*} SOURCE5_ALPHA_EXT
 * @property {*} SOURCE5_RGB_EXT
 * @property {*} SOURCE6_ALPHA_EXT
 * @property {*} SOURCE6_RGB_EXT
 * @property {*} SOURCE7_ALPHA_EXT
 * @property {*} SOURCE7_RGB_EXT
 * @property {*} SPARE0_NV
 * @property {*} SPARE0_PLUS_SECONDARY_COLOR_NV
 * @property {*} SPARE1_NV
 * @property {*} SPECULAR
 * @property {*} SPHERE_MAP
 * @property {*} SPOT_CUTOFF
 * @property {*} SPOT_DIRECTION
 * @property {*} SPOT_EXPONENT
 * @property {*} SPRITE_AXIAL_SGIX
 * @property {*} SPRITE_AXIS_SGIX
 * @property {*} SPRITE_EYE_ALIGNED_SGIX
 * @property {*} SPRITE_MODE_SGIX
 * @property {*} SPRITE_OBJECT_ALIGNED_SGIX
 * @property {*} SPRITE_SGIX
 * @property {*} SPRITE_TRANSLATION_SGIX
 * @property {*} SRC_ALPHA_SATURATE
 * @property {*} SRC_ALPHA
 * @property {*} SRC_COLOR
 * @property {*} STACK_OVERFLOW
 * @property {*} STACK_UNDERFLOW
 * @property {*} STENCIL_BITS
 * @property {*} STENCIL_BUFFER_BIT
 * @property {*} STENCIL_CLEAR_VALUE
 * @property {*} STENCIL_FAIL
 * @property {*} STENCIL_FUNC
 * @property {*} STENCIL_INDEX
 * @property {*} STENCIL_PASS_DEPTH_FAIL
 * @property {*} STENCIL_PASS_DEPTH_PASS
 * @property {*} STENCIL_REF
 * @property {*} STENCIL_TEST
 * @property {*} STENCIL_VALUE_MASK
 * @property {*} STENCIL_WRITEMASK
 * @property {*} STENCIL
 * @property {*} STEREO
 * @property {*} STRICT_DEPTHFUNC_HINT_PGI
 * @property {*} STRICT_LIGHTING_HINT_PGI
 * @property {*} STRICT_SCISSOR_HINT_PGI
 * @property {*} SUBPIXEL_BITS
 * @property {*} T
 * @property {*} T2F_C3F_V3F
 * @property {*} T2F_C4F_N3F_V3F
 * @property {*} T2F_C4UB_V3F
 * @property {*} T2F_IUI_N3F_V2F_EXT
 * @property {*} T2F_IUI_N3F_V3F_EXT
 * @property {*} T2F_IUI_V2F_EXT
 * @property {*} T2F_IUI_V3F_EXT
 * @property {*} T2F_N3F_V3F
 * @property {*} T2F_V3F
 * @property {*} T4F_C4F_N3F_V4F
 * @property {*} T4F_V4F
 * @property {*} TABLE_TOO_LARGE_EXT
 * @property {*} TABLE_TOO_LARGE
 * @property {*} TANGENT_ARRAY_EXT
 * @property {*} TANGENT_ARRAY_POINTER_EXT
 * @property {*} TANGENT_ARRAY_STRIDE_EXT
 * @property {*} TANGENT_ARRAY_TYPE_EXT
 * @property {*} TEXCOORD1_BIT_PGI
 * @property {*} TEXCOORD2_BIT_PGI
 * @property {*} TEXCOORD3_BIT_PGI
 * @property {*} TEXCOORD4_BIT_PGI
 * @property {*} TEXTURE_1D_BINDING_EXT
 * @property {*} TEXTURE_1D
 * @property {*} TEXTURE_2D_BINDING_EXT
 * @property {*} TEXTURE_2D
 * @property {*} TEXTURE_3D_BINDING_EXT
 * @property {*} TEXTURE_3D_EXT
 * @property {*} TEXTURE_3D
 * @property {*} TEXTURE_3D
 * @property {*} TEXTURE_4D_BINDING_SGIS
 * @property {*} TEXTURE_4D_SGIS
 * @property {*} TEXTURE_4DSIZE_SGIS
 * @property {*} TEXTURE_ALPHA_SIZE_EXT
 * @property {*} TEXTURE_ALPHA_SIZE
 * @property {*} TEXTURE_APPLICATION_MODE_EXT
 * @property {*} TEXTURE_BASE_LEVEL_SGIS
 * @property {*} TEXTURE_BASE_LEVEL
 * @property {*} TEXTURE_BINDING_1D
 * @property {*} TEXTURE_BINDING_2D
 * @property {*} TEXTURE_BINDING_3D
 * @property {*} TEXTURE_BINDING_CUBE_MAP_ARB
 * @property {*} TEXTURE_BINDING_CUBE_MAP_EXT
 * @property {*} TEXTURE_BIT
 * @property {*} TEXTURE_BLUE_SIZE_EXT
 * @property {*} TEXTURE_BLUE_SIZE
 * @property {*} TEXTURE_BORDER_COLOR
 * @property {*} TEXTURE_BORDER
 * @property {*} TEXTURE_CLIPMAP_CENTER_SGIX
 * @property {*} TEXTURE_CLIPMAP_DEPTH_SGIX
 * @property {*} TEXTURE_CLIPMAP_FRAME_SGIX
 * @property {*} TEXTURE_CLIPMAP_LOD_OFFSET_SGIX
 * @property {*} TEXTURE_CLIPMAP_OFFSET_SGIX
 * @property {*} TEXTURE_CLIPMAP_VIRTUAL_DEPTH_SGIX
 * @property {*} TEXTURE_COLOR_TABLE_SGI
 * @property {*} TEXTURE_COLOR_WRITEMASK_SGIS
 * @property {*} TEXTURE_COMPARE_OPERATOR_SGIX
 * @property {*} TEXTURE_COMPARE_SGIX
 * @property {*} TEXTURE_COMPONENTS
 * @property {*} TEXTURE_COMPRESSED_ARB
 * @property {*} TEXTURE_COMPRESSED_IMAGE_SIZE_ARB
 * @property {*} TEXTURE_COMPRESSION_HINT_ARB
 * @property {*} TEXTURE_CONSTANT_DATA_SUNX
 * @property {*} TEXTURE_COORD_ARRAY_COUNT_EXT
 * @property {*} TEXTURE_COORD_ARRAY_EXT
 * @property {*} TEXTURE_COORD_ARRAY_LIST_IBM
 * @property {*} TEXTURE_COORD_ARRAY_LIST_STRIDE_IBM
 * @property {*} TEXTURE_COORD_ARRAY_PARALLEL_POINTERS_INTEL
 * @property {*} TEXTURE_COORD_ARRAY_POINTER_EXT
 * @property {*} TEXTURE_COORD_ARRAY_POINTER
 * @property {*} TEXTURE_COORD_ARRAY_SIZE_EXT
 * @property {*} TEXTURE_COORD_ARRAY_SIZE
 * @property {*} TEXTURE_COORD_ARRAY_STRIDE_EXT
 * @property {*} TEXTURE_COORD_ARRAY_STRIDE
 * @property {*} TEXTURE_COORD_ARRAY_TYPE_EXT
 * @property {*} TEXTURE_COORD_ARRAY_TYPE
 * @property {*} TEXTURE_COORD_ARRAY
 * @property {*} TEXTURE_CUBE_MAP_ARB
 * @property {*} TEXTURE_CUBE_MAP_EXT
 * @property {*} TEXTURE_CUBE_MAP_NEGATIVE_X_ARB
 * @property {*} TEXTURE_CUBE_MAP_NEGATIVE_X_EXT
 * @property {*} TEXTURE_CUBE_MAP_NEGATIVE_Y_ARB
 * @property {*} TEXTURE_CUBE_MAP_NEGATIVE_Y_EXT
 * @property {*} TEXTURE_CUBE_MAP_NEGATIVE_Z_ARB
 * @property {*} TEXTURE_CUBE_MAP_NEGATIVE_Z_EXT
 * @property {*} TEXTURE_CUBE_MAP_POSITIVE_X_ARB
 * @property {*} TEXTURE_CUBE_MAP_POSITIVE_X_EXT
 * @property {*} TEXTURE_CUBE_MAP_POSITIVE_Y_ARB
 * @property {*} TEXTURE_CUBE_MAP_POSITIVE_Y_EXT
 * @property {*} TEXTURE_CUBE_MAP_POSITIVE_Z_ARB
 * @property {*} TEXTURE_CUBE_MAP_POSITIVE_Z_EXT
 * @property {*} TEXTURE_DEFORMATION_BIT_SGIX
 * @property {*} TEXTURE_DEFORMATION_SGIX
 * @property {*} TEXTURE_DEPTH_EXT
 * @property {*} TEXTURE_DEPTH
 * @property {*} TEXTURE_DEPTH
 * @property {*} TEXTURE_ENV_BIAS_SGIX
 * @property {*} TEXTURE_ENV_COLOR
 * @property {*} TEXTURE_ENV_MODE
 * @property {*} TEXTURE_ENV
 * @property {*} TEXTURE_FILTER_CONTROL_EXT
 * @property {*} TEXTURE_FILTER4_SIZE_SGIS
 * @property {*} TEXTURE_GEN_MODE
 * @property {*} TEXTURE_GEN_Q
 * @property {*} TEXTURE_GEN_R
 * @property {*} TEXTURE_GEN_S
 * @property {*} TEXTURE_GEN_T
 * @property {*} TEXTURE_GEQUAL_R_SGIX
 * @property {*} TEXTURE_GREEN_SIZE_EXT
 * @property {*} TEXTURE_GREEN_SIZE
 * @property {*} TEXTURE_HEIGHT
 * @property {*} TEXTURE_INDEX_SIZE_EXT
 * @property {*} TEXTURE_INTENSITY_SIZE_EXT
 * @property {*} TEXTURE_INTENSITY_SIZE
 * @property {*} TEXTURE_INTERNAL_FORMAT
 * @property {*} TEXTURE_LEQUAL_R_SGIX
 * @property {*} TEXTURE_LIGHT_EXT
 * @property {*} TEXTURE_LIGHTING_MODE_HP
 * @property {*} TEXTURE_LOD_BIAS_EXT
 * @property {*} TEXTURE_LOD_BIAS_R_SGIX
 * @property {*} TEXTURE_LOD_BIAS_S_SGIX
 * @property {*} TEXTURE_LOD_BIAS_T_SGIX
 * @property {*} TEXTURE_LUMINANCE_SIZE_EXT
 * @property {*} TEXTURE_LUMINANCE_SIZE
 * @property {*} TEXTURE_MAG_FILTER
 * @property {*} TEXTURE_MATERIAL_FACE_EXT
 * @property {*} TEXTURE_MATERIAL_PARAMETER_EXT
 * @property {*} TEXTURE_MATRIX
 * @property {*} TEXTURE_MAX_ANISOTROPY_EXT
 * @property {*} TEXTURE_MAX_LEVEL_SGIS
 * @property {*} TEXTURE_MAX_LEVEL
 * @property {*} TEXTURE_MAX_LOD_SGIS
 * @property {*} TEXTURE_MAX_LOD
 * @property {*} TEXTURE_MIN_FILTER
 * @property {*} TEXTURE_MIN_LOD_SGIS
 * @property {*} TEXTURE_MIN_LOD
 * @property {*} TEXTURE_MULTI_BUFFER_HINT_SGIX
 * @property {*} TEXTURE_NORMAL_EXT
 * @property {*} TEXTURE_POST_SPECULAR_HP
 * @property {*} TEXTURE_PRE_SPECULAR_HP
 * @property {*} TEXTURE_PRIORITY_EXT
 * @property {*} TEXTURE_PRIORITY
 * @property {*} TEXTURE_RED_SIZE_EXT
 * @property {*} TEXTURE_RED_SIZE
 * @property {*} TEXTURE_RESIDENT_EXT
 * @property {*} TEXTURE_RESIDENT
 * @property {*} TEXTURE_STACK_DEPTH
 * @property {*} TEXTURE_TOO_LARGE_EXT
 * @property {*} TEXTURE_WIDTH
 * @property {*} TEXTURE_WRAP_Q_SGIS
 * @property {*} TEXTURE_WRAP_R_EXT
 * @property {*} TEXTURE_WRAP_R
 * @property {*} TEXTURE_WRAP_R
 * @property {*} TEXTURE_WRAP_S
 * @property {*} TEXTURE_WRAP_T
 * @property {*} TEXTURE
 * @property {*} TEXTURE0_ARB
 * @property {*} TEXTURE1_ARB
 * @property {*} TEXTURE10_ARB
 * @property {*} TEXTURE11_ARB
 * @property {*} TEXTURE12_ARB
 * @property {*} TEXTURE13_ARB
 * @property {*} TEXTURE14_ARB
 * @property {*} TEXTURE15_ARB
 * @property {*} TEXTURE16_ARB
 * @property {*} TEXTURE17_ARB
 * @property {*} TEXTURE18_ARB
 * @property {*} TEXTURE19_ARB
 * @property {*} TEXTURE2_ARB
 * @property {*} TEXTURE20_ARB
 * @property {*} TEXTURE21_ARB
 * @property {*} TEXTURE22_ARB
 * @property {*} TEXTURE23_ARB
 * @property {*} TEXTURE24_ARB
 * @property {*} TEXTURE25_ARB
 * @property {*} TEXTURE26_ARB
 * @property {*} TEXTURE27_ARB
 * @property {*} TEXTURE28_ARB
 * @property {*} TEXTURE29_ARB
 * @property {*} TEXTURE3_ARB
 * @property {*} TEXTURE30_ARB
 * @property {*} TEXTURE31_ARB
 * @property {*} TEXTURE4_ARB
 * @property {*} TEXTURE5_ARB
 * @property {*} TEXTURE6_ARB
 * @property {*} TEXTURE7_ARB
 * @property {*} TEXTURE8_ARB
 * @property {*} TEXTURE9_ARB
 * @property {*} TRANSFORM_BIT
 * @property {*} TRANSFORM_HINT_APPLE
 * @property {*} TRANSPOSE_COLOR_MATRIX_ARB
 * @property {*} TRANSPOSE_MODELVIEW_MATRIX_ARB
 * @property {*} TRANSPOSE_PROJECTION_MATRIX_ARB
 * @property {*} TRANSPOSE_TEXTURE_MATRIX_ARB
 * @property {*} TRIANGLE_FAN
 * @property {*} TRIANGLE_LIST_SUN
 * @property {*} TRIANGLE_STRIP
 * @property {*} TRIANGLES
 * @property {*} TRUE
 * @property {*} UNPACK_ALIGNMENT
 * @property {*} UNPACK_CMYK_HINT_EXT
 * @property {*} UNPACK_CONSTANT_DATA_SUNX
 * @property {*} UNPACK_IMAGE_DEPTH_SGIS
 * @property {*} UNPACK_IMAGE_HEIGHT_EXT
 * @property {*} UNPACK_IMAGE_HEIGHT
 * @property {*} UNPACK_IMAGE_HEIGHT
 * @property {*} UNPACK_LSB_FIRST
 * @property {*} UNPACK_RESAMPLE_SGIX
 * @property {*} UNPACK_ROW_LENGTH
 * @property {*} UNPACK_SKIP_IMAGES_EXT
 * @property {*} UNPACK_SKIP_IMAGES
 * @property {*} UNPACK_SKIP_IMAGES
 * @property {*} UNPACK_SKIP_PIXELS
 * @property {*} UNPACK_SKIP_ROWS
 * @property {*} UNPACK_SKIP_VOLUMES_SGIS
 * @property {*} UNPACK_SUBSAMPLE_RATE_SGIX
 * @property {*} UNPACK_SWAP_BYTES
 * @property {*} UNSIGNED_BYTE_2_3_3_REV
 * @property {*} UNSIGNED_BYTE_3_3_2_EXT
 * @property {*} UNSIGNED_BYTE_3_3_2
 * @property {*} UNSIGNED_BYTE
 * @property {*} UNSIGNED_IDENTITY_NV
 * @property {*} UNSIGNED_INT_10_10_10_2_EXT
 * @property {*} UNSIGNED_INT_10_10_10_2
 * @property {*} UNSIGNED_INT_2_10_10_10_REV
 * @property {*} UNSIGNED_INT_8_8_8_8_EXT
 * @property {*} UNSIGNED_INT_8_8_8_8_REV
 * @property {*} UNSIGNED_INT_8_8_8_8
 * @property {*} UNSIGNED_INT
 * @property {*} UNSIGNED_INVERT_NV
 * @property {*} UNSIGNED_SHORT_1_5_5_5_REV
 * @property {*} UNSIGNED_SHORT_4_4_4_4_EXT
 * @property {*} UNSIGNED_SHORT_4_4_4_4_REV
 * @property {*} UNSIGNED_SHORT_4_4_4_4
 * @property {*} UNSIGNED_SHORT_5_5_5_1_EXT
 * @property {*} UNSIGNED_SHORT_5_5_5_1
 * @property {*} UNSIGNED_SHORT_5_6_5_REV
 * @property {*} UNSIGNED_SHORT_5_6_5
 * @property {*} UNSIGNED_SHORT
 * @property {*} V2F
 * @property {*} V3F
 * @property {*} VARIABLE_A_NV
 * @property {*} VARIABLE_B_NV
 * @property {*} VARIABLE_C_NV
 * @property {*} VARIABLE_D_NV
 * @property {*} VARIABLE_E_NV
 * @property {*} VARIABLE_F_NV
 * @property {*} VARIABLE_G_NV
 * @property {*} VENDOR
 * @property {*} VERSION
 * @property {*} VERTEX_ARRAY_COUNT_EXT
 * @property {*} VERTEX_ARRAY_EXT
 * @property {*} VERTEX_ARRAY_LIST_IBM
 * @property {*} VERTEX_ARRAY_LIST_STRIDE_IBM
 * @property {*} VERTEX_ARRAY_PARALLEL_POINTERS_INTEL
 * @property {*} VERTEX_ARRAY_POINTER_EXT
 * @property {*} VERTEX_ARRAY_POINTER
 * @property {*} VERTEX_ARRAY_RANGE_LENGTH_NV
 * @property {*} VERTEX_ARRAY_RANGE_NV
 * @property {*} VERTEX_ARRAY_RANGE_POINTER_NV
 * @property {*} VERTEX_ARRAY_RANGE_VALID_NV
 * @property {*} VERTEX_ARRAY_SIZE_EXT
 * @property {*} VERTEX_ARRAY_SIZE
 * @property {*} VERTEX_ARRAY_STRIDE_EXT
 * @property {*} VERTEX_ARRAY_STRIDE
 * @property {*} VERTEX_ARRAY_TYPE_EXT
 * @property {*} VERTEX_ARRAY_TYPE
 * @property {*} VERTEX_ARRAY
 * @property {*} VERTEX_BLEND_ARB
 * @property {*} VERTEX_CONSISTENT_HINT_PGI
 * @property {*} VERTEX_DATA_HINT_PGI
 * @property {*} VERTEX_PRECLIP_HINT_SGIX
 * @property {*} VERTEX_PRECLIP_SGIX
 * @property {*} VERTEX_WEIGHT_ARRAY_EXT
 * @property {*} VERTEX_WEIGHT_ARRAY_POINTER_EXT
 * @property {*} VERTEX_WEIGHT_ARRAY_SIZE_EXT
 * @property {*} VERTEX_WEIGHT_ARRAY_STRIDE_EXT
 * @property {*} VERTEX_WEIGHT_ARRAY_TYPE_EXT
 * @property {*} VERTEX_WEIGHTING_EXT
 * @property {*} VERTEX23_BIT_PGI
 * @property {*} VERTEX4_BIT_PGI
 * @property {*} VIEWPORT_BIT
 * @property {*} VIEWPORT
 * @property {*} WEIGHT_ARRAY_ARB
 * @property {*} WEIGHT_ARRAY_POINTER_ARB
 * @property {*} WEIGHT_ARRAY_SIZE_ARB
 * @property {*} WEIGHT_ARRAY_STRIDE_ARB
 * @property {*} WEIGHT_ARRAY_TYPE_ARB
 * @property {*} WEIGHT_SUM_UNITY_ARB
 * @property {*} WIDE_LINE_HINT_PGI
 * @property {*} WRAP_BORDER_SUN
 * @property {*} XOR
 * @property {*} YCRCB_422_SGIX
 * @property {*} YCRCB_444_SGIX
 * @property {*} YCRCB_SGIX
 * @property {*} YCRCBA_SGIX
 * @property {*} ZERO
 * @property {*} ZOOM_X
 * @property {*} ZOOM_Y
 */

// 0 param
/** */
function glInit() { }
/** */
function glShutdown() { }
/** */
function glFlush() { }
/** */
function glFinish() { }
/** */
function glEnd() { }
/** */
function glLoadIdentity() { }
/** */
function glPushMatrix() { }
/** */
function glPopMatrix() { }
/** */
function glEndList() { }
/** */
function glGetError() { }
/** */
function glPopAttrib() { }
/** */
function glPopClientAttrib() { }
/** */
function glGetPolygonStipple() { }
/** */
function glutWireDodecahedron() { }
/** */
function glutSolidDodecahedron() { }
/** */
function glutWireOctahedron() { }
/** */
function glutSolidOctahedron() { }
/** */
function glutWireTetrahedron() { }
/** */
function glutSolidTetrahedron() { }
/** */
function glutWireIcosahedron() { }
/** */
function glutSolidIcosahedron() { }

// 1 param
/** 
 * @param {boolean} b
 */
function glDepthMask(b) { }
/** 
 * @param {boolean} b
 */
function glEdgeFlag(b) { }
/** 
 * @param {number} num
 */
function glBegin(num) { }
/** 
 * @param {number} num
 */
function glCallList(num) { }
/** 
 * @param {number} num
 */
function glClear(num) { }
/** 
 * @param {number} num
 */
function glCullFace(num) { }
/** 
 * @param {number} num
 */
function glDisable(num) { }
/** 
 * @param {number} num
 */
function glEnable(num) { }
/** 
 * @param {number} num
 */
function glIndex(num) { }
/** 
 * @param {number} num
 */
function glListBase(num) { }
/** 
 * @param {number} num
 */
function glMatrixMode(num) { }
/** 
 * @param {number} num
 */
function glClearIndex(num) { }
/** 
 * @param {number} num
 */
function glLineWidth(num) { }
/** 
 * @param {number} num
 */
function glPointSize(num) { }
/** 
 * @param {number} num
 */
function glDisableClientState(num) { }
/** 
 * @param {number} num
 */
function glDrawBuffer(num) { }
/** 
 * @param {number} num
 */
function glEnableClientState(num) { }
/** 
 * @param {number} num
 */
function glIndexMask(num) { }
/** 
 * @param {number} num
 */
function glFrontFace(num) { }
/** 
 * @param {number} num
 */
function glLogicOp(num) { }
/** 
 * @param {number} num
 */
function glPushAttrib(num) { }
/** 
 * @param {number} num
 */
function glPushClientAttrib(num) { }
/** 
 * @param {number} num
 */
function glReadBuffer(num) { }
/** 
 * @param {number} num
 */
function glClearDepth(num) { }
/** 
 * @param {number} num
 */
function glDepthFunc(num) { }
/** 
 * @param {number} num
 */
function glShadeModel(num) { }
/** 
 * @param {number} num
 */
function glStencilMask(num) { }
/** 
 * @param {number} num
 */
function glClearStencil(num) { }
/**
 * @param {number} num
 * @returns {number[]} 
 */
function glGetClipPlane(num) { }
/**
 * @param {number} num
 * @returns {boolean[]} 
 */
function glGetBoolean(num) { }
/**
 * @param {number} num
 * @returns {number[]} 
 */
function glGetDouble(num) { }
/**
 * @param {number} num
 * @returns {number[]} 
 */
function glGetFloat(num) { }
/**
 * @param {number} num
 * @returns {number[]} 
 */
function glGetInteger(num) { }
/**
 * @param {number[]} num
 */
function glLoadMatrix(num) { }
/**
 * @param {number[]} num
 */
function glMultMatrix(num) { }
/**
 * @param {number} num
 */
function glCallLists(num) { }
/**
 * @param {number} num
 * @returns {number[]} 
 */
function glGenTextures(num) { }
/**
 * @param {number[]} num
 */
function glPolygonStipple(num) { }
/**
 * @param {Bitmap} bm
 */
function glDrawPixels(bm) { }
/**
 * @param {number} num
 */
function glutWireCube(num) { }
/**
 * @param {number} num
 */
function glutSolidCube(num) { }
/**
 * @param {number|number[]} num
 */
function glDeleteTextures(num) { }
/**
 * @param {number} num
 * @returns {string} 
 */
function glGetString(num) { }
/**
 * @param {number} num
 * @returns {number} 
 */
function glGenLists(num) { }
/**
 * @param {number} num
 * @returns {number} 
 */
function glRenderMode(num) { }
/**
 * @param {number} num
 * @returns {boolean} 
 */
function glIsTexture(num) { }
/**
 * @param {number} num
 * @returns {boolean} 
 */
function glIsEnabled(num) { }
/**
 * @param {number} num
 * @returns {boolean} 
 */
function glIsList(num) { }
/**
 * @param {number|number[]} num
 */
function glTexCoord1(num) { }
/**
 * @param {number} num
 * @returns {number[]} 
 */
function glGetPixelMap(num) { }

// 2 param
/**
 * @param {number} num1
 * @param {number} num2
 */
function glDeleteLists(num1, num2) { }
/**
 * @param {number} num1
 * @param {number} num2
 */
function glNewList(num1, num2) { }
/**
 * @param {number} num1
 * @param {number} num2
 */
function glBindTexture(num1, num2) { }
/**
 * @param {number} num1
 * @param {number} num2
 */
function glPolygonOffset(num1, num2) { }
/**
 * @param {number} num1
 * @param {number} num2
 */
function glAlphaFunc(num1, num2) { }
/**
 * @param {number} num1
 * @param {number} num2
 */
function glBlendFunc(num1, num2) { }
/**
 * @param {number} num1
 * @param {number} num2
 */
function glPolygonMode(num1, num2) { }
/**
 * @param {number} num1
 * @param {number} num2
 */
function glHint(num1, num2) { }
/**
 * @param {number} num1
 * @param {number} num2
 */
function glLineStipple(num1, num2) { }
/**
 * @param {number} num1
 * @param {number} num2
 */
function glAccum(num1, num2) { }
/**
 * @param {number} num1
 * @param {number} num2
 */
function glDepthRange(num1, num2) { }
/**
 * @param {number} num1
 * @param {number} num2
 */
function glColorMaterial(num1, num2) { }
/**
 * @param {number} num1
 * @param {number} num2
 */
function glPixelZoom(num1, num2) { }
/**
 * @param {number} num1
 * @param {number} num2
 */
function glPixelStore(num1, num2) { }
/**
 * @param {number} num1
 * @param {number} num2
 */
function glPixelTransfer(num1, num2) { }
/**
 * @param {number} num1
 * @param {number} num2
 * @returns {number[]}
 */
function glGetLight(num1, num2) { }
/**
 * @param {number} num1
 * @param {number} num2
 * @returns {number[]}
 */
function glGetMaterial(num1, num2) { }
/**
 * @param {number} num1
 * @param {number} num2
 * @returns {number[]}
 */
function glGetTexParameter(num1, num2) { }
/**
 * @param {number} num1
 * @param {number} num2
 * @returns {number[]}
 */
function glGetTexEnv(num1, num2) { }
/**
 * @param {number} num1
 * @param {number} num2
 * @returns {number[]}
 */
function glGetTexGen(num1, num2) { }
/**
 * @param {number} num1
 * @param {number[]} num2
 */
function glClipPlane(num1, num2) { }
/**
 * @param {number|number[]} num1
 * @param {number} [num2]
 */
function glVertex2(num1, num2) { }
/**
 * @param {number|number[]} num1
 * @param {number} [num2]
 */
function glTexCoord2(num1, num2) { }
/**
 * @param {number|number[]} num1
 * @param {number} [num2]
 */
function glRasterPos2(num1, num2) { }
/**
* @param {number} num1
* @param {number[]} num2
*/
function glPixelMap(num1, num2) { }
/**
* @param {number} num1
* @param {number|number[]} num2
*/
function glFog(num1, num2) { }
/**
 * @param {number} num1
 * @param {number} num2
 * @returns {Bitmap}
 */
function glGetTexImage(num1, num2) { }

// 3 param
/**
 * @param {number} num1
 * @param {number} num2
 * @param {number} num3
 */
function glutWireSphere(num1, num2, num3) { }
/**
 * @param {number} num1
 * @param {number} num2
 * @param {number} num3
 */
function glutSolidSphere(num1, num2, num3) { }
/**
 * @param {number} num1
 * @param {number} num2
 * @param {number} num3
 */
function glTranslate(num1, num2, num3) { }
/**
 * @param {number} num1
 * @param {number} num2
 * @param {number} num3
 */
function glScale(num1, num2, num3) { }
/**
 * @param {number} num1
 * @param {number} num2
 * @param {number} num3
 */
function glStencilFunc(num1, num2, num3) { }
/**
 * @param {number} num1
 * @param {number} num2
 * @param {number} num3
 */
function glStencilOp(num1, num2, num3) { }
/**
 * @param {number} num1
 * @param {number} num2
 * @param {number} num3
 * @returns {number[]}
 */
function glGetTexLevelParameter(num1, num2, num3) { }
/**
 * @param {number|number[]} num1
 * @param {number} [num2]
 * @param {number} [num3]
 */
function glTexCoord3(num1, num2, num3) { }
/**
 * @param {number|number[]} num1
 * @param {number} [num2]
 * @param {number} [num3]
 */
function glColor3(num1, num2, num3) { }
/**
 * @param {number|number[]} num1
 * @param {number} [num2]
 * @param {number} [num3]
 */
function glVertex3(num1, num2, num3) { }
/**
 * @param {number|number[]} num1
 * @param {number} [num2]
 * @param {number} [num3]
 */
function glNormal3(num1, num2, num3) { }
/**
 * @param {number|number[]} num1
 * @param {number} [num2]
 * @param {number} [num3]
 */
function glRasterPos3(num1, num2, num3) { }
/**
 * @param {number} num1
 * @param {number} num2
 * @param {number|number[]} num3
 */
function glMaterial(num1, num2, num3) { }
/**
 * @param {number} num1
 * @param {number} num2
 * @param {number|number[]} num3
 */
function glLight(num1, num2, num3) { }
/**
 * @param {number} num1
 * @param {number} num2
 * @param {number|number[]} num3
 */
function glLightModel(num1, num2, num3) { }
/**
 * @param {number} num1
 * @param {number} num2
 * @param {number|number[]} num3
 */
function glTexParameter(num1, num2, num3) { }
/**
 * @param {number} num1
 * @param {number} num2
 * @param {number|number[]} num3
 */
function glTexEnv(num1, num2, num3) { }
/**
 * @param {number} num1
 * @param {number} num2
 * @param {number|number[]} num3
 */
function glTexGen(num1, num2, num3) { }

// 4 param
/**
 * @param {number} num1
 * @param {number} num2
 * @param {number} num3
 * @param {number} num4
 */
function glRotate(num1, num2, num3, num4) { }
/**
 * @param {number} num1
 * @param {number} num2
 * @param {number} num3
 * @param {number} num4
 */
function glClearColor(num1, num2, num3, num4) { }
/**
 * @param {number} num1
 * @param {number} num2
 * @param {number} num3
 * @param {number} num4
 */
function glRect(num1, num2, num3, num4) { }
/**
 * @param {number} num1
 * @param {number} num2
 * @param {number} num3
 * @param {number} num4
 */
function glViewport(num1, num2, num3, num4) { }
/**
 * @param {number} num1
 * @param {number} num2
 * @param {number} num3
 * @param {number} num4
 */
function glScissor(num1, num2, num3, num4) { }
/**
 * @param {number} num1
 * @param {number} num2
 * @param {number} num3
 * @param {number} num4
 */
function glClearAccum(num1, num2, num3, num4) { }
/**
 * @param {number} num1
 * @param {number} num2
 * @param {number} num3
 * @param {number} num4
 */
function glutWireTorus(num1, num2, num3, num4) { }
/**
 * @param {number} num1
 * @param {number} num2
 * @param {number} num3
 * @param {number} num4
 */
function glutSolidTorus(num1, num2, num3, num4) { }
/**
 * @param {number} num1
 * @param {number} num2
 * @param {number} num3
 * @param {number} num4
 */
function glutWireCone(num1, num2, num3, num4) { }
/**
 * @param {number} num1
 * @param {number} num2
 * @param {number} num3
 * @param {number} num4
 */
function glutSolidCone(num1, num2, num3, num4) { }
/**
 * @param {number} num1
 * @param {number} num2
 * @param {number} num3
 * @param {number} num4
 */
function gluPerspective(num1, num2, num3, num4) { }
/**
 * @param {number} num1
 * @param {number} num2
 * @param {number} num3
 * @param {number} num4
 */
function gluOrtho2D(num1, num2, num3, num4) { }
/**
 * @param {number} b1
 * @param {number} b2
 * @param {number} b3
 * @param {number} b4
 */
function glColorMask(b1, b2, b3, b4) { }
/**
 * @param {number|number[]} num1
 * @param {number} [num2]
 * @param {number} [num3]
 * @param {number} [num4]
 */
function glTexCoord4(num1, num2, num3, num4) { }
/**
 * @param {number|number[]} num1
 * @param {number} [num2]
 * @param {number} [num3]
 * @param {number} [num4]
 */
function glVertex4(num1, num2, num3, num4) { }
/**
 * @param {number|number[]} num1
 * @param {number} [num2]
 * @param {number} [num3]
 * @param {number} [num4]
 */
function glColor4(num1, num2, num3, num4) { }
/**
 * @param {number|number[]} num1
 * @param {number} [num2]
 * @param {number} [num3]
 * @param {number} [num4]
 */
function glRasterPos4(num1, num2, num3, num4) { }
/**
 * @param {number} num1
 * @param {number} num2
 * @param {number} num3
 * @param {Bitmap} bm
 */
function glTexImage2D(num1, num2, num3, bm) { }
/**
 * @param {number} num1
 * @param {number} num2
 * @param {number} num3
 * @param {Bitmap} bm
 */
function glTexImage1D(num1, num2, num3, bm) { }
/**
 * @param {number} num1
 * @param {number} num2
 * @param {number} num3
 * @param {number} num4
 * @returns {Bitmap}
 */
function glReadPixels(num1, num2, num3, num4) { }

// 5 param
/**
 * @param {number} num1
 * @param {number} num2
 * @param {number} num3
 * @param {number} num4
 * @param {number} num5
 */
function glCopyPixels(num1, num2, num3, num4, num5) { }

// 6 param
/**
 * @param {number} num1
 * @param {number} num2
 * @param {number} num3
 * @param {number} num4
 * @param {number} num5
 * @param {number} num6
 */
function glFrustum(num1, num2, num3, num4, num5, num6) { }
/**
 * @param {number} num1
 * @param {number} num2
 * @param {number} num3
 * @param {number} num4
 * @param {number} num5
 * @param {number} num6
 */
function glOrtho(num1, num2, num3, num4, num5, num6) { }

// 7 param
/**
 * 
 * @param {number} num1 
 * @param {number} num2 
 * @param {number} num3 
 * @param {number} num4 
 * @param {number} num5 
 * @param {number} num6 
 * @param {number[]} num7 
 */
function glBitmap(num1, num2, num3, num4, num5, num6, num7) { }

// 9 param
/**
 * @param {number} num1
 * @param {number} num2
 * @param {number} num3
 * @param {number} num4
 * @param {number} num5
 * @param {number} num6
 * @param {number} num7
 * @param {number} num8
 * @param {number} num9
 */
function gluLookAt(num1, num2, num3, num4, num5, num6, num7, num8, num9) { }
