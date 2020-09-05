/*!\file fortify.c
 * A fortified memory allocation shell - V2.2.
 */

/*
 * This software is not public domain. All material in
 * this archive is (C) Copyright 1995 Simon P. Bullen. The
 * software is freely distributable, with the condition that
 * no more than a nominal fee is charged for media.
 * Everything in this distribution must be kept together, in
 * original, unmodified form.
 *
 * The software may be modified for your own personal use,
 * but modified files may not be distributed.
 *
 * The material is provided "as is" without warranty of
 * any kind. The author accepts no responsibility for damage
 * caused by this software.
 *
 * This software may not be used in any way by Microsoft
 * Corporation or its subsidiaries, or current employees of
 * Microsoft Corporation or its subsidiaries.
 *
 * This software may not be used for the construction,
 * development, production, or testing of weapon systems of
 * any kind.
 *
 * This software may not be used for the construction,
 * development, production, or use of plants/installations
 * which include the processing of radioactive/fissionable
 * material.
 */

/*
 * If you use this software at all, I'd love to hear from
 * you.  All questions, criticisms, suggestions, praise and
 * postcards are most welcome.
 *
 *            email:    sbullen@cybergraphic.com.au
 *
 *            snail:    Simon P. Bullen
 *                      PO BOX 12138
 *                      A'Beckett St.
 *                      Melbourne 3000
 *                      Australia
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include <time.h>

/* Prototypes and such */
#define __FORTIFY_C__

#include "wattcp.h"
#include "misc.h"
#include "strings.h"
#include "nochkstk.h"
#include "fortify.h"

#if defined(USE_FORTIFY) /* && !defined(USE_CRTDBG) */  /* rest of file */

#if defined(__CYGWIN__)
#error Fortify does not work with Cygwin. Have no idea why yet.
#endif

#if defined(WIN32)
#include <windowsx.h>
#endif

/*
 * struct Header - this structure is used
 * internally by Fortify to manage it's
 * own private lists of memory.
 */
struct Header {
       WORD           Checksum;     /* For the integrity of our goodies  */
       const char    *File;         /* The sourcefile of the allocator   */
       DWORD          Line;         /* The sourceline of the allocator   */
#ifdef FORTIFY_TRACK_DEALLOCATED_MEMORY
       const char    *FreedFile;    /* The sourcefile of the deallocator */
       DWORD          FreedLine;    /* The sourceline of the deallocator */
       BYTE           Deallocator;  /* The deallocator used              */
#endif
       size_t         Size;         /* The size of the malloc'd block    */
       struct Header *Prev;         /* Previous link                     */
       struct Header *Next;         /* Next link                         */
       char          *Label;        /* User's Label (may be null)        */
       BYTE           Scope;        /* Scope level of the caller         */
       BYTE           Allocator;    /* malloc/realloc/new/etc            */
     };

/*
 * Round x up to the nearest multiple of n.
 */
#define ROUND_UP(x,n)        ((((x) + (n)-1)/(n))*(n))

#define FORTIFY_HEADER_SIZE  ROUND_UP(sizeof(struct Header), sizeof(WORD))

/*
 * FORTIFY_ALIGNED_BEFORE_SIZE is FORTIFY_BEFORE_SIZE rounded up to the
 * next multiple of FORTIFY_ALIGNMENT. This is so that we can guarantee
 * the alignment of user memory for such systems where this is important
 * (eg storing doubles on a SPARC)
 */
#define FORTIFY_ALIGNED_BEFORE_SIZE ( \
        ROUND_UP (FORTIFY_HEADER_SIZE+FORTIFY_BEFORE_SIZE, FORTIFY_ALIGNMENT) \
        - FORTIFY_HEADER_SIZE)

/*
 * FORTIFY_OVERHEAD is the total overhead added by Fortify to each
 * memory block.
 */
#define FORTIFY_OVERHEAD (FORTIFY_HEADER_SIZE         + \
                          FORTIFY_ALIGNED_BEFORE_SIZE + \
                          FORTIFY_AFTER_SIZE)

#if (DOSX)
  #define VALIDATE(p,len,neg_action)  \
          ((int) (valid_addr ((const void*)(p),len) ? 1 : (int)(neg_action)))
#else
  #define VALIDATE(p,len,neg_action)  (1)
#endif

/*
 * Static Function Prototypes
 */
static int  st_CheckBlock         (struct Header *h, const char *file, DWORD line);
static int  st_CheckFortification (BYTE *ptr, BYTE value, size_t size);
static void st_SetFortification   (BYTE *ptr, BYTE value, size_t size);
static void st_OutputFortification(BYTE *ptr, BYTE value, size_t size);
static void st_HexDump            (BYTE *ptr, size_t offset, size_t size, int title);
static void st_MakeHeaderValid    (struct Header *h);
static int  st_IsHeaderValid      (const struct Header *h);
static WORD st_ChecksumHeader     (const struct Header *h);
static int  st_IsOnAllocatedList  (const struct Header *h);
static int  st_OutputHeader       (struct Header *h);
static void st_OutputMemory       (struct Header *h);
static void st_OutputLastVerifiedPoint(void);
static void st_DefaultOutput      (const char *String);
static const char *st_MemoryBlockString(struct Header *h);
static void st_OutputDeleteTrace (void);

#if defined(FORTIFY_TRACK_DEALLOCATED_MEMORY)
  #if defined(FORTIFY_WARN_WHEN_DISCARDING_DEALLOCATED_MEMORY) && \
      defined(FORTIFY_VERBOSE_WARN_WHEN_DISCARDING_DEALLOCATED_MEMORY)
    static const char *st_DeallocatedMemoryBlockString (struct Header *h);
  #endif

  static int  st_IsOnDeallocatedList (const struct Header *h);
  static int  st_PurgeDeallocatedBlocks (DWORD Bytes, const char *file, DWORD line);
  static int  st_PurgeDeallocatedScope (BYTE Scope, const char *file, DWORD line);
  static int  st_CheckDeallocatedBlock (struct Header *h, const char *file, DWORD line);
  static void st_FreeDeallocatedBlock (struct Header *h, const char *file, DWORD line);
#endif


/*
 * Static variables
 */
static char                  st_Buffer[4096]     = { 0 }; /* don't use BSS */
static struct Header        *st_AllocatedHead    = 0;
static int                   st_AllocateFailRate = 0;
static Fortify_OutputFuncPtr st_Output           = st_DefaultOutput;
static const char           *st_LastVerifiedFile = "unknown";
static DWORD                 st_LastVerifiedLine = 0;
static BYTE                  st_Scope            = 0;
static BYTE                  st_Disabled         = 0;

#if defined(__HIGHC__) && 0 /* !! not yet */
static char st_LockDataStart = 0;
#endif

#ifdef __cplusplus
  using namespace std;

  int    gbl_FortifyMagic = 0;
  static const char *st_DeleteFile[FORTIFY_DELETE_STACK_SIZE];
  static DWORD       st_DeleteLine[FORTIFY_DELETE_STACK_SIZE];
  static DWORD       st_DeleteStackTop;
#endif

#ifndef ST_OUTPUT
#define ST_OUTPUT(s) do { if (st_Output) (*st_Output)(s); } while (0)
#endif

static void message (BOOL warn, const char *fmt, ...)
{
  va_list args;
  char   *pos = st_Buffer;

  if (!st_Output)
     return;

#if 0
  if (!valid_addr(&st_Output,sizeof(*st_Output)))
  {
    (*_printf) ("%s(%u): st_Output = 0x%08lX !!??\n", __FILE__, __LINE__, (DWORD)st_Output);
    return;
  }
#endif

  va_start (args, fmt);
  if (warn)
  {
    strcpy (pos, "\nFortify: ");
    pos += 10;
  }

#if defined(VSNPRINTF)
  VSNPRINTF (pos, sizeof(st_Buffer) - (pos - st_Buffer) - 1, fmt, args);
#else
  vsprintf (pos, fmt, args);
#endif
  (*st_Output) (st_Buffer);
}


/* statistics */
static DWORD  st_MaxBlocks        = 0;
static DWORD  st_MaxAllocation    = 0;
static DWORD  st_CurBlocks        = 0;
static DWORD  st_CurAllocation    = 0;
static DWORD  st_Allocations      = 0;
static DWORD  st_Frees            = 0;
static DWORD  st_TotalAllocation  = 0;
static DWORD  st_AllocationLimit  = 0xffffffff;

#ifdef FORTIFY_TRACK_DEALLOCATED_MEMORY
  static struct Header *st_DeallocatedHead  = 0;
  static struct Header *st_DeallocatedTail  = 0;
  static DWORD          st_TotalDeallocated = 0;
#endif


/* allocators */
static const char *st_AllocatorName[] = {
                  "malloc()",
                  "calloc()",
                  "realloc()",
                  "strdup()",
                  "new",
                  "new[]",
                  "GlobalAllocPtr()"
                };

/* deallocators */
static const char *st_DeallocatorName[] = {
                  "nobody",
                  "free()",
                  "realloc()",
                  "delete",
                  "delete[]",
                  "GlobalFreePtr()"
                };

