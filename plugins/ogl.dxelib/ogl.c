/*
MIT License

Copyright (c) 2019-2021 Andre Seidelt <superilu@yahoo.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "bitmap.h"
#include "ogl.h"

#if LINUX != 1
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>

#include <GL/fxmesa.h>
#else
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#endif

#define OGL_BBP 16  //!< bit depth of 3dfx screen

//! extract floats from parameter 'idx' that is an array into 'name', maximum of 'mx' entries
#define OGL_EXTRACT_PARAMS_F(name, idx, mx)       \
    GLfloat name[mx] = {0};                       \
    {                                             \
        int len = js_getlength(J, idx);           \
        for (int i = 0; i < len && i < mx; i++) { \
            js_getindex(J, idx, i);               \
            name[i] = js_tonumber(J, -1);         \
            js_pop(J, 1);                         \
        }                                         \
    }

//! extract n parameters into a float array if param1 is an array, else extract n descreete parameters
#define OGL_EXTRACT_PARAMS(n)                                         \
    float f[n] = {0};                                                 \
    if (js_isarray(J, 1)) {                                           \
        if (js_getlength(J, 1) >= n) {                                \
            for (int i = 0; i < n; i++) {                             \
                js_getindex(J, 1, i);                                 \
                f[i] = js_tonumber(J, -1);                            \
                js_pop(J, 1);                                         \
            }                                                         \
        } else {                                                      \
            js_error(J, "Array must contain at least %d values!", n); \
            return;                                                   \
        }                                                             \
    } else {                                                          \
        for (int i = 0; i < n; i++) {                                 \
            f[i] = js_tonumber(J, i + 1);                             \
        }                                                             \
    }

#define OGL_ARRAY_SET(dt, sz, idx, name, func)                                \
    dt name[sz];                                                              \
    if (!js_isarray(J, idx) || (js_getlength(J, idx) < sz)) {                 \
        js_error(J, "parameter must be an array of at least %d numbers", sz); \
        return;                                                               \
    }                                                                         \
    for (int i = 0; i < sz; i++) {                                            \
        js_getindex(J, i, 1);                                                 \
        name[i] = func(J, -1);                                                \
        js_pop(J, 1);                                                         \
    }

#define OGL_ARRAY_GET(dt, sz, func, push) \
    dt data[sz] = {0};                    \
    func;                                 \
    js_newarray(J);                       \
    for (int i = 0; i < sz; i++) {        \
        push(J, data[i]);                 \
        js_setindex(J, -2, i);            \
    }

//! check for GL error and return to JS with an Error
#define OGL_ERROR()                                \
    {                                              \
        GLenum err = glGetError();                 \
        if (err != GL_NO_ERROR) {                  \
            js_error(J, "GL state error %d", err); \
            return;                                \
        }                                          \
    }

//! OpenGL context from the used library
#if LINUX != 1
static fxMesaContext fc = NULL;
#else
static GLFWwindow *window;
#endif

void init_ogl(js_State *J);
void shutdown_ogl(void);

/*********************
** static functions **
*********************/

//////
// 0 param
static void f_glInit(js_State *J) {
#if LINUX != 1
    GLint attribs[32];

    attribs[0] = FXMESA_DOUBLEBUFFER;
    attribs[1] = FXMESA_ALPHA_SIZE;
    attribs[2] = 1;
    attribs[3] = FXMESA_DEPTH_SIZE;
    attribs[4] = 1;
    attribs[5] = FXMESA_NONE;

    fc = fxMesaCreateContext(0, GR_RESOLUTION_640x480, GR_REFRESH_60Hz, attribs);
    if (!fc) {
        js_error(J, "MESA: No context");
        return;
    }
    fxMesaMakeCurrent(fc);

    LOG("3dfx Voodoo based OpenGL module initialized\n");
    LOGF("  OpenGL Vendor    : %s\n", glGetString(GL_VENDOR));
    LOGF("  OpenGL Renderer  : %s\n", glGetString(GL_RENDERER));
    LOGF("  OpenGL Version   : %s\n", glGetString(GL_VERSION));
    LOGF("  OpenGL Extensions: %s\n", glGetString(GL_EXTENSIONS));
    LOGF("  GLU Version      : %s\n", gluGetString(GLU_VERSION));
    LOGF("  GLU Extensions   : %s\n", gluGetString(GLU_EXTENSIONS));
#else
    glewExperimental = true;  // Needed for core profile
    if (!glfwInit()) {
        js_error(J, "Failed to initialize GLFW");
        return;
    }

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

    // Open a window and create its OpenGL context
    window = glfwCreateWindow(640, 480, "DOjS GL", NULL, NULL);
    if (window == NULL) {
        js_error(J, "Failed to open GLFW window.");
        glfwTerminate();
        return;
    }
    glfwMakeContextCurrent(window);  // Initialize GLEW
    glewExperimental = true;         // Needed in core profile
    if (glewInit() != GLEW_OK) {
        js_error(J, "Failed to initialize GLEW");
        return;
    }
#endif
}

static void f_glShutdown(js_State *J) { shutdown_ogl(); }

static void f_glFlush(js_State *J) {
    glFlush();

#if LINUX != 1
    fxMesaSwapBuffers();
#else
    glfwSwapBuffers(window);
#endif
}

static void f_glEnd(js_State *J) { glEnd(); }
static void f_glLoadIdentity(js_State *J) { glLoadIdentity(); }
static void f_glPushMatrix(js_State *J) { glPushMatrix(); }
static void f_glPopMatrix(js_State *J) { glPopMatrix(); }
static void f_glEndList(js_State *J) { glEndList(); }
static void f_glGetError(js_State *J) { js_pushnumber(J, glGetError()); }
static void f_glFinish(js_State *J) { glFinish(); }
static void f_glPopAttrib(js_State *J) { glPopAttrib(); }
static void f_glPopClientAttrib(js_State *J) { glPopClientAttrib(); }

static void f_glGetPolygonStipple(js_State *J) { OGL_ARRAY_GET(GLubyte, 128, glGetPolygonStipple(data), js_pushnumber); }

static void f_glutWireDodecahedron(js_State *J) { glutWireDodecahedron(); }
static void f_glutSolidDodecahedron(js_State *J) { glutSolidDodecahedron(); }
static void f_glutWireOctahedron(js_State *J) { glutWireOctahedron(); }
static void f_glutSolidOctahedron(js_State *J) { glutSolidOctahedron(); }
static void f_glutWireTetrahedron(js_State *J) { glutWireTetrahedron(); }
static void f_glutSolidTetrahedron(js_State *J) { glutSolidTetrahedron(); }
static void f_glutWireIcosahedron(js_State *J) { glutWireIcosahedron(); }
static void f_glutSolidIcosahedron(js_State *J) { glutSolidIcosahedron(); }

