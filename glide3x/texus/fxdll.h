/*
** THIS SOFTWARE IS SUBJECT TO COPYRIGHT PROTECTION AND IS OFFERED ONLY
** PURSUANT TO THE 3DFX GLIDE GENERAL PUBLIC LICENSE. THERE IS NO RIGHT
** TO USE THE GLIDE TRADEMARK WITHOUT PRIOR WRITTEN PERMISSION OF 3DFX
** INTERACTIVE, INC. A COPY OF THIS LICENSE MAY BE OBTAINED FROM THE 
** DISTRIBUTOR OR BY CONTACTING 3DFX INTERACTIVE INC(info@3dfx.com). 
** THIS PROGRAM IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND, EITHER 
** EXPRESSED OR IMPLIED. SEE THE 3DFX GLIDE GENERAL PUBLIC LICENSE FOR A
** FULL TEXT OF THE NON-WARRANTY PROVISIONS.  
** 
** USE, DUPLICATION OR DISCLOSURE BY THE GOVERNMENT IS SUBJECT TO
** RESTRICTIONS AS SET FORTH IN SUBDIVISION (C)(1)(II) OF THE RIGHTS IN
** TECHNICAL DATA AND COMPUTER SOFTWARE CLAUSE AT DFARS 252.227-7013,
** AND/OR IN SIMILAR OR SUCCESSOR CLAUSES IN THE FAR, DOD OR NASA FAR
** SUPPLEMENT. UNPUBLISHED RIGHTS RESERVED UNDER THE COPYRIGHT LAWS OF
** THE UNITED STATES.  
** 
** COPYRIGHT 3DFX INTERACTIVE, INC. 1999, ALL RIGHTS RESERVED
*/
/* preprocessor defines for libraries to support DLL creation */

/* in header file, use FX_ENTRY return-type FX_CALL function-name ( ... ) */
/* in source file, use FX_EXPORT return-type FX_CSTYLE function-name (... ) */

/* in source file, set FX_DLL_DEFINITION, include this file, then include
   header file for library. */

/* we need to use two macros per declaration/definition because MSVC
   requires __stdcall and __declspec( dllexport ) be in different parts
   of the prototype! */

/* I use two sets in case we need to control declarations and definitions
   differently.  If it turns out we don't, it should be easy to do a search
   and replace to eliminate one set */

/* NOTE: this header file may be included more than once, and FX_DLL_DEFINITION
   may have changed, so we do not protect this with an #fndef __FXDLL_H__
   statement like we normally would. */

#ifdef FX_ENTRY
#undef FX_ENTRY
#endif
#ifdef FX_CALL
#undef FX_CALL
#endif
#ifdef FX_EXPORT
#undef FX_EXPORT
#endif
#ifdef FX_CSTYLE
#undef FX_CSTYLE
#endif

#if defined(FX_DLL_DEFINITION)
  #if defined(FX_DLL_ENABLE)
    #if defined(__MSC__) || defined (__MINGW32__)
      #ifndef KERNEL
        #define FX_ENTRY __declspec( dllexport )
        #define FX_EXPORT __declspec( dllexport )
      #else
        #define FX_ENTRY
        #define FX_EXPORT
     #endif /* #ifndef KERNEL */
     #define FX_CALL __stdcall
     #define FX_CSTYLE __stdcall

    #elif defined(__WATCOMC__)
      #define FX_ENTRY
      #define FX_CALL __stdcall __export

      #define FX_EXPORT
      #define FX_CSTYLE __stdcall __export

    #else /* compiler */
      #error define FX_ENTRY,FX_CALL & FX_EXPORT,FX_CSTYLE for your compiler
    #endif /* compiler */

  #else /* FX_DLL_ENABLE */
    #define FX_ENTRY
    #define FX_CALL __stdcall

    #define FX_EXPORT
    #define FX_CSTYLE __stdcall
  #endif /* FX_DLL_ENABLE */

#else /* FX_DLL_DEFINITION */
  #define FX_ENTRY extern
  #define FX_CALL __stdcall
#endif /* FX_DLL_DEFINITION */

/*
 * We don't want any of this DLL junk for DJGPP or UNIX
 * so undo what is done above.
 */
#if defined(__DJGPP__) || defined(__unix__) || defined(macintosh)
  #ifdef FX_CALL
    #undef FX_CALL
  #endif

  #ifdef FX_CSTYLE
    #undef FX_CSTYLE
  #endif

  #ifdef FX_EXPORT
    #undef FX_EXPORT
  #endif

  #ifdef FX_ENTRY
    #undef FX_ENTRY
  #endif

  #define FX_CALL
  #define FX_CSTYLE
  #define FX_EXPORT
  #define FX_ENTRY
#endif

#if defined (MSVC16)
  #undef FX_CALL
  #define FX_CALL
#endif
