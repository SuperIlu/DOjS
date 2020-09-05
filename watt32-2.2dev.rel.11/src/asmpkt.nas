; !\asmpkt.nas
;
; Packet-driver receiver upcall handler. This module is used for all
; targets if USE_FAST_PKT is defined in config.h. Not needed otherwise.
; For 32-bit targets this code is patched and copied to allocated DOS
; memory at runtime.
;
; Requires NASM 0.98+ to assemble. Use the one in ..\util.
;

BITS 16
ORG  0

%define DEBUG              0
%define RX_BUFS            40           ; same as in pcqueue.h
%define ETH_MAX            1514         ; max Ethernet size
%define RX_SIZE            (ETH_MAX+10) ; add some margin

%define IP4_TYPE           0x0008       ; these are in network order
%define IP6_TYPE           0xDD86
%define ARP_TYPE           0x0608
%define RARP_TYPE          0x3580
%define PPPOE_DISC_TYPE    0x6388
%define PPPOE_SESS_TYPE    0x6488

struc RX_ELEMENT                ; struct pkt_rx_element
      rx_tstamp_put: resd  2    ; RDTSC timestamp on put
      rx_tstamp_get: resd  2    ; RDTSC timestamp on get
      rx_handle:     resw  1    ; handle for this Rx upcall
      rx_length_1:   resw  1    ; packet length on 1st upcall
      rx_length_2:   resw  1    ; packet length on 2nd upcall
      rx_filler      resw  1    ; align on DWORD boundary
    RX_ELEMENT_HEAD:            ; (24)
      rx_buf:        resb (RX_SIZE)
    RX_ELEMENT_SIZE:
endstruc

struc PKT_RINGBUF
      in_index:      resw  1    ; queue index head
      out_index:     resw  1    ; queue index tail
      buf_size:      resw  1    ; size of each buffer
      num_buf:       resw  1    ; number of buffers
      num_drop:      resd  1    ; number of dropped pkts
      buf_start:     resd  1    ; start of buffer pool      (not used here)
      dos_ofs:       resw  1    ; DOS offset of RX_ELEMENTs (not used here)
    PKT_RINGBUF_SIZE:           ; (18)
endstruc

;
; Only used indirectly by reading 'size_chk' in pcpkt2.c to check
; the structure layouts are the same here and in pcpkt.h.
;
struc PKT_INFO
      rm_sel_seg_ofs: resw  3                       ; not used here (6)
      dos_ds:         resw  1                       ;   -- " --
      use_near        resw  1                       ;   -- " --
      handle:         resw  1                       ;   -- " --
      is_serial:      resw  1                       ;   -- " --     16 total
      pkt_ip_ofs:     resw  1
      pkt_type_ofs:   resw  1
  ;   error:          resd  1                       ; ofs 32
      pkt_queue:      resb  PKT_RINGBUF_SIZE        ; size 18
      rx_bufs:        resb (RX_BUFS*RX_ELEMENT_SIZE)
    PKT_INFO_SIZE:
endstruc

asmpkt_inf   dd 0                  ; CS:0
index        dw -1                 ; CS:4
patch_nop    dw @patch_it          ; CS:6
size_chk     dw PKT_INFO_SIZE      ; CS:8
xy_pos       dw 160*10             ; CS:10

%macro PUTCH 1
   %if DEBUG
       push es
       push di
       push ax
       mov  ax, 0xB800             ; colour screen segment
       mov  es, ax
       mov  di, word [cs:xy_pos]
       mov  al, %1                 ; AL = arg 1
       mov  ah, 15+16              ; white on blue
       stosw
       add  word [cs:xy_pos], 2    ; increment X-pos
       pop  ax
       pop  di
       pop  es
  %endif
%endmacro


pkt_receiver_rm:
        push gs
        push fs                    ; I suspect NDIS3PKT changes FS/GS
        pushf
        cli                        ; no interruptions now
        or   al, al                ; AL = 0 if 1st call from pkt-driver
        jnz  @enque                ; AL <>0, 2nd call; enqueue packet

        PUTCH ' '
        PUTCH '0'

        call pkt_get_buffer        ; get a buffer, return ES:DI (may be 0:0)
        jmp short @quit

        ;
        ; enque packet, DS:SI=ES:DI from 1st call, i.e. DS=CS
        ;
@enque:
      ; call pkt_filter            ; not any point really
      ; jc @quit

        PUTCH '1'
        call pkt_enqueue           ; copy packet at DS:SI to queue-head
@quit:
        popf
        pop fs
        pop gs
        retf