static const BYTE st_ValidDeallocator[] = {
    (1 << Fortify_Deallocator_free) | (1 << Fortify_Deallocator_realloc),
    (1 << Fortify_Deallocator_free) | (1 << Fortify_Deallocator_realloc),
    (1 << Fortify_Deallocator_free) | (1 << Fortify_Deallocator_realloc),
    (1 << Fortify_Deallocator_free) | (1 << Fortify_Deallocator_realloc),
#if defined(FORTIFY_PROVIDE_ARRAY_NEW) && defined(FORTIFY_PROVIDE_ARRAY_DELETE)
    (1 << Fortify_Deallocator_delete),
    (1 << Fortify_Deallocator_array_delete)
#else
    (1 << Fortify_Deallocator_delete) | (1 << Fortify_Deallocator_array_delete),
    (1 << Fortify_Deallocator_delete) | (1 << Fortify_Deallocator_array_delete)
#endif
};


#if defined(__HIGHC__) && defined(__MSDOS__)
  #define FORTIFY_LOCK()     /* Fortify_LockLocalData(1) */ /**< \todo data locking */
  #define FORTIFY_UNLOCK()   /* Fortify_LockLocalData(0) */
#elif defined(WIN32)
  #define FORTIFY_LOCK()     EnterCriticalSection (&_watt_crit_sect)
  #define FORTIFY_UNLOCK()   LeaveCriticalSection (&_watt_crit_sect)
#else
  #define FORTIFY_LOCK()
  #define FORTIFY_UNLOCK()
#endif


#if defined(__HIGHC__)
  #if 0  /* !! not yet */
  #include <pharlap.h>
  #include <hw386.h>

  static char st_LockDataEnd = 0;

  /*
   *  Change page-attributes for local data.
   *  Set to present or non-present pages.
   */
  static Fortify_LockLocalData (int lock)
  {
    UINT  lockSize  = (ULONG)&st_LockDataEnd - (ULONG)&st_LockDataStart;
    ULONG num_pages = (lockSize + 4095) / 4096;
    ULONG page      = ((ULONG)&st_LockDataStart + 4095) / 4096;

    for ( ; page < page + num_pages; page++)
    {
      ULONG pte, ptInfo;
      if (_dx_rd_ptinfl (page << 12, &pte, &ptInfo))
         break;
      if (lock)
           pte &= ~PE_PRESENT;
      else pte |=  PE_PRESENT;
      if (_dx_wr_ptinfl (page << 12, pte, ptInfo))
         break;
    }
  }
  #endif  /* 0 */

  #pragma data (common,"?_MWHEAP");

  BYTE Check_heap_integrity_flag;  /* must be public */
  BYTE Heap_init_byte;
  BYTE Init_allocated_storage;

  #pragma data;

  static void heap_integrity_check (BOOL on)
  {
    if (on)
    {
      Check_heap_integrity_flag = Init_allocated_storage = 1;
      Heap_init_byte = 0xFE;
    }
    else
      Check_heap_integrity_flag = Init_allocated_storage = 0;
  }

  #define HEAP_INTEGRITY_CHECK(x)  heap_integrity_check(x)

#else

  #define HEAP_INTEGRITY_CHECK(x)  ((void)0)
#endif  /* __HIGHC__ */



/*
 * Fortify_Allocate() - allocate a block of fortified memory
 */
void *FORTIFY_STORAGE
Fortify_Allocate (size_t size, unsigned char allocator, unsigned long flags,
                  const char *file, DWORD line)
{
  struct Header *h;
  int    another_try;
  BYTE  *ptr = NULL;

  /* If Fortify has been disabled, then it's easy
   */
  if (st_Disabled)
  {
#ifdef FORTIFY_FAIL_ON_ZERO_MALLOC
    if (size == 0 &&
        (allocator == Fortify_Allocator_new ||
         allocator == Fortify_Allocator_array_new))
    {
      /* A new of zero bytes must succeed, but a malloc of
       * zero bytes probably won't.
       */
      return malloc (1);
    }
#endif
#ifdef WIN32
    if (allocator == Fortify_Allocator_GlobalAlloc)
       return GlobalAlloc (flags, size);
#endif
    return malloc (size);
  }

  if (allocator != Fortify_Allocator_GlobalAlloc)
  {
#ifdef FORTIFY_CHECK_ALL_MEMORY_ON_ALLOCATE
    HEAP_INTEGRITY_CHECK (TRUE);
    Fortify_CheckAllMemory (file, line);
#endif
  }

  if (st_AllocateFailRate > 0)
  {
    if ((rand() % 100) < st_AllocateFailRate)
    {
#ifdef FORTIFY_WARN_ON_FALSE_FAIL
      message (1, "A \"%s\" of %lu bytes failed at %s.%lu\n",
               st_AllocatorName[allocator], (DWORD)size, file, line);
#endif
      return (NULL);
    }
  }

  /* Check to see if this allocation will
   * push us over the artificial limit
   */
  if (st_CurAllocation + size > st_AllocationLimit)
  {
#ifdef FORTIFY_WARN_ON_FALSE_FAIL
    message (1, "A \"%s\" of %lu bytes \"false failed\" at %s.%lu\n",
             st_AllocatorName[allocator], (DWORD)size, file, line);
#endif
    return (0);
  }

#ifdef FORTIFY_WARN_ON_ZERO_MALLOC
  if (size == 0 && (allocator == Fortify_Allocator_malloc ||
                    allocator == Fortify_Allocator_calloc ||
                    allocator == Fortify_Allocator_realloc))
    message (1, "A \"%s\" of 0 bytes attempted at %s.%lu\n",
             st_AllocatorName[allocator], file, line);
#endif

#ifdef FORTIFY_FAIL_ON_ZERO_MALLOC
  if (size == 0 && (allocator == Fortify_Allocator_malloc ||
                    allocator == Fortify_Allocator_calloc ||
                    allocator == Fortify_Allocator_realloc))
  {
#ifdef FORTIFY_WARN_ON_ALLOCATE_FAIL
    message (1, "A \"%s\" of %lu bytes failed at %s.%lu\n",
             st_AllocatorName[allocator], (DWORD)size, file, line);
#endif
    return (NULL);
  }
#endif

#ifdef FORTIFY_WARN_ON_SIZE_T_OVERFLOW
  /*
   * Ensure the size of the memory block plus the overhead isn't
   * bigger than size_t (that'd be a drag)
   */
  {
    size_t private_size = FORTIFY_HEADER_SIZE +
                          FORTIFY_ALIGNED_BEFORE_SIZE + size +
                          FORTIFY_AFTER_SIZE;

    if (private_size < size)
    {
      message (1, "A \"%s\" of %lu bytes has overflowed size_t at %s.%lu\n",
               st_AllocatorName[allocator], (DWORD)size, file, line);
      return (0);
    }
#if (DOSX) && defined(__WATCOMC__) && 0
    if (private_size >= 65536UL)
    {
      message (1, "A \"%s\" of %lu bytes has exceeded 64kB limit by %d bytes, "
               "at %s.%lu\n", st_AllocatorName[allocator], (DWORD)size,
               private_size - size, file, line);
      return (NULL);
    }
#endif
  }
#endif

  another_try = 1;
  do
  {
    /* malloc the memory, including the space
     * for the header and fortification buffers
     */
    ptr = malloc (FORTIFY_HEADER_SIZE + FORTIFY_ALIGNED_BEFORE_SIZE +
                  size + FORTIFY_AFTER_SIZE);

#ifdef FORTIFY_TRACK_DEALLOCATED_MEMORY
    /* If we're tracking deallocated memory, then we can free some of
     * it, rather than let this malloc fail
     */
    if (!ptr)
       another_try = st_PurgeDeallocatedBlocks (size, file, line);

#endif /* FORTIFY_TRACK_DEALLOCATED_MEMORY */
  }
  while (!ptr && another_try);

  if (!ptr)
  {
#ifdef FORTIFY_WARN_ON_ALLOCATE_FAIL
    message (1, "A \"%s\" of %lu bytes failed at %s.%lu\n",
             st_AllocatorName[allocator], (DWORD)size, file, line);
#endif
    return (NULL);
  }

  /* Begin Critical Region
   */
  FORTIFY_LOCK();

  /* Make the head's prev pointer point to us
   * ('cos we're about to become the head)
   */
  if (st_AllocatedHead)
  {
    st_CheckBlock (st_AllocatedHead, file, line);
    /* what should we do if this fails? (apart from panic) */

    st_AllocatedHead->Prev = (struct Header*) ptr;
    st_MakeHeaderValid (st_AllocatedHead);
  }

  /* Initialize and validate the header
   */
  h = (struct Header *)ptr;
  h->Size      = size;
  h->File      = file;
  h->Line      = line;
  h->Next      = st_AllocatedHead;
  h->Prev      = 0;
  h->Scope     = st_Scope;
  h->Allocator = allocator;
  h->Label     = 0;
#ifdef FORTIFY_TRACK_DEALLOCATED_MEMORY
  h->FreedFile = 0;
  h->FreedLine = 0;
  h->Deallocator = Fortify_Deallocator_nobody;
#endif
  st_MakeHeaderValid (h);
  st_AllocatedHead = h;

  /* Initialize the fortifications
   */
  st_SetFortification (ptr + FORTIFY_HEADER_SIZE,
                       FORTIFY_BEFORE_VALUE, FORTIFY_ALIGNED_BEFORE_SIZE);
  st_SetFortification (ptr + FORTIFY_HEADER_SIZE + FORTIFY_ALIGNED_BEFORE_SIZE +
                       size, FORTIFY_AFTER_VALUE, FORTIFY_AFTER_SIZE);

#ifdef FORTIFY_FILL_ON_ALLOCATE
  /* Fill the actual user memory
   */
  st_SetFortification (ptr + FORTIFY_HEADER_SIZE + FORTIFY_ALIGNED_BEFORE_SIZE,
                       FORTIFY_FILL_ON_ALLOCATE_VALUE, size);
#endif

  /* End Critical Region
   */
  FORTIFY_UNLOCK();

  /* update the statistics
   */
  st_TotalAllocation += size;
  st_Allocations++;
  st_CurBlocks++;
  st_CurAllocation += size;
  if (st_CurBlocks > st_MaxBlocks)
      st_MaxBlocks = st_CurBlocks;
  if (st_CurAllocation > st_MaxAllocation)
      st_MaxAllocation = st_CurAllocation;

  /* We return the address of the user's memory, not the start of the block,
   * which points to our magic cookies
   */
  ARGSUSED (flags);
  return (ptr + FORTIFY_HEADER_SIZE + FORTIFY_ALIGNED_BEFORE_SIZE);
}


