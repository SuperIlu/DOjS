/*!\file stkwalk.cpp
 * Stackwalker (backtrace) for Win32 / MSVC
 */

/*
 * File:
 *    stkwalk.cpp
 *
 * Remarks:
 *    Dumps memory leaks (unreleased allocations) for CRT-Allocs and COM-Allocs
 *    Dumps the stack of an thread if an exepction occurs
 *
 * Known bugs:
 *    - If the allocation-RequestID wrap, then allocations will get lost...
 *
 * Author:
 *    Jochen Kalmbach, Germany
 *    (c) 2002-2004 (Freeware)
 *    http://www.codeproject.com/tools/leakfinder.asp
 *
 * License (The zlib/libpng License, http://www.opensource.org/licenses/zlib-license.php):
 *
 * Copyright (c) 2004 Jochen Kalmbach
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * In no event will the authors be held liable for any damages arising from the
 * use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose, including
 * commercial applications, and to alter it and redistribute it freely, subject to
 * the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not claim
 *    that you wrote the original software. If you use this software in a product,
 *    an acknowledgment in the product documentation would be appreciated but is
 *    not required.
 *
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source distribution.
 *
 */

/*
 * Aug 2011   Adapted for Watt-32 TCP/IP - G. Vanem (gvanem@yahoo.no)
 *            No longer Unicode aware. Simplified.
 *
 * \todo: Make this all C-code to avoid having a dependency on MSVCP100?.DLL.
 */

#include "wattcp.h"

#if defined(USE_STACKWALKER)      /* Rest of file */

#ifndef _CRT_NONSTDC_NO_DEPRECATE
#define _CRT_NONSTDC_NO_DEPRECATE
#endif

#include <string>
#include <vector>

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <crtdbg.h>
#include <tchar.h>

#include "misc.h"
#include "stkwalk.h"


// If the following is defined, only the used memories are stored in the hash-table.
// If the memory is freed, it will be removed from the hash-table (to reduce memory)
// Consequences: At DeInitAllocHook, only Leaks will be reported
#define HASH_ENTRY_REMOVE_AT_FREE

// 0 = Do not write any output during runtime-alloc-call
// 1 = Write only the alloc action (malloc, realloc, free)
// 2 = Write alloc action and callstack only for malloc/realloc
// 3 = Write alloc action and callstack for all actions
static ULONG g_ulShowStackAtAlloc = 0;

// the form of the output file
static enum eAllocCheckOutput g_CallstackOutputType = ACOutput_Simple;

// Size of Hash-Table (this should be a prime number to avoid collisions)
#define ALLOC_HASH_ENTRIES 1023

// Size of Callstack-trace in bytes (0x500 => appr. 5-9 functions, depending on parameter count for each function)
#define MAX_ESP_LEN_BUF 0x500

// Normally we can ignore allocations from the Runtime-System
#define IGNORE_CRT_ALLOC

// MaxSize: 1 MByte (only for StackwalkFilter)
#define LOG_FILE_MAX_SIZE 1024*1024

// #############################################################################################
// Here I have included the API-Version 9 declarations, so it will also compile on systems,
// where the new PSDK is not installed
// Normally we just need to include the "dbghelp.h" file

#undef CMSG_DATA
#include <imagehlp.h>

#if API_VERSION_NUMBER < 9

typedef BOOL (__stdcall *PREAD_PROCESS_MEMORY_ROUTINE64)(
    HANDLE      hProcess,
    DWORD64     qwBaseAddress,
    PVOID       lpBuffer,
    DWORD       nSize,
    LPDWORD     lpNumberOfBytesRead
    );

typedef struct _IMAGEHLP_LINE64 {
    DWORD                       SizeOfStruct;           // set to sizeof(IMAGEHLP_LINE64)
    PVOID                       Key;                    // internal
    DWORD                       LineNumber;             // line number in file
    PCHAR                       FileName;               // full filename
    DWORD64                     Address;                // first instruction of line
} IMAGEHLP_LINE64, *PIMAGEHLP_LINE64;

typedef struct _IMAGEHLP_MODULE64 {
    DWORD                       SizeOfStruct;           // set to sizeof(IMAGEHLP_MODULE64)
    DWORD64                     BaseOfImage;            // base load address of module
    DWORD                       ImageSize;              // virtual size of the loaded module
    DWORD                       TimeDateStamp;          // date/time stamp from pe header
    DWORD                       CheckSum;               // checksum from the pe header
    DWORD                       NumSyms;                // number of symbols in the symbol table
    SYM_TYPE                    SymType;                // type of symbols loaded
    CHAR                        ModuleName[32];         // module name
    CHAR                        ImageName[256];         // image name
    CHAR                        LoadedImageName[256];   // symbol file name
} IMAGEHLP_MODULE64, *PIMAGEHLP_MODULE64;

typedef struct _IMAGEHLP_SYMBOL64 {
    DWORD                       SizeOfStruct;           // set to sizeof(IMAGEHLP_SYMBOL64)
    DWORD64                     Address;                // virtual address including dll base address
    DWORD                       Size;                   // estimated size of symbol, can be zero
    DWORD                       Flags;                  // info about the symbols, see the SYMF defines
    DWORD                       MaxNameLength;          // maximum size of symbol name in 'Name'
    CHAR                        Name[1];                // symbol name (null terminated string)
} IMAGEHLP_SYMBOL64, *PIMAGEHLP_SYMBOL64;

typedef struct _tagADDRESS64 {
    DWORD64       Offset;
    WORD          Segment;
    ADDRESS_MODE  Mode;
} ADDRESS64, *LPADDRESS64;

typedef struct _KDHELP64 {
    //
    // address of kernel thread object, as provided in the
    // WAIT_STATE_CHANGE packet.
    //
    DWORD64   Thread;

    //
    // offset in thread object to pointer to the current callback frame
    // in kernel stack.
    //
    DWORD   ThCallbackStack;

    //
    // offset in thread object to pointer to the current callback backing
    // store frame in kernel stack.
    //
    DWORD   ThCallbackBStore;

    //
    // offsets to values in frame:
    //
    // address of next callback frame
    DWORD   NextCallback;

    // address of saved frame pointer (if applicable)
    DWORD   FramePointer;

    //
    // Address of the kernel function that calls out to user mode
    //
    DWORD64   KiCallUserMode;

    //
    // Address of the user mode dispatcher function
    //
    DWORD64   KeUserCallbackDispatcher;

    //
    // Lowest kernel mode address
    //
    DWORD64   SystemRangeStart;

    DWORD64  Reserved[8];

} KDHELP64, *PKDHELP64;


typedef struct _tagSTACKFRAME64 {
    ADDRESS64   AddrPC;               // program counter
    ADDRESS64   AddrReturn;           // return address
    ADDRESS64   AddrFrame;            // frame pointer
    ADDRESS64   AddrStack;            // stack pointer
    ADDRESS64   AddrBStore;           // backing store pointer
    PVOID       FuncTableEntry;       // pointer to pdata/fpo or NULL
    DWORD64     Params[4];            // possible arguments to the function
    BOOL        Far;                  // WOW far call
    BOOL        Virtual;              // is this a virtual frame?
    DWORD64     Reserved[3];
    KDHELP64    KdHelp;
} STACKFRAME64, *LPSTACKFRAME64;

typedef
PVOID
(__stdcall *PFUNCTION_TABLE_ACCESS_ROUTINE64)(
    HANDLE  hProcess,
    DWORD64 AddrBase
    );

typedef
DWORD64
(__stdcall *PGET_MODULE_BASE_ROUTINE64)(
    HANDLE  hProcess,
    DWORD64 Address
    );

typedef
DWORD64
(__stdcall *PTRANSLATE_ADDRESS_ROUTINE64)(
    HANDLE    hProcess,
    HANDLE    hThread,
    LPADDRESS64 lpaddr
    );
#endif  /* API_VERSION_NUMBER < 9 */

// #############################################################################################

static void ShowStackRM (HANDLE hThread, const CONTEXT  *c, FILE *fLogFile,
                         PREAD_PROCESS_MEMORY_ROUTINE64 ReadMemoryFunction, HANDLE hProcess);
static void ShowStack (HANDLE hThread, const CONTEXT *c, FILE *fLogFile);

static DWORD StackwalkFilter (const EXCEPTION_POINTERS *ep, DWORD status, const char *pszLogFile);

static void AllocHashOut (FILE*);
static ULONG AllocHashOutLeaks (FILE*);

// Globale Vars:
static char *g_pszAllocLogName = NULL;
static FILE *g_fFile = NULL;

// AllocCheckFileOpen
//  Checks if the log-file is already opened
//  if not, try to open file (append or create if not exists)
//  if open failed, redirect output to stdout

static void AllocCheckFileOpen (bool bAppend = true)
{
  // is the File already open? If not open it...
  if (!g_fFile)
    if (g_pszAllocLogName)
    {
      if (bAppend == false)
           g_fFile = fopen(g_pszAllocLogName, "w");
      else g_fFile = fopen(g_pszAllocLogName, "a");
    }
  if (!g_fFile)
     g_fFile = stdout;
}

