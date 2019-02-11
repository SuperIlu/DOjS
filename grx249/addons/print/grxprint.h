/****************************************************************************
**
**  grxprint.h : Output of graphics on printer from GRX graphic library
**  Version 0.5 (beta)   98/01/26   Andris Pavenis (pavenis@acad.latnet.lv)
**
**  Version 0.6          98/03/03   A.Pavenis
**                       Renamed some procedures
**
**  Version 0.66         98/05/07   H.Schirmer
**         - made cpp # start on first column
**
**  Version 0.67         98/05/10   H.Schirmer
**         - eleminated C++ style comments for better portability
**
**  Version 0.68         98/05/13   H.Schirmer
**         - clean source for better portability / ANSI-C conformance
**
**  This code is port of part of printer BGI driver
**  (C) 1990-1995 Ullrich von Bassewitz (see copying.uz).
**
**  Full version of printer BGI driver version 4.0 can be found
**  at URL   ftp://ftp.musoftware.com/pub/uz/
**  An alternate URL is http://www.lanet.lv/~pavenis/printerbgi+src.zip
**
*****************************************************************************/

#ifndef __GRXPRINT_H
#define __GRXPRINT_H

  enum __grxPrintModes
    {
	 FX_240x72              = 0,   /* EPSON FX (8-Nadel), 240 * 72 DPI */
	 FX_240x216             = 1,   /* EPSON FX (8-Nadel), 240 * 216 DPI */
	 LQ_180x180             = 2,   /* EPSON LQ (24-Nadel), 180 * 180 DPI */
	 LQ_360x180             = 3,   /* EPSON LQ (24-Nadel), 360 * 180 DPI */
	 LQ_360x360             = 4,   /* EPSON LQ (24-Nadel), 360 * 360 DPI */

	 P6_360x360             = 5,   /* NEC P6, P6+, P60 (24-Nadel), 360 * 360 DPI (1) */
	 X24_180x180            = 6,   /* IBM Proprinter X24 (24-Nadel), 180 * 180 DPI */
	 X24_360x180            = 7,   /* IBM Proprinter X24 (24-Nadel), 360 * 180 DPI */

	 LQ_P6_180x180x9        = 8,   /* EPSON LQ / NEC P6, P6+, 180 * 180 DPI, 9 Farben (6) */
	 LQ_P6_360x180x9        = 9,   /* EPSON LQ / NEC P6, P6+, 360 * 180 DPI, 9 Farben (6) */
	 LQ_P6_360x360x9        = 10,  /* EPSON LQ, 360 * 360 DPI, 9 Farben               (6) */
	 P6_360x360x9           = 11,  /* NEC P6, P6+, 360 * 360 DPI, 9 Farben            (6) */

	 GRX_PRN_RESERVED_1     = 12,  /* Reserviert */
	 GRX_PRN_USER1          = 13,  /* Benutzerdefinierter Modus 1 (not implemented)   (2) */
	 GRX_PRN_USER2          = 14,  /* Benutzerdefinierter Modus 2 (not implemented)   (2) */
	 GRX_PRN_USER3          = 15,  /* Benutzerdefinierter Modus 3 (not implemented)   (2) */

	 HPLJ_75x75             = 16,  /* HP LJ, 75 * 75 DPI                              (3) */
	 HPLJ_100x100           = 17,  /* HP LJ, 100 * 100 DPI                            (3) */
	 HPLJ_150x150           = 18,  /* HP LJ, 150 * 150 DPI                            (3) */
	 HPLJ_300x300           = 19,  /* HP LJ, 300 * 300 DPI                            (3) */
	 HPLJ_75x75_NC          = 20,  /* HP LJ, 75 * 75 DPI, keine Kompression           (4) */
	 HPLJ_100x100_NC        = 21,  /* HP LJ, 100 * 100 DPI, keine Kompression         (4) */
	 HPLJ_150x150_NC        = 22,  /* HP LJ, 150 * 150 DPI, keine Kompression         (4) */
	 HPLJ_300x300_NC        = 23,  /* HP LJ, 300 * 300 DPI, keine Kompression         (4) */

	 HPDJ500C_75x75x8       = 24,  /* HP DJ 500C, 75 * 75 DPI, 8 Farben, A4 */
	 HPDJ500C_100x100x8     = 25,  /* HP DJ 500C, 100 * 100 DPI, 8 Farben, A4 */
	 HPDJ500C_150x150x8     = 26,  /* HP DJ 500C, 150 * 150 DPI, 8 Farben, A4 */
	 HPDJ500C_300x300x8     = 27,  /* HP DJ 500C, 300 * 300 DPI, 8 Farben, A4 */

	 HPDJ500C_75x75x8_B     = 28,  /* HP DJ 550C, 75 * 75 DPI, 8 Farben, echtes Schwarz       (7) */
	 HPDJ500C_100x100x8_B   = 29,  /* HP DJ 550C, 100 * 100 DPI, 8 Farben, echtes Schwarz     (7) */
	 HPDJ500C_150x150x8_B   = 30,  /* HP DJ 550C, 150 * 150 DPI, 8 Farben, echtes Schwarz     (7) */
	 HPDJ500C_300x300x8_B   = 31,  /* HP DJ 550C, 300 * 300 DPI, 8 Farben, echtes Schwarz     (7) */

	 HPLJ_600x600           = 32,  /* HP LJ IV, 600 * 600 DPI                                 (5) */
	 LQ_180x180_A3          = 33,  /* EPSON LQ (24-Nadel), 180 * 180 DPI, DIN A3 */
	 LQ_360x180_A3          = 34,  /* EPSON LQ (24-Nadel), 360 * 180 DPI, DIN A3 */
	 LQ_360x360_A3          = 35,  /* EPSON LQ (24-Nadel), 360 * 360 DPI, DIN A3 */
	 P7_360x360_A3          = 36,  /* NEC P7 (24-Nadel), 360 * 360 DPI, DIN A3                (1) */
	 LQ_180x180x9_A3        = 37,  /* EPSON LQ (24-Nadel), 180 * 180 DPI, DIN A3, 9 Farben    (6) */
	 LQ_360x180x9_A3        = 38,  /* EPSON LQ (24-Nadel), 360 * 180 DPI, DIN A3, 9 Farben    (6) */
	 LQ_360x360x9_A3        = 39,  /* EPSON LQ (24-Nadel), 360 * 360 DPI, DIN A3, 9 Farben    (6) */
	 P7_360x360x9_A3        = 40,  /* NEC P7 (24-Nadel), 360 * 360 DPI, DIN A3, 9 Farben    (1) (6) */

	 HPDJ1200C150           = 41,  /* Deskjet 1200C, 256 Farben */
	 HPLJ_1200x1200         = 42   /* HP LJ4000, 1200 * 1200 DPI                              (5) */
    };