//////
// 1 param
static void f_glBegin(js_State *J) { glBegin(js_toint32(J, 1)); }
static void f_glCallList(js_State *J) { glCallList(js_touint32(J, 1)); }
static void f_glClear(js_State *J) { glClear(js_toint32(J, 1)); }
static void f_glCullFace(js_State *J) { glCullFace(js_toint32(J, 1)); }
static void f_glDisable(js_State *J) { glDisable(js_toint32(J, 1)); }
static void f_glEnable(js_State *J) { glEnable(js_toint32(J, 1)); }
static void f_glGenLists(js_State *J) { js_pushnumber(J, glGenLists(js_touint32(J, 1))); }
static void f_glGetString(js_State *J) { js_pushstring(J, (char *)glGetString(js_toint32(J, 1))); }
static void f_glIndex(js_State *J) { glIndexf(js_tonumber(J, 1)); }
static void f_glIsEnabled(js_State *J) { js_pushboolean(J, glIsEnabled(js_toint32(J, 1))); }
static void f_glIsList(js_State *J) { js_pushboolean(J, glIsList(js_touint32(J, 1))); }
static void f_glListBase(js_State *J) { glListBase(js_touint32(J, 1)); }
static void f_glMatrixMode(js_State *J) { glMatrixMode(js_toint32(J, 1)); }
static void f_glRenderMode(js_State *J) { js_pushnumber(J, glRenderMode(js_touint32(J, 1))); }
static void f_glClearIndex(js_State *J) { glClearIndex(js_tonumber(J, 1)); }
static void f_glLineWidth(js_State *J) { glLineWidth(js_tonumber(J, 1)); }
static void f_glPointSize(js_State *J) { glPointSize(js_tonumber(J, 1)); }
static void f_glEdgeFlag(js_State *J) { glEdgeFlag(js_toboolean(J, 1)); }
static void f_glDisableClientState(js_State *J) { glDisableClientState(js_touint32(J, 1)); }
static void f_glDrawBuffer(js_State *J) { glDrawBuffer(js_touint32(J, 1)); }
static void f_glEnableClientState(js_State *J) { glEnableClientState(js_touint32(J, 1)); }
static void f_glIndexMask(js_State *J) { glIndexMask(js_touint32(J, 1)); }
static void f_glFrontFace(js_State *J) { glFrontFace(js_touint32(J, 1)); }
static void f_glLogicOp(js_State *J) { glLogicOp(js_touint32(J, 1)); }
static void f_glPushAttrib(js_State *J) { glPushAttrib(js_touint32(J, 1)); }
static void f_glPushClientAttrib(js_State *J) { glPushClientAttrib(js_touint32(J, 1)); }
static void f_glReadBuffer(js_State *J) { glReadBuffer(js_touint32(J, 1)); }
static void f_glGetClipPlane(js_State *J) { OGL_ARRAY_GET(GLdouble, 4, glGetClipPlane(js_toint32(J, 1), data), js_pushnumber); }
static void f_glGetBoolean(js_State *J) { OGL_ARRAY_GET(GLboolean, 16, glGetBooleanv(js_toint32(J, 1), data), js_pushboolean); }
static void f_glGetDouble(js_State *J) { OGL_ARRAY_GET(GLdouble, 16, glGetDoublev(js_toint32(J, 1), data), js_pushnumber); }
static void f_glGetFloat(js_State *J) { OGL_ARRAY_GET(GLfloat, 16, glGetFloatv(js_toint32(J, 1), data), js_pushnumber); }
static void f_glGetInteger(js_State *J) { OGL_ARRAY_GET(GLint, 16, glGetIntegerv(js_toint32(J, 1), data), js_pushnumber); }
static void f_glClearDepth(js_State *J) { glClearDepth(js_tonumber(J, 1)); }
static void f_glDepthFunc(js_State *J) { glDepthFunc(js_touint32(J, 1)); }
static void f_glDepthMask(js_State *J) { glDepthMask(js_toboolean(J, 1)); }
static void f_glShadeModel(js_State *J) { glShadeModel(js_touint32(J, 1)); }
static void f_glStencilMask(js_State *J) { glStencilMask(js_touint32(J, 1)); }
static void f_glClearStencil(js_State *J) { glClearStencil(js_toint32(J, 1)); }

static void f_glLoadMatrix(js_State *J) {
    OGL_ARRAY_SET(GLdouble, 16, 1, m, js_tonumber);
    glLoadMatrixd(m);
}

static void f_glMultMatrix(js_State *J) {
    OGL_ARRAY_SET(GLdouble, 16, 1, m, js_tonumber);
    glMultMatrixd(m);
}

static void f_glCallLists(js_State *J) {
    if (js_isarray(J, 1)) {
        // just call each list by hand, no need to make an integer array and call glCallList()
        int n = js_getlength(J, 1);
        for (int i = 0; i < n; i++) {
            js_getindex(J, i, 1);
            glCallList(js_tonumber(J, -1));
            js_pop(J, 1);
        }
    } else {
        JS_ENOARR(J);
    }
}

static void f_glGenTextures(js_State *J) {
    // get number of textures wanted
    int num = js_toint32(J, 1);
    if (num < 1) {
        js_error(J, "Number of textures must be >=1");
        return;
    }

    // allocate mem for return value
    GLuint *textures = calloc(num, sizeof(GLuint));
    if (!textures) {
        JS_ENOMEM(J);
        return;
    }

    // generate texture
    glGenTextures(num, textures);

    // put values into JS array
    js_newarray(J);
    for (int i = 0; i < num; i++) {
        js_pushnumber(J, textures[i]);
        js_setindex(J, -2, i);
    }

    free(textures);
}

static void f_glTexCoord1(js_State *J) {
    OGL_EXTRACT_PARAMS(1);
    glTexCoord1f(f[0]);
}

static void f_glPolygonStipple(js_State *J) {
    OGL_ARRAY_SET(GLubyte, 128, 1, pattern, js_touint16);
    glPolygonStipple(pattern);
}

static void f_glDrawPixels(js_State *J) {
    if (!js_isuserdata(J, 1, TAG_BITMAP)) {
        js_error(J, "parameter must be a bitmap");
        return;
    }
    BITMAP *bm = js_touserdata(J, 1, TAG_BITMAP);

    // allocate memory
    size_t tx_size = bm->w * bm->h * sizeof(uint32_t);
    void *tx_data = malloc(tx_size);
    if (!tx_data) {
        JS_ENOMEM(J);
        return;
    }

    // convert bitmap into new memory (argb ->rgba)
    int p = 0;
    for (int y = 0; y < bm->h; y++) {
        for (int x = 0; x < bm->w; x++) {
            uint32_t argb = getpixel(bm, x, y);

            ((uint8_t *)tx_data)[p++] = (argb >> 16) & 0xFF;  // R
            ((uint8_t *)tx_data)[p++] = (argb >> 8) & 0xFF;   // G
            ((uint8_t *)tx_data)[p++] = (argb >> 0) & 0xFF;   // B
            ((uint8_t *)tx_data)[p++] = (argb >> 24) & 0xFF;  // A
        }
    }
    glDrawPixels(bm->w, bm->h, GL_RGBA, GL_UNSIGNED_BYTE, tx_data);
    free(tx_data);
}

static void f_glutWireCube(js_State *J) { glutWireCube(js_tonumber(J, 1)); }
static void f_glutSolidCube(js_State *J) { glutSolidCube(js_tonumber(J, 1)); }

