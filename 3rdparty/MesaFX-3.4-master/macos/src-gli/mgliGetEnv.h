/*
	File:		mgliGetEnv.h

	Contains:	Header file for Mesa's configuration file reader.

	Written by:	Miklos Fazekas

	Copyright:	Copyright(C) 1995-99 Miklos Fazekas
				E-Mail: boga@velerie.inf.elte.hu
				WWW: http://www.mesa3d.org/mac/
				
				Part of the Mesa 3D Graphics Library for MacOS.

	Change History (most recent first):

         <3>      7/5/99    miklos  Revison.
         <2>     4/28/99    miklos  Initial Revision.
*/


#ifndef __MGLI_GETENV_H__
#define __MGLI_GETENV_H__

#include <Files.h>

#define MGLI_CONFIG_FILE_NAME		"MesaSettings"

/********************************************************************
 * Function: MCFG_readMesaConfigurationFile                         *
 *																	*
 * Description: Read's the configuration file from current          *
 * directory's MesaSettings file.                                   *	
 * 																	*
 ********************************************************************/
void MCFG_readMesaConfigurationFile(char *filename);
/********************************************************************
 * Function: MCFG_readMesaConfigurationFileFromFSSpec               *
 *																	*
 * Description: Read's the configuration file from MesaSettings     *	
 * 																	*
 ********************************************************************/
Boolean MCFG_readMesaConfigurationFileFromFSSpec(const FSSpec *fileSpec);
/********************************************************************
 * Function: MCFG_readMesaConfigurationFile                         *
 *																	*
 * Description: Read's the confituration file memory.               *	
 * 																	*
 ********************************************************************/
void MCFG_setMesaConfigurationFile(int line_num,char **lines);
/********************************************************************
 * Function: MCFG_getEnv                         					*
 *																	*
 * Description: Get's the value of a confiuration named name,       *
 *	            returns null if this variable wasn't defined        *	
 * 																	*
 ********************************************************************/
char *MCFG_getEnv(const char *name);
/********************************************************************
 * Function: MCFG_disposeConfigData                         		*
 *																	*
 * Description: Destroys data used by the configurtaion infos.      *	
 * 																	*
 ********************************************************************/
void MCFG_disposeConfigData(void);
#endif /* __MGLI_GETENV_H__ */