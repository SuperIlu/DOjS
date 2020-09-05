/*!\file asmpkt.h
 *
 * Prototypes for functions in \b asmpkt.asm and \b asmpkt4.asm
 */
#ifndef _w32_ASMPKT_H
#define _w32_ASMPKT_H

#if (DOSX == 0)                                  /* for real-mode targets */
  extern void far cdecl pkt_receiver_rm (void);  /* in asmpkt.asm */

#elif (DOSX & DOS4GW) && !defined(USE_FAST_PKT)  /* in asmpkt4.asm */
  extern WORD             cdecl asmpkt_size_chk;
  extern struct pkt_info *cdecl asmpkt_inf;

  extern void cdecl pkt_receiver4_start (void);
  extern void cdecl pkt_receiver4_rm    (void);
  extern void cdecl pkt_receiver4_end   (void);
#endif

#endif

