/*
 * This symbol could be missing when using 'system x32' with Watcom.
 */
#ifdef _NEED__x32_stacksize
  unsigned long __x32_stack_size = 16*1024;
#endif