/*
 * Fortify_Deallocate() - Free a block of memory allocated with Fortify_Allocate()
 */
void FORTIFY_STORAGE
Fortify_Deallocate (void *uptr, BYTE deallocator, const char *file, DWORD line)
{
  BYTE *ptr = (BYTE*) uptr - FORTIFY_HEADER_SIZE -
              FORTIFY_ALIGNED_BEFORE_SIZE;
  struct Header *h = (struct Header*) ptr;

#ifdef FORTIFY_CHECK_ALL_MEMORY_ON_DEALLOCATE
  HEAP_INTEGRITY_CHECK (TRUE);
  Fortify_CheckAllMemory(file, line);
#endif

  /* If Fortify has been disabled, then it's easy (well, almost)
   */
  if (st_Disabled)
  {
    /* there is a possibility that this memory block was allocated
     * when Fortify was enabled, so we must check the Allocated
     * list before we free it.
     */
    if (!st_IsOnAllocatedList(h))
    {
      free (uptr);
      return;
    }

    /* the block was allocated by Fortify, so we
     * gotta free it differently.
     */

    /* Begin critical region
     */
    FORTIFY_LOCK();

    /* Remove the block from the list
     */
    if (h->Prev)
         h->Prev->Next = h->Next;
    else st_AllocatedHead = h->Next;

    if (h->Next)
        h->Next->Prev = h->Prev;

    /* End Critical Region
     */
    FORTIFY_UNLOCK();

    /* actually free the memory
     */
    free (ptr);
    return;
  }

#ifdef FORTIFY_PARANOID_DEALLOCATE
  if (!st_IsOnAllocatedList(h))
  {
#ifdef FORTIFY_TRACK_DEALLOCATED_MEMORY
    if (st_IsOnDeallocatedList(h))
    {
      message (1, "\"%s\" twice of %s detected at %s.%lu\n",
               st_DeallocatorName[deallocator],
               st_MemoryBlockString(h), file, line);

      sprintf (st_Buffer, "         Memory block was deallocated by \"%s\" at %s.%lu\n",
               st_DeallocatorName[h->Deallocator], h->FreedFile, h->FreedLine);
      ST_OUTPUT (st_Buffer);
      st_OutputDeleteTrace();
      return;
    }
#endif

    message (1, "Possible \"%s\" twice of (%08lX) was detected at %s.%lu\n",
             st_DeallocatorName[deallocator], (DWORD_PTR)uptr, file, line);

    st_OutputDeleteTrace();
    return;
  }
#endif /* FORTIFY_PARANOID_DEALLOCATE */

  /*
   * Make sure the block is okay before we free it.
   * If it's not okay, don't free it - it might not
   * be a real memory block. Or worse still, someone
   * might still be writing to it
   */
  if (!st_CheckBlock(h, file, line))
  {
    st_OutputDeleteTrace();
    return;
  }

#ifdef FORTIFY_TRACK_DEALLOCATED_MEMORY
  /*
   * Make sure the block hasn't been freed already
   * (we can get to here if FORTIFY_PARANOID_DELETE
   * is off, but FORTIFY_TRACK_DEALLOCATED_MEMORY
   * is on).
   */
  if (h->Deallocator != Fortify_Deallocator_nobody)
  {
    message (1, "\"%s\" twice of %s detected at %s.%lu\n",
             st_DeallocatorName[deallocator], st_MemoryBlockString(h),
             file, line);

    sprintf (st_Buffer, "         Memory block was deallocated by \"%s\" at %s.%lu\n",
             st_DeallocatorName[h->Deallocator], h->FreedFile, h->FreedLine);
    ST_OUTPUT (st_Buffer);
    st_OutputDeleteTrace();
    return;
  }
#endif

  /* Make sure the block is being freed with a valid
   * deallocator. If not, complain. (but free it anyway)
   */
  if ((st_ValidDeallocator[h->Allocator] & (1 << deallocator)) == 0)
  {
    message (1, "Incorrect deallocator \"%s\" detected at %s.%lu\n",
             st_DeallocatorName[deallocator], file, line);
    sprintf (st_Buffer,   "         %s was allocated with \"%s\"\n",
             st_MemoryBlockString(h), st_AllocatorName[h->Allocator]);
    ST_OUTPUT (st_Buffer);
    st_OutputDeleteTrace();
  }

  /* Begin critical region
   */
  FORTIFY_LOCK();

  /* Remove the block from the list
   */
  if (h->Prev)
  {
    if (!st_CheckBlock(h->Prev, file, line))
    {
      FORTIFY_UNLOCK();
      st_OutputDeleteTrace();
      return;
    }
    h->Prev->Next = h->Next;
    st_MakeHeaderValid (h->Prev);
  }
  else
    st_AllocatedHead = h->Next;

  if (h->Next)
  {
    if (!st_CheckBlock(h->Next, file, line))
    {
      FORTIFY_UNLOCK();
      st_OutputDeleteTrace();
      return;
    }
    h->Next->Prev = h->Prev;
    st_MakeHeaderValid (h->Next);
  }

  /* End Critical Region
   */
  FORTIFY_UNLOCK();

  /* update the statistics
   */
  st_Frees++;
  st_CurBlocks--;
  st_CurAllocation -= h->Size;

#ifdef FORTIFY_TRACK_DEALLOCATED_MEMORY
  if (st_Scope > 0)
  {
    /*
     * Don't _actually_ free the memory block, just yet.
     * Place it onto the deallocated list, instead, so
     * we can check later to see if it's been written to.
     */
  #ifdef FORTIFY_FILL_ON_DEALLOCATE
    /*
     * Nuke out all user memory that is about to be freed
     */
    st_SetFortification (ptr + FORTIFY_HEADER_SIZE + FORTIFY_ALIGNED_BEFORE_SIZE,
                         FORTIFY_FILL_ON_DEALLOCATE_VALUE, h->Size);
  #endif

    /* Begin critical region
     */
    FORTIFY_LOCK();

    /* Place the block on the deallocated list
     */
    if (st_DeallocatedHead)
    {
      st_DeallocatedHead->Prev = (struct Header*) ptr;
      st_MakeHeaderValid (st_DeallocatedHead);
    }

    h = (struct Header *)ptr;
    h->FreedFile   = file;
    h->FreedLine   = line;
    h->Deallocator = deallocator;
    h->Next        = st_DeallocatedHead;
    h->Prev        = 0;
    st_MakeHeaderValid (h);
    st_DeallocatedHead = h;

    if (!st_DeallocatedTail)
       st_DeallocatedTail = h;

    st_TotalDeallocated += h->Size;

  #ifdef FORTIFY_DEALLOCATED_MEMORY_LIMIT
    /*
     * If we've got too much on the deallocated list; free some
     */
    if (st_TotalDeallocated > FORTIFY_DEALLOCATED_MEMORY_LIMIT)
       st_PurgeDeallocatedBlocks (st_TotalDeallocated -
                                  FORTIFY_DEALLOCATED_MEMORY_LIMIT, file, line);
  #endif

    /* End critical region
     */
    FORTIFY_UNLOCK();
  }
  else
#endif /* FORTIFY_TRACK_DEALLOCATED_MEMORY */
  {
    /* Free the User Label
     */
    if (h->Label)
       free(h->Label);

#ifdef FORTIFY_FILL_ON_DEALLOCATE
    /* Nuke out all memory that is about to be freed, including the header
     */
    st_SetFortification (ptr, FORTIFY_FILL_ON_DEALLOCATE_VALUE,
                         FORTIFY_HEADER_SIZE + FORTIFY_ALIGNED_BEFORE_SIZE +
                         h->Size + FORTIFY_AFTER_SIZE);
#endif
    /* And do the actual free
     */
    free(ptr);
  }
}


/*
 * Fortify_LabelPointer() - Labels the memory block
 * with a string provided by the user. This function
 * takes a copy of the passed in string.
 * The pointer MUST be one returned by a Fortify
 * allocation function.
 */