// Write Date/Time to specified file.
static void WriteDateTime (FILE *fFile)
{
  SYSTEMTIME st;

  if (!fFile)
     return;
  memset (&st, 0, sizeof(st));
  GetLocalTime (&st);
  fprintf (fFile,  "At: %02u-%02u-%u, %02u:%02u:%02u.%u",
           st.wDay, st.wMonth, st.wYear, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
}


/*******************************************************************************
 * Hash-Table
 *******************************************************************************/

// Memory for the EIP-Address (is used by the ShowStack-method)
#define MAX_EIP_LEN_BUF 4

#define ALLOC_ENTRY_NOT_FOUND 0xFFFFFFFF

typedef struct AllocHashEntryType {
  long                       lRequestID;    // RequestID from CRT (if 0, then this entry is empty)
  size_t                     nDataSize;     // Size of the allocated memory
  char                       cRemovedFlag;  // 0 => memory was not yet released
  struct AllocHashEntryType  *Next;
  // Callstack for EIP
  DWORD                      dwEIPOffset;
  DWORD                      dwEIPLen;
  char                       pcEIPAddr[MAX_EIP_LEN_BUF];
  // Callstack for ESP
  DWORD                      dwESPOffset;
  DWORD                      dwESPLen;
  char                       pcESPAddr[MAX_ESP_LEN_BUF];
} AllocHashEntryType;

static AllocHashEntryType AllocHashTable[ALLOC_HASH_ENTRIES];
static ULONG AllocHashEntries = 0;
static ULONG AllocHashCollisions = 0;
static ULONG AllocHashFreed = 0;
static ULONG AllocHashMaxUsed = 0; // maximal number of concurrent entries
static ULONG AllocHashCurrentCount = 0;

static ULONG AllocHashMaxCollisions = 0;
static ULONG AllocHashCurrentCollisions = 0;

static void AllocHashInit (void)
{
  memset(AllocHashTable, 0, sizeof(AllocHashTable));
  AllocHashEntries = 0;
  AllocHashCollisions = 0;
  AllocHashFreed = 0;
  AllocHashCurrentCount = 0;
  AllocHashMaxUsed = 0;
  AllocHashMaxCollisions = 0;
  AllocHashCurrentCollisions = 0;
}

// AllocHashDeinit
// Returns the number of bytes, that are not freed (leaks)
static ULONG AllocHashDeinit (void)
{
  ULONG ulRet = 0;

  AllocCheckFileOpen (TRUE);  // open global log-file

  fprintf (g_fFile, "\n##### Memory Report ########################################\n");
  WriteDateTime (g_fFile);
  fprintf (g_fFile, "\n");

#ifndef HASH_ENTRY_REMOVE_AT_FREE
  // output the used memory
  if (g_CallstackOutputType)
     fprintf (g_fFile, "##### Memory used: #########################################\n");
  AllocHashOut (g_fFile);
#endif

  // output the Memory leaks
  if (g_CallstackOutputType)
     fprintf (g_fFile, "\n##### Leaks: ###############################################\n");
  ulRet = AllocHashOutLeaks (g_fFile);

  if (g_CallstackOutputType == ACOutput_Advanced)
  {
    // output some statistics from the hash-table
    fprintf (g_fFile, "***** Hash-Table statistics:\n");
    fprintf (g_fFile, "      Table-Size:     %i\n", ALLOC_HASH_ENTRIES);
    fprintf (g_fFile, "      Inserts:        %i\n", AllocHashEntries);
    fprintf (g_fFile, "      Freed:          %i\n", AllocHashFreed);
    fprintf (g_fFile, "      Sum Collisions: %i\n", AllocHashCollisions);
    fprintf (g_fFile, "\n");
    fprintf (g_fFile, "      Max used:       %i\n", AllocHashMaxUsed);
    fprintf (g_fFile, "      Max Collisions: %i\n", AllocHashMaxCollisions);
  }

  // Free Hash-Table
  ULONG ulTemp;
  AllocHashEntryType *pHashEntry, *pHashEntryOld;

  // Now, free my own memory
  for(ulTemp = 0; ulTemp < ALLOC_HASH_ENTRIES; ulTemp++) {
    pHashEntry = &AllocHashTable[ulTemp];
    while(pHashEntry != NULL) {
      pHashEntryOld = pHashEntry;
      pHashEntry = pHashEntry->Next;
      if (pHashEntryOld != &AllocHashTable[ulTemp]) {
        // now free the dynamically allocated memory
        free(pHashEntryOld);
      }
    }  // while
  }  // for
  // empty the hash-table
  memset(AllocHashTable, 0, sizeof(AllocHashTable));
  return ulRet;
}

// AllocHashFunction
// The has-function (very simple)
static inline ULONG AllocHashFunction (long lRequestID)
{
  // I couldn´t find any better and faster
  return lRequestID % ALLOC_HASH_ENTRIES;
}

// AllocHashInsert
//   lRequestID: Key-Word (RequestID from AllocHook)
//   pContext:   Context-Record, for retrieving Callstack (EIP and EBP is only needed)
//   nDataSize:  How many bytes
static void AllocHashInsert (long lRequestID, CONTEXT &Context, size_t nDataSize)
{
  ULONG HashIdx;
  AllocHashEntryType *pHashEntry;

  // change statistical data
  AllocHashEntries++;
  AllocHashCurrentCount++;
  if (AllocHashCurrentCount > AllocHashMaxUsed)
    AllocHashMaxUsed = AllocHashCurrentCount;

  // generate hash-value
  HashIdx = AllocHashFunction(lRequestID);

  pHashEntry = &AllocHashTable[HashIdx];
  if (pHashEntry->lRequestID == 0)
  {
    // Entry is empty...
  }
  else {
    // Entry is not empy! make a list of entries for this hash value...
    // change statistical data
    // if this happens often, you should increase the hah size or change the heash-function;
    // to fasten the allocation time
    AllocHashCollisions++;
    AllocHashCurrentCollisions++;
    if (AllocHashCurrentCollisions > AllocHashMaxCollisions)
      AllocHashMaxCollisions = AllocHashCurrentCollisions;

    while (pHashEntry->Next)
    {
      pHashEntry = pHashEntry->Next;
    }
    pHashEntry->Next = (AllocHashEntryType*) calloc(sizeof(AllocHashEntryType), 1);
    pHashEntry = pHashEntry->Next;

  }
  pHashEntry->lRequestID = lRequestID;  // Key-Word
  pHashEntry->nDataSize = nDataSize;
  pHashEntry->Next = NULL;
  // Get EIP and save it in the record
  pHashEntry->dwEIPOffset = Context.Eip;
  if (ReadProcessMemory (GetCurrentProcess(), (LPCVOID)Context.Eip,
                        &pHashEntry->pcEIPAddr, MAX_EIP_LEN_BUF, &pHashEntry->dwEIPLen) == 0)
  {
    // Could not read memory... remove everything...
    memset(pHashEntry->pcEIPAddr, 0, MAX_EIP_LEN_BUF);
    pHashEntry->dwEIPLen = 0;
    pHashEntry->dwEIPOffset = 0;
  }

  // Get ESP and save it in the record
  pHashEntry->dwESPOffset = Context.Ebp;
  if (ReadProcessMemory (GetCurrentProcess(), (LPCVOID)Context.Ebp,
                         &pHashEntry->pcESPAddr, MAX_ESP_LEN_BUF, &pHashEntry->dwESPLen) == 0)
  {
    // Could not read memory... remove everything...
    memset(pHashEntry->pcESPAddr, 0, MAX_ESP_LEN_BUF);
    pHashEntry->dwESPLen = 0;
    pHashEntry->dwESPOffset = 0;

    // Check if I tried to read too much...
    if (GetLastError() == ERROR_PARTIAL_COPY)
    {
      // ask how many I can read:
      MEMORY_BASIC_INFORMATION MemBuffer;
      DWORD dwRet = VirtualQuery((LPCVOID) Context.Ebp, &MemBuffer, sizeof(MemBuffer));
      if (dwRet > 0)
      {
        // calculate the length
        DWORD len = ((DWORD) MemBuffer.BaseAddress + MemBuffer.RegionSize) - Context.Ebp;
        if ( (len > 0) && (len < MAX_ESP_LEN_BUF) )
        {
          // try to read it again (with the shorter length)
          if (ReadProcessMemory (GetCurrentProcess(), (LPCVOID)Context.Ebp,
                                 &pHashEntry->pcESPAddr, len, &pHashEntry->dwESPLen) == 0)
          {
            // ok, now everything goes wrong... remove it...
            memset(pHashEntry->pcESPAddr, 0, MAX_ESP_LEN_BUF);
            pHashEntry->dwESPLen = 0;
            pHashEntry->dwESPOffset = 0;
          }
          else
          {
            pHashEntry->dwESPOffset = Context.Ebp;
          }
        }
      }
    }
  }
}

// AllocHashFind
//   If ALLOC_ENTRY_NOT_FOUND is returned, the Key was not found!
//   If the Key was found, a pointer to the entry is returned
static AllocHashEntryType *AllocHashFind (long lRequestID)
{
  ULONG HashIdx;
  AllocHashEntryType *pHashEntry;

  // get the Hash-Value
  HashIdx = AllocHashFunction(lRequestID);

  // Just do some simple checks:
  _ASSERTE(HashIdx < ALLOC_HASH_ENTRIES);

  pHashEntry = &AllocHashTable[HashIdx];
  while (pHashEntry)
  {
    if (pHashEntry->lRequestID == lRequestID)
       return pHashEntry;
    pHashEntry = pHashEntry->Next;
  }

  return (AllocHashEntryType*) ALLOC_ENTRY_NOT_FOUND;
}

// AllocHashRemove
//   Return: FALSE (0) : Key was found and removed/marked
//           TRUE (!=0): Key was not found
static BOOL AllocHashRemove (long lRequestID)
{
  AllocHashEntryType *pHashEntry, *pHashEntryLast;
  ULONG HashIdx = AllocHashFunction(lRequestID);   // get the Hash-Value

  // Just do some simple checks:
  _ASSERTE(HashIdx < ALLOC_HASH_ENTRIES);

  pHashEntryLast = NULL;
  pHashEntry = &AllocHashTable[HashIdx];

  while (pHashEntry)
  {
    if (pHashEntry->lRequestID == lRequestID) {
#ifdef HASH_ENTRY_REMOVE_AT_FREE
      AllocHashFreed++;
      AllocHashCurrentCount--;
      // release my memory
      if (pHashEntryLast == NULL) {
        // It is an entry in the table, so do not release this memory
        if (pHashEntry->Next == NULL) {
          // It was the last entry, so empty the table entry
          memset(&AllocHashTable[HashIdx], 0, sizeof(AllocHashTable[HashIdx]));
        }
        else {
          // There are some more entries, so shorten the list
          AllocHashEntryType *pTmp = pHashEntry->Next;
          *pHashEntry = *(pHashEntry->Next);
          free(pTmp);
        }
        return TRUE;
      }
      else {
        // now, I am in an dynamic allocated entry
        // it was a collision, so decrease the current collision count
        AllocHashCurrentCollisions--;
        pHashEntryLast->Next = pHashEntry->Next;
        free(pHashEntry);
        return TRUE;
      }
#else
      // increase the Remove-Count and let the objet stay in memory
      pHashEntry->cRemovedFlag++;
      return TRUE;
#endif
    }
    pHashEntryLast = pHashEntry;
    pHashEntry = pHashEntry->Next;
  }

  // if we are here, we could not find the RequestID
  return FALSE;
}

// ReadProcMemoryFromHash
//   Callback-Funtion for StackWalk for my own CallStack from the Hash-Table-Entries
static BOOL __stdcall ReadProcMemoryFromHash (HANDLE hRequestID, DWORD64 lpBaseAddress, PVOID lpBuffer, DWORD nSize, PDWORD lpNumberOfBytesRead) {
  // Try to find the RequestID
  AllocHashEntryType *pHashEntry;
  *lpNumberOfBytesRead = 0;

  pHashEntry = AllocHashFind((LONG) hRequestID);
  if (pHashEntry == (AllocHashEntryType*) ALLOC_ENTRY_NOT_FOUND) {
    // Not found, so I cannot return any memory
    *lpNumberOfBytesRead = 0;
    return FALSE;
  }
  if ( ((DWORD) lpBaseAddress >= pHashEntry->dwESPOffset) && ((DWORD) lpBaseAddress <= (pHashEntry->dwESPOffset+pHashEntry->dwESPLen)) ) {
    // Memory is located in ESP:
    // Calculate the offset
    DWORD dwOffset = (DWORD) lpBaseAddress - pHashEntry->dwESPOffset;
    DWORD dwSize = __min(nSize, MAX_ESP_LEN_BUF-dwOffset);
    memcpy(lpBuffer, &(pHashEntry->pcESPAddr[dwOffset]), dwSize);
    *lpNumberOfBytesRead = dwSize;
    if (dwSize != nSize)
      return FALSE;
  }

  if ( ((DWORD) lpBaseAddress >= pHashEntry->dwEIPOffset) && ((DWORD) lpBaseAddress <= (pHashEntry->dwEIPOffset+pHashEntry->dwEIPLen)) ) {
    // Memory is located in EIP:
    // Calculate the offset
    DWORD dwOffset = (DWORD) lpBaseAddress - pHashEntry->dwEIPOffset;
    DWORD dwSize = __min(nSize, MAX_ESP_LEN_BUF-dwOffset);
    memcpy(lpBuffer, &(pHashEntry->pcEIPAddr[dwOffset]), dwSize);
    *lpNumberOfBytesRead = dwSize;
    if (dwSize != nSize)
      return FALSE;
  }

  if (*lpNumberOfBytesRead == 0)  // Memory could not be found
    return FALSE;

  return TRUE;
}

// AllocHashOutLeaks
// Write all Memory (with callstack) which was not freed yet
//   Returns the number of bytes, that are not freed (leaks)
ULONG AllocHashOutLeaks (FILE *fFile)
{
  ULONG ulTemp;
  AllocHashEntryType *pHashEntry;
  ULONG ulCount = 0;
  ULONG ulLeaksByte = 0;

  // Move throu every entry
  for (ulTemp = 0; ulTemp < ALLOC_HASH_ENTRIES; ulTemp++)
  {
    pHashEntry = &AllocHashTable[ulTemp];
    if (pHashEntry->lRequestID)
    {
      while (pHashEntry)
      {
        if ((pHashEntry->cRemovedFlag <= 0) || (pHashEntry->cRemovedFlag > 1))
        {
          CONTEXT c;

          ulCount++;
          fprintf (fFile, "RequestID: %12i, Removed: %i, Size: %12i\n",
                   pHashEntry->lRequestID, pHashEntry->cRemovedFlag, pHashEntry->nDataSize);
          memset (&c, 0, sizeof(c));
          c.Eip = pHashEntry->dwEIPOffset;
          c.Ebp = pHashEntry->dwESPOffset;
          ShowStackRM (NULL, &c, fFile, &ReadProcMemoryFromHash, (HANDLE)pHashEntry->lRequestID);
          // Count the number of leaky bytes
          if (pHashEntry->nDataSize > 0)
               ulLeaksByte += pHashEntry->nDataSize;
          else ulLeaksByte++;  // If memory was allocated with zero bytes, then just increase the counter 1
        }
        pHashEntry = pHashEntry->Next;
      }
    }
  }
  fprintf (fFile, "\n**** Number of leaks: %i\n", ulCount);
  return (ulLeaksByte);
}

// Write all used memory to a file
static void AllocHashOut (FILE *fFile)
{
  ULONG ulTemp;

  for (ulTemp = 0; ulTemp < ALLOC_HASH_ENTRIES; ulTemp++)
  {
    AllocHashEntryType *pHashEntry = &AllocHashTable[ulTemp];

    if (pHashEntry->lRequestID)
    {
      while (pHashEntry)
      {
        fprintf (fFile, "RequestID: %12i, Removed: %i, Size: %12i\n",
                 pHashEntry->lRequestID, pHashEntry->cRemovedFlag, pHashEntry->nDataSize);
        pHashEntry = pHashEntry->Next;
      }
    }
  }
}

/*******************************************************************************
 * End of Hash-Table
 *******************************************************************************/

// The follwoing is copied from dbgint.h:
// <CRT_INTERNALS>
/*
 * For diagnostic purpose, blocks are allocated with extra information and
 * stored in a doubly-linked list.  This makes all blocks registered with
 * how big they are, when they were allocated, and what they are used for.
 */

#define nNoMansLandSize 4

typedef struct _CrtMemBlockHeader
{
        struct _CrtMemBlockHeader * pBlockHeaderNext;
        struct _CrtMemBlockHeader * pBlockHeaderPrev;
        char *                      szFileName;
        int                         nLine;
#ifdef _WIN64
        /* These items are reversed on Win64 to eliminate gaps in the struct
         * and ensure that sizeof(struct)%16 == 0, so 16-byte alignment is
         * maintained in the debug heap.
         */
        int                         nBlockUse;
        size_t                      nDataSize;
#else  /* _WIN64 */
        size_t                      nDataSize;
        int                         nBlockUse;
#endif  /* _WIN64 */
        long                        lRequest;
        unsigned char               gap[nNoMansLandSize];
        /* followed by:
         *  unsigned char           data[nDataSize];
         *  unsigned char           anotherGap[nNoMansLandSize];
         */
} _CrtMemBlockHeader;
#define pbData(pblock) ((unsigned char *)((_CrtMemBlockHeader *)pblock + 1))
#define pHdr(pbData) (((_CrtMemBlockHeader *)pbData)-1)

// </CRT_INTERNALS>


// Global data:
static BOOL g_bInitialized = FALSE;
static HINSTANCE g_hImagehlpDll = NULL;

static DWORD g_dwShowCount = 0;  // increase at every ShowStack-Call
static CRITICAL_SECTION g_csFileOpenClose = {0};

// Is used for syncronising call to MyAllocHook (to prevent reentrant calls)
static LONG g_lMallocCalled = 0;

static _CRT_ALLOC_HOOK pfnOldCrtAllocHook = NULL;

// Deaktivate AllocHook, by increasing the Syncronisation-Counter
static void DeactivateMallocStackwalker (void)
{
  InterlockedIncrement(&g_lMallocCalled);
}

/*
 * MyAllocHook is Single-Threaded, that means the the calls are serialized in the calling function!
 * Special case for VC 5.
 */
#if defined(_MSC_VER) && (_MSC_VER <= 1100)
  #define CRTSETALLOCHOOK_FILENAME_TYPE const char
#else
  #define CRTSETALLOCHOOK_FILENAME_TYPE const unsigned char
#endif

extern __declspec(dllimport) int _crtDbgFlag;

static int cdecl MyAllocHook (int nAllocType, void *pvData,
                              size_t nSize, int nBlockUse, long lRequest,
                              CRTSETALLOCHOOK_FILENAME_TYPE *szFileName, int nLine)
{
  static const char *operation[] = { "", "ALLOCATIONG", "RE-ALLOCATING", "FREEING" };
  static const char *blockType[] = { "Free", "Normal", "CRT", "Ignore", "Client" };

#ifdef IGNORE_CRT_ALLOC
  if (_BLOCK_TYPE(nBlockUse) == _CRT_BLOCK)  // Ignore internal C runtime library allocations
     return TRUE;
#endif

  if  ( ((_CRTDBG_ALLOC_MEM_DF & _crtDbgFlag) == 0) &&
       ( (nAllocType == _HOOK_ALLOC) || (nAllocType == _HOOK_REALLOC) ) )
  {
    // Someone has disabled that the runtime should log this allocation
    // so we do not log this allocation
    if (pfnOldCrtAllocHook != NULL)
      pfnOldCrtAllocHook(nAllocType, pvData, nSize, nBlockUse, lRequest, szFileName, nLine);
    return TRUE;
  }

  // Prevent from reentrat calls
  if (InterlockedIncrement(&g_lMallocCalled) > 1) { // I was already called
    InterlockedDecrement(&g_lMallocCalled);
    // call the previous alloc hook
    if (pfnOldCrtAllocHook != NULL)
      pfnOldCrtAllocHook(nAllocType, pvData, nSize, nBlockUse, lRequest, szFileName, nLine);
    return TRUE;
  }

  if (g_ulShowStackAtAlloc > 0) {
    AllocCheckFileOpen();  // Open logfile
  }

   _ASSERT( (nAllocType == _HOOK_ALLOC) || (nAllocType == _HOOK_REALLOC) || (nAllocType == _HOOK_FREE) );
   _ASSERT( ( _BLOCK_TYPE(nBlockUse) >= 0 ) && ( _BLOCK_TYPE(nBlockUse) < 5 ) );

  if (nAllocType == _HOOK_FREE) { // freeing
    // Try to get the header information
    if (_CrtIsValidHeapPointer(pvData)) {  // it is a valid Heap-Pointer
      // get the ID
      _CrtMemBlockHeader *pHead;
      // get a pointer to memory block header
      pHead = pHdr(pvData);
      nSize = pHead->nDataSize;
      lRequest = pHead->lRequest; // This is the ID!

      if (pHead->nBlockUse == _IGNORE_BLOCK)
      {
        InterlockedDecrement(&g_lMallocCalled);
        if (pfnOldCrtAllocHook != NULL)
          pfnOldCrtAllocHook(nAllocType, pvData, nSize, nBlockUse, lRequest, szFileName, nLine);
        return TRUE;
      }
    }
  }

  if (g_ulShowStackAtAlloc > 0) {
    fprintf (g_fFile, "##### Memory operation: %s a %d-byte '%s' block (# %ld)",
             operation[nAllocType], nSize, blockType[_BLOCK_TYPE(nBlockUse)], lRequest);
    if (pvData)
       fprintf (g_fFile, " at 0x%X", pvData );
    fprintf (g_fFile, "\n");
  }

  if (nAllocType == _HOOK_FREE) {   // freeing:
    if (lRequest != 0)              // RequestID was found
    {
      BOOL bRet;
      // Try to find the RequestID in the Hash-Table, mark it that it was freed
      bRet = AllocHashRemove(lRequest);
      if(g_ulShowStackAtAlloc > 0) {
        if (bRet == FALSE)
        {
          // RequestID not found!
          fprintf (g_fFile, "###### RequestID not found in hash table for FREEING (%i)!\n", lRequest);
        }
      }
    }
    else
    {
      if(g_ulShowStackAtAlloc > 0)
      {
        // No valid RequestID found, display error
        fprintf (g_fFile, "###### No valid RequestID for FREEING! (0x%X)\n", pvData);
      }
    }
  }

  if (nAllocType == _HOOK_REALLOC)  // re-allocating
  {
    // Try to get the header information
    if (_CrtIsValidHeapPointer(pvData))   // it is a valid Heap-Pointer
    {
      BOOL bRet;
      LONG lReallocRequest;
      _CrtMemBlockHeader *pHead = pHdr (pvData);  // get a pointer to memory block header

      // Try to find the RequestID in the Hash-Table, mark it that it was freed
      lReallocRequest = pHead->lRequest;
      bRet = AllocHashRemove (lReallocRequest);
      if (g_ulShowStackAtAlloc > 0)
      {
        if (bRet == FALSE)
        {
          // RequestID not found!
          fprintf (g_fFile, "###### RequestID not found in hash table for RE-ALLOCATING (%i)!\n", lReallocRequest);
        }
        else
        {
          fprintf (g_fFile, "##### Implicit freeing because of re-allocation (# old: %ld, new: %ld)\n",
                   lReallocRequest, lRequest);
        }
      }
    }
  }

  if ( (g_ulShowStackAtAlloc < 3) && (nAllocType == _HOOK_FREE) )
  {
    InterlockedDecrement(&g_lMallocCalled);
    // call the previous alloc hook
    if (pfnOldCrtAllocHook != NULL)
      pfnOldCrtAllocHook(nAllocType, pvData, nSize, nBlockUse, lRequest, szFileName, nLine);
    return TRUE;
  }

  HANDLE hThread;
  if (DuplicateHandle (GetCurrentProcess(), GetCurrentThread(),
                       GetCurrentProcess(), &hThread, 0, FALSE,
                       DUPLICATE_SAME_ACCESS) == 0)
  {
    // Something was wrong...
    fprintf (g_fFile, "###### Could not call 'DuplicateHandle' successfully\n");
    InterlockedDecrement (&g_lMallocCalled);
    // call the previous alloc hook
    if (pfnOldCrtAllocHook)
       pfnOldCrtAllocHook (nAllocType, pvData, nSize, nBlockUse, lRequest, szFileName, nLine);
    return (TRUE);
  }

  CONTEXT c;
  memset (&c, 0, sizeof(c));
  c.ContextFlags = CONTEXT_FULL;

#if 0
  RtlCaptureContext (&c);  // init CONTEXT record so we know where to start the stackwalk
#else
  __asm                    // Get the context of this thread using inline-asm
  {
      call x
 x:   pop eax
      mov c.Eip, eax
      mov c.Ebp, ebp
  }
#endif

  if(g_ulShowStackAtAlloc > 1)
  {
    if(g_ulShowStackAtAlloc > 2)
    {
      // output the callstack
      ShowStack( hThread, &c, g_fFile);
    }
    else {
      // Output only (re)allocs
      if (nAllocType != _HOOK_FREE) {
        ShowStack( hThread, &c, g_fFile);
      }
    }
  }  // g_ulShowStackAtAlloc > 1
  CloseHandle( hThread );

  // Only isert in the Hash-Table if it is not a "freeing"
  if (nAllocType != _HOOK_FREE) {
    if(lRequest != 0) // Always a valid RequestID should be provided (see comments in the header)
      AllocHashInsert(lRequest, c, nSize);
  }

  InterlockedDecrement(&g_lMallocCalled);
  // call the previous alloc hook
  if (pfnOldCrtAllocHook != NULL)
    pfnOldCrtAllocHook(nAllocType, pvData, nSize, nBlockUse, lRequest, szFileName, nLine);
  return TRUE; // allow the memory operation to proceed
}


// ##########################################################################################
// ##########################################################################################
// ##########################################################################################
// ##########################################################################################

#define gle (GetLastError())
#define MAXNAMELEN 1024 // max name length for found symbols
#define IMGSYMLEN ( sizeof IMAGEHLP_SYMBOL64 )
#define TTBUFLEN 8096 // for a temp buffer (2^13)


// SymCleanup()
typedef BOOL (__stdcall *tSC)( IN HANDLE hProcess );
static tSC pSC = NULL;

// SymFunctionTableAccess64()
typedef PVOID (__stdcall *tSFTA)( HANDLE hProcess, DWORD64 AddrBase );
static tSFTA pSFTA = NULL;

// SymGetLineFromAddr64()
typedef BOOL (__stdcall *tSGLFA)( IN HANDLE hProcess, IN DWORD64 dwAddr,
  OUT PDWORD pdwDisplacement, OUT PIMAGEHLP_LINE64 Line );
static tSGLFA pSGLFA = NULL;

// SymGetModuleBase64()
typedef DWORD64 (__stdcall *tSGMB)( IN HANDLE hProcess, IN DWORD64 dwAddr );
static tSGMB pSGMB = NULL;

// SymGetModuleInfo64()
typedef BOOL (__stdcall *tSGMI)( IN HANDLE hProcess, IN DWORD64 dwAddr, OUT PIMAGEHLP_MODULE64 ModuleInfo );
static tSGMI pSGMI = NULL;

// SymGetOptions()
typedef DWORD (__stdcall *tSGO)( VOID );
static tSGO pSGO = NULL;

// SymGetSymFromAddr64()
typedef BOOL (__stdcall *tSGSFA)( IN HANDLE hProcess, IN DWORD64 dwAddr,
  OUT PDWORD64 pdwDisplacement, OUT PIMAGEHLP_SYMBOL64 Symbol );
static tSGSFA pSGSFA = NULL;

// SymInitialize()
typedef BOOL (__stdcall *tSI)( IN HANDLE hProcess, IN PSTR UserSearchPath, IN BOOL fInvadeProcess );
static tSI pSI = NULL;

// SymLoadModule64()
typedef DWORD (__stdcall *tSLM)( IN HANDLE hProcess, IN HANDLE hFile,
  IN PSTR ImageName, IN PSTR ModuleName, IN DWORD64 BaseOfDll, IN DWORD SizeOfDll );
static tSLM pSLM = NULL;

// SymSetOptions()
typedef DWORD (__stdcall *tSSO)( IN DWORD SymOptions );
static tSSO pSSO = NULL;

// StackWalk64()
typedef BOOL (__stdcall *tSW)(
  DWORD MachineType,
  HANDLE hProcess,
  HANDLE hThread,
  LPSTACKFRAME64 StackFrame,
  PVOID ContextRecord,
  PREAD_PROCESS_MEMORY_ROUTINE64 ReadMemoryRoutine,
  PFUNCTION_TABLE_ACCESS_ROUTINE64 FunctionTableAccessRoutine,
  PGET_MODULE_BASE_ROUTINE64 GetModuleBaseRoutine,
  PTRANSLATE_ADDRESS_ROUTINE64 TranslateAddress );

static tSW pSW = NULL;

// UnDecorateSymbolName()
typedef DWORD (__stdcall WINAPI *tUDSN)( PCSTR DecoratedName, PSTR UnDecoratedName,
  DWORD UndecoratedLength, DWORD Flags );

static tUDSN pUDSN = NULL;

struct ModuleEntry
{
  std::string imageName;
  std::string moduleName;
  DWORD baseAddress;
  DWORD size;
};
typedef std::vector< ModuleEntry > ModuleList;
typedef ModuleList::iterator ModuleListIter;

// **************************************** ToolHelp32 ************************
#define MAX_MODULE_NAME32 255
#define TH32CS_SNAPMODULE   0x00000008
#pragma pack( push, 8 )
typedef struct tagMODULEENTRY32
{
    DWORD   dwSize;
    DWORD   th32ModuleID;       // This module
    DWORD   th32ProcessID;      // owning process
    DWORD   GlblcntUsage;       // Global usage count on the module
    DWORD   ProccntUsage;       // Module usage count in th32ProcessID's context
    BYTE  * modBaseAddr;        // Base address of module in th32ProcessID's context
    DWORD   modBaseSize;        // Size in bytes of module starting at modBaseAddr
    HMODULE hModule;            // The hModule of this module in th32ProcessID's context
    char    szModule[MAX_MODULE_NAME32 + 1];
    char    szExePath[MAX_PATH];
} MODULEENTRY32;
typedef MODULEENTRY32 *  PMODULEENTRY32;
typedef MODULEENTRY32 *  LPMODULEENTRY32;
#pragma pack( pop )


static bool GetModuleListTH32 (ModuleList &modules, DWORD pid, FILE *fLogFile)
{
  // CreateToolhelp32Snapshot()
  typedef HANDLE (__stdcall *tCT32S)(DWORD dwFlags, DWORD th32ProcessID);
  // Module32First()
  typedef BOOL (__stdcall *tM32F)(HANDLE hSnapshot, LPMODULEENTRY32 lpme);
  // Module32Next()
  typedef BOOL (__stdcall *tM32N)(HANDLE hSnapshot, LPMODULEENTRY32 lpme);

  // try both dlls...
  const char *dllname[] = { "kernel32.dll", "tlhelp32.dll" };
  HINSTANCE hToolhelp;
  tCT32S pCT32S;
  tM32F pM32F;
  tM32N pM32N;

  HANDLE hSnap;
  MODULEENTRY32 me;
  me.dwSize = sizeof(me);
  bool keepGoing;
  ModuleEntry e;
  int i;

  for (i = 0; i < DIM(dllname); i++ )
  {
    hToolhelp = LoadLibrary( dllname[i] );
    if (hToolhelp == NULL)
      continue;
    pCT32S = (tCT32S) GetProcAddress(hToolhelp, "CreateToolhelp32Snapshot");
    pM32F = (tM32F) GetProcAddress(hToolhelp, "Module32First");
    pM32N = (tM32N) GetProcAddress(hToolhelp, "Module32Next");
    if ( pCT32S != 0 && pM32F != 0 && pM32N != 0 )
      break; // found the functions!
    FreeLibrary(hToolhelp);
    hToolhelp = NULL;
  }

  if (hToolhelp == NULL)
    return false;

  hSnap = pCT32S( TH32CS_SNAPMODULE, pid );
  if (hSnap == (HANDLE) -1)
    return false;

  keepGoing = !!pM32F( hSnap, &me );
  while (keepGoing)
  {
    e.imageName = me.szExePath;
    e.moduleName = me.szModule;
    e.baseAddress = (DWORD) me.modBaseAddr;
    e.size = me.modBaseSize;
    modules.push_back( e );
    keepGoing = !!pM32N( hSnap, &me );
  }

  CloseHandle (hSnap);
  FreeLibrary (hToolhelp);

  return (modules.size() != 0);
}


// **************************************** PSAPI ************************
typedef struct _MODULEINFO {
    LPVOID lpBaseOfDll;
    DWORD SizeOfImage;
    LPVOID EntryPoint;
} MODULEINFO, *LPMODULEINFO;


static BOOL GetModuleListPSAPI (ModuleList &modules, DWORD pid, HANDLE hProcess, FILE *fLogFile)
{
  // EnumProcessModules()
  typedef BOOL (__stdcall *tEPM)(HANDLE hProcess, HMODULE *lphModule, DWORD cb, LPDWORD lpcbNeeded );
  // GetModuleFileNameEx()
  typedef DWORD (__stdcall *tGMFNE)(HANDLE hProcess, HMODULE hModule, LPSTR lpFilename, DWORD nSize );
  // GetModuleBaseName()
  typedef DWORD (__stdcall *tGMBN)(HANDLE hProcess, HMODULE hModule, LPSTR lpFilename, DWORD nSize );
  // GetModuleInformation()
  typedef BOOL (__stdcall *tGMI)(HANDLE hProcess, HMODULE hModule, LPMODULEINFO pmi, DWORD nSize );

  HINSTANCE  hPsapi;
  tEPM       pEPM;
  tGMFNE     pGMFNE;
  tGMBN      pGMBN;
  tGMI       pGMI;
  DWORD      i, cbNeeded;
  MODULEINFO mi;
  HMODULE   *hMods = 0;
  char      *tt = NULL;
  ModuleEntry e;

  hPsapi = LoadLibrary ("psapi.dll");
  if (!hPsapi)
     return (FALSE);

  modules.clear();

  pEPM   = (tEPM)   GetProcAddress (hPsapi, "EnumProcessModules");
  pGMFNE = (tGMFNE) GetProcAddress (hPsapi, "GetModuleFileNameExA");
  pGMBN  = (tGMFNE) GetProcAddress (hPsapi, "GetModuleBaseNameA");
  pGMI   = (tGMI)   GetProcAddress (hPsapi, "GetModuleInformation");

  if (!pEPM || !pGMFNE || !pGMBN  || !pGMI)
  {
    // we couldn´t find all functions
    FreeLibrary (hPsapi);
    return FALSE;
  }

  hMods = (HMODULE*) malloc (sizeof(HMODULE) * (TTBUFLEN / sizeof HMODULE));
  tt = (char*) malloc (sizeof(char) * TTBUFLEN);

  if ( ! (*pEPM)( hProcess, hMods, TTBUFLEN, &cbNeeded ) )
  {
    fprintf (fLogFile, "%lu: EPM failed, GetLastError = %lu\n", g_dwShowCount, gle );
    goto cleanup;
  }

  if ( cbNeeded > TTBUFLEN )
  {
    fprintf (fLogFile, "%lu: More than %lu module handles. Huh?\n", g_dwShowCount, DIM(hMods));
    goto cleanup;
  }

  for ( i = 0; i < cbNeeded / sizeof(hMods[0]); i++ )
  {
    // base address, size
    (*pGMI) (hProcess, hMods[i], &mi, sizeof mi );
    e.baseAddress = (DWORD) mi.lpBaseOfDll;
    e.size = mi.SizeOfImage;
    // image file name
    tt[0] = 0;
    (*pGMFNE) (hProcess, hMods[i], tt, TTBUFLEN );
    e.imageName = tt;
    // module name
    tt[0] = 0;
    (*pGMBN) (hProcess, hMods[i], tt, TTBUFLEN );
    e.moduleName = tt;

    modules.push_back(e);
  }

cleanup:
  if (hPsapi)
     FreeLibrary (hPsapi);
  free (tt);
  free (hMods);

  return (modules.size() != 0);
}

static BOOL GetModuleList (ModuleList &modules, DWORD pid, HANDLE hProcess, FILE *fLogFile)
{
  if (GetModuleListTH32(modules, pid, fLogFile))   // first try toolhelp32
     return (TRUE);
  return GetModuleListPSAPI(modules, pid, hProcess, fLogFile); // then try psapi
}

static void EnumAndLoadModuleSymbols (HANDLE hProcess, DWORD pid, FILE *fLogFile)
{
  static ModuleList modules;
  static ModuleListIter it;

  // fill in module list
  GetModuleList (modules, pid, hProcess, fLogFile);

  for (it = modules.begin(); it != modules.end(); ++ it)
  {
    char *img = strdup (it->imageName.c_str()); // SymLoadModule() wants writeable strings
    char *mod = strdup (it->moduleName.c_str());

    (*pSLM) (hProcess, 0, img, mod, it->baseAddress, it->size);

    free (img);
    free (mod);
  }
}

int InitStackWalk (void)
{
  if (g_bInitialized)
     return (0);

  // 02-12-19: Now we only support dbghelp.dll!
  //           To use it on NT you have to install the redistrubutable for DBGHELP.DLL
  g_hImagehlpDll = LoadLibrary ("dbghelp.dll");
  if (!g_hImagehlpDll)
  {
    printf ("LoadLibrary(\"dbghelp.dll\") failed: GetLastError = %lu\n", gle);
    g_bInitialized = FALSE;
    return (1);
  }

  // now we only support the newer dbghlp.dll with the "64"-functions (StackWalk64, a.s.o.)
  // If your dbghlp.dll does not support this, please download the redistributable from MS
  // Normally from: http://www.microsoft.com/downloads/details.aspx?displaylang=en&FamilyID=CD1FC4B2-0885-47F4-AF45-7FD5E14DB6C0

  pSC = (tSC) GetProcAddress (g_hImagehlpDll, "SymCleanup");
  pSFTA = (tSFTA) GetProcAddress (g_hImagehlpDll, "SymFunctionTableAccess64");
  pSGLFA = (tSGLFA) GetProcAddress (g_hImagehlpDll, "SymGetLineFromAddr64");
  pSGMB = (tSGMB) GetProcAddress (g_hImagehlpDll, "SymGetModuleBase64");
  pSGMI = (tSGMI) GetProcAddress (g_hImagehlpDll, "SymGetModuleInfo64");
  pSGO = (tSGO) GetProcAddress (g_hImagehlpDll, "SymGetOptions");
  pSGSFA = (tSGSFA) GetProcAddress (g_hImagehlpDll, "SymGetSymFromAddr64");
  pSI = (tSI) GetProcAddress (g_hImagehlpDll, "SymInitialize");
  pSSO = (tSSO) GetProcAddress (g_hImagehlpDll, "SymSetOptions");
  pSW = (tSW) GetProcAddress (g_hImagehlpDll, "StackWalk64");
  pUDSN = (tUDSN) GetProcAddress (g_hImagehlpDll, "UnDecorateSymbolName");
  pSLM = (tSLM) GetProcAddress (g_hImagehlpDll, "SymLoadModule64");

  if (!pSC || !pSFTA || !pSGMB || !pSGMI ||
      !pSGO || !pSGSFA || !pSI || !pSSO  ||
      !pSW || !pUDSN || !pSLM)
  {
    printf ("GetProcAddress(): some required function not found.\n");
    FreeLibrary (g_hImagehlpDll);
    g_bInitialized = FALSE;
    return (1);
  }

  g_bInitialized = TRUE;
  InitializeCriticalSection(&g_csFileOpenClose);
  return 0;
}

// This function if NOT multi-threading capable
// It should only be called from the main-Function!
int InitAllocCheckWN (enum eAllocCheckOutput eOutput, const char *pszFileName, ULONG ulShowStackAtAlloc)
{
  if (g_bInitialized)
     return 2;  // already initialized!

  if (ulShowStackAtAlloc <= 3)
       g_ulShowStackAtAlloc = ulShowStackAtAlloc;
  else g_ulShowStackAtAlloc = 0;

  if (pszFileName)
       g_pszAllocLogName = _tcsdup(pszFileName);
  else g_pszAllocLogName = NULL;

  g_CallstackOutputType = eOutput;

#ifdef _DEBUG
  AllocHashInit();

  // save the previous alloc hook
  pfnOldCrtAllocHook = _CrtSetAllocHook (MyAllocHook);
#endif

  return InitStackWalk();
}

void SetCallStackOutputType (enum eAllocCheckOutput eOutput)
{
  g_CallstackOutputType = eOutput;
}

static char s_szExceptionLogFileName[_MAX_PATH] = "\\exceptions.log";  // default
static BOOL s_bUnhandledExeptionFilterSet = FALSE;

static char global_szModName[_MAX_PATH] = "";
static char global_szAppName[_MAX_PATH] = "";

const char *StackWalkLogFile (void)
{
  return global_szModName;
}

static LONG __stdcall CrashHandlerExceptionFilter (const EXCEPTION_POINTERS* pExPtrs)
{
  if (pExPtrs->ExceptionRecord->ExceptionCode == EXCEPTION_STACK_OVERFLOW)
  {
    static char MyStack[1024*128];  // be sure that we have enought space...
    // it assumes that DS and SS are the same!!! (this is the case for Win32)
    // change the stack only if the selectors are the same (this is the case for Win32)
    //__asm push offset MyStack[1024*128];
    //__asm pop esp;
    __asm mov eax,offset MyStack[1024*128];
    __asm mov esp,eax;
  }

  LONG lRet;
  lRet = StackwalkFilter (pExPtrs, /*EXCEPTION_CONTINUE_SEARCH*/ EXCEPTION_EXECUTE_HANDLER,
                          s_szExceptionLogFileName);
  char lString[500];
  SNPRINTF (lString, sizeof(lString),
            "*** Unhandled Exception in %s.\r\n"
            "   Look at %s for details\r\n",
            global_szAppName, s_szExceptionLogFileName);
  FatalAppExit (-1, lString);
  return (lRet);
}

int InitAllocCheck (enum eAllocCheckOutput eOutput, BOOL bSetUnhandledExeptionFilter, ULONG ulShowStackAtAlloc)
{
  if (GetModuleFileName(NULL, global_szModName, sizeof(global_szModName)) != 0)
  {
    strcpy (global_szAppName, global_szModName);
    strcat (global_szModName, ".mem.log");
    strcpy (s_szExceptionLogFileName, global_szAppName);
    strcat (s_szExceptionLogFileName, ".exp.log");
  }
  else
  {
    strcpy (global_szModName, "\\mem-leaks.log");
  }

  if (bSetUnhandledExeptionFilter && !s_bUnhandledExeptionFilterSet)
  {
    // set global exception handler (for handling all unhandled exceptions)
    SetUnhandledExceptionFilter ((LPTOP_LEVEL_EXCEPTION_FILTER)CrashHandlerExceptionFilter);
    s_bUnhandledExeptionFilterSet = TRUE;
  }

  return InitAllocCheckWN(eOutput, global_szModName, ulShowStackAtAlloc);
}


// This function if NOT multi-threading capable
// It should only be called from the main-Function!
//   Returns the number of bytes that are not freed (leaks)
ULONG DeInitAllocCheck (void)
{
  ULONG ulRet = 0;

  if (g_bInitialized)
  {
#ifdef _DEBUG
    InterlockedIncrement (&g_lMallocCalled); // No deactivate MyAllocHook, because StackWalker will allocate some memory)
    ulRet = AllocHashDeinit();  // output the not freed memory
    // remove the hook and set the old one
    _CrtSetAllocHook (pfnOldCrtAllocHook);
#endif

    EnterCriticalSection(&g_csFileOpenClose);  // wait until a running stack dump was created
    g_bInitialized = FALSE;

    // de-init symbol handler etc. (SymCleanup())
    if (pSC != NULL)
      pSC( GetCurrentProcess() );
    FreeLibrary( g_hImagehlpDll );

    LeaveCriticalSection(&g_csFileOpenClose);
    if (g_pszAllocLogName != NULL) {
      free(g_pszAllocLogName);
      g_pszAllocLogName = NULL;
    }
    if (g_fFile != NULL) {
       fclose(g_fFile);
       g_fFile = NULL;
    }

    DeleteCriticalSection(&g_csFileOpenClose);
    InterlockedDecrement(&g_lMallocCalled);
  }

  if (s_bUnhandledExeptionFilterSet != TRUE)
  {
    SetUnhandledExceptionFilter (NULL);
    s_bUnhandledExeptionFilterSet = FALSE;
  }
  return ulRet;
}



void OnlyInstallUnhandeldExceptionFilter (enum eAllocCheckOutput eOutput)
{
  char szModName[_MAX_PATH];

  if (s_bUnhandledExeptionFilterSet)
     return;

  if (GetModuleFileName(NULL, szModName, sizeof(szModName)))
  {
    strcpy (s_szExceptionLogFileName, szModName);
    strcat (s_szExceptionLogFileName, ".exp.log");
    strcat (szModName, ".mem.log");
  }
  else
    strcpy (szModName, "\\mem-leaks.log");

  // set it again; WARNING: this will override the setting for a possible AllocCheck-Setting
  g_CallstackOutputType = eOutput;

  // set global exception handler (for handling all unhandled exceptions)
  SetUnhandledExceptionFilter ((LPTOP_LEVEL_EXCEPTION_FILTER)CrashHandlerExceptionFilter);
  s_bUnhandledExeptionFilterSet = TRUE;
}


static const char *GetExpectionCodeText (DWORD dwExceptionCode)
{
  switch (dwExceptionCode)
  {
    case EXCEPTION_ACCESS_VIOLATION:
         return "ACCESS VIOLATION";
    case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
         return ("ARRAY BOUNDS EXCEEDED");
    case EXCEPTION_BREAKPOINT:
         return ("BREAKPOINT");
    case EXCEPTION_DATATYPE_MISALIGNMENT:
         return ("DATATYPE MISALIGNMENT");
    case EXCEPTION_FLT_DENORMAL_OPERAND:
         return ("FLT DENORMAL OPERAND");
    case EXCEPTION_FLT_DIVIDE_BY_ZERO:
         return ("FLT DIVIDE BY ZERO");
    case EXCEPTION_FLT_INEXACT_RESULT:
         return ("FLT INEXACT RESULT");
    case EXCEPTION_FLT_INVALID_OPERATION:
         return ("FLT INVALID OPERATION");
    case EXCEPTION_FLT_OVERFLOW:
         return ("FLT OVERFLOW");
    case EXCEPTION_FLT_STACK_CHECK:
         return ("FLT STACK CHECK");
    case EXCEPTION_FLT_UNDERFLOW:
         return ("FLT UNDERFLOW");
    case EXCEPTION_ILLEGAL_INSTRUCTION:
         return ("ILLEGAL INSTRUCTION");
    case EXCEPTION_IN_PAGE_ERROR:
         return ("IN PAGE ERROR");
    case EXCEPTION_INT_DIVIDE_BY_ZERO:
         return ("INT DIVIDE BY ZERO");
    case EXCEPTION_INT_OVERFLOW:
         return ("INT OVERFLOW");
    case EXCEPTION_INVALID_DISPOSITION:
         return ("INVALID DISPOSITION");
    case EXCEPTION_NONCONTINUABLE_EXCEPTION:
         return ("NONCONTINUABLE EXCEPTION");
    case EXCEPTION_PRIV_INSTRUCTION:
         return ("PRIV INSTRUCTION");
    case EXCEPTION_SINGLE_STEP:
         return ("SINGLE STEP");
    case EXCEPTION_STACK_OVERFLOW:
         return ("STACK OVERFLOW");
    case DBG_CONTROL_C:
         return ("DBG CONTROL C ");
  }
  return ("<unkown exception>");
}

/*
 * Function is not multi-threading safe, because of static char!
 */
static const char *GetAdditionalExpectionCodeText (const EXCEPTION_RECORD *pExceptionRecord)
{
  static char szTemp[100];

  switch (pExceptionRecord->ExceptionCode)
  {
    case EXCEPTION_ACCESS_VIOLATION:
         if (pExceptionRecord->NumberParameters == 2)
         {
           switch (pExceptionRecord->ExceptionInformation[0])
           {
             case 0: // read attempt
                  sprintf (szTemp, " read attempt to address 0x%8.8X ",
                           pExceptionRecord->ExceptionInformation[1]);
                  return (szTemp);
             case 1: // write attempt
                  sprintf (szTemp, " write attempt to address 0x%8.8X ",
                           pExceptionRecord->ExceptionInformation[1]);
                  return (szTemp);
             default:
                  return ("");
           }
         }
         return ("");
    default:
         return ("");
  }
}


// #################################################################################
// #################################################################################
// Here the Stackwalk-Part begins.
//   Some of the code is from an example from a book
//   But I couldn´t find the reference anymore... sorry...
//   If someone knowns, please let me know...
// #################################################################################
// #################################################################################


// if you use C++ exception handling: install a translator function
// with set_se_translator(). In the context of that function (but *not*
// afterwards), you can either do your stack dump, or save the CONTEXT
// record as a local copy. Note that you must do the stack sump at the
// earliest opportunity, to avoid the interesting stackframes being gone
// by the time you do the dump.

// status:
// - EXCEPTION_CONTINUE_SEARCH: exception wird weitergereicht
// - EXCEPTION_CONTINUE_EXECUTION:
// - EXCEPTION_EXECUTE_HANDLER:

static DWORD StackwalkFilter (const EXCEPTION_POINTERS *ep, DWORD status, const char *pszLogFile)
{
  HANDLE hThread;
  FILE  *fFile = stdout;  // default to stdout

  if (pszLogFile)
  {
    fFile = fopen (pszLogFile, "a");
    if (fFile)
    {
      long size;

      fseek (fFile, 0, SEEK_END);
      size = ftell (fFile);  // Get the size of the file
      if (size >= LOG_FILE_MAX_SIZE)  // Is the file too big?
      {
        char *pszTemp = (char*) alloca (MAX_PATH);

        fclose (fFile);
        strcpy (pszTemp, pszLogFile);
        strcat (pszTemp, ".old");
        remove (pszTemp);                 // Remove an old file, if exists
        rename (pszLogFile, pszTemp);     // rename the actual file
        fFile = fopen (pszLogFile, "w");  // create a new file
      }
    }
  }

  if (!fFile)
     fFile = stdout;

  // Write infos about the exception
  fprintf (fFile, "######## EXCEPTION: 0x%8.8X at address: 0x%8.8X",
           ep->ExceptionRecord->ExceptionCode,
           ep->ExceptionRecord->ExceptionAddress);
  fprintf (fFile, ": %s %s\n",
           GetExpectionCodeText(ep->ExceptionRecord->ExceptionCode),
           GetAdditionalExpectionCodeText(ep->ExceptionRecord));

  DuplicateHandle (GetCurrentProcess(), GetCurrentThread(),
                   GetCurrentProcess(), &hThread, 0, FALSE, DUPLICATE_SAME_ACCESS);
  ShowStack (hThread, ep->ContextRecord, fFile);
  CloseHandle (hThread);

  fclose (fFile);

  return status;
}

void ShowStack (HANDLE hThread, const CONTEXT *c, const char *pszLogFile)
{
  FILE *fFile = stdout;  // default to stdout
  long  size;

  if (pszLogFile)
  {
    fFile = fopen (pszLogFile, "a"); // Open the logfile
    if (fFile)
    {
      fseek (fFile, 0, SEEK_END);
      size = ftell (fFile);             // Get the size of the file
      if (size >= LOG_FILE_MAX_SIZE)    // Is the file too big?
      {
        char *pszTemp = (char*) alloca (MAX_PATH);

        fclose (fFile);
        strcpy (pszTemp, pszLogFile);
        strcat (pszTemp, ".old");
        remove (pszTemp);                 // Remove an old file, if exists
        rename (pszLogFile, pszTemp);     // rename the actual file
        fFile = fopen (pszLogFile, "w");  // open new file
        free(pszTemp);
      }
    }
  }
  if (!fFile)
     fFile = stdout;

  ShowStackRM (hThread, c, fFile, NULL, GetCurrentProcess());
  fclose (fFile);
}

static void ShowStack (HANDLE hThread, const CONTEXT *c, FILE *fLogFile)
{
  ShowStackRM (hThread, c, fLogFile, NULL, GetCurrentProcess());
}

static void ShowStackRM (HANDLE hThread, const CONTEXT *c, FILE *fLogFile,
                         PREAD_PROCESS_MEMORY_ROUTINE64 ReadMemoryFunction,
                         HANDLE hSWProcess)
{
  // normally, call ImageNtHeader() and use machine info from PE header
  // but we assume that it is an I386 image...
  DWORD imageType = IMAGE_FILE_MACHINE_I386;
  HANDLE hProcess = GetCurrentProcess(); // hProcess normally comes from outside but we only do the
                                         // stackdump in our own process
  int frameNum;                          // counts walked frames
  DWORD64 offsetFromSymbol;              // tells us how far from the symbol we were
  DWORD offsetFromLine;                  // tells us how far from the line we were
  DWORD symOptions;                      // symbol handler settings

  static IMAGEHLP_SYMBOL64 *pSym = NULL;
  char undName[MAXNAMELEN]; // undecorated name
  char undFullName[MAXNAMELEN]; // undecorated name with all shenanigans
  IMAGEHLP_MODULE64 Module;
  IMAGEHLP_LINE64 Line;

  std::string symSearchPath;

  static int bFirstTime = TRUE;

  // If no logfile is present, output to "stdout"
  if (fLogFile == NULL)
    fLogFile = stdout;

  STACKFRAME64 s; // in/out stackframe
  memset (&s, 0, sizeof(s));

  if (!g_bInitialized && bFirstTime)
    InitStackWalk();

  if (g_bInitialized == FALSE)
  {
    // Could not init!!!!
    bFirstTime = FALSE;
    fprintf (fLogFile, "%lu: Stackwalker not initialized (or was not able to initialize)!\n", g_dwShowCount);
    return;
  }

  EnterCriticalSection(&g_csFileOpenClose);

  InterlockedIncrement((long*) &g_dwShowCount);  // increase counter

  // NOTE: normally, the exe directory and the current directory should be taken
  // from the target process. The current dir would be gotten through injection
  // of a remote thread; the exe fir through either ToolHelp32 or PSAPI.

  if (pSym == NULL)
  {
    pSym = (IMAGEHLP_SYMBOL64 *) malloc( IMGSYMLEN + MAXNAMELEN );
    if (!pSym)
       goto cleanup;
  }

  fprintf (fLogFile, "%lu: ", g_dwShowCount);
  WriteDateTime (fLogFile);
  fprintf (fLogFile, "\n");

  if (bFirstTime) {

    CHAR *tt, *p;

    tt = (CHAR*) malloc(sizeof(CHAR) * TTBUFLEN); // Get the temporary buffer
    if (!tt) goto cleanup;  // not enough memory...

    // build symbol search path from:
    symSearchPath = "";
    // current directory
    if ( GetCurrentDirectoryA( TTBUFLEN, tt ) )
      symSearchPath += tt + std::string( ";" );
    // dir with executable
    if ( GetModuleFileNameA( 0, tt, TTBUFLEN ) )
    {
      for ( p = tt + strlen( tt ) - 1; p >= tt; -- p )
      {
        // locate the rightmost path separator
        if ( *p == '\\' || *p == '/' || *p == ':' )
          break;
      }
      // if we found one, p is pointing at it; if not, tt only contains
      // an exe name (no path), and p points before its first byte
      if ( p != tt ) // path sep found?
      {
        if ( *p == ':' ) // we leave colons in place
          ++ p;
        *p = '\0'; // eliminate the exe name and last path sep
        symSearchPath += tt + std::string( ";" );
      }
    }
    // environment variable _NT_SYMBOL_PATH
    if ( GetEnvironmentVariableA( "_NT_SYMBOL_PATH", tt, TTBUFLEN ) )
      symSearchPath += tt + std::string( ";" );
    // environment variable _NT_ALTERNATE_SYMBOL_PATH
    if ( GetEnvironmentVariableA( "_NT_ALTERNATE_SYMBOL_PATH", tt, TTBUFLEN ) )
      symSearchPath += tt + std::string( ";" );
    // environment variable SYSTEMROOT
    if ( GetEnvironmentVariableA( "SYSTEMROOT", tt, TTBUFLEN ) )
      symSearchPath += tt + std::string( ";" );

    if ( symSearchPath.size() > 0 ) // if we added anything, we have a trailing semicolon
      symSearchPath = symSearchPath.substr( 0, symSearchPath.size() - 1 );

    // why oh why does SymInitialize() want a writeable string?
    strncpy( tt, symSearchPath.c_str(), TTBUFLEN );
    tt[TTBUFLEN - 1] = '\0'; // if strncpy() overruns, it doesn't add the null terminator

    // init symbol handler stuff (SymInitialize())
    if ( ! (*pSI)( hProcess, tt, FALSE ))
    {
      fprintf (fLogFile, "%lu: SymInitialize(): GetLastError = %lu\n", g_dwShowCount, gle);
      if (tt)
         free (tt);
      goto cleanup;
    }

    // SymGetOptions()
    symOptions = (*pSGO)();
    symOptions |= SYMOPT_LOAD_LINES;
    symOptions &= ~SYMOPT_UNDNAME;
    symOptions &= ~SYMOPT_DEFERRED_LOADS;
    pSSO( symOptions ); // SymSetOptions()

    // Enumerate modules and tell dbghlp.dll about them.
    // On NT, this is not necessary, but it won't hurt.
    EnumAndLoadModuleSymbols( hProcess, GetCurrentProcessId(), fLogFile );

    if (tt)
      free( tt );
  }  // bFirstTime = TRUE

  bFirstTime = FALSE;

  // init STACKFRAME for first call
  // Notes: AddrModeFlat is just an assumption. I hate VDM debugging.
  // Notes: will have to be #ifdef-ed for Alphas; MIPSes are dead anyway,
  // and good riddance.
  s.AddrPC.Offset = c->Eip;
  s.AddrPC.Mode = AddrModeFlat;
  s.AddrFrame.Offset = c->Ebp;
  s.AddrFrame.Mode = AddrModeFlat;

  memset( pSym, '\0', IMGSYMLEN + MAXNAMELEN );
  pSym->SizeOfStruct = IMGSYMLEN;
  pSym->MaxNameLength = MAXNAMELEN;

  memset( &Line, '\0', sizeof Line );
  Line.SizeOfStruct = sizeof Line;

  memset( &Module, '\0', sizeof Module );
  Module.SizeOfStruct = sizeof Module;

  for ( frameNum = 0; ; ++ frameNum )
  {
    // get next stack frame (StackWalk64(), SymFunctionTableAccess64(), SymGetModuleBase64())
    // if this returns ERROR_INVALID_ADDRESS (487) or ERROR_NOACCESS (998), you can
    // assume that either you are done, or that the stack is so hosed that the next
    // deeper frame could not be found.
    // CONTEXT need not to be suplied if imageTyp is IMAGE_FILE_MACHINE_I386!
    if ( ! pSW( imageType, hSWProcess, hThread, &s, NULL, ReadMemoryFunction, pSFTA, pSGMB, NULL ) )
      break;

    if (g_CallstackOutputType == ACOutput_Advanced)
      fprintf (fLogFile, "\n%lu: %3d", g_dwShowCount, frameNum);
    if ( s.AddrPC.Offset == 0 )
    {
      // Special case: If we are here, we have no valid callstack entry!
      switch(g_CallstackOutputType)
      {
      case ACOutput_Simple:
        fprintf (fLogFile, "%lu: (-nosymbols- PC == 0)\n", g_dwShowCount);
        break;
      case ACOutput_Advanced:
        fprintf (fLogFile, "   (-nosymbols- PC == 0)\n");
        break;
      }
    }
    else
    {
      // we seem to have a valid PC
      undName[0] = 0;
      undFullName[0] = 0;
      offsetFromSymbol = 0;
      // show procedure info (SymGetSymFromAddr())
      if ( ! (*pSGSFA)( hProcess, s.AddrPC.Offset, &offsetFromSymbol, pSym ) )
      {
        if (g_CallstackOutputType == ACOutput_Advanced)
        {
          if ( gle != 487 )
            fprintf (fLogFile, "   SymGetSymFromAddr(): GetLastError = %lu\n", gle );
          else
            fprintf (fLogFile, "\n");
        }
      }
      else
      {
        // UnDecorateSymbolName()
        (*pUDSN) (pSym->Name, undName, MAXNAMELEN, UNDNAME_NAME_ONLY);
        (*pUDSN) ( pSym->Name, undFullName, MAXNAMELEN, UNDNAME_COMPLETE);
        if (g_CallstackOutputType == ACOutput_Advanced)
        {
          if (strlen(undName) > 0)
            fprintf (fLogFile, "     %s %+ld bytes\n", undName, (long)offsetFromSymbol );
          else
          {
            fprintf (fLogFile, "     Sig:  %s %+ld bytes\n", pSym->Name, (long)offsetFromSymbol );
            strcpy(undName, pSym->Name);
          }
          fprintf (fLogFile, "%lu:     Decl: %s\n", g_dwShowCount, undFullName );
        }
      }

      // show line number info, NT5.0-method (SymGetLineFromAddr())
      offsetFromLine = 0;
      if ( pSGLFA != NULL )
      { // yes, we have SymGetLineFromAddr()
        if ( ! pSGLFA( hProcess, s.AddrPC.Offset, &offsetFromLine, &Line ) )
        {
          if ( (gle != 487) && (frameNum > 0) )  // ignore error for first frame
          {
            fprintf (fLogFile, "%lu: SymGetLineFromAddr(): GetLastError = %lu\n", g_dwShowCount, gle );
          }
        }
        else
        {
          switch(g_CallstackOutputType)
          {
          case ACOutput_Advanced:
            fprintf (fLogFile, "%lu:     Line: %s(%lu) %+ld bytes\n", g_dwShowCount,
              Line.FileName, Line.LineNumber, offsetFromLine );
            break;
          case ACOutput_Simple:
            fprintf (fLogFile, "%lu: %s(%lu) %+ld bytes (%s)\n", g_dwShowCount,
              Line.FileName, Line.LineNumber, offsetFromLine, undName);
            break;
          }
        }
      } // yes, we have SymGetLineFromAddr()

      // show module info (SymGetModuleInfo())
      if (g_CallstackOutputType == ACOutput_Advanced)
      {
        if ( ! pSGMI( hProcess, s.AddrPC.Offset, &Module ) )
        {
          if (g_CallstackOutputType == ACOutput_Advanced)
            fprintf (fLogFile, "%lu: SymGetModuleInfo): GetLastError = %lu\n", g_dwShowCount, gle );
        }
        else
        { // got module info OK
          char ty[80];
          switch ( Module.SymType )
          {
          case SymNone:
            strcpy( ty, "-nosymbols-" );
            break;
          case SymCoff:
            strcpy( ty, "COFF" );
            break;
          case SymCv:
            strcpy( ty, "CV" );
            break;
          case SymPdb:
            strcpy( ty, "PDB" );
            break;
          case SymExport:
            strcpy( ty, "-exported-" );
            break;
          case SymDeferred:
            strcpy( ty, "-deferred-" );
            break;
          case SymSym:
            strcpy( ty, "SYM" );
            break;
#if API_VERSION_NUMBER >= 9
          case SymDia:
            strcpy( ty, "DIA" );
            break;
#endif
          default:
            SNPRINTF (ty, sizeof(ty), "symtype=%ld", (long) Module.SymType );
            break;
          }

          if (g_CallstackOutputType == ACOutput_Advanced)
          {
            fprintf (fLogFile, "%lu:     Mod:  %s, base: %08lxh\n", g_dwShowCount,
              Module.ModuleName, Module.BaseOfImage );
            if (Module.SymType == SymNone) { // Gebe nur aus, wenn keine Symbole vorhanden sind!
              fprintf (fLogFile, "%lu:     Offset: 0x%8.8x\n", g_dwShowCount, s.AddrPC.Offset);
              fprintf (fLogFile, "%lu:     Sym:  type: %s, file: %s\n",
                       g_dwShowCount, ty, Module.LoadedImageName);
            }
          }
        }
      }
    }

    // no return address means no deeper stackframe
    if ( s.AddrReturn.Offset == 0 )
    {
      // avoid misunderstandings in the printf() following the loop
      SetLastError( 0 );
      break;
    }
  }

  if (gle)
    fprintf (fLogFile, "\n%lu: StackWalk(): GetLastError = %lu\n", g_dwShowCount, gle);

cleanup:
  if (fLogFile) {
    fprintf (fLogFile, "\n\n");
    if (g_dwShowCount % 1000)
      fflush(fLogFile);
  }
  LeaveCriticalSection(&g_csFileOpenClose);
}
#endif  /* USE_STACKWALKER */
