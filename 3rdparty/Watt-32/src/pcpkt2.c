/*
 * This must be included from pcpkt.c (not nice I admit)
 */
#if defined(USE_FAST_PKT)

W32_CLANG_PACK_WARN_OFF()

#include <sys/pack_on.h>

/* Data located in a real-mode segment. This becomes far at runtime
 */
typedef struct {             /* must match data/code in asmpkt.nas */
        DWORD  _asmpkt_inf;
        WORD   _index;
        WORD   _patch_nop;
        WORD   _size_chk;
        WORD   _xy_pos;
        BYTE   _PktReceiver; /* starts on a paragraph (16byte) boundary */
      } PktRealStub;

typedef struct {             /* structure asmpkt.nas sees */
        struct pkt_info       info;
        struct pkt_rx_element rx_buf [RX_BUFS];
      } PKT_INFO;

#include <sys/pack_off.h>

W32_CLANG_PACK_WARN_DEF()

static BYTE real_stub_array [] = {
       #include "pkt_stub.h"    /* generated opcode array */
     };

#define asmpkt_inf   (unsigned) offsetof (PktRealStub, _asmpkt_inf)
#define patch_nop    (unsigned) offsetof (PktRealStub, _patch_nop)
#define size_chk     (unsigned) offsetof (PktRealStub, _size_chk)
#define PktReceiver  (unsigned) offsetof (PktRealStub, _PktReceiver)

#undef  PKT_TMP
#define PKT_TMP()    (sizeof(real_stub_array) + sizeof(PKT_INFO))

#define ASMPKT_INF   (rm_base + sizeof(real_stub_array))
#define QUEUE_OFS(x) (ASMPKT_INF + offsetof(struct pkt_info,pkt_queue.x))
#define RX_OFS(n,x)  (ASMPKT_INF + offsetof(struct pkt_info,rx_buf[n]) + \
                      offsetof(struct pkt_rx_element,x))


/*
 * Called from setup_pkt_inf() in pcpkt.c
 */
static DWORD setup_pkt_inf_fast (void)
{
  /* Allocate space for asmpkt_inf, temp area, pkt-stub and Tx buffer.
   */
  DWORD  rdata_size = PKT_TMP();
  size_t stub_size  = sizeof(real_stub_array);

#if (DOSX)
  rdata_size += PKT_TMP_SIZE + ETH_MAX + 10;
#endif

  if (rdata_size >= 64*1024UL)
  {
    TRACE_CONSOLE (0, "%s (%u): Development error:\nsize %u too large\n",
                   __FILE__, __LINE__, (unsigned int)rdata_size);
    return (0);
  }
  if (stub_size < 0xC0)
  {
    TRACE_CONSOLE (0, "%s (%u): Development error:\n\"pkt_stub.h\""
                      " seems truncated (%d bytes)\n",
                   __FILE__, __LINE__, SIZEOF(real_stub_array));
    return (0);
  }

#if (DOSX & DJGPP)
  {
    int i, seg, sel;

    seg = __dpmi_allocate_dos_memory ((rdata_size+15)/16, &sel);
    if (seg < 0)
       return (0);

    _pkt_inf->rm_seg = seg;
    _pkt_inf->rm_sel = sel;
    rm_base = (_pkt_inf->rm_seg << 4);

    for (i = 0; i < (int)rdata_size; i += 4)  /* clear area */
        POKEL (_pkt_inf->rm_seg, i, 0);
  }

#elif (DOSX & DOS4GW)
  {
    WORD seg, sel;

    seg = dpmi_real_malloc (rdata_size, &sel);
    if (!seg)
       return (0);

    _pkt_inf->rm_seg = seg;
    _pkt_inf->rm_sel = sel;
    rm_base = (_pkt_inf->rm_seg) << 4;
    memset ((void*)rm_base, 0, rdata_size);
  }

#elif (DOSX & PHARLAP)
  {
    WORD largest, seg;
    int  i;

    if (_dx_real_alloc ((rdata_size+15)/16, &seg, &largest) != 0)
       return (0);

    _pkt_inf->rm_seg = seg;
    _pkt_inf->rm_sel = 0;
    RP_SET (rm_base, 0, _pkt_inf->rm_seg);
    for (i = 0; i < (int)rdata_size; i += 4)
        POKEL (_pkt_inf->rm_seg, i, 0);
  }

#elif (DOSX & X32VM)
  {
    int i;

    rm_base = _x32_real_alloc (rdata_size);
    if (!rm_base)
       return (0);

    _pkt_inf->rm_seg = _x32_real_segment (rm_base);
    _pkt_inf->rm_sel = 0;
    for (i = 0; i < (int)rdata_size; i += 4)
        POKEL (_pkt_inf->rm_seg, i, 0);
  }

#elif (DOSX & POWERPAK)
  {
    WORD seg, sel;
    int  i;

    seg = dpmi_real_malloc ((rdata_size+15)/16, &sel);
    if (!seg)
       return (0);

    _pkt_inf->rm_seg = seg;
    _pkt_inf->rm_sel = sel;
    RP_SET (rm_base, 0, _pkt_inf->rm_seg);
    for (i = 0; i < (int)rdata_size; i += 4)
        POKEL (_pkt_inf->rm_seg, i, 0);
  }

#elif (DOSX == 0)
  {
    char _far *buf = _fcalloc (rdata_size, 0);

    if (!buf)
       return (0);
    _pkt_inf->rm_seg = FP_SEG (buf);
    rm_base = (DWORD) buf;
  }

#else
  #error Help!
#endif

  pktq_far_init (sizeof(struct pkt_rx_element), RX_BUFS, ASMPKT_INF);
  _pkt_inf->pkt_queue.num_buf = RX_BUFS;
  return (rm_base);
}

