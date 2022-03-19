;
;  int ffs (int val);
;
;  return index to first bit set in `val'
;
;  For all targets except djgpp (it already have ffs())
;  NB! requires 386+ CPU

%ifdef DOSX
[BITS 32]
[SEGMENT CODE]

global __ffs
  __ffs: bsf eax, [esp+4]
         jnz @1
         xor eax,eax
  @1:    ret

%else
[BITS 16]
[SEGMENT CODE]

global __ffs
  __ffs: enter 0,0
         bsf ax, [bp+12]
         jnz @2
         xor ax,ax
  @2:    leave
         retf

%endif

end