void
Fortify_LabelPointer (void *uptr, const char *label, const char *file, DWORD line)
{
  if (!st_Disabled)
  {
    BYTE *ptr = (BYTE*) uptr - FORTIFY_HEADER_SIZE -
                FORTIFY_ALIGNED_BEFORE_SIZE;
    struct Header *h = (struct Header*) ptr;

    /* make sure the pointer is okay
     */
    Fortify_CheckPointer (uptr, file, line);

    /* free the previous label
     */
    if (h->Label)
       free (h->Label);

    h->Label = label ? strdup (label) : NULL;

    /* update the checksum
     */
    st_MakeHeaderValid (h);
  }
}

/*
 * Fortify_CheckPointer() - Returns true if the uptr
 * points to a valid piece of Fortify_Allocated()'d
 * memory. The memory must be on the allocated list,
 * and it's fortifications must be intact.
 * Always returns TRUE if Fortify is disabled.
 */
int FORTIFY_STORAGE
Fortify_CheckPointer (void *uptr, const char *file, DWORD line)
{
  BYTE *ptr = (BYTE*) uptr - FORTIFY_HEADER_SIZE -
              FORTIFY_ALIGNED_BEFORE_SIZE;
  struct Header *h = (struct Header*) ptr;
  int    r;

  if (st_Disabled)
     return (1);

  FORTIFY_LOCK();

  if (!st_IsOnAllocatedList(h))
  {
    message (1, "Invalid pointer (%08lX) detected at %s.%lu\n",
             (DWORD_PTR)uptr, file, line);
    FORTIFY_UNLOCK();
    return (0);
  }

#ifdef FORTIFY_TRACK_DEALLOCATED_MEMORY
  if (st_IsOnDeallocatedList(h))
  {
    message (1, "Deallocated pointer (%08lX) detected at %s.%lu\n",
             (DWORD_PTR)uptr, file, line);

    sprintf (st_Buffer, "         Memory block was deallocated by \"%s\" at %s.%lu\n",
             st_DeallocatorName[h->Deallocator], h->FreedFile, h->FreedLine);
    ST_OUTPUT (st_Buffer);
    FORTIFY_UNLOCK();
    return (0);
  }
#endif

  r = st_CheckBlock (h, file, line);
  FORTIFY_UNLOCK();
  return (r);
}

/*
 * Fortify_SetOutputFunc (Fortify_OutputFuncPtr Output) -
 * Sets the function used to output all error and
 * diagnostic messages. The output function  takes
 * a single const BYTE * argument, and must be
 * able to handle newlines. This function returns the
 * old output function.
 */
Fortify_OutputFuncPtr FORTIFY_STORAGE
Fortify_SetOutputFunc (Fortify_OutputFuncPtr Output)
{
  Fortify_OutputFuncPtr Old = st_Output;

  st_Output = Output;
  return (Old);
}

/*
 * Fortify_SetAllocateFailRate(int Percent) -
 * Fortify_Allocate() will "fail" this Percent of
 * the time, even if the memory is available.
 * Useful to "stress-test" an application.
 * Returns the old value.
 * The fail rate defaults to 0 (a good default I think).
 */
int FORTIFY_STORAGE Fortify_SetAllocateFailRate (int Percent)
{
  int Old = st_AllocateFailRate;

  Percent = max (0, Percent);
  Percent = min (100, Percent);
  st_AllocateFailRate = Percent;
  return (Old);
}

/*
 * Fortify_CheckAllMemory() - Checks the fortifications
 * of all memory on the allocated list. And, if
 * FORTIFY_DEALLOCATED_MEMORY is enabled, all the
 * known deallocated memory as well.
 * Returns the number of blocks that failed.
 * Always returns 0 if Fortify is disabled.
 */
DWORD FORTIFY_STORAGE Fortify_CheckAllMemory (const char *file, DWORD line)
{
  struct Header *curr = st_AllocatedHead;
  DWORD  count = 0;

  if (st_Disabled)
     return (DWORD)-1;

  FORTIFY_LOCK();

  /* Check the allocated memory
   */
  while (curr)
  {
    if (!st_CheckBlock(curr, file, line))
    {
      count++;
      break;    /* !! added, GV 13/7-98 */
    }
    curr = curr->Next;
  }

  /* Check the deallocated memory while you're at it
   */
#ifdef FORTIFY_TRACK_DEALLOCATED_MEMORY
  curr = st_DeallocatedHead;
  while (curr)
  {
    if (!st_CheckDeallocatedBlock(curr, file, line))
    {
      count++;
      break;    /* !! added, GV 13/7-98 */
    }
    curr = curr->Next;
  }
#endif

  /* If we know where we are, and everything is cool,
   * remember that. It might be important.
   */
  if (file && count == 0)
  {
    st_LastVerifiedFile = file;
    st_LastVerifiedLine = line;
  }
  FORTIFY_UNLOCK();
  return (count);
}


/*
 * Fortify_EnterScope() - enters a new Fortify scope
 * level. Returns the new scope level.
 */
BYTE FORTIFY_STORAGE Fortify_EnterScope (const char *file, DWORD line)
{
  message (1, "Entering new scope %d at %s.%lu\n", st_Scope+1, file, line);
  ST_OUTPUT (st_Buffer);
  return (++st_Scope);
}

/* Fortify_LeaveScope - leaves a Fortify scope level,
 * also prints a memory dump of all non-freed memory
 * that was allocated during the scope being exited.
 * Does nothing and returns 0 if Fortify is disabled.
 */
BYTE FORTIFY_STORAGE Fortify_LeaveScope (const char *file, DWORD line)
{
  struct Header *curr = st_AllocatedHead;
  DWORD size = 0, count = 0;

  if (st_Disabled)
     return (0);

  FORTIFY_LOCK();

  st_Scope--;
  while (curr)
  {
    if (curr->Scope > st_Scope)
    {
      if (count == 0)
      {
        message (1, "Memory leak detected leaving scope at %s.%lu\n",
                 file, line);
        sprintf (st_Buffer, "%8s %8s %s\n", "Address", "Size", "Allocator");
        ST_OUTPUT (st_Buffer);
      }
      if (!st_OutputHeader(curr))
         break;
      count++;
      size += curr->Size;
    }
    curr = curr->Next;
  }

  if (count)
  {
    sprintf (st_Buffer,"%8s %8lu bytes in %lu blocks with %lu bytes overhead\n",
             "total", size, count, count * FORTIFY_OVERHEAD);
    ST_OUTPUT (st_Buffer);
  }

#ifdef FORTIFY_TRACK_DEALLOCATED_MEMORY
  /*
   * Quietly free all the deallocated memory
   * that was allocated in this scope that
   * we are still tracking
   */
  st_PurgeDeallocatedScope (st_Scope, file, line);
#endif

  FORTIFY_UNLOCK();
  return (st_Scope);
}

/*
 * Fortify_ListAllMemory() - Outputs the entire
 * list of currently allocated memory. For each block
 * is output it's Address, Size, and the SourceFile and
 * Line that allocated it.
 *
 * If there is no memory on the list, this function
 * outputs nothing.
 *
 * It returns the number of blocks on the list, unless
 * Fortify has been disabled, in which case it always
 * returns 0.
 */
DWORD FORTIFY_STORAGE Fortify_ListAllMemory (const char *file, DWORD line)
{
  struct Header *curr = st_AllocatedHead;
  DWORD  size = 0, count = 0;

  if (st_Disabled)
  {
    ST_OUTPUT ("Fortify: disabled\n");
    return (0);
  }

  Fortify_CheckAllMemory (file, line);

  FORTIFY_LOCK();

  if (curr)
  {
    sprintf (st_Buffer, "\n\nFortify: Memory List at %s.%lu\n", file, line);
    ST_OUTPUT (st_Buffer);
    sprintf (st_Buffer, "%8s %8s %s\n", "Address", "Size", "Allocator");
    ST_OUTPUT (st_Buffer);

    while (curr)
    {
      if (!st_OutputHeader (curr))
         break;
      count++;
      size += curr->Size;
      curr = curr->Next;
    }
    sprintf (st_Buffer, "%8s %8lu bytes in %lu blocks and %lu bytes overhead\n",
             "total", size, count, count * FORTIFY_OVERHEAD);
    ST_OUTPUT (st_Buffer);
  }
  FORTIFY_UNLOCK();
  return (count);
}

/*
 * Fortify_DumpAllMemory() - Outputs the entire list of
 * currently allocated memory. For each allocated block
 * is output it's Address, Size, the SourceFile and Line
 * that allocated it, a hex dump of the contents of the
 * memory and an ascii dump of printable characters.
 *
 * If there is no memory on the list, this function outputs nothing.
 */
DWORD FORTIFY_STORAGE Fortify_DumpAllMemory (const char *file, DWORD line)
{
  struct Header *curr = st_AllocatedHead;
  DWORD  count = 0;

  if (st_Disabled)
     return (0);

  Fortify_CheckAllMemory (file, line);

  FORTIFY_LOCK();

  while (curr)
  {
    message (1, "Hex Dump of %s at %s.%lu\n",
             st_MemoryBlockString(curr), file, line);
    st_OutputMemory (curr);
    ST_OUTPUT ("\n");
    count++;
    curr = curr->Next;
  }
  FORTIFY_UNLOCK();
  return (count);
}