static int setup_rmode_receiver (void)
{
  WORD rx_seg;
  WORD asmpkt_size_chk;
  int  head_size = RX_ELEMENT_HEAD_SIZE;

  WATT_ASSERT (rm_base);
  WATT_ASSERT ((head_size % 4) == 0);

  rx_seg = _pkt_inf->rm_seg;

#if 0  /* test */
  printf ("PktReceiver @ %04X:%04X, ", rx_seg, PktReceiver);
  printf ("_asmpkt_inf @ %04X:%04X\n", rx_seg, (int)sizeof(real_stub_array));
#endif

  *(WORD*)&real_stub_array[asmpkt_inf+0] = sizeof(real_stub_array);
  *(WORD*)&real_stub_array[asmpkt_inf+2] = rx_seg;

  asmpkt_size_chk = *(WORD*) (real_stub_array + size_chk);
  if (asmpkt_size_chk != sizeof(PKT_INFO))
  {
    TRACE_CONSOLE (0, "%s (%u): Development error:\n"
                      "  sizeof(pkt_info) = %ld in pcpkt.h\n"
                      "  sizeof(pkt_info) = %u in asmpkt.nas, (diff %ld)\n",
                   __FILE__, __LINE__,
                   (long)sizeof(PKT_INFO), asmpkt_size_chk,
                   (long)(sizeof(PKT_INFO) - asmpkt_size_chk));
    return (-1);
  }

  if (*(WORD*)&real_stub_array[PktReceiver]   != 0xA80F ||  /* push gs */
      *(WORD*)&real_stub_array[PktReceiver+2] != 0xA00F)    /* push fs */
  {
    TRACE_CONSOLE (0, "%s (%u): Development error:\n"
                      "  PktReceiver misaligned\n", __FILE__, __LINE__);
    return (-1);
  }

  if (!has_rdtsc || !use_rdtsc)
  {
    DWORD patch_it = (*(WORD*) &real_stub_array[patch_nop]) +
                     (DWORD) &real_stub_array;

    TRACE_CONSOLE (4, "patch_it (%04X): %02X,%02X,%02X\n",
                   *(WORD*)&real_stub_array[patch_nop],
                   ((BYTE*)patch_it)[0],
                   ((BYTE*)patch_it)[1],
                   ((BYTE*)patch_it)[2]);

    ((BYTE*)patch_it) [0] = 0x90;  /* NOP */
    ((BYTE*)patch_it) [1] = 0x90;
    ((BYTE*)patch_it) [2] = 0x90;
  }

#if (DOSX & (PHARLAP|POWERPAK|X32VM))
  WriteRealMem (rm_base, &real_stub_array, sizeof(real_stub_array));
#elif (DOSX & DJGPP)
  dosmemput (&real_stub_array, sizeof(real_stub_array), rm_base);
#elif (DOSX & DOS4GW) || (DOSX == 0)
  memcpy ((void*)rm_base, &real_stub_array, sizeof(real_stub_array));
#else
  #error Help me!
#endif
  return (0);
}