static void f_glIsTexture(js_State *J) { js_pushboolean(J, glIsTexture(js_toint32(J, 1))); }

static void f_glDeleteTextures(js_State *J) {
    if (js_isarray(J, 1)) {
        GLsizei n = js_getlength(J, 1);
        GLuint *textures = calloc(n, sizeof(GLuint));
        if (!textures) {
            JS_ENOMEM(J);
            return;
        }
        for (int i = 0; i < n; i++) {
            js_getindex(J, 1, i);
            textures[i] = js_touint32(J, -1);
            js_pop(J, 1);
        }
        glDeleteTextures(n, textures);
        free(textures);
    } else {
        GLuint tex = js_touint32(J, 1);
        glDeleteTextures(1, &tex);
    }
}

static void f_glGetPixelMap(js_State *J) {
    GLfloat *values;
    GLenum map = js_touint32(J, 1);
    GLint mapSize;
    switch (map) {
        case GL_PIXEL_MAP_S_TO_S:
            glGetIntegerv(GL_PIXEL_MAP_S_TO_S_SIZE, &mapSize);
            break;
        case GL_PIXEL_MAP_I_TO_I:
            glGetIntegerv(GL_PIXEL_MAP_I_TO_I_SIZE, &mapSize);
            break;
        case GL_PIXEL_MAP_I_TO_R:
            glGetIntegerv(GL_PIXEL_MAP_I_TO_R_SIZE, &mapSize);
            break;
        case GL_PIXEL_MAP_I_TO_G:
            glGetIntegerv(GL_PIXEL_MAP_I_TO_G_SIZE, &mapSize);
            break;
        case GL_PIXEL_MAP_I_TO_B:
            glGetIntegerv(GL_PIXEL_MAP_I_TO_B_SIZE, &mapSize);
            break;
        case GL_PIXEL_MAP_I_TO_A:
            glGetIntegerv(GL_PIXEL_MAP_I_TO_A_SIZE, &mapSize);
            break;
        case GL_PIXEL_MAP_R_TO_R:
            glGetIntegerv(GL_PIXEL_MAP_R_TO_R_SIZE, &mapSize);
            break;
        case GL_PIXEL_MAP_G_TO_G:
            glGetIntegerv(GL_PIXEL_MAP_G_TO_G_SIZE, &mapSize);
            break;
        case GL_PIXEL_MAP_B_TO_B:
            glGetIntegerv(GL_PIXEL_MAP_B_TO_B_SIZE, &mapSize);
            break;
        case GL_PIXEL_MAP_A_TO_A:
            glGetIntegerv(GL_PIXEL_MAP_A_TO_A_SIZE, &mapSize);
            break;
        default:
            js_error(J, "unkown pixel map: %d", map);
            return;
    }

    values = calloc(mapSize, sizeof(GLfloat));
    if (!values) {
        JS_ENOMEM(J);
        return;
    }

    glGetPixelMapfv(map, values);

    // put values into JS array
    js_newarray(J);
    for (int i = 0; i < mapSize; i++) {
        js_pushnumber(J, values[i]);
        js_setindex(J, -2, i);
    }

    free(values);
}

//////
// 2 param
static void f_glDeleteLists(js_State *J) { glDeleteLists(js_touint32(J, 1), js_touint32(J, 2)); }
static void f_glNewList(js_State *J) { glNewList(js_touint32(J, 1), js_touint32(J, 2)); }
static void f_glBindTexture(js_State *J) { glBindTexture(js_touint32(J, 1), js_touint32(J, 2)); }
static void f_glPolygonOffset(js_State *J) { glPolygonOffset(js_tonumber(J, 1), js_tonumber(J, 2)); }
static void f_glAlphaFunc(js_State *J) { glAlphaFunc(js_touint32(J, 1), js_tonumber(J, 2)); }
static void f_glBlendFunc(js_State *J) { glBlendFunc(js_touint32(J, 1), js_touint32(J, 2)); }
static void f_glPolygonMode(js_State *J) { glPolygonMode(js_touint32(J, 1), js_touint32(J, 2)); }
static void f_glHint(js_State *J) { glHint(js_touint32(J, 1), js_touint32(J, 2)); }
static void f_glLineStipple(js_State *J) { glLineStipple(js_toint32(J, 1), js_touint16(J, 2)); }
static void f_glAccum(js_State *J) { glAccum(js_touint32(J, 1), js_tonumber(J, 2)); }
static void f_glDepthRange(js_State *J) { glDepthRange(js_tonumber(J, 1), js_tonumber(J, 2)); }
static void f_glColorMaterial(js_State *J) { glColorMaterial(js_touint32(J, 1), js_touint32(J, 2)); }
static void f_glGetLight(js_State *J) { OGL_ARRAY_GET(GLfloat, 4, glGetLightfv(js_toint32(J, 1), js_touint32(J, 2), data), js_pushnumber); }
static void f_glGetMaterial(js_State *J) { OGL_ARRAY_GET(GLfloat, 4, glGetMaterialfv(js_toint32(J, 1), js_touint32(J, 2), data), js_pushnumber); }
static void f_glGetTexParameter(js_State *J) { OGL_ARRAY_GET(GLfloat, 4, glGetTexParameterfv(js_touint32(J, 1), js_touint32(J, 2), data), js_pushnumber); }
static void f_glGetTexEnv(js_State *J) { OGL_ARRAY_GET(GLfloat, 4, glGetTexEnvfv(js_touint32(J, 1), js_touint32(J, 2), data), js_pushnumber); }
static void f_glGetTexGen(js_State *J) { OGL_ARRAY_GET(GLfloat, 4, glGetTexGenfv(js_touint32(J, 1), js_touint32(J, 2), data), js_pushnumber); }
static void f_glPixelZoom(js_State *J) { glPixelZoom(js_tonumber(J, 1), js_tonumber(J, 2)); }
static void f_glPixelStore(js_State *J) { glPixelStoref(js_touint32(J, 1), js_tonumber(J, 2)); }
static void f_glPixelTransfer(js_State *J) { glPixelTransferf(js_touint32(J, 1), js_tonumber(J, 2)); }
static void f_glClipPlane(js_State *J) {
    OGL_ARRAY_SET(GLdouble, 4, 2, plane, js_tonumber);
    glClipPlane(js_toint32(J, 1), plane);
}

static void f_glVertex2(js_State *J) {
    OGL_EXTRACT_PARAMS(2);
    glVertex2f(f[0], f[1]);
}

static void f_glTexCoord2(js_State *J) {
    OGL_EXTRACT_PARAMS(2);
    glTexCoord2f(f[0], f[1]);
}

static void f_glRasterPos2(js_State *J) {
    OGL_EXTRACT_PARAMS(2);
    glRasterPos2f(f[0], f[1]);
}