;
; If room in ring-buffer, return next head buffer, else NULL.
;
; static unsigned short index;
;
; char * pkt_get_buffer (int rx_len)
; {
;   struct _far pkt_ringbuf *queue = asmpkt_inf->pkt_queue;
;
;   if (rx_len > queue->buf_size - 4)
;   {
;     queue->num_drop++;
;     return (NULL);
;   }
;
;   index = queue->in_index + 1;
;   if (index >= queue->num_buf)
;       index = 0;
;
;   if (index == queue->out_index)
;   {
;     queue->num_drop++;
;     return (NULL);
;   }
;   return MK_FP (_ES, queue->dos_ofs + (queue->buf_size * queue->in_index);
; }
;
; BX = protocol handle
; CX = buffer size
;
; return ES:DI, AX,BX,DX changed

pkt_get_buffer:
        push ds                           ; save DS
        mov  ax, cs
        mov  ds, ax                       ; DS = CS
        les  di, [asmpkt_inf]

        add  di, pkt_queue                ; ES:DI = queue
        mov  ax, RX_SIZE-4
        cmp  cx, ax                       ; if (rx_len > queue->buf_size - 4)
        ja   @drop_it                     ;    goto drop it

        mov  ax, [di+in_index]
        inc  ax
        cmp  ax, RX_BUFS                  ; index < queue->num_buf?
        jb   @no_wrap                     ; no, don't wrap
        xor  ax, ax                       ; yes, wrap it.

        PUTCH 'w'

@no_wrap:
        mov  [index], ax                  ; remember in_index for 2nd upcall
        cmp  ax, [di+out_index]
        je   @drop_it                     ; if (index == queue->out_index)
                                          ;   drop packet (queue full)
        mov  ax, RX_ELEMENT_SIZE
        mul  word [di+in_index]           ; AX += (queue->buf_size * queue->in_index)
        add  ax, di
        add  ax, rx_bufs - pkt_queue      ; AX = rx_bufs[queue->in_in_index]
        mov  di, ax

@patch_it:
        call get_tstamp
        mov  word [di+rx_handle], bx      ; save the handle and length for
        mov  word [di+rx_length_1], cx    ; pkt_poll_recv()

        add  di, RX_ELEMENT_HEAD          ; ES:DI -> receive buffer

        PUTCH '!'
        pop  ds
        ret                               ; return ES:DI

@drop_it:
        PUTCH 'd'

        add  word [di+num_drop+0], 1
        adc  word [di+num_drop+2], 0
        xor  di, di
        mov  es, di                       ; return (NULL)
        pop  ds
        ret

;
; NOT USED
;
; If not using a serial driver, filter off the protocol types we're
; not interested in.
;
; int pkt_filter (const unsigned char *rx_buf == DS:SI)
; {
;   WORD typ;
;
;   if (asmpkt_inf->pkt_type_ofs == 0)
;      return (NOCARRY);
;
;   typ = *(unsigned short*)rx_buf [asmpkt_inf->pkt_type_ofs];
;   if (typ == IP4_TYPE || typ == IP6_TYPE || typ == ARP_TYPE ||
;       typ == RARP_TYPE || typ == PPPOE_DISC_TYPE || typ == PPPOE_SESS_TYPE)
;      return (NOCARRY);
;   return (CARRY);
; }

pkt_filter:
      les  di, [asmpkt_inf]
      cmp  word [di+pkt_type_ofs], 0
      je   @pass_it
      add  si, [di+pkt_type_ofs]
      cmp  word [si], IP4_TYPE
      je   @pass_it
      cmp  word [si], IP6_TYPE
      je   @pass_it
      cmp  word [si], ARP_TYPE
      je   @pass_it
      cmp  word [si], RARP_TYPE
      je   @pass_it
      cmp  word [si], PPPOE_DISC_TYPE
      je   @pass_it
      cmp  word [si], PPPOE_SESS_TYPE
      je   @pass_it
      stc
      ret
@pass_it:
      clc
      ret


;
; Enqueue a packet to the head of queue.
;
; DS:SI = receiver buffer to enqueue.
; CX    = rx_len
;
pkt_enqueue:
        PUTCH '*'
        les  di, [asmpkt_inf]

        or   si, si                ; NULL-ptr is not valid
        jz   @no_enque
        mov  ax, es
        mov  dx, ds
        cmp  dx, ax
        jne  @no_enque             ; DS != ES on 1st upcall

        mov  [si-RX_ELEMENT_HEAD+rx_length_2], cx
        mov  ax, [index]
        mov  [di+pkt_queue+in_index], ax  ; update queue->in_index
        ret

@no_enque:
        add  word [di+pkt_queue+num_drop+0], 1
        adc  word [di+pkt_queue+num_drop+2], 0
        ret

;
; Fill in RDTSC timestamp at [ES:DI]
; (the call to this function will be patched to NOPs if not a Pentium+ CPU)
;
get_tstamp:
        push eax
        push edx
        rdtsc
        mov  dword [di+rx_tstamp_put], eax
        mov  dword [di+rx_tstamp_put+4], edx
        pop  edx
        pop  eax
        ret

align 16, db 0xCB

;
; asmpkt_inf gets allocated here at run time
;
end:

