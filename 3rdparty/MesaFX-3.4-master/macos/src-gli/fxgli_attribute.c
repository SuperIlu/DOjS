/*
	File:		fxgli_attribute.c

	Contains:	Implementation of gliSet/Get Attribute.

	Written by:	miklos

	Copyright:	Copyright ©

	Change History (most recent first):

         <3>      7/4/99    miklos  Revision.
*/


#include "fxgli.h"

/*
** Attribute operations
*/
GLboolean gliGetAttribute(GLIContext ctx, GLenum type, void *attrib)
{
	(void)ctx;
	(void)type;
	(void)attrib;
 	DebugStr("\pMesa3DfxEngine -gliGetAttribute not implemented");
    return GL_FALSE;
}
GLboolean gliSetAttribute(GLIContext ctx, GLenum type, const void *attrib)
{
	(void)ctx;
	(void)type;
	(void)attrib;
 	DebugStr("\pMesa3DfxEngine -gliGetAttribute not implemented");
    return GL_FALSE;
}
GLboolean gliCopyAttributes(GLIContext ctx, GLenum type, const void *attrib)
{
	(void)ctx;
	(void)type;
	(void)attrib;
 	DebugStr("\pMesa3DfxEngine -gliGetAttribute not implemented");
    return GL_FALSE;
}