static __inline void get_tstamp (DWORD *ts)
{
  ts[0] = ts[1] = 0UL;
#if defined(HAVE_UINT64)
  if (has_rdtsc && use_rdtsc)
     *(uint64*)ts = get_rdtsc();
#else
  ARGSUSED (ts);
#endif
}

/*
 * Check the rx-buffer header.
 */
static BOOL check_rx_element (const struct pkt_rx_element FARDATA *buf)
{
  /* We got an upcall with wrong handle. This can happen if we
   * failed to release handle at program exit. But receive it anyway
   * before releasing the wrong handle.
   */
  if (buf->handle != _pkt_inf->handle)
  {
    pkt_error = "Wrong handle";
    STAT (macstats.num_wrong_handle++);
    pkt_release_handle (buf->handle);
    return (TRUE);
  }
  if (buf->rx_length_1 != buf->rx_length_2)
  {
    pkt_error = "Bad sync";
    STAT (macstats.num_bad_sync++);
    return (FALSE);
  }
  if (buf->rx_length_1 > ETH_MAX)
  {
    pkt_error = "Large size";
    STAT (macstats.num_too_large++);
    return (FALSE);
  }
#if 0  /* this is okay */
  if (buf->rx_length_1 < ETH_MIN)
  {
    pkt_error = "Small size";
    STAT (macstats.num_too_small++);
    return (FALSE);
  }
#endif
  return (TRUE);
}

/*
 * Copy rx-data (if no near-ptr) to 'dest'.
 * 'size' doesn't have to be a multiple of 4.
 */
static __inline void pullup_rx_element (void *dest, DWORD rm_addr, size_t size)
{
#if (DOSX & (PHARLAP|POWERPAK|X32VM))
  ReadRealMem (dest, rm_addr, size);
#elif (DOSX & DOS4GW)
  memcpy (dest, (const void*)rm_addr, size);
#elif (DOSX & DJGPP)
  DOSMEMGETL (rm_addr, size/4, dest);
#elif (DOSX == 0)
  memcpy (dest, (const void*)rm_addr, size);
#else
  #error Help me!
#endif
}


struct pkt_rx_element *pkt_poll_recv (void)
{
  struct pkt_rx_element *rc;
  WORD   out_idx = PEEKW (0, QUEUE_OFS(out_index));
  WORD   in_idx  = PEEKW (0, QUEUE_OFS(in_index));