#ifdef __cplusplus
  extern "C" {
#endif

  int   GrPrintSetMode (int mode);

  int   GrPrintToFile (const char * DestFile);

  int   GrDoPrinting (void);

  void  GrPrintGetAspectRatio ( unsigned * x , unsigned * y );


  /******************************************************************************/
  /*  Setting and quering some printer settings. These procedures               */
  /*  are usufull only with LaserJet or DeskJet printers                        */
  /******************************************************************************/


  struct  GrPrintOptionsType
	  {
	      short Quality;    /* see enum __GrPrintQuality */

	      short Shingling;  /* 0*   - normal       */
				/* 1    - 25% (2 pass) */
				/* 2    - 50% (4 pass) */

	      short Depletion;  /* 0    - none */
				/* 1*   - 25%  */
				/* 2    - 50%  */

	      short MediaType;  /* 0*   - plain paper       */
				/* 1    - bond paper        */
				/* 2    - special paper     */
				/* 3    - glossy film       */
				/* 4    - transparency film */
	  };


  void    GrPrintGetDefaultOptions ( struct GrPrintOptionsType * opt );
  void    GrPrintGetCurrentOptions ( struct GrPrintOptionsType * opt );
  void    GrPrintSetOptions        ( struct GrPrintOptionsType * opt );

#ifdef __cplusplus
  };
#endif


#endif