static void f_glPixelMap(js_State *J) {
    if (!js_isarray(J, 2)) {
        JS_ENOARR(J);
        return;
    }

    GLfloat map = js_tonumber(J, 1);
    int mapsize = js_getlength(J, 2);
    GLfloat *values = calloc(mapsize, sizeof(GLfloat));
    for (int i = 0; i < mapsize; i++) {
        js_getindex(J, i, 2);
        values[i] = js_tonumber(J, -1);
        js_pop(J, 1);
    }

    glPixelMapfv(map, mapsize, values);
    free(values);
}

static void f_glFog(js_State *J) {
    if (js_isarray(J, 2)) {
        OGL_EXTRACT_PARAMS_F(params, 2, 4);
        glFogfv(js_touint32(J, 1), params);
    } else {
        glFogf(js_touint32(J, 1), js_tonumber(J, 2));
    }
}

static void f_glGetTexImage(js_State *J) {
    GLint width;
    GLint height;

    GLenum target = js_touint32(J, 1);
    GLint level = js_toint32(J, 2);

    glGetTexLevelParameteriv(target, level, GL_TEXTURE_WIDTH, &width);
    OGL_ERROR();
    glGetTexLevelParameteriv(target, level, GL_TEXTURE_HEIGHT, &height);
    OGL_ERROR();

    uint8_t *pixels = malloc(width * height * 4 * sizeof(uint8_t));
    if (!pixels) {
        JS_ENOMEM(J);
    }

    glGetTexImage(target, level, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    Bitmap_fromRGBA(J, pixels, width, height);
}

//////
// 3 param
static void f_glTranslate(js_State *J) { glTranslatef(js_tonumber(J, 1), js_tonumber(J, 2), js_tonumber(J, 3)); }
static void f_glScale(js_State *J) { glScalef(js_tonumber(J, 1), js_tonumber(J, 2), js_tonumber(J, 3)); }
static void f_glStencilFunc(js_State *J) { glStencilFunc(js_touint32(J, 1), js_toint32(J, 2), js_touint32(J, 3)); }
static void f_glStencilOp(js_State *J) { glStencilOp(js_touint32(J, 1), js_touint32(J, 2), js_touint32(J, 3)); }
static void f_glGetTexLevelParameter(js_State *J) {
    OGL_ARRAY_GET(GLfloat, 1, glGetTexLevelParameterfv(js_touint32(J, 1), js_toint32(J, 2), js_touint32(J, 3), data), js_pushnumber);
}

static void f_glTexCoord3(js_State *J) {
    OGL_EXTRACT_PARAMS(3);
    glTexCoord3f(f[0], f[1], f[2]);
}

static void f_glColor3(js_State *J) {
    OGL_EXTRACT_PARAMS(3);
    glColor3f(f[0], f[1], f[2]);
}

static void f_glVertex3(js_State *J) {
    OGL_EXTRACT_PARAMS(3);
    glVertex3f(f[0], f[1], f[2]);
}

static void f_glNormal3(js_State *J) {
    OGL_EXTRACT_PARAMS(3);
    glNormal3f(f[0], f[1], f[2]);
}

static void f_glRasterPos3(js_State *J) {
    OGL_EXTRACT_PARAMS(3);
    glRasterPos3f(f[0], f[1], f[2]);
}

static void f_glMaterial(js_State *J) {
    if (js_isarray(J, 3)) {
        OGL_EXTRACT_PARAMS_F(params, 3, 4);
        glMaterialfv(js_touint32(J, 1), js_touint32(J, 2), params);
    } else {
        glMaterialf(js_touint32(J, 1), js_touint32(J, 2), js_tonumber(J, 3));
    }
}

static void f_glLight(js_State *J) {
    if (js_isarray(J, 3)) {
        OGL_EXTRACT_PARAMS_F(params, 3, 4);
        glLightfv(js_touint32(J, 1), js_touint32(J, 2), params);
    } else {
        glLightf(js_touint32(J, 1), js_touint32(J, 2), js_tonumber(J, 3));
    }
}

static void f_glLightModel(js_State *J) {
    if (js_isarray(J, 2)) {
        OGL_EXTRACT_PARAMS_F(params, 2, 4);
        glLightModelfv(js_touint32(J, 1), params);
    } else {
        glLightModelf(js_touint32(J, 1), js_tonumber(J, 3));
    }
}

static void f_glTexParameter(js_State *J) {
    if (js_isarray(J, 3)) {
        OGL_EXTRACT_PARAMS_F(params, 3, 4);
        glTexParameterfv(js_touint32(J, 1), js_touint32(J, 2), params);
    } else {
        glTexParameterf(js_touint32(J, 1), js_touint32(J, 2), js_tonumber(J, 3));
    }
}

static void f_glTexEnv(js_State *J) {
    if (js_isarray(J, 3)) {
        OGL_EXTRACT_PARAMS_F(params, 3, 4);
        glTexEnvfv(js_touint32(J, 1), js_touint32(J, 2), params);
    } else {
        glTexEnvf(js_touint32(J, 1), js_touint32(J, 2), js_tonumber(J, 3));
    }
}

static void f_glTexGen(js_State *J) {
    if (js_isarray(J, 3)) {
        OGL_EXTRACT_PARAMS_F(params, 3, 4);
        glTexGenfv(js_touint32(J, 1), js_touint32(J, 2), params);
    } else {
        glTexGenf(js_touint32(J, 1), js_touint32(J, 2), js_tonumber(J, 3));
    }
}

static void f_glutWireSphere(js_State *J) { glutWireSphere(js_tonumber(J, 1), js_toint32(J, 2), js_toint32(J, 3)); }
static void f_glutSolidSphere(js_State *J) { glutSolidSphere(js_tonumber(J, 1), js_toint32(J, 2), js_toint32(J, 3)); }

//////
// 4 param
static void f_glRotate(js_State *J) { glRotatef(js_tonumber(J, 1), js_tonumber(J, 2), js_tonumber(J, 3), js_tonumber(J, 4)); }
static void f_glClearColor(js_State *J) { glClearColor(js_tonumber(J, 1), js_tonumber(J, 2), js_tonumber(J, 3), js_tonumber(J, 4)); }
static void f_glRect(js_State *J) { glRectf(js_tonumber(J, 1), js_tonumber(J, 2), js_tonumber(J, 3), js_tonumber(J, 4)); }
static void f_glViewport(js_State *J) { glViewport(js_toint32(J, 1), js_toint32(J, 2), js_toint32(J, 3), js_toint32(J, 4)); }
static void f_glColorMask(js_State *J) { glColorMask(js_toboolean(J, 1), js_toboolean(J, 2), js_toboolean(J, 3), js_toboolean(J, 4)); }
static void f_glScissor(js_State *J) { glScissor(js_toint32(J, 1), js_toint32(J, 2), js_toint32(J, 3), js_toint32(J, 4)); }
static void f_glClearAccum(js_State *J) { glClearAccum(js_tonumber(J, 1), js_tonumber(J, 2), js_tonumber(J, 3), js_tonumber(J, 4)); }

static void f_glTexCoord4(js_State *J) {
    OGL_EXTRACT_PARAMS(4);
    glTexCoord4f(f[0], f[1], f[2], f[3]);
}

static void f_glVertex4(js_State *J) {
    OGL_EXTRACT_PARAMS(4);
    glVertex4f(f[0], f[1], f[2], f[3]);
}

static void f_glColor4(js_State *J) {
    OGL_EXTRACT_PARAMS(4);
    glColor4f(f[0], f[1], f[2], f[3]);
}

static void f_glRasterPos4(js_State *J) {
    OGL_EXTRACT_PARAMS(4);
    glRasterPos4f(f[0], f[1], f[2], f[3]);
}

static void f_glTexImage2D(js_State *J) {
    GLint lvl = js_toint32(J, 1);
    GLint intform = js_toint32(J, 2);
    GLint border = js_toint32(J, 3);

    if (!js_isuserdata(J, 4, TAG_BITMAP)) {
        js_error(J, "parameter must be a bitmap");
        return;
    }
    BITMAP *bm = js_touserdata(J, 4, TAG_BITMAP);

    // allocate memory
    size_t tx_size = bm->w * bm->h * sizeof(uint32_t);
    void *tx_data = malloc(tx_size);
    if (!tx_data) {
        JS_ENOMEM(J);
        return;
    }

    // convert bitmap into new memory (argb ->rgba)
    int p = 0;
    for (int y = 0; y < bm->h; y++) {
        for (int x = 0; x < bm->w; x++) {
            uint32_t argb = getpixel(bm, x, y);

            ((uint8_t *)tx_data)[p++] = (argb >> 16) & 0xFF;  // R
            ((uint8_t *)tx_data)[p++] = (argb >> 8) & 0xFF;   // G
            ((uint8_t *)tx_data)[p++] = (argb >> 0) & 0xFF;   // B
            ((uint8_t *)tx_data)[p++] = (argb >> 24) & 0xFF;  // A
        }
    }
    glTexImage2D(GL_TEXTURE_2D, lvl, intform, bm->w, bm->h, border, GL_RGBA, GL_UNSIGNED_BYTE, tx_data);
    free(tx_data);
}

static void f_glTexImage1D(js_State *J) {
    GLint lvl = js_toint32(J, 1);
    GLint intform = js_toint32(J, 2);
    GLint border = js_toint32(J, 3);

    if (!js_isuserdata(J, 4, TAG_BITMAP)) {
        js_error(J, "parameter must be an bitmap");
        return;
    }
    BITMAP *bm = js_touserdata(J, 4, TAG_BITMAP);

    if (bm->w > 1 && bm->h > 1) {
        js_error(J, "one dimension of the bitmap must be 1");
        return;
    }

    // allocate memory
    size_t tx_size = bm->w * bm->h * sizeof(uint32_t);
    void *tx_data = malloc(tx_size);
    if (!tx_data) {
        JS_ENOMEM(J);
        return;
    }

    // convert bitmap into new memory (argb ->rgba)
    int p = 0;
    for (int y = 0; y < bm->h; y++) {
        for (int x = 0; x < bm->w; x++) {
            uint32_t argb = getpixel(bm, x, y);

            ((uint8_t *)tx_data)[p++] = (argb >> 16) & 0xFF;  // R
            ((uint8_t *)tx_data)[p++] = (argb >> 8) & 0xFF;   // G
            ((uint8_t *)tx_data)[p++] = (argb >> 0) & 0xFF;   // B
            ((uint8_t *)tx_data)[p++] = (argb >> 24) & 0xFF;  // A
        }
    }

    glTexImage1D(GL_TEXTURE_1D, lvl, intform, bm->w * bm->h, border, GL_RGBA, GL_UNSIGNED_BYTE, tx_data);
    free(tx_data);
}

static void f_glReadPixels(js_State *J) {
    GLint x = js_touint32(J, 1);
    GLint y = js_touint32(J, 2);
    GLint w = js_touint32(J, 3);
    GLint h = js_touint32(J, 4);

    size_t pixels_size = w * h * sizeof(uint32_t);
    uint8_t *pixels = malloc(pixels_size);
    if (!pixels) {
        JS_ENOMEM(J);
        return;
    }

    glReadPixels(x, y, w, h, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

    size_t pos = 0;
    size_t i = 0;
    js_newarray(J);
    while (pos < pixels_size) {
        uint8_t r = pixels[pos++];
        uint8_t g = pixels[pos++];
        uint8_t b = pixels[pos++];
        uint8_t a = pixels[pos++];

        js_pushnumber(J, makeacol32(r, g, b, a));
        js_setindex(J, -2, i);
        i++;
    }

    free(pixels);
}

static void f_glutWireTorus(js_State *J) { glutWireTorus(js_tonumber(J, 1), js_tonumber(J, 2), js_toint32(J, 3), js_toint32(J, 4)); }
static void f_glutSolidTorus(js_State *J) { glutSolidTorus(js_tonumber(J, 1), js_tonumber(J, 2), js_toint32(J, 3), js_toint32(J, 4)); }
static void f_glutWireCone(js_State *J) { glutWireCone(js_tonumber(J, 1), js_tonumber(J, 2), js_toint32(J, 3), js_toint32(J, 4)); }
static void f_glutSolidCone(js_State *J) { glutSolidCone(js_tonumber(J, 1), js_tonumber(J, 2), js_toint32(J, 3), js_toint32(J, 4)); }
static void f_gluPerspective(js_State *J) { gluPerspective(js_tonumber(J, 1), js_tonumber(J, 2), js_tonumber(J, 3), js_tonumber(J, 4)); }
static void f_gluOrtho2D(js_State *J) { gluOrtho2D(js_tonumber(J, 1), js_tonumber(J, 2), js_tonumber(J, 3), js_tonumber(J, 4)); }

//////
// 5 param
static void f_glCopyPixels(js_State *J) { glCopyPixels(js_touint32(J, 1), js_touint32(J, 2), js_touint32(J, 3), js_touint32(J, 4), js_touint32(J, 5)); }

//////
// 6 param
static void f_glFrustum(js_State *J) { glFrustum(js_tonumber(J, 1), js_tonumber(J, 2), js_tonumber(J, 3), js_tonumber(J, 4), js_tonumber(J, 5), js_tonumber(J, 6)); }
static void f_glOrtho(js_State *J) { glOrtho(js_tonumber(J, 1), js_tonumber(J, 2), js_tonumber(J, 3), js_tonumber(J, 4), js_tonumber(J, 5), js_tonumber(J, 6)); }

//////
// 7 param
static void f_glBitmap(js_State *J) {
    if (!js_isarray(J, 7)) {
        JS_ENOARR(J);
        return;
    }

    GLsizei w = js_touint32(J, 1);
    GLsizei h = js_touint32(J, 2);
    GLfloat xorig = js_tonumber(J, 3);
    GLfloat yorig = js_tonumber(J, 4);
    GLfloat xmove = js_tonumber(J, 5);
    GLfloat ymove = js_tonumber(J, 6);

    if ((w < 1) || (h < 1)) {
        js_error(J, "width and height must be >=1");
        return;
    }

    // calculate size and check array
    size_t bytes_per_row = (w + 8 - 1) / 8;
    size_t bytes_for_image = bytes_per_row * h;
    if (js_getlength(J, 7) < bytes_for_image) {
        js_error(J, "Image array must have at least %ld entries", bytes_for_image);
        return;
    }

    // get buffer for bitmap data
    uint8_t *data = malloc(bytes_for_image);
    if (!data) {
        JS_ENOMEM(J);
        return;
    }

    // get bitmap data
    for (int i = 0; i < bytes_for_image; i++) {
        js_getindex(J, 7, i);
        data[i] = js_touint16(J, -1);
        js_pop(J, 1);
    }

    glBitmap(w, h, xorig, yorig, xmove, ymove, data);
    free(data);
}

//////
// 9 param
static void f_gluLookAt(js_State *J) {
    gluLookAt(js_tonumber(J, 1), js_tonumber(J, 2), js_tonumber(J, 3), js_tonumber(J, 4), js_tonumber(J, 5), js_tonumber(J, 6), js_tonumber(J, 7), js_tonumber(J, 8),
              js_tonumber(J, 9));
}

/**
 * @brief initialize OpenGL (dummy).
 *
 * @param J VM state.
 */
static void f_dummy_glInit(js_State *J) {
    js_error(J, "GLIDE3X.DXE missing, please run one of the V_x.BAT scripts to get the driver matching your hardware! All OpenGL functions are disabled!");
}

/***********************
** exported functions **
***********************/
/**
 * @brief initialize grx subsystem.
 *
 * @param J VM state.
 * @param argc number of parameters.
 * @param argv command line parameters.
 * @param args first script parameter.
 */
void init_ogl(js_State *J) {
    DEBUGF("%s\n", __PRETTY_FUNCTION__);

#if LINUX != 1
    FILE *f = fopen("GLIDE3X.DXE", "r");
    if (!f) {
        LOG("GLIDE3X.DXE missing, please run one of the V_x.BAT scripts to get the driver matching your hardware! All OpenGL functions are disabled!\n");

        js_newcfunction(J, f_dummy_glInit, "glInit", 0);
        js_setglobal(J, "glInit");
    } else {
        fclose(f);
        // define global functions
#endif
        // 0 param
        NFUNCDEF(J, glInit, 0);
        NFUNCDEF(J, glShutdown, 0);
        NFUNCDEF(J, glFlush, 0);
        NFUNCDEF(J, glFinish, 0);
        NFUNCDEF(J, glEnd, 0);
        NFUNCDEF(J, glLoadIdentity, 0);
        NFUNCDEF(J, glPushMatrix, 0);
        NFUNCDEF(J, glPopMatrix, 0);
        NFUNCDEF(J, glEndList, 0);
        NFUNCDEF(J, glGetError, 0);
        NFUNCDEF(J, glPopAttrib, 0);
        NFUNCDEF(J, glPopClientAttrib, 0);
        NFUNCDEF(J, glGetPolygonStipple, 0);
        NFUNCDEF(J, glutWireDodecahedron, 0);
        NFUNCDEF(J, glutSolidDodecahedron, 0);
        NFUNCDEF(J, glutWireOctahedron, 0);
        NFUNCDEF(J, glutSolidOctahedron, 0);
        NFUNCDEF(J, glutWireTetrahedron, 0);
        NFUNCDEF(J, glutSolidTetrahedron, 0);
        NFUNCDEF(J, glutWireIcosahedron, 0);
        NFUNCDEF(J, glutSolidIcosahedron, 0);

        // 1 param
        NFUNCDEF(J, glClear, 1);
        NFUNCDEF(J, glBegin, 1);
        NFUNCDEF(J, glEnable, 1);
        NFUNCDEF(J, glDisable, 1);
        NFUNCDEF(J, glCullFace, 1);
        NFUNCDEF(J, glMatrixMode, 1);
        NFUNCDEF(J, glIsEnabled, 1);
        NFUNCDEF(J, glDrawBuffer, 1);
        NFUNCDEF(J, glReadBuffer, 1);
        NFUNCDEF(J, glGenLists, 1);
        NFUNCDEF(J, glIsList, 1);
        NFUNCDEF(J, glCallList, 1);
        NFUNCDEF(J, glCallLists, 1);
        NFUNCDEF(J, glListBase, 1);
        NFUNCDEF(J, glGetString, 1);
        NFUNCDEF(J, glGenTextures, 1);
        NFUNCDEF(J, glIndex, 1);
        NFUNCDEF(J, glTexCoord1, 1);
        NFUNCDEF(J, glRenderMode, 1);
        NFUNCDEF(J, glPolygonStipple, 1);
        NFUNCDEF(J, glClearIndex, 1);
        NFUNCDEF(J, glDisableClientState, 1);
        NFUNCDEF(J, glEdgeFlag, 1);
        NFUNCDEF(J, glEnableClientState, 1);
        NFUNCDEF(J, glFrontFace, 1);
        NFUNCDEF(J, glIndexMask, 1);
        NFUNCDEF(J, glLineWidth, 1);
        NFUNCDEF(J, glLogicOp, 1);
        NFUNCDEF(J, glPointSize, 1);
        NFUNCDEF(J, glPushAttrib, 1);
        NFUNCDEF(J, glPushClientAttrib, 1);
        NFUNCDEF(J, glLoadMatrix, 1);
        NFUNCDEF(J, glMultMatrix, 1);
        NFUNCDEF(J, glShadeModel, 1);
        NFUNCDEF(J, glStencilMask, 1);
        NFUNCDEF(J, glClearStencil, 1);
        NFUNCDEF(J, glClearDepth, 1);
        NFUNCDEF(J, glDepthFunc, 1);
        NFUNCDEF(J, glDepthMask, 1);
        NFUNCDEF(J, glDrawPixels, 1);
        NFUNCDEF(J, glutWireCube, 1);
        NFUNCDEF(J, glutSolidCube, 1);
        NFUNCDEF(J, glDeleteTextures, 1);
        NFUNCDEF(J, glIsTexture, 1);
        NFUNCDEF(J, glGetPixelMap, 1);

        // 2 param
        NFUNCDEF(J, glVertex2, 2);
        NFUNCDEF(J, glDeleteLists, 2);
        NFUNCDEF(J, glNewList, 2);
        NFUNCDEF(J, glBindTexture, 2);
        NFUNCDEF(J, glTexCoord2, 2);
        NFUNCDEF(J, glRasterPos2, 2);
        NFUNCDEF(J, glPolygonOffset, 2);
        NFUNCDEF(J, glAlphaFunc, 2);
        NFUNCDEF(J, glBlendFunc, 2);
        NFUNCDEF(J, glPolygonMode, 2);
        NFUNCDEF(J, glHint, 2);
        NFUNCDEF(J, glLineStipple, 2);
        NFUNCDEF(J, glClipPlane, 2);
        NFUNCDEF(J, glGetClipPlane, 2);
        NFUNCDEF(J, glGetBoolean, 2);
        NFUNCDEF(J, glGetDouble, 2);
        NFUNCDEF(J, glGetFloat, 2);
        NFUNCDEF(J, glGetInteger, 2);
        NFUNCDEF(J, glAccum, 2);
        NFUNCDEF(J, glDepthRange, 2);
        NFUNCDEF(J, glColorMaterial, 2);
        NFUNCDEF(J, glGetLight, 2);
        NFUNCDEF(J, glGetMaterial, 2);
        NFUNCDEF(J, glGetTexParameter, 2);
        NFUNCDEF(J, glGetTexEnv, 2);
        NFUNCDEF(J, glGetTexGen, 2);
        NFUNCDEF(J, glPixelZoom, 2);
        NFUNCDEF(J, glPixelStore, 2);
        NFUNCDEF(J, glPixelTransfer, 2);
        NFUNCDEF(J, glPixelMap, 2);
        NFUNCDEF(J, glFog, 2);
        NFUNCDEF(J, glGetTexImage, 2);

        // 3 param
        NFUNCDEF(J, glColor3, 3);
        NFUNCDEF(J, glVertex3, 3);
        NFUNCDEF(J, glTranslate, 3);
        NFUNCDEF(J, glScale, 3);
        NFUNCDEF(J, glMaterial, 3);
        NFUNCDEF(J, glLight, 3);
        NFUNCDEF(J, glLightModel, 3);
        NFUNCDEF(J, glTexParameter, 3);
        NFUNCDEF(J, glNormal3, 3);
        NFUNCDEF(J, glTexCoord3, 3);
        NFUNCDEF(J, glRasterPos3, 3);
        NFUNCDEF(J, glStencilFunc, 3);
        NFUNCDEF(J, glStencilOp, 3);
        NFUNCDEF(J, glTexImage1D, 3);
        NFUNCDEF(J, glTexEnv, 3);
        NFUNCDEF(J, glTexGen, 3);
        NFUNCDEF(J, glGetTexLevelParameter, 3);
        NFUNCDEF(J, glutWireSphere, 3);
        NFUNCDEF(J, glutSolidSphere, 3);

        //////
        // 4 param
        NFUNCDEF(J, glColor4, 4);
        NFUNCDEF(J, glVertex4, 4);
        NFUNCDEF(J, glRotate, 4);
        NFUNCDEF(J, glClearColor, 4);
        NFUNCDEF(J, glRect, 4);
        NFUNCDEF(J, glViewport, 4);
        NFUNCDEF(J, glTexImage2D, 4);
        NFUNCDEF(J, glTexCoord4, 4);
        NFUNCDEF(J, glRasterPos4, 4);
        NFUNCDEF(J, glColorMask, 4);
        NFUNCDEF(J, glScissor, 4);
        NFUNCDEF(J, glClearAccum, 4);
        NFUNCDEF(J, glReadPixels, 4);
        NFUNCDEF(J, glutWireTorus, 4);
        NFUNCDEF(J, glutSolidTorus, 4);
        NFUNCDEF(J, glutWireCone, 4);
        NFUNCDEF(J, glutSolidCone, 4);
        NFUNCDEF(J, gluPerspective, 4);
        NFUNCDEF(J, gluOrtho2D, 4);

        //////
        // 6 param
        NFUNCDEF(J, glCopyPixels, 5);

        //////
        // 6 param
        NFUNCDEF(J, glFrustum, 6);
        NFUNCDEF(J, glOrtho, 6);

        //////
        // 7 param
        NFUNCDEF(J, glBitmap, 7);

        //////
        // 9 param
        NFUNCDEF(J, gluLookAt, 9);

        // opengl constants
        ogl_create_constants(J);
#if LINUX != 1
    }
#endif

    DEBUGF("%s DONE\n", __PRETTY_FUNCTION__);
}

void shutdown_ogl() {
#if LINUX != 1
    if (fc) {
        fxMesaDestroyContext(fc);
        fc = NULL;
    }
#else
    glfwTerminate();
#endif
}

// TODO:

// /*
//  * Vertex Arrays  (1.1)
//  */

// GLAPI void GLAPIENTRY glVertexPointer(GLint size, GLenum type, GLsizei stride, const GLvoid *ptr);

// GLAPI void GLAPIENTRY glNormalPointer(GLenum type, GLsizei stride, const GLvoid *ptr);

// GLAPI void GLAPIENTRY glColorPointer(GLint size, GLenum type, GLsizei stride, const GLvoid *ptr);

// GLAPI void GLAPIENTRY glIndexPointer(GLenum type, GLsizei stride, const GLvoid *ptr);

// GLAPI void GLAPIENTRY glTexCoordPointer(GLint size, GLenum type, GLsizei stride, const GLvoid *ptr);

// GLAPI void GLAPIENTRY glEdgeFlagPointer(GLsizei stride, const GLvoid *ptr);

// GLAPI void GLAPIENTRY glGetPointerv(GLenum pname, void **params);

// GLAPI void GLAPIENTRY glArrayElement(GLint i);

// GLAPI void GLAPIENTRY glDrawArrays(GLenum mode, GLint first, GLsizei count);

// GLAPI void GLAPIENTRY glDrawElements(GLenum mode, GLsizei count, GLenum type, const GLvoid *indices);

// GLAPI void GLAPIENTRY glInterleavedArrays(GLenum format, GLsizei stride, const GLvoid *pointer);

// /*
//  * Raster functions
//  */

// /*
//  * Texture mapping
//  */

// /* 1.1 functions */

// GLAPI void GLAPIENTRY glPrioritizeTextures(GLsizei n, const GLuint *textures, const GLclampf *priorities);

// GLAPI GLboolean GLAPIENTRY glAreTexturesResident(GLsizei n, const GLuint *textures, GLboolean *residences);

// GLAPI void GLAPIENTRY glTexSubImage1D(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const GLvoid *pixels);

// GLAPI void GLAPIENTRY glTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels);

// GLAPI void GLAPIENTRY glCopyTexImage1D(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLint border);

// GLAPI void GLAPIENTRY glCopyTexImage2D(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border);

// GLAPI void GLAPIENTRY glCopyTexSubImage1D(GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width);

// GLAPI void GLAPIENTRY glCopyTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);

// /*
//  * Evaluators
//  */

// GLAPI void GLAPIENTRY glMap1d(GLenum target, GLdouble u1, GLdouble u2, GLint stride, GLint order, const GLdouble *points);
// GLAPI void GLAPIENTRY glMap1f(GLenum target, GLfloat u1, GLfloat u2, GLint stride, GLint order, const GLfloat *points);

// GLAPI void GLAPIENTRY glMap2d(GLenum target, GLdouble u1, GLdouble u2, GLint ustride, GLint uorder, GLdouble v1, GLdouble v2, GLint vstride, GLint vorder, const GLdouble
// *points); GLAPI void GLAPIENTRY glMap2f(GLenum target, GLfloat u1, GLfloat u2, GLint ustride, GLint uorder, GLfloat v1, GLfloat v2, GLint vstride, GLint vorder, const GLfloat
// *points);

// GLAPI void GLAPIENTRY glGetMapdv(GLenum target, GLenum query, GLdouble *v);
// GLAPI void GLAPIENTRY glGetMapfv(GLenum target, GLenum query, GLfloat *v);
// GLAPI void GLAPIENTRY glGetMapiv(GLenum target, GLenum query, GLint *v);

// GLAPI void GLAPIENTRY glEvalCoord1d(GLdouble u);
// GLAPI void GLAPIENTRY glEvalCoord1f(GLfloat u);

// GLAPI void GLAPIENTRY glEvalCoord1dv(const GLdouble *u);
// GLAPI void GLAPIENTRY glEvalCoord1fv(const GLfloat *u);

// GLAPI void GLAPIENTRY glEvalCoord2d(GLdouble u, GLdouble v);
// GLAPI void GLAPIENTRY glEvalCoord2f(GLfloat u, GLfloat v);

// GLAPI void GLAPIENTRY glEvalCoord2dv(const GLdouble *u);
// GLAPI void GLAPIENTRY glEvalCoord2fv(const GLfloat *u);

// GLAPI void GLAPIENTRY glMapGrid1d(GLint un, GLdouble u1, GLdouble u2);
// GLAPI void GLAPIENTRY glMapGrid1f(GLint un, GLfloat u1, GLfloat u2);

// GLAPI void GLAPIENTRY glMapGrid2d(GLint un, GLdouble u1, GLdouble u2, GLint vn, GLdouble v1, GLdouble v2);
// GLAPI void GLAPIENTRY glMapGrid2f(GLint un, GLfloat u1, GLfloat u2, GLint vn, GLfloat v1, GLfloat v2);

// GLAPI void GLAPIENTRY glEvalPoint1(GLint i);

// GLAPI void GLAPIENTRY glEvalPoint2(GLint i, GLint j);

// GLAPI void GLAPIENTRY glEvalMesh1(GLenum mode, GLint i1, GLint i2);

// GLAPI void GLAPIENTRY glEvalMesh2(GLenum mode, GLint i1, GLint i2, GLint j1, GLint j2);

// /*
//  * Selection and Feedback
//  */

// GLAPI void GLAPIENTRY glFeedbackBuffer(GLsizei size, GLenum type, GLfloat *buffer);

// GLAPI void GLAPIENTRY glPassThrough(GLfloat token);

// GLAPI void GLAPIENTRY glSelectBuffer(GLsizei size, GLuint *buffer);

// GLAPI void GLAPIENTRY glInitNames(void);

// GLAPI void GLAPIENTRY glLoadName(GLuint name);

// GLAPI void GLAPIENTRY glPushName(GLuint name);

// GLAPI void GLAPIENTRY glPopName(void);

// /* 1.2 functions */
// GLAPI void GLAPIENTRY glDrawRangeElements(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid *indices);

// GLAPI void GLAPIENTRY glTexImage3D(GLenum target, GLint level, GLint internalFormat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type,
//                                    const GLvoid *pixels);

// GLAPI void GLAPIENTRY glTexSubImage3D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format,
//                                       GLenum type, const GLvoid *pixels);

// GLAPI void GLAPIENTRY glCopyTexSubImage3D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height);

// /* 1.2 imaging extension functions */

// GLAPI void GLAPIENTRY glColorTable(GLenum target, GLenum internalformat, GLsizei width, GLenum format, GLenum type, const GLvoid *table);

// GLAPI void GLAPIENTRY glColorSubTable(GLenum target, GLsizei start, GLsizei count, GLenum format, GLenum type, const GLvoid *data);

// GLAPI void GLAPIENTRY glColorTableParameteriv(GLenum target, GLenum pname, const GLint *params);

// GLAPI void GLAPIENTRY glColorTableParameterfv(GLenum target, GLenum pname, const GLfloat *params);

// GLAPI void GLAPIENTRY glCopyColorSubTable(GLenum target, GLsizei start, GLint x, GLint y, GLsizei width);

// GLAPI void GLAPIENTRY glCopyColorTable(GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width);

// GLAPI void GLAPIENTRY glGetColorTable(GLenum target, GLenum format, GLenum type, GLvoid *table);

// GLAPI void GLAPIENTRY glGetColorTableParameterfv(GLenum target, GLenum pname, GLfloat *params);

// GLAPI void GLAPIENTRY glGetColorTableParameteriv(GLenum target, GLenum pname, GLint *params);

// GLAPI void GLAPIENTRY glBlendEquation(GLenum mode);

// GLAPI void GLAPIENTRY glBlendColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);

// GLAPI void GLAPIENTRY glHistogram(GLenum target, GLsizei width, GLenum internalformat, GLboolean sink);

// GLAPI void GLAPIENTRY glResetHistogram(GLenum target);

// GLAPI void GLAPIENTRY glGetHistogram(GLenum target, GLboolean reset, GLenum format, GLenum type, GLvoid *values);

// GLAPI void GLAPIENTRY glGetHistogramParameterfv(GLenum target, GLenum pname, GLfloat *params);

// GLAPI void GLAPIENTRY glGetHistogramParameteriv(GLenum target, GLenum pname, GLint *params);

// GLAPI void GLAPIENTRY glMinmax(GLenum target, GLenum internalformat, GLboolean sink);

// GLAPI void GLAPIENTRY glResetMinmax(GLenum target);

// GLAPI void GLAPIENTRY glGetMinmax(GLenum target, GLboolean reset, GLenum format, GLenum types, GLvoid *values);

// GLAPI void GLAPIENTRY glGetMinmaxParameterfv(GLenum target, GLenum pname, GLfloat *params);

// GLAPI void GLAPIENTRY glGetMinmaxParameteriv(GLenum target, GLenum pname, GLint *params);

// GLAPI void GLAPIENTRY glConvolutionFilter1D(GLenum target, GLenum internalformat, GLsizei width, GLenum format, GLenum type, const GLvoid *image);

// GLAPI void GLAPIENTRY glConvolutionFilter2D(GLenum target, GLenum internalformat, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *image);

// GLAPI void GLAPIENTRY glConvolutionParameterf(GLenum target, GLenum pname, GLfloat params);

// GLAPI void GLAPIENTRY glConvolutionParameterfv(GLenum target, GLenum pname, const GLfloat *params);

// GLAPI void GLAPIENTRY glConvolutionParameteri(GLenum target, GLenum pname, GLint params);

// GLAPI void GLAPIENTRY glConvolutionParameteriv(GLenum target, GLenum pname, const GLint *params);

// GLAPI void GLAPIENTRY glCopyConvolutionFilter1D(GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width);

// GLAPI void GLAPIENTRY glCopyConvolutionFilter2D(GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height);

// GLAPI void GLAPIENTRY glGetConvolutionFilter(GLenum target, GLenum format, GLenum type, GLvoid *image);

// GLAPI void GLAPIENTRY glGetConvolutionParameterfv(GLenum target, GLenum pname, GLfloat *params);

// GLAPI void GLAPIENTRY glGetConvolutionParameteriv(GLenum target, GLenum pname, GLint *params);

// GLAPI void GLAPIENTRY glSeparableFilter2D(GLenum target, GLenum internalformat, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *row, const GLvoid
// *column);

// GLAPI void GLAPIENTRY glGetSeparableFilter(GLenum target, GLenum format, GLenum type, GLvoid *row, GLvoid *column, GLvoid *span);