  if (out_idx != in_idx)
  {
    static struct pkt_rx_element rx_buf;
    struct pkt_ringbuf *q = &_pkt_inf->pkt_queue;
    DWORD  addr = ASMPKT_INF + offsetof(PKT_INFO,rx_buf[out_idx]);

    /* It might be faster to copy the whole thing (head and rx-buffer)
     * in one operation ??
     */
    pullup_rx_element (&rx_buf, addr, RX_ELEMENT_HEAD_SIZE);

    if (check_rx_element(&rx_buf))
    {
      BYTE *pad;
      int   pad_len, size;

      get_tstamp (rx_buf.tstamp_get);
      size    = min (rx_buf.rx_length_1, q->buf_size - 4 - RX_ELEMENT_HEAD_SIZE);
      pad_len = q->buf_size - 4 - RX_ELEMENT_HEAD_SIZE - size;
      addr   += RX_ELEMENT_HEAD_SIZE;
      pullup_rx_element (&rx_buf.rx_buf, addr, ROUND_UP32(size));
      if (pad_len > 0)
      {
        pad = &rx_buf.rx_buf[0] + size;
        memset (pad, 0, pad_len);
      }
      rc = &rx_buf;
    }
    else
    {
      TRACE_CONSOLE (1, "pkt-error %s\n", pkt_error);
      rc = NULL;
    }

    if (++out_idx >= q->num_buf)
       out_idx = 0;
    POKEW (0, QUEUE_OFS(out_index), out_idx);
    return (rc);
  }
  return (NULL);
}

int pkt_buffers_used (void)
{
  BYTE in_idx, out_idx;

  DISABLE();
  out_idx = PEEKW (0, QUEUE_OFS(out_index));
  in_idx  = PEEKW (0, QUEUE_OFS(in_index));
  ENABLE();
  if (in_idx >= out_idx)
     return (in_idx - out_idx);
  return (RX_BUFS + in_idx - out_idx);
}

DWORD pkt_rx_dropped (void)
{
  DWORD rc = PEEKL (0, QUEUE_OFS(num_drop));
  return (rc);
}

/*
 * Append a transmit buffer at end of receive buffer.
 * Must not be called before checking if there is room.
 */
int pkt_append_recv (const void *tx, unsigned len)
{
  struct pkt_rx_element buf;
  struct pkt_ringbuf   *q = &_pkt_inf->pkt_queue;
  DWORD  addr;
  int    idx;

  DISABLE();
  idx = PEEKW (0, QUEUE_OFS(in_index));

  if (idx < 0 || idx >= q->num_buf)
  {
#if defined(USE_DEBUG)
    (*_printf) ("pkt_append_recv(): illegal index %d\n", idx);
 /* pkt_dump_real_mem(); */
#endif
    ENABLE();
    return (0);
  }

  buf.rx_length_1 = buf.rx_length_2 = len;
  buf.handle      = _pkt_inf->handle;
  buf.filler      = 0;
  get_tstamp (buf.tstamp_put);

  addr = ASMPKT_INF + offsetof (PKT_INFO, rx_buf[idx]);

#if (DOSX & (PHARLAP|POWERPAK|X32VM))
  WriteRealMem (addr, &buf, RX_ELEMENT_HEAD_SIZE);
#elif (DOSX & DOS4GW)
  memcpy ((void*)addr, &buf, RX_ELEMENT_HEAD_SIZE);
#elif (DOSX & DJGPP)
  DOSMEMPUTL (&buf, RX_ELEMENT_HEAD_SIZE/4, addr);
#elif (DOSX == 0)
  memcpy ((void*)addr, &buf, RX_ELEMENT_HEAD_SIZE);
#else
  #error Help me!
#endif

  addr += RX_ELEMENT_HEAD_SIZE;

#if (DOSX & (PHARLAP|POWERPAK|X32VM))
  WriteRealMem (addr, (void*)tx, len);
#elif (DOSX & DOS4GW)
  memcpy ((void*)addr, tx, len);
#elif (DOSX & DJGPP)
  DOSMEMPUTL (tx, (len+3)/4, addr);
#elif (DOSX == 0)
  memcpy ((void*)addr, tx, len);
#else
  #error Help me!
#endif

  if (++idx == q->num_buf)
     idx = 0;

  POKEW (0, QUEUE_OFS(in_index), idx);
  ENABLE();
/* pkt_dump_real_mem(); */
  return (len);
}