/* Fortify_OutputStatistics() - displays statistics
 * about the maximum amount of memory that was
 * allocated at any one time.
 */
void FORTIFY_STORAGE Fortify_OutputStatistics (const char *file, DWORD line)
{
  if (st_Disabled)
     return;

  message (1, "Statistics at %s.%lu\n", file, line);

  sprintf (st_Buffer, "         Memory currently allocated: %lu bytes in %lu blocks\n",
           st_CurAllocation, st_CurBlocks);
  ST_OUTPUT (st_Buffer);

  sprintf (st_Buffer, "         Maximum memory allocated at one time: %lu bytes in %lu blocks\n",
           st_MaxAllocation, st_MaxBlocks);
  ST_OUTPUT (st_Buffer);

  sprintf (st_Buffer, "         There have been %lu allocations and %lu deallocations\n",
           st_Allocations, st_Frees);
  ST_OUTPUT (st_Buffer);

  sprintf (st_Buffer, "         There was a total of %lu bytes allocated\n",
           st_TotalAllocation);
  ST_OUTPUT (st_Buffer);

  if (st_Allocations > 0)
  {
    sprintf (st_Buffer, "         The average allocation was %lu bytes\n",
             st_TotalAllocation / st_Allocations);
    ST_OUTPUT (st_Buffer);
  }
}

/* Fortify_GetCurrentAllocation() - returns the number of
 * bytes currently allocated.
 */
DWORD FORTIFY_STORAGE Fortify_GetCurrentAllocation (const char *file, DWORD line)
{
  ARGSUSED (file);
  ARGSUSED (line);

  if (st_Disabled)
     return (0);

  return (st_CurAllocation);
}

/* Fortify_SetAllocationLimit() - set a limit on the total
 * amount of memory allowed for this application.
 */
void FORTIFY_STORAGE
Fortify_SetAllocationLimit (DWORD NewLimit, const char *file, DWORD line)
{
  ARGSUSED (file);
  ARGSUSED (line);
  st_AllocationLimit = NewLimit;
}

/*
 * Fortify_Disable() - Run time method of disabling Fortify.
 * Useful if you need to turn off Fortify without recompiling
 * everything. Not as effective as compiling out, of course.
 * The less memory allocated by Fortify when it is disabled
 * the better.
 * (Previous versions of Fortify did not allow it to be
 * disabled if there was any memory allocated at the time,
 * but since in C++ memory is often allocated before main
 * is even entered, this was useless so Fortify is now
 * able to cope).
 */
void FORTIFY_STORAGE Fortify_Disable (const char *file, DWORD line)
{
#ifdef FORTIFY_TRACK_DEALLOCATED_MEMORY
  /* free all deallocated memory we might be tracking */
  if (file && line)
     st_PurgeDeallocatedScope (0, file, line);
#endif
  ARGSUSED (file);
  ARGSUSED (line);
  st_Disabled = 1;
}

void FORTIFY_STORAGE Fortify_Enable (const char *file, DWORD line)
{
  ARGSUSED (file);
  ARGSUSED (line);
  st_Disabled = 0;
}

/*
 * st_CheckBlock - Check a block's header and fortifications.
 * Returns true if the block is happy.
 */
static int st_CheckBlock (struct Header *h, const char *file, DWORD line)
{
  BYTE *ptr = (BYTE *)h;
  int  result = 1;

  if (!st_IsHeaderValid(h))
  {
    message (1, "Invalid pointer (%08lX) or corrupted header detected at %s.%lu\n",
             (DWORD_PTR)(ptr + FORTIFY_HEADER_SIZE + FORTIFY_ALIGNED_BEFORE_SIZE),
             file, line);
//  st_MakeHeaderValid (h);             // !!
    st_OutputLastVerifiedPoint();
    return (0);
  }

  if (!st_CheckFortification(ptr + FORTIFY_HEADER_SIZE,
                             FORTIFY_BEFORE_VALUE, FORTIFY_ALIGNED_BEFORE_SIZE))
  {
    message (1, "Underwrite detected before block %s at %s.%lu\n",
             st_MemoryBlockString(h), file, line);

    st_OutputLastVerifiedPoint();
    st_OutputFortification (ptr + FORTIFY_HEADER_SIZE,
                            FORTIFY_BEFORE_VALUE, FORTIFY_ALIGNED_BEFORE_SIZE);
    result = 0;

#ifdef FORTIFY_FILL_ON_CORRUPTION
    st_SetFortification (ptr + FORTIFY_HEADER_SIZE, FORTIFY_BEFORE_VALUE,
                         FORTIFY_ALIGNED_BEFORE_SIZE);
#endif
  }

  if (!st_CheckFortification(ptr + FORTIFY_HEADER_SIZE + FORTIFY_ALIGNED_BEFORE_SIZE +
                             h->Size, FORTIFY_AFTER_VALUE, FORTIFY_AFTER_SIZE))
  {
    message (1, "Overwrite detected after block %s at %s.%lu\n",
             st_MemoryBlockString(h), file, line);

    st_OutputLastVerifiedPoint();
    st_OutputFortification (ptr + FORTIFY_HEADER_SIZE + FORTIFY_ALIGNED_BEFORE_SIZE +
                            h->Size, FORTIFY_AFTER_VALUE, FORTIFY_AFTER_SIZE);
    result = 0;

#ifdef FORTIFY_FILL_ON_CORRUPTION
    st_SetFortification (ptr + FORTIFY_HEADER_SIZE + FORTIFY_ALIGNED_BEFORE_SIZE +
                         h->Size, FORTIFY_AFTER_VALUE, FORTIFY_AFTER_SIZE);
#endif
  }
  return (result);
}

#ifdef FORTIFY_TRACK_DEALLOCATED_MEMORY

/*
 * st_CheckDeallocatedBlock - Check a deallocated block's header and
 * fortifications. Returns true if the block is happy.
 */
static int
st_CheckDeallocatedBlock (struct Header *h, const char *file, DWORD line)
{
  BYTE *ptr = (BYTE *)h;
  int result = 1;

  if (!st_IsHeaderValid(h))
  {
    message (1, "Invalid deallocated pointer (%08lX) or corrupted "
                "header detected at %s.%lu\n",
             (DWORD_PTR)(ptr + FORTIFY_HEADER_SIZE +
                         FORTIFY_ALIGNED_BEFORE_SIZE),
             file, line);
    st_OutputLastVerifiedPoint();
    return (0);
  }

  if (!st_CheckFortification(ptr + FORTIFY_HEADER_SIZE,
          FORTIFY_BEFORE_VALUE, FORTIFY_ALIGNED_BEFORE_SIZE))
  {
    message (1, "Underwrite detected before deallocated block %s at %s.%lu\n",
             st_MemoryBlockString(h), file, line);

    sprintf (st_Buffer, "         Memory block was deallocated by \"%s\" "
                        "at %s.%lu\n",
             st_DeallocatorName[h->Deallocator], h->FreedFile, h->FreedLine);
    ST_OUTPUT (st_Buffer);

    st_OutputLastVerifiedPoint();
    st_OutputFortification (ptr + FORTIFY_HEADER_SIZE,
                            FORTIFY_BEFORE_VALUE, FORTIFY_ALIGNED_BEFORE_SIZE);

#ifdef FORTIFY_FILL_ON_CORRUPTION
    st_SetFortification (ptr + FORTIFY_HEADER_SIZE, FORTIFY_BEFORE_VALUE,
                         FORTIFY_ALIGNED_BEFORE_SIZE);
#endif
    result = 0;
  }

  if (!st_CheckFortification(ptr + FORTIFY_HEADER_SIZE +
                             FORTIFY_ALIGNED_BEFORE_SIZE + h->Size,
                             FORTIFY_AFTER_VALUE, FORTIFY_AFTER_SIZE))
  {
    message (1, "Overwrite detected after deallocated block %s at %s.%lu\n",
             st_MemoryBlockString(h), file, line);

    sprintf (st_Buffer, "         Memory block was deallocated by \"%s\" "
                        "at %s.%lu\n",
             st_DeallocatorName[h->Deallocator], h->FreedFile, h->FreedLine);
    ST_OUTPUT (st_Buffer);

    st_OutputLastVerifiedPoint();
    st_OutputFortification (ptr + FORTIFY_HEADER_SIZE + FORTIFY_ALIGNED_BEFORE_SIZE +
                            h->Size, FORTIFY_AFTER_VALUE, FORTIFY_AFTER_SIZE);

#ifdef FORTIFY_FILL_ON_CORRUPTION
    st_SetFortification (ptr + FORTIFY_HEADER_SIZE + FORTIFY_ALIGNED_BEFORE_SIZE +
                         h->Size, FORTIFY_AFTER_VALUE, FORTIFY_AFTER_SIZE);
#endif
    result = 0;
  }

#ifdef FORTIFY_FILL_ON_DEALLOCATE
  if (!st_CheckFortification (ptr + FORTIFY_HEADER_SIZE + FORTIFY_ALIGNED_BEFORE_SIZE,
                             FORTIFY_FILL_ON_DEALLOCATE_VALUE, h->Size))
  {
    message (1, "Write to deallocated block %s detected at %s.%lu\n",
             st_MemoryBlockString(h), file, line);

    sprintf (st_Buffer, "         Memory block was deallocated by \"%s\" at %s.%lu\n",
             st_DeallocatorName[h->Deallocator], h->FreedFile, h->FreedLine);
    ST_OUTPUT (st_Buffer);
    st_OutputLastVerifiedPoint();

    st_OutputFortification (ptr + FORTIFY_HEADER_SIZE + FORTIFY_ALIGNED_BEFORE_SIZE,
                            FORTIFY_FILL_ON_DEALLOCATE_VALUE, h->Size);

#ifdef FORTIFY_FILL_ON_CORRUPTION
    st_SetFortification (ptr + FORTIFY_HEADER_SIZE + FORTIFY_ALIGNED_BEFORE_SIZE,
                         FORTIFY_FILL_ON_DEALLOCATE_VALUE, h->Size);
#endif
    result = 0;
  }
#endif
  return (result);
}
#endif /* FORTIFY_TRACK_DEALLOCATED_MEMORY */


