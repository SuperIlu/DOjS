/*!\file nochkstk.h
 *
 * pragmas to disable stack-checking
 */
#if defined(__HIGHC__) || defined(__WATCOMC__)
  #pragma Off (check_stack)

#elif defined(_MSC_VER) && (_MSC_VER >= 800)
  #pragma check_stack (off)

#elif defined(__BORLANDC__)
  #pragma option -N-
#endif
