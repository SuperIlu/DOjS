/*
 *  File:
 *    stkwalk.h
 *
 *  Remarks:
 *
 *  Note:
 *
 *  Author:
 *    Jochen Kalmbach
 *
 *////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef __STACKWALKER_H__
#define __STACKWALKER_H__

#ifndef _X86_
#error Only INTEL environments are supported!
#endif

enum eAllocCheckOutput {
     ACOutput_Simple = 1,
     ACOutput_Advanced
    };

#ifdef __cplusplus
extern "C" {
#endif

int InitAllocCheckWN (enum eAllocCheckOutput eOutput,
                      LPCTSTR pszFilename,
                      ULONG ulShowStackAtAlloc);

int InitAllocCheck (enum eAllocCheckOutput eOutput,
                    BOOL bSetUnhandledExeptionFilter,
                    ULONG ulShowStackAtAlloc);

ULONG       DeInitAllocCheck (void);
void        OnlyInstallUnhandeldExceptionFilter (enum eAllocCheckOutput eOutput);
void        ShowStack (HANDLE hThread, const CONTEXT *c, const char *pszLogFile);
const char *StackWalkLogFile (void);
void        SetCallStackOutputType (enum eAllocCheckOutput eOutput);

#ifdef __cplusplus
};
#endif

#endif  // __STACKWALKER_H__