/*
 * st_CheckFortification - Checks if the _size_
 * bytes from _ptr_ are all set to _value_
 * Returns true if all is happy.
 */
static int
st_CheckFortification (BYTE *ptr, BYTE value, size_t size)
{
  while (size--)
        if (*ptr++ != value)
           return (0);
  return (1);
}

/*
 * st_SetFortification - Set the _size_ bytes from _ptr_ to _value_.
 */
static void
st_SetFortification (BYTE *ptr, BYTE value, size_t size)
{
  memset (ptr, value, size);
}

/*
 * st_OutputFortification - Output the corrupted section of the fortification
 */
static void
st_OutputFortification (BYTE *ptr, BYTE value, size_t size)
{
  size_t offset = 0, skipped, advance;

  sprintf (st_Buffer, "   Address   Offset Data (%02x)", value);
  ST_OUTPUT (st_Buffer);

  while (offset < size)
  {
    /* Skip 3 or more 'correct' lines
     */
    if ((size - offset) < 3 * 16)
         advance = size - offset;
    else advance = 3 * 16;

    if (advance > 0 && st_CheckFortification(ptr+offset, value, advance))
    {
      offset += advance;
      skipped = advance;

      if (size - offset < 16)
           advance = size - offset;
      else advance = 16;

      while (advance > 0 && st_CheckFortification(ptr+offset, value, advance))
      {
        offset  += advance;
        skipped += advance;
        if (size - offset < 16)
             advance = size - offset;
        else advance = 16;
      }
      sprintf (st_Buffer, "\n                        ...%lu bytes skipped...",
               (DWORD)skipped);
      ST_OUTPUT (st_Buffer);
      continue;
    }

    if (size - offset < 16)
         st_HexDump (ptr, offset, size-offset, 0);
    else st_HexDump (ptr, offset, 16, 0);
    offset += 16;
  }
  ST_OUTPUT ("\n");
}

/*
 * st_HexDump - output a nice hex dump of "size" bytes, starting at
 * "ptr" + "offset"
 */
static void st_HexDump (BYTE *ptr, size_t offset, size_t size, int title)
{
  char   ascii[17];
  int    column;
  size_t output;

  if (title)
     ST_OUTPUT ("   Address   Offset Data");

  column = 0;
  ptr += offset;
  output = 0;

  while (output < size)
  {
    if (column == 0)
       message (0, "\n%08lX %8lu ", (DWORD_PTR)ptr, (DWORD)offset);

    message (0, "%02x%s", *ptr, ((column % 4) == 3) ? " " : "");

    ascii [column]   = isprint(*ptr) ? (char)*ptr : '.';
    ascii [column+1] = '\0';

    ptr++;
    offset++;
    output++;
    column++;

    if (column == 16)
    {
      ST_OUTPUT ("   \"");
      ST_OUTPUT (ascii);
      ST_OUTPUT ("\"");
      column = 0;
    }
  }

  if (column != 0)
  {
    while  (column < 16)
    {
      if (column % 4 == 3)
           ST_OUTPUT ("   ");
      else ST_OUTPUT ("  ");
      column++;
    }
    ST_OUTPUT ("   \"");
    ST_OUTPUT (ascii);
    ST_OUTPUT ("\"");
  }
}

/*
 * st_IsHeaderValid - Returns true if the
 * supplied pointer does indeed point to a
 * real Header
 */
static int st_IsHeaderValid (const struct Header *h)
{
  return (st_ChecksumHeader(h) == FORTIFY_CHECKSUM_VALUE);
}

/*
 * st_MakeHeaderValid - Updates the checksum
 * to make the header valid
 */
static void st_MakeHeaderValid (struct Header *h)
{
  h->Checksum = 0;
  h->Checksum = (WORD)(FORTIFY_CHECKSUM_VALUE - st_ChecksumHeader(h));
}

/*
 * st_ChecksumHeader - Calculate (and return)
 * the checksum of the header. (Including the
 * Checksum field itself. If all is well, the
 * checksum returned by this function should
 * be FORTIFY_CHECKSUM_VALUE
 */
static WORD st_ChecksumHeader (const struct Header *h)
{
  WORD  c, checksum;
  WORD *p = (WORD*)h;

  for (c = checksum = 0; c < FORTIFY_HEADER_SIZE/sizeof(WORD); c++)
      checksum += (*p++) & 0xFF;
  return (checksum);
}

/*
 * st_IsOnAllocatedList - Examines the allocated
 * list to see if the given header is on it.
 */
static int st_IsOnAllocatedList (const struct Header *h)
{
  const struct Header *curr;

  for (curr = st_AllocatedHead; curr; curr = curr->Next)
      if (curr == h)
         return (1);
  return (0);
}

#ifdef FORTIFY_TRACK_DEALLOCATED_MEMORY
/*
 * st_IsOnDeallocatedList - Examines the deallocated
 * list to see if the given header is on it.
 */
static int st_IsOnDeallocatedList (const struct Header *h)
{
  struct Header *curr;

  for (curr = st_DeallocatedHead; curr; curr = curr->Next)
      if (curr == h)
         return (1);
  return (0);
}

/*
 * st_PurgeDeallocatedBlocks - free at least "Bytes"
 * worth of deallocated memory, starting at the
 * oldest deallocated block.
 * Returns true if any blocks were freed.
 */
static int st_PurgeDeallocatedBlocks (DWORD Bytes, const char *file, DWORD line)
{
  DWORD FreedBytes = 0;
  DWORD FreedBlocks = 0;

#ifdef FORTIFY_WARN_WHEN_DISCARDING_DEALLOCATED_MEMORY
  message (1, "Warning - Discarding deallocated memory at %s.%lu\n", file, line);
#endif

  while (st_DeallocatedTail && FreedBytes < Bytes)
  {
    st_CheckDeallocatedBlock (st_DeallocatedTail, file, line);
    FreedBytes += st_DeallocatedTail->Size;
    FreedBlocks++;

#ifdef FORTIFY_WARN_WHEN_DISCARDING_DEALLOCATED_MEMORY
#ifdef FORTIFY_VERBOSE_WARN_WHEN_DISCARDING_DEALLOCATED_MEMORY
    sprintf (st_Buffer, "                %s\n",
             st_DeallocatedMemoryBlockString(st_DeallocatedTail));
    ST_OUTPUT (st_Buffer);
#endif
#endif
    st_FreeDeallocatedBlock(st_DeallocatedTail, file, line);
  }
  return (FreedBlocks != 0);
}

/*
 * st_PurgeDeallocatedScope - free all deallocated
 * memory blocks that were allocated within "Scope"
 */
static int st_PurgeDeallocatedScope (BYTE Scope, const char *file, DWORD line)
{
  struct Header *curr = st_DeallocatedHead;
  struct Header *next;
  DWORD  FreedBlocks  = 0;

  while (curr)
  {
    next = curr->Next;
    if (curr->Scope >= Scope)
    {
      st_FreeDeallocatedBlock (curr, file, line);
      FreedBlocks++;
    }
    curr = next;
  }
  return (FreedBlocks != 0);
}

/*
 * st_FreeDeallocatedBlock - actually remove
 * a deallocated block from the deallocated
 * list, and actually free it's memory.
 */
static void st_FreeDeallocatedBlock (struct Header *h, const char *file, DWORD line)
{
  st_CheckDeallocatedBlock (h, file, line);

 /* Begin Critical region
  */
  FORTIFY_LOCK();

  st_TotalDeallocated -= h->Size;

  if (st_DeallocatedHead == h)
      st_DeallocatedHead = h->Next;

  if (st_DeallocatedTail == h)
      st_DeallocatedTail = h->Prev;

  if (h->Prev)
  {
    st_CheckDeallocatedBlock (h->Prev, file, line);
    h->Prev->Next = h->Next;
    st_MakeHeaderValid (h->Prev);
  }

  if (h->Next)
  {
    st_CheckDeallocatedBlock (h->Next, file, line);
    h->Next->Prev = h->Prev;
    st_MakeHeaderValid (h->Next);
  }

  /* Free the label
   */
  if (h->Label)
     free (h->Label);

  /* Nuke out all memory that is about to be freed, including the header
   */
  st_SetFortification ((BYTE*)h, FORTIFY_FILL_ON_DEALLOCATE_VALUE,
                       FORTIFY_HEADER_SIZE + FORTIFY_ALIGNED_BEFORE_SIZE +
                       h->Size + FORTIFY_AFTER_SIZE);

  /* And do the actual free
   */
  free (h);

  /* End critical region
   */
  FORTIFY_UNLOCK();
}
#endif /* FORTIFY_TRACK_DEALLOCATED_MEMORY */

