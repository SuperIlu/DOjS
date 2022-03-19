/*
	File:		fxgli_default.c

	Contains:	Routines for reading from the configuration file/

	Written by:	miklos

	Copyright:	Copyright ©

	Change History (most recent first):

         <3>      7/5/99    miklos  Revison.
*/


#include "fxgli.h"
#include <string.h>

int fxgli_GetDefaultTextureMemorySize(void)
{
	int def = 0;
	if (MCFG_getEnv(kEnv_CheatTextureMemory) != NULL)
	{
		int ans = atoi(MCFG_getEnv(kEnv_CheatTextureMemory));
		if (ans > 0)
			def = ans;
	}
	return def;
}
int fxgli_GetDefaultVideoMemorySize(void)
{
	int def = 0;
	if (MCFG_getEnv(kEnv_CheatVideoMemory) != NULL)
	{
		int ans = atoi(MCFG_getEnv(kEnv_CheatVideoMemory));
		if (ans > 0) 
			def = ans;
	}
	return def;
}
int fxgli_GetDefaultDitherMode(void)
{
	int def = -1;
	if (MCFG_getEnv(kEnv_DitherMode) != NULL)
	{
		int ans = atoi(MCFG_getEnv(kEnv_DitherMode));
		if (ans == 0 || ans == 2 || ans == 4)
		{ 
			def = ans;
		}
	}
	return def;
}

int fxgli_GetDefaultScreenRefresh(void)
{
	int def = 0;
	if (MCFG_getEnv(kEnv_ScreenRefresh) != NULL)
	{
		int ans = atoi(MCFG_getEnv(kEnv_ScreenRefresh));
		if (ans > 0)
		{ 
			def = ans;
		}
	}
	return def;	
}

int fxgli_GetDefaultGammaValue(void)
{ 
    float def = 0.0;
    
	if (MCFG_getEnv(kEnv_DefaultGammaValue) != NULL)
	{
		float ans = atof(MCFG_getEnv(kEnv_DefaultGammaValue));
		if ((ans > 0.5) || (ans < 5.0))
			def = ans;
	}
	
	return (int)(65536*def);
}			
int fxgli_GetDefaultSwapIntervall(void)
{
	int def = 1;
	
	if (MCFG_getEnv(kEnv_SwapIntervall))
	{
		int ans = atoi(MCFG_getEnv(kEnv_SwapIntervall));
		if (ans >= 0 || ans <= 2)
			def = ans;
	}
	return def;	
}

int	fxgli_GetDefaultMaxSwapPendingBuffers(void)
{
	int def = 2;
	
	if (MCFG_getEnv("MESA_3DFX_MAX_PENDING_SWAP_BUFFERS"))
	{
		int ans = atoi(MCFG_getEnv("MESA_3DFX_MAX_PENDING_SWAP_BUFFERS"));
		if (ans >= 0 || ans <= 5)
			def = ans;	
	}
	return def;
}

GLboolean fxgli_IsEmulateWindowRendering(void)
{
	if (MCFG_getEnv(kEnv_EmulateWindowRendering))
		return GL_TRUE;
	else
		return GL_FALSE;
}
GLboolean fxgli_IsEmulateCompliance(void)
{
	if (MCFG_getEnv(kEnv_EmulateCompliance))
		return GL_TRUE;
	else
		return GL_FALSE;
}
GLboolean fxgli_IsEmulateCursor(void)
{
	if (MCFG_getEnv(kEnv_EmulateMouseCursor))
		return GL_TRUE;
	else
		return GL_FALSE;
}