/*
 * Mesa 3-D graphics library - MacOS port.
 * Version:  3.1
 * 
 * Copyright (C) 1999  Miklos Fazekas   All Rights Reserved.
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
 * MIKLOS FAZEKAS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
 * AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
 
/*
 * AGL Constants for internal usage...
 */

#ifndef __AGL_MESA_INTERNAL_H__
#define __AGL_MESA_INTERNAL_H__

#include "aglMesa.h"

/*
** This defines the special interface between Mesa+tweaker.
*/
#define AGL_MESA_3DFX_SET_CONFIGURATION_FILE	0x1101D

typedef struct AGLMesaConfigurationFile {
	int lines_num;
	char **lines;
} AGLMesaConfigurationFile;

/*
 * Software Mesa Rendere ID: unofficial, for internal use only....
 */
#define AGL_RENDERER_MESA_SOFTWARE_ID          			AGL_RENDERER_MESA_3DFX_ID+3

#endif /* __AGL_MESA_INTERNAL_H__ */