/*
 * st_OutputMemory - Hex and ascii dump the
 * user memory of a block.
 */
static void st_OutputMemory (struct Header *h)
{
  int hlen = FORTIFY_HEADER_SIZE + FORTIFY_ALIGNED_BEFORE_SIZE;

  if (!VALIDATE (h, hlen, sprintf(st_Buffer, "Invalid header %p",h)))
       ST_OUTPUT (st_Buffer);
  else st_HexDump ((BYTE*)h + hlen, 0, h->Size, 1);
}


/*
 * st_OutputHeader - Output the header
 */
static int st_OutputHeader (struct Header *h)
{
  int   hlen = FORTIFY_HEADER_SIZE + FORTIFY_ALIGNED_BEFORE_SIZE;
  BYTE *addr = (BYTE*)h + hlen;
  char *p    = st_Buffer;

  if (!VALIDATE(h, hlen, sprintf(st_Buffer, "Invalid header %p",h)))
  {
    ST_OUTPUT (st_Buffer);
    return (0);
  }

  p += sprintf (p, "%08lX %8lu ", (DWORD_PTR)addr, (DWORD)h->Size);

  if (VALIDATE (h->File, 13, p += sprintf(p, "file?.")))
     p += sprintf (p, "%s.", h->File);

  p += sprintf (p, "%lu  ", h->Line);

  if (h->Label)
  {
    if (VALIDATE (h->Label, 50, p += sprintf(p, "label?")))
       p += sprintf (p, "%s", h->Label);
  }
  *p++ = '\n';
  *p = '\0';

  ST_OUTPUT (st_Buffer);
  return (1);
}

/*
 * st_OutputLastVerifiedPoint - output the last
 * known point where everything was hoopy.
 */
