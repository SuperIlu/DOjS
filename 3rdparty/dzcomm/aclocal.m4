# Place m4 macros here.

dnl
dnl Test if asm files can be used (which ones).
dnl
dnl Variables:
dnl  allegro_enable_asm=(yes|),
dnl  allegro_cv_support_asm=(i386|no).
dnl
AC_DEFUN(ALLEGRO_ACTEST_SUPPORT_ASM,
[AC_BEFORE([$0], [ALLEGRO_ACTEST_SUPPORT_MMX])
AC_ARG_ENABLE(asm,
[  --enable-asm[=x]        enable the use of assembler files [default=yes]],
test "X$enableval" != "Xno" && allegro_enable_asm=yes,
allegro_enable_asm=yes)
AC_MSG_CHECKING(for asm support)
AC_CACHE_VAL(allegro_cv_support_asm,
[if test "$allegro_enable_asm" = yes; then
  AC_TRY_COMPILE([], [asm (".globl _dummy_function\n"
  "_dummy_function:\n"
  "     pushl %%ebp\n"
  "     movl %%esp, %%ebp\n"
  "     movl %%eax, %%ebx\n"
  "     movl %%ecx, %%edx\n"
  "     movl %%esi, %%edi\n"
  "     leal 10(%%ebx, %%ecx, 4), %%edx\n"
  "     call *%%edx\n"
  "     addl %0, %%eax\n"
  "     popl %%ebp\n"
  "     ret" : : "i" (0))],
  allegro_cv_support_asm=i386,
  allegro_cv_support_asm=no)
  dnl Add more tests for assembler types here.
  dnl if test "$allegro_cv_support_asm" = no; then
  dnl   ...
  dnl fi
else
  allegro_cv_support_asm=no
fi
])
AC_MSG_RESULT($allegro_cv_support_asm)])

dnl
dnl Test if MMX instructions are supported by assembler.
dnl
dnl Variables:
dnl  allegro_enable_mmx=(yes|),
dnl  allegro_cv_support_mmx=(yes|no).
dnl
AC_DEFUN(ALLEGRO_ACTEST_SUPPORT_MMX,
[AC_REQUIRE([ALLEGRO_ACTEST_SUPPORT_ASM])
AC_ARG_ENABLE(mmx,
[  --enable-mmx[=x]        enable the use of MMX instructions [default=yes]],
test "X$enableval" != "Xno" && allegro_enable_mmx=yes,
allegro_enable_mmx=yes)
AC_MSG_CHECKING(for MMX support)
AC_CACHE_VAL(allegro_cv_support_mmx,
[if test "$allegro_cv_support_asm" != no -a "$allegro_enable_mmx" = yes; then
  AC_TRY_COMPILE([], [asm (".globl _dummy_function\n"
  "_dummy_function:\n"
  "     pushl %%ebp\n"
  "     movl %%esp, %%ebp\n"
  "     movq -8(%%ebp), %%mm0\n"
  "     movd %%edi, %%mm1\n"
  "     punpckldq %%mm1, %%mm1\n"
  "     psrld $ 19, %%mm7\n"
  "     pslld $ 10, %%mm6\n"
  "     por %%mm6, %%mm7\n"
  "     paddd %%mm1, %%mm0\n"
  "     emms\n"
  "     popl %%ebp\n"
  "     ret" : : )],
  allegro_cv_support_mmx=yes,
  allegro_cv_support_mmx=no)
else
  allegro_cv_support_mmx=no
fi
])
AC_MSG_RESULT($allegro_cv_support_mmx)])

dnl
dnl Test for prefix prepended by compiler to global symbols.
dnl
dnl Variables:
dnl  allegro_cv_asm_prefix=(_|)
dnl
AC_DEFUN(ALLEGRO_ACTEST_ASM_PREFIX,
[AC_MSG_CHECKING(for asm prefix before symbols)
AC_CACHE_VAL(allegro_cv_asm_prefix,
[AC_TRY_LINK([int test_for_underscore(void);
asm (".globl _test_for_underscore\n"
"_test_for_underscore:");], [test_for_underscore ();],
allegro_cv_asm_prefix=_,
allegro_cv_asm_prefix=no)
if test "$allegro_cv_asm_prefix" = no; then
  AC_TRY_LINK([int test_for_underscore(void);
  asm (".globl test_for_undercore\n"
  "test_for_underscore:");], [test_for_underscore ();],
    allegro_cv_asm_prefix=,
    allegro_cv_asm_prefix=no)
fi
dnl Add more tests for asm prefix here.
dnl if test "$allegro_cv_asm_prefix" = no; then
dnl   AC_TRY_LINK([], [],
dnl     allegro_cv_asm_prefix=,
dnl     allegro_cv_asm_prefix=no)
dnl fi
if test "$allegro_cv_asm_prefix" = no; then
  allegro_cv_asm_prefix=
fi
])
AC_MSG_RESULT(\"$allegro_cv_asm_prefix\")])
