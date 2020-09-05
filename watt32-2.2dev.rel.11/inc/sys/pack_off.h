/*!\file sys/pack_off.h
 *
 * Default packing of structures.
 */

/*++

Copyright (c) 1990,91  Microsoft Corporation

Module Name:

    pack_off.h

Abstract:

    This file turns packing of structures off.  (That is, it enables
    automatic alignment of structure fields.)  An include file is needed
    because various compilers do this in different ways.

    pack_off.h is the complement to pack_on.h.  An inclusion of pack_off.h
    MUST ALWAYS be preceded by an inclusion of pack_on.h, in one-to-one
    correspondence.

Author:

    Chuck Lenzmeier (chuckl) 4-Mar-1990

Revision History:

    15-Apr-1991 JohnRo
        Created lint-able variant.

    20-Oct-1997 G.Vanem
        Added Metaware support

    05-Jul-1999 G.Vanem
        Added LADsoft support

    01-Nov-2000 G. Vanem
        Added Visual C/C++ support

--*/

#if !(defined(lint) || defined(_lint))
  #if defined(__BORLANDC__) && (__BORLANDC__ >= 0x550)
    #pragma option push -b -a8 -pc -A- /*P_O_Push*/
  #endif

  #if defined(_MSC_VER) && (_MSC_VER >= 800)
    #pragma warning(disable:4103)
  #endif

  #if defined(__CCDL__)
    #pragma pack()
  #elif defined(__HIGHC__)
    #pragma pop_align_members();
  #elif defined(__WATCOMC__) && (__WATCOMC__ >= 1000)
    #pragma pack(__pop);
  #elif (defined(_MSC_VER) && (_MSC_VER > 800))            || \
        (defined(__GNUC__) && ((__GNUC__ > 2)              || \
              (__GNUC__ == 2 && __GNUC_MINOR__ > 95)))     || \
        (defined(__BORLANDC__) && (__BORLANDC__ >= 0x500)) || \
         defined(__POCC__) || defined(__LCC__)
    #pragma pack(pop)
  #else
    #pragma pack()
  #endif

  #if defined(__BORLANDC__) && (__BORLANDC__ >= 0x550)
    #pragma option pop  /*P_O_Pop*/
  #endif
#endif