static void st_OutputLastVerifiedPoint (void)
{
  if (!st_Output)
     return;

#if defined(VSNPRINTF)
  SNPRINTF (st_Buffer, sizeof(st_Buffer),
#else
  sprintf (st_Buffer,
#endif
           "         Memory integrity was last verified at %s.%lu\n",
           st_LastVerifiedFile, st_LastVerifiedLine);
  (*st_Output) (st_Buffer);
}

/*
 * st_MemoryBlockString - constructs a string that
 * desribes a memory block. (pointer,size,allocator,label)
 */
static const char *st_MemoryBlockString (struct Header *h)
{
  static char st_BlockString[512];

  if (!VALIDATE (h, sizeof(*h),
                 sprintf (st_BlockString, "(invalid header %p)", h)))
     return (st_BlockString);

  if (h->Label == 0)
       sprintf (st_BlockString, "(%08lX,%lu,%s.%lu)",
                (DWORD_PTR)h + FORTIFY_HEADER_SIZE + FORTIFY_ALIGNED_BEFORE_SIZE,
                (DWORD)h->Size, h->File, h->Line);
  else sprintf (st_BlockString, "(%08lX,%lu,%s.%lu,%s)",
                (DWORD_PTR)h + FORTIFY_HEADER_SIZE + FORTIFY_ALIGNED_BEFORE_SIZE,
                (DWORD)h->Size, h->File, h->Line, h->Label);
  return (st_BlockString);
}

#ifdef FORTIFY_TRACK_DEALLOCATED_MEMORY
#ifdef FORTIFY_WARN_WHEN_DISCARDING_DEALLOCATED_MEMORY
#ifdef FORTIFY_VERBOSE_WARN_WHEN_DISCARDING_DEALLOCATED_MEMORY

/*
 * st_DeallocatedMemoryBlockString - constructs
 * a string that desribes a deallocated memory
 * block. (pointer,size,allocator,deallocator)
 */
static const char *st_DeallocatedMemoryBlockString (struct Header *h)
{
  static char st_BlockString[256];

  if (h->Label == 0)
       sprintf (st_BlockString,"(%08lX,%lu,%s.%lu,%s.%lu)",
                (DWORD)h + FORTIFY_HEADER_SIZE + FORTIFY_ALIGNED_BEFORE_SIZE,
                (DWORD)h->Size, h->File, h->Line, h->FreedFile, h->FreedLine);
  else sprintf (st_BlockString,"(%08lX,%lu,%s.%lu,%s.%lu,%s)",
                (DWORD)h + FORTIFY_HEADER_SIZE + FORTIFY_ALIGNED_BEFORE_SIZE,
                (DWORD)h->Size, h->File, h->Line, h->FreedFile, h->FreedLine,
                h->Label);

  return (st_BlockString);
}
#endif
#endif
#endif


/*
 * st_DefaultOutput - the default output function
 */
static void st_DefaultOutput (const char *String)
{
  fprintf (stdout, String);
  fflush (stdout);
}

/*
 * Fortify_malloc - Fortify's replacement malloc()
 */
void *FORTIFY_STORAGE
Fortify_malloc (size_t size, const char *file, DWORD line)
{
  return Fortify_Allocate (size, Fortify_Allocator_malloc, 0, file, line);
}

/*
 * Fortify_realloc - Fortify's replacement realloc()
 */
void *Fortify_realloc (void *uptr, size_t new_size, const char *file, DWORD line)
{
  BYTE          *ptr = (BYTE*) uptr - FORTIFY_HEADER_SIZE - FORTIFY_ALIGNED_BEFORE_SIZE;
  struct Header *h   = (struct Header*) ptr;
  void          *new_ptr;

  /* If Fortify is disabled, we gotta do this a little differently.
   */
  if (!st_Disabled)
  {
    if (!uptr)
       return Fortify_Allocate (new_size, Fortify_Allocator_realloc, 0, file, line);

    if (!st_IsOnAllocatedList(h))
    {
#ifdef FORTIFY_TRACK_DEALLOCATED_MEMORY
      if (st_IsOnDeallocatedList(h))
      {
        message (1, "Deallocated memory block passed to \"%s\" at %s.%lu\n",
                 st_AllocatorName[Fortify_Allocator_realloc], file, line);

        sprintf (st_Buffer,   "         Memory block %s was deallocated by \"%s\" at %s.%lu\n",
                 st_MemoryBlockString(h),
                 st_DeallocatorName[h->Deallocator], h->FreedFile, h->FreedLine);
        ST_OUTPUT (st_Buffer);
        return (0);
      }
#endif
      message (1, "Invalid pointer (%08lX) passed to realloc at %s.%lu\n",
               (DWORD_PTR)ptr, file, line);
      return (0);
    }

    if (!st_CheckBlock(h, file, line))
       return (0);

    new_ptr = Fortify_Allocate (new_size, Fortify_Allocator_realloc, 0, file, line);
    if (!new_ptr)
       return (0);

    if (h->Size < new_size)
         memcpy (new_ptr, uptr, h->Size);
    else memcpy (new_ptr, uptr, new_size);

    Fortify_Deallocate (uptr, Fortify_Deallocator_realloc, file, line);
    return (new_ptr);
  }

  /* If the old block was fortified, we can't use normal realloc.
   */
  if (st_IsOnAllocatedList(h))
  {
    new_ptr = Fortify_Allocate (new_size, Fortify_Allocator_realloc, 0, file, line);
    if (!new_ptr)
       return (0);

    if (h->Size < new_size)
         memcpy (new_ptr, uptr, h->Size);
    else memcpy (new_ptr, uptr, new_size);

    Fortify_Deallocate (uptr, Fortify_Deallocator_realloc, file, line);
    return (new_ptr);
  }
  return realloc (uptr, new_size);
}

/*
 * Fortify_calloc - Fortify's replacement calloc
 */
void *Fortify_calloc (size_t num, size_t size, const char *file, DWORD line)
{
  if (!st_Disabled)
  {
    void *ptr = Fortify_Allocate (size * num, Fortify_Allocator_calloc,
                                  0, file, line);
    if (ptr)
       memset (ptr, 0, size*num);
    return (ptr);
  }
  return calloc (num, size);
}

/*
 * Fortify_free - Fortify's replacement free
 */
void Fortify_free (void *uptr, const char *file, DWORD line)
{
  if (!uptr)
       message (1, "freeing of NULL ptr at %s.%lu\n", file, line);
  else Fortify_Deallocate (uptr, Fortify_Deallocator_free, file, line);
}


#if defined(WIN32) && defined(FORTIFY_GLOBAL_REPLACE)

HGLOBAL Fortify_GlobalAlloc (UINT flags, DWORD size, const char *file, DWORD line)
{
  void *ptr = GlobalAlloc (flags, size);

  if (size == 0)
     message (1, "A \"%s\" of 0 bytes attempted at %s.%lu\n",
              st_AllocatorName[Fortify_Allocator_GlobalAlloc], file, line);
  if (!ptr)
     message (1, "A \"%s\" of %lu bytes failed at %s.%lu\n",
              st_AllocatorName[Fortify_Allocator_GlobalAlloc], file, line);
  return (ptr ? GlobalLock(ptr) : NULL);
}

BOOL Fortify_GlobalFree (void *ptr, const char *file, DWORD line)
{
  HGLOBAL hnd;

  if (!ptr)
  {
    message (1, "A \"%s\" freeing of NULL ptr at %s.%lu\n",
             st_DeallocatorName[Fortify_Allocator_GlobalFree], file, line);
    hnd = NULL;
  }
  else
  {
    GlobalUnlockPtr (ptr);
    hnd = GlobalFree (GlobalPtrHandle(ptr));
  }
  return (hnd ? TRUE : FALSE);
}
#endif

/*
 * Fortify_strdup - Fortify's replacement strdup. Since strdup isn't
 * ANSI, it is only provided if FORTIFY_STRDUP is defined.
 */
#ifdef FORTIFY_STRDUP
char *FORTIFY_STORAGE Fortify_strdup (const char *oldStr, const char *file, DWORD line)
{
  if (!st_Disabled)
  {
    char *newStr = (char*) Fortify_Allocate (strlen(oldStr)+1, Fortify_Allocator_strdup,
                                             0, file, line);
    if (newStr)
       strcpy (newStr, oldStr);
    return (newStr);
  }
  return strdup (oldStr);
}
#endif /* FORTIFY_STRDUP */


static void st_OutputDeleteTrace (void)
{
#ifdef __cplusplus
  if (st_DeleteStackTop > 1)
  {
    message (0, "Delete Trace: %s.%lu\n",
             st_DeleteFile[st_DeleteStackTop-1],
             st_DeleteLine[st_DeleteStackTop-1]);
    for (int c = st_DeleteStackTop-2; c >= 0; c--)
        message (0, "              %s.%lu\n",
                 st_DeleteFile[c], st_DeleteLine[c]);
  }
#endif
}

#ifdef __cplusplus

/*
 * st_NewHandler() - there is no easy way to get
 * the new handler function. And isn't it great
 * how the new handler doesn't take a parameter
 * giving the size of the request that failed.
 * Thanks Bjarne!
 */
Fortify_NewHandlerFunc st_NewHandler (void)
{
  /* get the current handler
   */
  Fortify_NewHandlerFunc handler = set_new_handler (0);

  /* and set it back (since we cant
   * get it without changing it)
   */
  set_new_handler (handler);
  return (handler);
}

/*
 * operator new - Fortify's replacement new,
 * without source-code information.
 */
void *FORTIFY_STORAGE operator new (size_t size)
{
  void *p;

  while ((p = Fortify_Allocate(size, Fortify_Allocator_new,
              0, st_AllocatorName[Fortify_Allocator_new], 0)) == 0)
  {
    if (st_NewHandler())
         (*st_NewHandler())();
    else return (0);
  }
  return (p);
}

/*
 * operator new - Fortify's replacement new,
 * with source-code information
 */
void *FORTIFY_STORAGE
operator new (size_t size, const char *file, DWORD line)
{
  void *p;

  while ((p = Fortify_Allocate(size, Fortify_Allocator_new, 0, file, line)) == 0)
  {
    if (st_NewHandler())
         (*st_NewHandler())();
    else return (0);
  }
  return (p);
}

#ifdef FORTIFY_PROVIDE_ARRAY_NEW
/*
 * operator new[], without source-code info
 */
void *FORTIFY_STORAGE operator new[] (size_t size)
{
  void *p;

  while ((p = Fortify_Allocate(size, Fortify_Allocator_array_new,
              0, st_AllocatorName[Fortify_Allocator_array_new], 0)) == 0)
  {
    if (st_NewHandler())
         (*st_NewHandler())();
    else return (0);
  }
  return (p);
}

/*
 * operator new[], with source-code info
 */
void *FORTIFY_STORAGE operator new[] (size_t size, const char *file, DWORD line)
{
  void *p;

  while ((p = Fortify_Allocate(size, Fortify_Allocator_array_new, 0, file, line)) == 0)
  {
    if (st_NewHandler())
         (*st_NewHandler())();
    else return (0);
  }
  return (p);
}
#endif /* FORTIFY_PROVIDE_ARRAY_NEW */

/*
 * Fortify_PreDelete - C++ does not allow overloading
 * of delete, so the delete macro calls Fortify_PreDelete
 * with the source-code info, and then calls delete.
 */
void FORTIFY_STORAGE Fortify_PreDelete (const char *file, DWORD line)
{
  FORTIFY_LOCK();

  /* Push the source code info for the delete onto the delete stack
   * (if we have enough room, of course)
   */
  if (st_DeleteStackTop < FORTIFY_DELETE_STACK_SIZE)
  {
    st_DeleteFile[st_DeleteStackTop] = file;
    st_DeleteLine[st_DeleteStackTop] = line;
  }
  st_DeleteStackTop++;
}

/*
 * Fortify_PostDelete() - Pop the delete source-code info
 * off the source stack.
 */
void FORTIFY_STORAGE Fortify_PostDelete (void)
{
  st_DeleteStackTop--;

  FORTIFY_UNLOCK();
}

/*
 * operator delete - fortify's replacement delete
 */
void FORTIFY_STORAGE operator delete (void *uptr)
{
  const char *file;
  DWORD line;

  /* It is defined to be harmless to delete 0
   */
  if (uptr == 0)
     return;

  /* find the source-code info
   */
  if (st_DeleteStackTop)
  {
    if (st_DeleteStackTop < FORTIFY_DELETE_STACK_SIZE)
    {
      file = st_DeleteFile[st_DeleteStackTop-1];
      line = st_DeleteLine[st_DeleteStackTop-1];
    }
    else
    {
      file = st_DeleteFile[FORTIFY_DELETE_STACK_SIZE-1];
      line = st_DeleteLine[FORTIFY_DELETE_STACK_SIZE-1];
    }
  }
  else
  {
    file = st_DeallocatorName[Fortify_Deallocator_delete];
    line = 0;
  }
  Fortify_Deallocate (uptr, Fortify_Deallocator_delete, file, line);
}

#ifdef FORTIFY_PROVIDE_ARRAY_DELETE

/*
 * operator delete[] - fortify's replacement delete[]
 */
void FORTIFY_STORAGE operator delete[] (void *uptr)
{
  const char *file;
  DWORD line;

  /* It is defined to be harmless to delete 0
   */
  if (uptr == 0)
     return;

  /* find the source-code info
   */
  if (st_DeleteStackTop)
  {
    if (st_DeleteStackTop < FORTIFY_DELETE_STACK_SIZE)
    {
      file = st_DeleteFile[st_DeleteStackTop-1];
      line = st_DeleteLine[st_DeleteStackTop-1];
    }
    else
    {
      file = st_DeleteFile[FORTIFY_DELETE_STACK_SIZE-1];
      line = st_DeleteLine[FORTIFY_DELETE_STACK_SIZE-1];
    }
  }
  else
  {
    file = st_DeallocatorName[Fortify_Deallocator_array_delete];
    line = 0;
  }
  Fortify_Deallocate (uptr, Fortify_Deallocator_array_delete, file, line);
}
#endif /* FORTIFY_PROVIDE_ARRAY_DELETE */


#ifdef FORTIFY_AUTOMATIC_LOG_FILE
/*
 * Automatic log file stuff!
 *
 * AutoLogFile class. There can only ever be ONE of these
 * instantiated! It is a static class, which means that
 * it's constructor will be called at program initialization,
 * and it's destructor will be called at program termination.
 * We don't know if the other static class objects have been
 * constructed/destructed yet, but this pretty much the best
 * we can do with standard C++ language features.
 */
class Fortify_AutoLogFile {
      static FILE *fp;
      static int   written_something;
      static char *init_string, *term_string;

public:
      Fortify_AutoLogFile (void)
      {
        written_something = 0;
        Fortify_SetOutputFunc (Fortify_AutoLogFile::Output);
        Fortify_EnterScope (init_string, 0);
      }

      static void Output(const char *s)
      {
        if (written_something == 0)
        {
          FORTIFY_FIRST_ERROR_FUNCTION;
          fp = fopen (FORTIFY_LOG_FILENAME, "w");
          if (fp)
          {
            time_t t;
            time (&t);
            fprintf (fp, "Fortify log started at %s\n", ctime(&t));
            written_something = 1;
          }
        }
        if (fp)
        {
          fputs (s, fp);
          fflush (fp);
        }
      }

      ~Fortify_AutoLogFile (void)
      {
        Fortify_LeaveScope (term_string, 0);
        Fortify_CheckAllMemory (term_string, 0);
        if (fp)
        {
          time_t t;
          time (&t);
          fprintf (fp, "\nFortify log closed at %s\n", ctime(&t));
          fclose (fp);
          fp = 0;
        }
      }
};

FILE *Fortify_AutoLogFile::fp = 0;
int   Fortify_AutoLogFile::written_something = 0;
char *Fortify_AutoLogFile::init_string = "Program Initialization";
char *Fortify_AutoLogFile::term_string = "Program Termination";

static Fortify_AutoLogFile Abracadabra;

#endif /* FORTIFY_AUTOMATIC_LOG_FILE */
#endif /* __cplusplus */
#endif /* USE_FORTIFY && !USE_CRTDBG */

