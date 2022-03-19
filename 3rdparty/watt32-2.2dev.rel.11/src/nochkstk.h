/*!\file nochkstk.h
 *
 * pragmas to disable stack-checking
 */
#if defined(__HIGHC__) || defined(__WATCOMC__)
  #pragma Off (check_stack)

#elif defined(__POCC__) && !defined(WIN32)
  #pragma check_stack (off)

#elif defined(_MSC_VER) && (_MSC_VER >= 800) && !defined(WIN32)
  #pragma check_stack (off)

#elif defined(__DMC__)
  /* #pragma ..  */

#elif (defined(__TURBOC__) || defined(__BORLANDC__)) && !defined(WIN32)
  #pragma option -N-
#endif
