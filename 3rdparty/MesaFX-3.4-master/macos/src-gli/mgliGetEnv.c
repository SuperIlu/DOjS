/*
	File:		mgliGetEnv.c

	Contains:	GetEnv for reading environment  variables from a file.

	Written by:	Miklos Fazekas

	Copyright:	Copyright(C) 1995-99 Miklos Fazekas
				E-Mail: boga@velerie.inf.elte.hu
				WWW: http://www.mesa3d.org/mac/
				
				Part of the Mesa 3D Graphics Library for MacOS.

	Change History (most recent first):

        <5+>      7/5/99    miklos  Better error checking.
         <5>      7/5/99    miklos  Revison.
         <4>      5/3/99    miklos  The configuration file is opened for the first getenv call!
         <3>     4/28/99    miklos  Configuration file closed !
         <2>     4/28/99    miklos  Initial Revision.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <MacTypes.h>
#include <Files.h>
/*
#include "mgliGetEnv.h"
#include "mgliMem.h"


#define mgliAlloc 	malloc
#define mgliFree	free
*/



static int config_inited = 0;
static int config_num = 0;
static char (*config_data)[2][255] = NULL;


static int is_comment_line(char *cfgline)
{
	if (!cfgline)
		return 1;
	if ((cfgline[0] == '/') && (cfgline[1] == '/'))
		return 1;
	if (strlen(cfgline) > 200)
		return 1;
	/* If no alphanum char in cfgline it is comment */
	{
		char *c = cfgline;
		while (!isalnum(*c) && (*c))
		{
			c++;
		}
		if ((*c) == 0)
			return 1;
	}
	
	return 0;
}
/***********************************************************************
 *
 * MCFG_setMesaConfigurationFile
 *	Set's the configuration file to an array of lines, it's used by
 *  MesaTweaker!
 *
 ***********************************************************************/
void MCFG_setMesaConfigurationFile(int line_num,char **lines)
{
	int linenum = 0;
	int i;
	
	config_num = 0;
	config_inited = 1;
	
	if (linenum == 0)
	   return;
	
	for (i = 0; i < line_num; i++)
	{
		if (lines[i] && !is_comment_line(lines[i]))
			linenum++;
	}
	
	if (config_inited && config_data)
		free(config_data);
		
	config_data = (void*)malloc(2*255*linenum);
	if (!config_data)
		return;
	
	for (i = 0; i < line_num; i++)
	{
		if (lines[i] && !is_comment_line(lines[i]))
		{
			sscanf(lines[i],"%s",config_data[config_num][0]);
			sscanf(&lines[i][strlen(config_data[config_num][0])],"%s",config_data[config_num][1]);
			config_num++;
		}
	}
}
/********************************************************************
 * Function: MCFG_readMesaConfigurationFileFromFSSpec               *
 *																	*
 * Description: Read's the configuration file from an FSSpec        *	
 * 																	*
 ********************************************************************/
Boolean MCFG_readMesaConfigurationFileFromFSSpec(const FSSpec *fileSpec)
{
	OSErr 	theErr;
	SInt16 	refNum;
	SInt32	fileLength;
	char	*memory;
	
	theErr = FSpOpenDF(fileSpec,fsRdPerm,&refNum);
	if (theErr != noErr)
		return false;
	theErr = GetEOF(refNum, &fileLength);
	if (theErr != noErr)
		return false;
	
	memory = (char*)malloc(fileLength+1);
	
	theErr = SetFPos(refNum, fsFromStart, 0);
	if (theErr != noErr)
		return false;
	
	theErr = FSRead(refNum, &fileLength, memory);
	if (theErr != noErr)
	{
		free(memory);
		return false;
	}
	theErr = FSClose(refNum);
	
	memory[fileLength] = '\r';
	/*
	 * Now decompose to lines:
	 */
	{
		int linebeg = 0;
		int linenum = 0;
		int currentchar = 0;
		char *lines[51];
		for (currentchar = 0; currentchar < fileLength+1; currentchar++)
		{
			if ((memory[currentchar] == '\r') || (memory[currentchar] == '\n'))
			{	
				if ((linenum == 50) || ((currentchar-linebeg) > 200) ||
					!(lines[linenum] = (char*)malloc((currentchar-linebeg)+1)) /* correct! */
					)
				{
					int i;
					free(memory);
					for (i = 0; i < linenum; i++)
						free(lines[i]);
					return false;
				}
				if ((currentchar-linebeg) > 0)
				{ 
					strncpy(lines[linenum],&memory[linebeg],currentchar-linebeg);
					lines[linenum][currentchar-linebeg] = 0;
					linebeg = currentchar+1;
					linenum++;
				}
			}
		}
		MCFG_setMesaConfigurationFile(linenum,lines);
	}
	

	return true;
}

/*
 * This will read the variables from the configuration file!
 */
void MCFG_readMesaConfigurationFile(char *filename)
{
	FILE *f;
	
	if (config_inited)
		return;
	else
		config_inited = 1;
		
	f = fopen(filename,"r");
	config_num = 0;
	
	if (!f)
		return;
	
	{
		/* First go and get the number of items... */
		int linenum = 0;
		char cfgline[255];
		while (fgets(cfgline,255,f))
		{
			if (!is_comment_line(cfgline))
				linenum++;
		}
		
		/* Allocate memory */
		config_data = (void*)malloc(2*255*linenum);
		if (!config_data)
		{
			fclose(f);
			return;
		}
		/* Read the configuration file lines! */
		if (fseek(f,0,SEEK_SET) != 0)
		{
			free((void*)config_data);
			fclose(f);
			return;
		}
		
		while (fgets(cfgline,255,f))
		{
			int i;
			if (!is_comment_line(cfgline))
			{
				i = sscanf(cfgline,"%s",config_data[config_num][0]);
				sscanf(&cfgline[strlen(config_data[config_num][0])],"%s",config_data[config_num][1]);
				config_num++;
			}
		}
		fclose(f);
	}
}


char* MCFG_getEnv(const char *name)
{
	int i;
	
	if (!config_inited)
		return;
	/*
		MCFG_readMesaConfigurationFile();
	*/
	for (i =0; i< config_num; i++)
	{
		if (strcmp(name,config_data[i][0])== 0)
		{
			return config_data[i][1];
		}
	}
	
	return NULL;
}


void MCFG_disposeConfigData(void)
{
	if (config_inited && config_data)
	{
		free((void*)config_data);
		config_inited = 0;
	}
}