#if defined(USE_DEBUG)
void pkt_dump_real_mem (void)
{
  unsigned i;

  for (i = 0; i < sizeof(PKT_INFO); i++)
  {
    if ((i % 16) == 0)
       printf ("\n%04X: ", i);
    printf ("%02X,", PEEKB(0,rm_base+i));
  }
  puts ("done");
}

int pkt_test_reordering (void)
{
  /* to-do */
  return (0);
}

#if (DOSX & DJGPP)
  #define DPMI_REAL_CALL(r) \
          !__dpmi_simulate_real_mode_procedure_retf (r)

#elif (DOSX & (DOS4GW|POWERPAK))
  #define DPMI_REAL_CALL(r) \
          dpmi_real_call_retf (r)

#elif (DOSX & (PHARLAP|X32VM)) && 0
  static RMC_BLK rmc;
  #define DPMI_REAL_CALL(r) \
          !_dx_call_real (SEG_OFS_ADDR(r.r_cs,r.r_ip), &rmc, 1)

#elif (DOSX == 0) && 0
  static BOOL DPMI_REAL_CALL (IREGS *r)
  {
    void interrupt (*func)(void) = MK_FP (cs, ip);
    (*func)();
    return (TRUE);
  }

#else
  #define NO_RMODE_CALL
#endif

#define SEG_OFS_ADDR(seg,ofs) (DWORD) (((WORD)(seg) << 4) + (WORD)(ofs))

int pkt_test_upcall (void)
{
#if !defined(NO_RMODE_CALL)
  int i, max = RX_BUFS + 3;

  for (i = 0; i < max; i++)
  {
    eth_Packet eth;
    IREGS      regs;
    DWORD      linear = 0;

    memset (&regs, 0, sizeof(regs));
    memset (&eth, 0, sizeof(eth));
    eth.head.type = IP4_TYPE;
    memset (&eth.head.destination, 0xFF, 6);
    memcpy (&eth.head.source, _eth_addr, 6);

#if (DOSX & (PHARLAP|X32VM))
    rmc.eax = 0;
    rmc.ebx = _pkt_inf->handle;
    rmc.ecx = ETH_MAX;
#endif
    regs.r_cx = ETH_MAX;
    regs.r_bx = _pkt_inf->handle;
    regs.r_ax = 0;
    regs.r_cs = _pkt_inf->rm_seg;
    regs.r_ip = PktReceiver;

    if (DPMI_REAL_CALL(&regs))
    {
      linear = SEG_OFS_ADDR (regs.r_es, regs.r_di);

      printf ("Upcall AX=0: ES:DI %04X:%04X\n",
              (WORD)regs.r_es, (WORD)regs.r_di);
#if (DOSX & DJGPP)
      if (linear)
         dosmemput (&eth, (WORD)regs.r_cx, linear);
#elif (DOSX & (DOS4GW|X32VM))
      if (linear)
         memcpy ((void*)linear, &eth, (WORD)regs.r_cx);
#elif (DOSX & (PHARLAP|POWERPAK))
      if (linear)
      {
        RP_SET (linear, regs.r_es, regs.r_di);
        WriteRealMem (linear, &eth, (WORD)regs.r_cx);
      }
#else
  #error Help me!
#endif
    }
    if (linear)
    {
      regs.r_ss = regs.r_sp = 0;
      regs.r_ax = 1;
      regs.r_cs = _pkt_inf->rm_seg;
      regs.r_ip = PktReceiver;
      regs.r_ds = regs.r_es;
      regs.r_si = regs.r_di;
      if (DPMI_REAL_CALL(&regs))
         printf ("Upcall AX=1\n");
    }
    printf ("buffers used %d, dropped %lu\n",
            pkt_buffers_used(), pkt_rx_dropped());
  }
#endif  /* !NO_RMODE_CALL */

  pkt_dump_real_mem();
  return (0);
}
#endif  /* USE_DEBUG */
#endif  /* USE_FAST_PKT */

