; !\file asmpkt.asm Packet-driver upcall handler for real-mode

ifndef DOSX    

ifdef ??version    ; Turbo Assembler
  JUMPS            ; use option /m2 (short jumps)
endif

.MODEL LARGE,C
.CODE

;
; Packet-driver receiver upcall handler. This module is only
; used by small/large real-mode targets.
;

PUBLIC pkt_receiver_rm, _pkt_receiver_rm

extrn _pkt_enque_ptr : far

ETH_MAX = 1514

rx_buf_ptr  label dword
            dw rx_buf, SEG rx_buf
nullPtr     dd 0

rx_buf      db ETH_MAX dup (0)  ; Only 1 buffer. Rest is done in pkt_enqueue()
rx_len      dw 0


.DATA
rx_stk      dw 64 dup (0)       ; a small work stack
rx_stk_end  label word
            dw 0                ; because 286 does push before decrement

.CODE

_pkt_receiver_rm:
 pkt_receiver_rm:
        pushf
        cli                     ; no interruptions now
        or   al, al             ; AL = 0 if 1st call from pkt-driver
        jnz  @enqueue           ; AL <>0, 2nd call; enqueue packet

        cmp  cx, ETH_MAX        ; check the packet length
        ja   @drop_it           ; too big!
        les  di, cs:rx_buf_ptr
        mov  cs:rx_len, cx      ; save packet length for enqueue
        popf
        retf

@drop_it:
        les  di, cs:nullPtr     ; return NULL ptr
        popf
        retf

      ; enqueue packet, DS:SI=ES:DI from 1st call, i.e. DS=CS

@enqueue:
        cmp  si, offset rx_buf  ; valid pointer?. If not, skip it
        jne  @no_enque          ; !!to-do: should inc drop counter

        mov  ax, DGROUP
        mov  ds, ax             ; load our DS

      ; Setup a new work-stack (C small/large requires SS=DS)

        mov  cx, ss
        mov  dx, sp
        mov  ss, ax             ; setup new stack (SS=DS)
        lea  sp, rx_stk_end
        push cx                 ; save SS on new stack
        push dx                 ; save SP on new stack

        cld                     ; C-code assumes forward direction
        push cs:rx_len          ; store length and buf address
        push cs
        push si                
      ; call dword ptr DGROUP:_pkt_enque_ptr
        call dword ptr ds:_pkt_enque_ptr
        add  sp, 6

        pop  dx                 ; restore old SP
        pop  cx                 ; restore old SS
        mov  ss, cx
        mov  sp, dx

@no_enque:
        popf
        retf

endif   ; ifndef DOSX

END
