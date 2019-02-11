/*****************************************************************************/
/*                                                                           */
/* grxprint.cc : Output of graphics on printer from GRX graphic library      */
/* Version 0.5 (beta)   98/01/26   Andris Pavenis (pavenis@acad.latnet.lv)   */
/*        - Initial version                                                  */
/*                                                                           */
/* Version 0.51         98/02/18   A.Pavenis                                 */
/*        - Fixes to allow to work under Linux (still printing to file only) */
/*                                                                           */
/* Version 0.52         98/02/24   A.Pavenis                                 */
/*        - Changed name of function from GrDoPrinting to GrPrintToFile      */
/*        - Added function GrDoPrinting(void) that prints to prn in MS-DOS   */
/*          and pipes output to lpr in Linux                                 */
/*        - Some other small changes, such as saving current video driver in */
/*          GrSetPrintMode and restoring it from GrDoPrinting                */
/*          or GrPrintToFile                                                 */
/*                                                                           */
/* Version 0.6          98/02/28   A.Pavenis                                 */
/*        - Many changes and fixed bugs, e.g. the mirror image was           */
/*          printed                                                          */
/*                                                                           */
/* Version 0.61         98/03/03   A.Pavenis                                 */
/*        - Fixed problem when printing directly to printer (DJGPP version). */
/*          The opened file was opened in text mode (old bug in libc).       */
/*          A workaround for this problem is used                            */
/*        - Get rid of most of warnings that appears with -Wall              */
/*                                                                           */
/* Version 0.65         98/03/09   A.Pavenis                                 */
/*        - Changed GrPrintToFile() under Linux. Now GrPrintToFile("|lpr")   */
/*          is equivalent to GrDoPrinting();                                 */
/*                                                                           */
/* Version 0.66         98/05/07   H.Schirmer                                */
/*        - minor changes for Watcom support                                 */
/*          made cpp # start on first column                                 */
/*                                                                           */
/* Version 0.67         98/05/10   H.Schirmer                                */
/*        - eleminated C++ style comments for better portability             */
/*                                                                           */
/* Version 0.68         98/05/13   H.Schirmer                                */
/*        - clean source for better portability / ANSI-C conformance         */
/*                                                                           */
/* Version 0.7          98/05/14   A.Pavenis                                 */
/*        - internal procedures that are used only from this file are        */
/*          defined static and the definitions are removed from prn000.h     */
/*        - changed restoring previous state after printing to avoid         */
/*          unnecessary autodetection of video driver                        */
/*                                                                           */
/* This code is port of part of printer BGI driver                           */
/* (C) 1990-1995 Ullrich von Bassewitz (see copying.uz).                     */
/* Only the code of printing itself is ported.                               */
/*                                                                           */
/* Full version of printer BGI driver version 4.0 can be found               */
/* at URL   ftp://ftp.musoftware.com/pub/uz/printerbgi+src.zip               */
/* An alternate URL is http://www.lanet.lv/~pavenis/printerbgi+src.zip       */
/*                                                                           */
/*****************************************************************************/

/*
>>>>> We need to configure this code on Unix like systems <<<<<
>>>>> for popen(), ...                                    <<<<<
*/

#if defined(__MSDOS__)
# include <io.h>
# include <dos.h>
#endif

#include <fcntl.h>
#include <grx20.h>
#include <stdio.h>
#include <assert.h>
#include <libgrx.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <grxprint.h>
#include <grxprn00.h>
#include <sys/stat.h>
#include <stddef.h>
#if !defined(__WATCOMC__) && !defined(__DJGPP__) && !defined(__WIN32__)
#include <sys/ioctl.h>
#endif

static  short         InitDone=0;
static  short         Mode=-1;                /* Print mode to be used */
static  GrColor       numGrxColors;           /* Number of colors for GRX to use */
static  int           MaxX=0, MaxY=0;
static  long          AspectRatio=10000L;

static  jmp_buf       PrintAborted;


/****************************************************************************/
/*                      Extern deklarierte Variable                         */
/****************************************************************************/

/* Die Farbtabelle wird (da nicht verwendet) als Moeglichkeit zur Einstellung */
/* von diversen Daten von aussen benutzt. Sie heisst daher nicht ColorTable   */
/* (wie im SVGA-Treiber), sondern Settings (fuer die aktuelle Tabelle) und    */
/* DefaultSettings (fuer die Default-Einstellungen).                          */

static struct GrPrintOptionsType  DefaultSettings = {
    0,                          /* Output quality, 0 --> keypad setting */
				/*                 1 --> draft */
				/*                 2 --> high */
    0,                          /* Shingling,      0 --> normal */
				/*                 1 --> 25% (2 pass) */
				/*                 2 --> 50% (4 pass) */
    1,                          /* Depletion       0 --> none */
				/*                 1 --> 25% */
				/*                 2 --> 50% */
    0                           /* Media type,     0 --> Plain paper */
				/*                 1 --> Bond paper */
				/*                 2 --> Special paper */
				/*                 3 --> Glossy film */
				/*                 4 --> Transparency film */
};



/* Die folgenden Farbtabelle wird als Einstellungstabelle "missbraucht" */
static struct GrPrintOptionsType  Settings = {
    0,                          /* Output quality, 0 --> keypad setting */
				/*                 1 --> draft */
				/*                 2 --> high */
    0,                          /* Shingling,      0 --> normal */
				/*                 1 --> 25% (2 pass) */
				/*                 2 --> 50% (4 pass) */
    1,                          /* Depletion       0 --> none */
				/*                 1 --> 25% */
				/*                 2 --> 50% */
    0                           /* Media type,     0 --> Plain paper */
				/*                 1 --> Bond paper */
				/*                 2 --> Special paper */
				/*                 3 --> Glossy film */
				/*                 4 --> Transparency film */
};




/* RGB-Palette */
static struct RGBEntry RGBPal [256];

/* Der Zeiger auf die aktuelle DST */
static struct _DST * DSTPtr = NULL;

/*****************************************************************************/
/*  Prototypes of internal procedures                                        */
/*****************************************************************************/

  static BOOLEAN InitPrinter (BYTE *Buf, int BufSize, int Handle);
  /* Stellt das Ausgabegeraet auf "binary" um und prueft gleichzeitig, ob    */
  /* das Handle 4 ok (offen) ist. Der aktuelle Zustand wird gemerkt und bei  */
  /* Post wiederhergestellt. Buf wird als Puffer fuer die Ausgabe verwendet  */
  /* und muss PrintBufSize Bytes gross sein, Handle ist das Handle auf das   */
  /* ausgegeben wird.                                                        */

  static void ResetPrinter (void);
  /* Stellt den orginalen Zustand des Ausgabegeraets wieder her. */

  static void Flush (void);
  /* Schreibt den Ausgabepuffer leer. Muss am Ende eines Ausdrucks           */
  /* aufgerufen werden. Springt PrintAbortLabel an bei Fehlern.              */

  static void PrintByte (BYTE B);
  /* Gibt ein Byte auf den Drucker (bzw. in den Ausgabepuffer) aus. */
  /* Springt PrintAbortLabel an bei Fehlern.                        */

  static void PrintData (BYTE * Data, unsigned Size);
  /* Gibt Daten auf den Drucker (bzw. in den Ausgabepuffer) aus. */
  /* Springt PrintAbortLabel an bei Fehlern.                     */

  static void PrintString (char * S);
  /* Gibt einen Pascal-String auf den Drucker (bzw. in den Ausgabepuffer) aus.*/
  /* Springt PrintAbortLabel an bei Fehlern.                                  */

  static void PrintZString (char * S);
  /* Gibt einen nullterminierten String auf den Drucker (bzw. in den */
  /* Ausgabepuffer) aus. Springt PrintAbortLabel an bei Fehlern.     */


/****************************************************************************/
/*                                                                          */
/*                             Interne Variablen                            */
/*                                                                          */
/****************************************************************************/


/* Default-Palette fuer die ersten Eintraege */
static struct RGBEntry RGBDefPal [8] = {
    {    0,   0,   0 },         /* Black */
    {  255,   0,   0 },         /* Red */
    {    0, 255,   0 },         /* Green */
    {  255, 255,   0 },         /* Yellow */
    {    0,   0, 255 },         /* Blue */
    {  255,   0, 255 },         /* Magenta */
    {  255, 255,   0 },         /* Cyan */
    {  255, 255, 255 }          /* White */
};


static GrVideoDriver * PrevDrv = NULL;


int   GrPrintSetMode ( int _mode_ )
  {
      int   rc;
      unsigned  I;
      struct RGBEntry * P;
      PrevDrv = GrDriverInfo->vdriver;

      Mode = _mode_;
      if (Mode<0 || Mode>MaxModes) return -1;        /* Check mode range */
      if (DSTTable[Mode]==0) return -1;

      DSTPtr = DSTTable[Mode];

      MaxX = (int) ((((long) (DSTPtr->XDPI))*((long) (DSTPtr->XInch)))/1000L);
      MaxY = (int) ((((long) (DSTPtr->YDPI))*((long) (DSTPtr->YInch)))/1000L);
      AspectRatio = (long) ((10000L*DSTPtr->YDPI)/DSTPtr->XDPI);

      if (DSTPtr->ColorBits==1) numGrxColors=2;
      else if (DSTPtr->ColorBits<=4) numGrxColors=16;
      else if (DSTPtr->ColorBits<=8) numGrxColors=256;
      else numGrxColors=256;

      /* Palette initialisieren, erste 8 Eintraege wie beim DeskJet */
      memcpy (RGBPal, RGBDefPal, sizeof (RGBDefPal));

      /* Rest der Palette mit Grautoenen vorbesetzen */
      I = sizeof (RGBDefPal) / sizeof (struct RGBEntry);
      P = &RGBPal [I];
      while (I < sizeof (RGBPal) / sizeof (struct RGBEntry)) {
	  P->R = P->G = P->B = I;
	  P++;
	  I++;
      }

      rc = GrSetDriver ("memory");
      if (rc==TRUE)
	{
	    rc = GrSetMode (GR_width_height_color_graphics, MaxX, MaxY, numGrxColors);
	    InitDone = 1;
	}
      return rc;
  }




int    GrPrintToFile (const char * DestFile)
  {
      int   handle;
#     ifdef __MSDOS__
	handle = creat (DestFile,S_IREAD+S_IWRITE);
	if (!handle) return -1;
	if (DSTPtr) (DSTPtr->Print) (DSTPtr,handle);
	close (handle);
	GrSetMode (GR_default_text);
#     else
	FILE * output;
	if (*DestFile=='|') output = popen (DestFile+1,"w");
		 else       output = fopen (DestFile  ,"w");
	if (!output) return -1;
	handle = fileno (output);
	if (DSTPtr) (DSTPtr->Print) (DSTPtr,handle);
	if (*DestFile=='|') pclose (output);
		  else      fclose (output);
#     endif
      if (PrevDrv) GrSetDriver (PrevDrv->name);
	  else     DRVINFO->vdriver = NULL;
      return 0;
  }



int    GrDoPrinting (void)
  {
      int    handle;
#     ifdef __MSDOS__
	handle = creat ("prn",S_IREAD+S_IWRITE);
	if (!handle) return -1;
#     else
	FILE * output = popen ("lpr","w");
	if (!output) return -1;
	handle = fileno (output);
#     endif
      if (DSTPtr) (DSTPtr->Print) (DSTPtr,handle);
#     ifdef __MSDOS__
	close (handle);
#     else
	pclose (output);
#     endif
      GrSetMode (GR_default_text);
      if (PrevDrv) GrSetDriver (PrevDrv->name);
	  else     DRVINFO->vdriver = NULL;
      return 0;
  }


void   GrPrintGetAspectRatio ( unsigned * x , unsigned * y )
  {
       *x = AspectRatio;
       *y = 10000U;
  }


/*****************************************************************************/
/*                                 NP.CPP                                    */
/*                                                                           */
/*                                                                           */
/* (C) 1994 by Ullrich von Bassewitz                                         */
/*             Zwehrenbuehlstrasse 33                                        */
/*             72070 Tuebingen                                               */
/*                                                                           */
/* E-Mail:     uz@ibb.schwaben.de                                            */
/*                                                                           */
/* Port of printing code to DJGPP                                            */
/* Revision 0.5   98/01/26   Andris Pavenis (pavenis@acad.latnet.lv)         */
/*                                                                           */
/*****************************************************************************/


/*****************************************************************************/
/*                                                                           */
/*      Printing on dot matrix printers                                      */
/*                                                                           */
/*****************************************************************************/
/*                                                                           */
/*                                                                           */
/* Anmerkung zur Speicheraufteilung: Die urspruengliche Speicheraufteilung   */
/* wurde so geaendert, dass sich alle Bytes einer Druckerspalte (also bis    */
/* zu 3 bei einem 24-Nadler) linear hintereinander im Speicher befinden.     */
/* Dadurch ergibt sich ein wesentlich schnellerer Ausdruck, da eine          */
/* komplette Zeile mit Druckdaten am Stueck im Speicher steht.               */
/*                                                                           */
/* Die Berechnung fuer Adresse und Pixel erfolgt nach folgenden Formeln:     */
/*                                                                           */
/* Pixelmaske im Byte:                                                       */
/*                                                                           */
/*      PixelMask = 0x01 << (7 - (X % 8));                                   */
/*                                                                           */
/* bzw. (bei Reverse)                                                        */
/*                                                                           */
/*      PixelMask = 0x01 << (X % 8);                                         */
/*                                                                           */
/* Lineare Adresse des Bytes in dem das Pixel steht (Achtung: die Formel     */
/* kann _nicht_ vereinfacht werden, d.h. es darf nichts weggekuerzt werden,  */
/* da es sich um Ganzzahlarithmetik handelt).                                */
/*                                                                           */
/*                X                                                          */
/*   Abs = -------------- * MaxY * ColBytes                                  */
/*          8 * ColBytes                                                     */
/*                                                                           */
/*                                                                           */
/*        + (MaxY - Y - 1) * ColBytes                                        */
/*                                                                           */
/*                                                                           */
/*           X % (8 * ColBytes)                                              */
/*        + --------------------                                             */
/*                   8                                                       */
/*                                                                           */

/****************************************************************************/
/*                           Drucker-Ausgaberoutinen                        */
/****************************************************************************/

void  EpsonPrint ( struct _DST * DSTPtr , int PRNHandle )
/* Universelle Drucker-Routine fuer Nadeldrucker */

{
    /*
    ** Steuerstrings fuer EPSON/NEC P6 Farbe. Diese sind bisher fest
    ** eincodiert, da die Drucker alle dieselben Steuerstrings haben.
    ** ACHTUNG: Zwei der Bit-Ebenen sind vertauscht um eine dem DeskJet
    ** aehnliche Farb-Verteilung zu erhalten.
    ** Alle Steuerstrings enthalten zusaetzlich zuerst einen Wagenruecklauf.
    */

    static char *ColorSel [4] = {
	"\x04\r\x1Br\x02",              /* Bit 0 = Cyan */
	"\x04\r\x1Br\x01",              /* Bit 1 = Magenta */
	"\x04\r\x1Br\x04",              /* Bit 2 = Gelb */
	"\x04\r\x1Br\x00"               /* Bit 3 = Schwarz */
    };

    WORD X, Y, EndX;
    BYTE Bytes, Pass, PassCount, ColBytes, ColorBits, Bits;
    DWORD Abs;
    /*DWORD A;*/
    /*BYTE Buf [6];     */ /* Maximale Groesse: ColBytes(max) * PassCount(max) */
    /*BYTE PrintBuf [3];*/ /* Maximale Groesse: ColBytes(max) */
    /*int   I,J,K;*/
    char  Reverse;
    int   MaxX = GrSizeX();
    int   MaxY = GrSizeY();
    unsigned char *Buffer[4];  /* Where to create buffer for printing */
    unsigned char *Curr[4] , Mask;
    char OrVal[4];
    int i, j, k, l, c, X1;
    int PlaneSize, BufSize;

    struct EpsonDST * dst = (struct EpsonDST *) DSTPtr;

    /* Handle auf "raw data" umstellen. Direkt Ende wenn Handle nicht Ok */
    BYTE * OutputBuf = (BYTE *) malloc (PrintBufSize);
    assert (OutputBuf!=0);
    if (InitPrinter (OutputBuf, PrintBufSize, PRNHandle) == FALSE) return;

    /* Vor Beginn der Druck-Ausgabe jetzt das Sprunglabel fuer Druckerfehler */
    /* setzen. */

    if (setjmp(PrintAborted)!=0) { /* Fehler! */  return; }

    PrintString (dst->GraphicsOn);      /* Variablen-Init */

    Abs          = 0L;
    PassCount    = dst->PassCount;
    ColBytes     = dst->ColBytes;
    ColorBits    = DSTPtr->ColorBits;
    Bytes        = ColBytes * PassCount;
    EndX         = MaxX / (ColBytes * 8);
    Reverse      = DSTPtr->Flags & pfReverse;

    X = 0;

    PlaneSize = ((int) ColBytes)*MaxY;
    BufSize = PlaneSize*ColorBits;
    Buffer[0] = (unsigned char *) malloc (BufSize);
    if (Buffer[0]==0) { fprintf(stderr,"Not enough memory\n");
			longjmp(PrintAborted,-1);             }
    for (i=1; i<ColorBits; i++) Buffer[i]=Buffer[i-1]+PlaneSize;

    while (X<MaxX)
      {
	 for (Pass=0; Pass<PassCount; Pass++)
	   {
	      memset (Buffer[0],0,BufSize);  /* Clear buffer */
	      for (i=0; i<ColorBits; i++)
		{
		    Curr[i] = Buffer[i];
		    OrVal[i] = 0;
		}
	      for (Y=0; Y<MaxY; Y++)
		{
		    X1 = Pass;
		    for (j=0; j<ColBytes; j++)
		      {
			 Mask = Reverse ? 0x01 : 0x80U;
			 for (k=0; k<8; k++)
			   {
			       int  cMask = 0x01;
			       int  xPos = MaxX - X - X1 - 1;
			       c = xPos>=0 ? (GrPixelNC (xPos,Y)) : 0;
			       /* Insert color conversion here if needed */
			       X1 += PassCount;
			       for (l=0; l<ColorBits; l++, cMask<<=1)
				 if (c & cMask)
				   *(Curr[l]) |= Mask;
			       if (Reverse) Mask <<=1; else Mask >>= 1;
			   }
			 for (l=0; l<ColorBits; l++)
			   {
			      OrVal[l] |= *((Curr[l])++);
			   }
		      }
		}

	      for (Bits=0; Bits<ColorBits; Bits++)
		{
		   if (OrVal[Bits])
		     {
			/* Falls mehr als eine Farbe existiert, */
			/* jetzt Farbe umschalten               */
			if (ColorBits > 1) PrintString (ColorSel [Bits]);
			PrintString (dst->PreBytes);
			PrintData   (Buffer[Bits], PlaneSize);
			PrintString (dst->PostBytes);
		     }
		}


	      if (Pass == (PassCount - 1))
		  PrintString (dst->LineFeed1);
	      else
		  PrintString (dst->LineFeed2);
	   }

	 X += PassCount*ColBytes*8;
	 Flush ();
      }
    /* Grafik beenden, Puffer leeren */
    Flush ();
    ResetPrinter ();
    free (Buffer[0]);
    free (OutputBuf);
}


/****************************************************************************/
/*                                                                          */
/*         LaserJet/DeskJet-Ausgaberoutinen fuer BGI-Druckertreiber         */
/*                                                                          */
/****************************************************************************/
/*                                                                          */
/* (C) 1990-1993 Ullrich von Bassewitz                                      */
/*                                                                          */
/* Port to printing code to DJGPP                                           */
/* Revision 0.5  1998/01/26  Andris Pavenis (pavenis@acad.latnet.lv)        */
/* This code is not yet optimized and is rather slow espacially for         */
/* color printing. Tested on HP LaserJet 4L (Modes HPLJ_300x300,            */
/* HPLJ_300x300_NC) and on HP DeskJet 690C (Mode HPDJ500C_300x300x8_B).     */
/* Other modes are not tested.                                              */
/*                                                                          */
/* $Id: lj.cpp 2.10 1995/04/28 16:20:46 Uz Exp $                            */
/*                                                                          */
/* $Log: lj.cpp $                                                           */
/* Revision 2.10  1995/04/28 16:20:46  Uz                                   */
/* Umstellung auf PCX-Treiber.                                              */
/*                                                                          */
/* Revision 2.9  95/04/22  17:32:48  Uz                                     */
/* Diverse Aenderungen, Funktionen LaserPrint und LJPrint zusammengefasst,  */
/* Anpassungen an geaenderte Struktur von _DST und Support fuer DeskJet     */
/* 1200C mit einstellbarer Palette.                                         */
/*                                                                          */
/* Revision 2.8  94/09/08  14:14:38  Uz                                     */
/* Kleinere Aenderungen zur Einsprung von ein paar Bytes.                   */
/*                                                                          */
/* Revision 2.7  94/09/08  09:32:52  Uz                                     */
/* Anpassung an extra modedata Modul.                                       */
/*                                                                          */
/* Revision 2.6  94/03/29  20:44:54  Uz                                     */
/* str.h anstelle von string.h verwendet                                    */
/*                                                                          */
/* Revision 2.5  94/03/19  16:16:51  Uz                                     */
/* Cleanup und Kosmetik.                                                    */
/*                                                                          */
/* Revision 2.4  94/01/13  11:32:50  Uz                                     */
/* Ausdruck erweitert um schaltbare Schwarzabtrennung                       */
/* fuer DeskJet 550C.                                                       */
/*                                                                          */
/* Revision 2.3  93/08/01  20:53:05  Uz                                     */
/* Neues Format mit DPMI-Support                                            */
/*                                                                          */

/****************************************************************************/
/*                   Controlstrings zur Ansteuerung des LJ                  */
/****************************************************************************/

/* defining LJ_LANDSCAPE_MODE changes order of data data output to          */
/* printer to more effectively use memory organization of GRX frame drivers */
/* Unfortunatelly I was not able to remove some shift of image so part      */
/* of it was not printed (top was shifted down by about 6 mm)               */
/* (A.Pavenis)                                                              */
/*#define  LJ_LANDSCAPE_MODE*/

/* Drucker-Kontrollstrings */
#ifndef LJ_LANDSCAPE_MODE
static char InitString [] =
    "\x1B""E"                           /* Reset Printer */
    "\x1B*rbC"                          /* End Raster Graphics */
    "\x1B&l26A"                         /* Format: DIN A4 */
    "\x1B&l0o"                          /* Portrait orientation */
    "0L";                               /* perf.-skip off */
#else
static char InitString [] =
    "\x1B""E"                           /* Reset Printer */
    "\x1B*rbC"                          /* End Raster Graphics */
    "\x1B&l26A"                         /* Format: DIN A4 */
    "\x1B&l1o"                          /* Landscape orientation */
    "\x1B*r0F"                          /* Follows orientation */
    "\x1B&l6D"                          /* Vertical line spacing */
    "\x1B&l0E"                          /* Top margin 0 */
    "\x1B*p0Y"                          /* Y position is 0 */
    "\x1B*p0Y\x1B*p0X"                  /* Cursor position 0,0 */
    "0L";                               /* perf.-skip off */
#endif

static char ColorString1 [] =
    "\x1B*r-3U";                        /* 3 planes, CMY palette */

static char ColorString2 [] =
    "\x1B*r-4U";                        /* 4 planes, KCMY palette */

static char ColorString3 [13] =
    "\x0B"                              /* Stringlaenge */
    "\x1B*v6W"                          /* CID Command */
    "\x00"                              /* Device RGB */
    "\x00"                              /* Indexed by plane */
    "\x00"                              /* Bits/Index (wird spaeter gesetzt) */
    "\x08\x08\x08";                     /* 8 Bits per Primary */

static char Quality [3][6] = {
    "\x1B*r0Q",                         /* Use keypad setting */
    "\x1B*r1Q",                         /* draft */
    "\x1B*r2Q"                          /* high */
};

static char Shingling [3][6] = {
    "\x1B*o0Q",                         /* None */
    "\x1B*o1Q",                         /* 25% (2 pass) */
    "\x1B*o2Q"                          /* 50% (4 pass) */
};

static char Depletion [3][6] = {
    "\x1B*o1D",                         /* None */
    "\x1B*o2D",                         /* 25% (default) */
    "\x1B*o3D"                          /* 50% */
};

static char MediaType [5][6] = {
    "\x1B&l0M",                         /* Plain paper */
    "\x1B&l1M",                         /* Bond paper */
    "\x1B&l2M",                         /* Special paper */
    "\x1B&l3M",                         /* Glossy film */
    "\x1B&l4M"                          /* Transparency film */
};

#ifdef LJ_LANDSCAPE_MODE
static char StartGraphics [] =
    "\x1B*r1A";                         /* Start graphics */
/*  "\x1B&a1N";                      */ /* No negative motion (DeskJet 1200C) */
#else
static char StartGraphics [] =
    "\x1B*r0A";                         /* Start graphics*/
/*  "\x1B&a1N";                      */ /* No negative motion (DeskJet 1200C) */
#endif


/* Steuerstring zur Einstellung der Palette.*/
static char RGBCompStr [] =
    "\x03"                              /* Stringlaenge */
    "\x1B*v";

/* String der nach der Grafik geschickt wird. */
static char EndGraphics [] =
    "\x1B*rbC"                          /* Grafik-Ende */
    "\x0C";                             /* Papier-Auswurf */

/* Steuerstring fuer TIFF-Pacbits Kompression */
static char Compression [] =
    "\x1B*b2M";

/* Steuerstring fuer keine Kompression */
static char NoCompression [] =
    "\x1B*b0M";

/* Hier werden die Steuercodes fuer eine Zeile zusammengebaut. */
static char PreBytes [25] =
    "\x1B*b";


/****************************************************************************/
/*                                                                          */
/*                 Kompressionspuffer und dessen Verwaltung                 */
/*                                                                          */
/****************************************************************************/

/*#define CompBufSize 600              */ /* 600 Bytes Puffer */
#define CompBufSize 6000                  /* 6000 Bytes Puffer */
static BYTE * CompBuf = 0;                /* Zeiger auf Kompressionspuffer */
static WORD   CompBufFill = 0;            /* Anzahl Zeichen im Puffer */



static  void  ToBuf (BYTE B)
/* Speichert das uebergebene Byte im Puffer wenn noch Platz ist. Der Zaehler */
/* wird auf jeden Fall hochgezaehlt, so dass sich spaeter die theoretische   */
/* Anzahl an Bytes im Puffer auf jeden Fall feststellen laesst.              */
{
    if (CompBufFill < (CompBufSize-1))
    {   /* Platz ist da, speichern */
	CompBuf [CompBufFill++] = B;
    }
}



static void RepeatByte (WORD bCount, BYTE B)
/* Speichert ein wiederholtes Byte im Kompressionspuffer, wobei Count die */
/* echte Anzahl ist und B das Byte. Der Zaehler wird auf Maximum          */
/* ueberprueft und eventuell auf zwei oder drei verteilt.                 */
{
    int RepeatCount;

    while (bCount) {
	/* Maximal koennen 128 Bytes am Stueck geschrieben werden, wobei  */
	/* der Wert 0 einem Byte entspricht usw. 127 entsprechen 128 Byte */
	RepeatCount = (bCount > 128) ? 128 : bCount;
	bCount -= RepeatCount;

	/* Byte schreiben */
	ToBuf (-(RepeatCount-1));
	ToBuf (B);
    }
}




static void  MultiByte (WORD bCount, char  *S)
/* Speichert eine Folge von ungleichen Bytes im Kompressionspuffer, wobei */
/* Count die echte Anzahl ist und S ein Zeiger auf das erste Byte. Der    */
/* Zaehler wird auf Maximum ueberprueft und die komplette Anzahl wird     */
/* evtl. auf mehrere Male rausgeschrieben.                                */
{
    WORD ByteCount;

    while (bCount) {
	/* Maximal koennen 128 Bytes am Stueck geschrieben werden, wobei der */
	/* Wert 0 einem Byte entspricht usw. 127 entsprechen 128 Byte        */
	ByteCount = (bCount > 128) ? 128 : bCount;
	bCount -= ByteCount;

	/* Bytefolge schreiben */
	ToBuf (ByteCount-1);
	while (ByteCount--) {
	    ToBuf (*S++);
	}
    }
}



static void  PrintNum (WORD W)
/* Gibt das Wort W vorzeichenlos in ASCII auf den Drucker aus. Zur Wandlung */
/* wird der Kompressionspuffer verwendet!                                   */
{
    /*Num (W, CompBuf);*/
    char numBuf[16];
    /*itoa (W,numBuf,10);*/
    sprintf (numBuf,"%u",(unsigned) W);
    PrintZString (numBuf);
}


/****************************************************************************/
/* Some setup functions for LaserJet and DeskJet printers                   */
/****************************************************************************/

static  void    ljSetRasterSize (int width, int height)
{
    char  cmd[32];
    sprintf (cmd,"\x1B*r%dT\x1B*r%dS",height,width);
    PrintZString (cmd);
}

/*****************************************************************************/
/*                                                                           */
/* Universelle Ausgaberoutine fuer den LaserJet/DeskJet/Color-DeskJet. Die   */
/* Routine fuehrt die Drucker-Initialisierung ueber den uebergebenen String  */
/* in Buf aus (der das passende Format haben muss, also mit fuehrendem       */
/* Laengenbyte) und druckt dann den Inhalt des Speichers. Es ist Farbe       */
/* moeglich (im DeskJet-Format), die passende Initialisierung muss aber in   */
/* Buf stehen. Die Routine behandelt nur (falls vorhanden) die Planes        */
/* korrekt.                                                                  */
/* Da Buf spaeter als Komprimierungs-Puffer verwendet wird, muss er im Daten-*/
/* segment liegen (sowieso weil Zeiger) und mindestens 600 Bytes gross sein. */
/*                                                                           */
/* Die Routine wurde spaeter noch erweitert um eine Moeglichkeit, die Daten  */
/* auch gezwungenermassen ohne Kompression rauszuschicken. Dabei wird der    */
/* Einfachheit halber die Zeile auch komprimiert, dann aber bei der          */
/* Abfrage, welcher Puffer kuerzer ist, gleichzeitig das Flag ausgewertet.   */
/* Die zusaetzliche Laufzeit wird hier in Kauf genommen, da diese Modi ja    */
/* wohl nur eine Notloesung fuer Uralt-Drucker sein koennen.                 */
/*                                                                           */
/*                                                                           */
/* Parameter:                                                                */
/*   Buf       wie oben beschrieben                                          */
/*                                                                           */
/* Ergebnisse:                                                               */
/*  (keine)    bzw. hoffentlich ein Ausdruck...                              */
/*                                                                           */
/*****************************************************************************/




void  LaserPrint ( struct _DST * DSTPtr , int PRNHandle )
{

    int      i,rc;
    int      X, Count, FullCount, Len, Plane, A, PlaneCount;
    int      RowBytes;
    unsigned I;
    BYTE     B, *S;
    BOOLEAN  SeparateBlack;
    BYTE    *OutputBuf=0 , *Buf=0;
    int     *PX=0;
    /*int     *PXCurr;*/
    struct RGBEntry *P;

    int   MaxX = GrSizeX();
    int   MaxY = GrSizeY();

    /* Variablen-Init */
    volatile enum {Unknown, Compressed, NotCompressed} CompStatus = Unknown;

    OutputBuf = (BYTE *) malloc (PrintBufSize);

#   ifdef LJ_LANDSCAPE_MODE
      PX = (int *) calloc (MaxX,sizeof(int));
#   else
      PX = (int *) calloc (MaxY,sizeof(int));
#   endif

    /* Bytes pro (Plane-) Zeile berechnen */
#   ifdef LJ_LANDSCAPE_MODE
      RowBytes = MaxX / 8;
#   else
      RowBytes = MaxY / 8;
#   endif

    /* Dynamischen Speicher fuer die Puffer belegen */
    Buf = (BYTE *) malloc (RowBytes);
    CompBuf = (BYTE *) malloc (CompBufSize);

    /* Vor Beginn der Druck-Ausgabe jetzt das Sprunglabel fuer Druckerfehler */
    /* setzen. */
    if ((rc=setjmp (PrintAborted))==0)
      {
	 /* Handle auf "raw data" umstellen. Direkt Ende wenn Handle nicht Ok*/
	 /* ACHTUNG: Ab hier Ausstieg mit return nicht mehr zulaessig, es    */
	 /*          muss goto Exit verwendet werden.                        */
	 if (OutputBuf==0 || PX==0) longjmp (PrintAborted,-1);

	 if (InitPrinter (OutputBuf, PrintBufSize, PRNHandle) == FALSE) {
	     longjmp (PrintAborted,-2);
	 }

	 if (Buf==0 || CompBuf==0) longjmp (PrintAborted,-1);

	 /* Variable aus DST zwischenspeichern */
	 SeparateBlack = hasSeparateBlack (DSTPtr);

	 /* Anzahl der Farb-Ebenen rechnen. Diese entspricht der Anzahl der */
	 /* Farb-Bits mit Ausnahme der Modi mit Schwarz-Abtrennung, hier    */
	 /* kommt eine Plane dazu.                                          */
	 PlaneCount = DSTPtr->ColorBits;
	 if (SeparateBlack) PlaneCount++;

	 /* Ausgabe der Initialisierungs-Sequenzen */
	 PrintZString (InitString);

#        ifdef LJ_LANDSCAPE_MODE
	   ljSetRasterSize (MaxX, MaxY);
#        else
	   ljSetRasterSize (MaxY, MaxX);
#        endif

	 if (DSTPtr->ColorCount == 8) {
	     /* DeskJet Farbe, Schwarz-Abtrennung hat 4 Ebenen, sonst 3 */
	     PrintZString (hasSeparateBlack (DSTPtr) ? ColorString2
						     : ColorString1);
	 } else if (DSTPtr->ColorCount > 8) {
	     /* Deskjet 1200C, Farbmodell und Palette setzen */
	     ColorString3 [8] = DSTPtr->ColorBits;
	     PrintString (ColorString3);

	     /* Zeiger auf die Palette */
	     P = RGBPal;

	     /* Alle Eintraege setzen */
	     for (I = 0; I < DSTPtr->ColorCount; I++) {

		 /* Steuerstring zum Setzen des Eintrags */
		 PrintString (RGBCompStr);
		 PrintNum (P->R);
		 PrintByte ('a');
		 PrintNum (P->G);
		 PrintByte ('b');
		 PrintNum (P->B);
		 PrintByte ('c');
		 PrintNum (I);
		 PrintByte ('I');

		 /* Naechster Eintrag */
		 P++;
	     }
	 }

	 PrintZString (((struct LJDST *) DSTPtr)->GraphicsOn);
	 PrintZString (Quality [djQuality]);
	 if (DSTPtr->ColorCount >= 8) {
	     /* Farb-Deskjets */
	     PrintZString (Shingling [djShingling]);
	     PrintZString (Depletion [djDepletion]);
	 }
	 if (DSTPtr->ColorCount > 8) {
	     /* Nur Deskjet 1200C */
	     PrintZString (MediaType [djMediaType]);
	 }
	 PrintZString (StartGraphics);

	 /* Ausgabe beginnt */
	 /*for (X = 0; X < MaxX; X++) {*/
#        ifdef LJ_LANDSCAPE_MODE
	   for (X = 0; X < MaxY; X++) {
	     for (Count = 0; Count<MaxX; Count++) {
	       PX[Count] = GrPixelNC (Count,X);
	     }
#        else
	   for (X = MaxX-1; X >= 0; X--) {
	     for (Count = 0; Count<MaxY; Count++) {
	       PX[Count] = GrPixelNC (X,Count);
	     }
	   /*GrReadVLineC (GrScreenContext(),X,PX);*/
#        endif

	   /* Daten fuer alle Ebenen ausgeben */
	   for (Plane = 1; Plane <= PlaneCount; Plane++) {

	     /*
	     ** Bytes in den Kompressionspuffer lesen. Wenn es sich um
	     ** einen Modus mit Schwarz-Abtrennung handelt, wird die erste
	     ** Plane aus den drei anderen zusammengestellt, die Schwarz-Bits
	     ** werden in den anderen Planes geloescht und zwar direkt im
	     ** Bildschirmspeicher (problemlos, weil nach dem Ausdruck die
	     ** Zeichenflaeche leer ist).
	     */
	     if (SeparateBlack && Plane == 1) {

		 /* Schwarz-Ebene zusammenstellen. */
		 for (Count = 0; Count < RowBytes; Count++)
		   {
		     /* Je ein Byte aus den drei Ebenen holen */
		     /*Y = *Map (Abs);*/
		     /*C = *Map (Abs + RowBytes);*/
		     /*M = *Map (Abs + 2 * RowBytes);*/
		     unsigned char  Mask = 0x80;
		     Buf[Count] = 0;
		     /* This code contains support of 8 color modes only !!! */
		     for (i=0; i<8; i++)     /* Remove black */
		       {
			   int  Y, px;
			   Y = (Count<<3)+i;
#                          ifdef LJ_LANDSCAPE_MODE
			     if (Y>=MaxX) break;
#                          else
			     if (Y>=MaxY) break;
#                          endif
			   px = PX[Y];        /* GrPixelNC (X,Y); */
			   if ((px & 7)==7)
			     {
				Buf[Count] |= Mask;
				PX[Y] = 0;    /* GrPlotNC (X,Y,0); */
			     }
			   Mask >>= 1;
		       }
		 }
	     }
	     else
	     {
		 /* Es handelt sich um eine ganz normale Farb-Ebene...*/
		 int cmask = 1 << ((SeparateBlack ? Plane-1 : Plane)-1);
		 for (Count = 0; Count < RowBytes; Count++)
		   {
		      unsigned char  Mask = 0x80;
		      int Y = Count<<3;
		      Buf[Count] = 0;
		      for (i=0; i<8; i++)
			{
			   int  px;
#                          ifdef LJ_LANDSCAPE_MODE
			     if (Y>=MaxX) break;
#                          else
			     if (Y>=MaxY) break;
#                          endif
			   px = PX[Y];   /* GrPixelNC (X,Y); */
			   if (px & cmask) Buf[Count] |= Mask;
			   Mask >>= 1;
			   Y++;
			}
		   }

	     }

	     /* Pruefen, wieviele Bytes am Ende des Puffers Null-Bytes sind */

	     S = Buf + RowBytes;
	     FullCount = RowBytes;
             while (FullCount>0 && *(--S)==0) FullCount--;

	     /* Versuchen die Zeile nach Algorithmus 2 (TIFF Pacbits) zu    */
	     /* komprimieren. Abgeschickt wird dann der kuerzere der beiden */
	     /* Puffer.                                                     */

	     S = Buf;
	     Count = FullCount;                /* Anzahl zu pruefender Bytes */
	     CompBufFill = 0;                  /* Puffer ist leer */
	     while (Count) {
		 /* Erstes Byte holen */
		 B = *S++;
		 Count--;
		 A = 1;

		 if (Count) {
		     /* Es kommen noch Bytes */
		     if (*S == B) {
			 /* Das naechste ist gleich, zaehlen wieviele
			 ** gleiche kommen */
			 while ((Count) && (*S == B)) {
			     A++;
			     Count--;
			     S++;
			 }
			 /* Abschicken */
			 RepeatByte (A, B);
		     } else {
			 /* Das naechste ist ungleich, zaehlen wieviele
			 ** ungleiche kommen */
			 while ((Count) && (*S != B)) {
			     A++;
			     Count--;
			     B = *(S++);
			 }
			 /* Wenn Count nicht 0 ist, dann sind die Bytes an
			 ** Position S und S-1 gleich, koennen also beim
			 ** naechsten Durchgang  mit einer RepeatByte-Sequenz
			 ** dargestellt werden. In diesem Fall wird der letzte
			 ** Durchgang wieder rueckgaengig gemacht. */
			 if (Count) {
			     S--;
			     A--;
			     Count++;
			 }

			 /* Abschicken */
			 MultiByte (A, (char *) (S-A));
		     }

		 } else {
		     /* Es kommen keine Bytes mehr, das letzte Byte als
		     ** Einzelbyte schicken */
		     MultiByte (1, (char *) (S-1));
		 }

	     }

	     /* Sodele, den kuerzeren der beiden Puffer abschicken. Hier auch
	     ** pruefen ob eine Kompression ueberhaupt gewuenscht ist... */
	     if (CompBufFill < FullCount && DoCompression (DSTPtr)) {
		 /* Der komprimierte Puffer ist kuerzer, Kontrollstring zur */
		 /* Festlegung der Kompression senden, falls diese nicht    */
		 /* schon eingestellt ist.                                  */
		 if (CompStatus != Compressed) {
		     CompStatus = Compressed;
		     PrintZString (Compression);
		 }

		 FullCount = CompBufFill;
		 S         = CompBuf;
	     } else {
		 /* Der unkomprimierte Puffer ist kuerzer, Kontrollstring zur*/
		 /* Festlegung der Kompression schicken, falls diese nicht   */
		 /* schon eingestellt ist.                                   */
		 if (CompStatus != NotCompressed) {
		     CompStatus = NotCompressed;
		     PrintZString (NoCompression);
		 }

		 S = Buf;
	     }

	     /* Die Steuersequenz fuer die nicht leeren Zeichen zusammenbauen.
	     ** Beachten, dass sich die letzte Plane in Code von den vorigen
	     ** unterscheidet. */
	     sprintf (&PreBytes[3],"%u",(unsigned) FullCount); /*Num (FullCount, &PreBytes [3]); */
	     Len = strlen (PreBytes);
	     PreBytes [Len]   = (Plane == PlaneCount) ? 'W' : 'V';
	     PreBytes [Len+1] = '\0';

	     /* Steuersequenz und dann die Zeichen senden */
	     PrintZString (PreBytes);

	     /* Puffer folgt */
	     PrintData (S, FullCount);

	   }
	 }

	 /* Das war's, Grafik beenden und Puffer leeren */
	 PrintZString (EndGraphics);
	 Flush ();
      }
    else switch (rc)
      {
	 case -1:  fprintf (stderr,"LaserPrint(): Memory allocation failed\n"); break;
	 case -2:  fprintf (stderr,"LaserPrint(): Error writing output file\n"); break;
      }

    /* Bei Fehlern geht's hier raus */
    /* Handle wieder in den Orginalzustand versetzen */
    if (OutputBuf) free(OutputBuf);
    if (Buf) free(Buf);
    if (PX) free(PX);
    if (CompBuf) { free(CompBuf); CompBuf=0; }

    ResetPrinter ();
}


void    GrPrintGetDefaultOptions ( struct GrPrintOptionsType * opt )
  {
      *opt=DefaultSettings;
  }


void    GrPrintGetCurrentOptions ( struct GrPrintOptionsType * opt )
  {
      *opt=Settings;
  }


void    GrPrintSetOptions ( struct GrPrintOptionsType * opt )
  {
      Settings.Quality     = opt->Quality<0 || opt->Quality>2
				   ? 0 : opt->Quality;
      Settings.Shingling   = opt->Shingling<0 || opt->Shingling>2
				   ? 0 : opt->Shingling;
      Settings.Depletion   = opt->Depletion<0 || opt->Depletion>2
				   ? 1 : opt->Depletion;
      Settings.MediaType   = opt->MediaType<0 || opt->MediaType>4
				   ? 0 : opt->MediaType;
  }




/****************************************************************************/
/*                                                                          */
/*              Modul fuer BGI-Treiber zum Ansprechen des Druckers          */
/*                                                                          */
/****************************************************************************/
/*                                                                          */
/* (C) 1990-1993 Ullrich von Bassewitz                                      */
/*                                                                          */
/* Port to DJGPP                                                            */
/* Revision 0.5   98/01/26  Andris Pavenis (pavenis@acad.latnet.lv)         */

/*                                                                          */
/* $Id: print.cpp 2.6 1995/04/28 16:21:14 Uz Exp $                          */
/*                                                                          */
/* $Log: print.cpp $                                                        */
/* Revision 2.6  1995/04/28 16:21:14  Uz                                    */
/* Umstellung auf PCX-Treiber.                                              */
/*                                                                          */
/* Revision 2.5  95/04/22  17:34:02  Uz                                     */
/* Neue Funktionen fuer den Ausdruck, PrintZString, PrintData, kleinere     */
/* Aenderungen bei existierenden Funktionen.                                */
/*                                                                          */
/* Revision 2.4  94/09/08  14:14:51  Uz                                     */
/* Kleinere Aenderungen zur Einsparung von ein paar Bytes.                  */
/*                                                                          */
/* Revision 2.3  93/08/01  20:53:10  Uz                                     */
/* Neues Format mit DPMI-Support                                            */
/*                                                                          */

/****************************************************************************/
/*                                                                          */
/*                               Variable                                   */
/*                                                                          */
/****************************************************************************/


/* Ausgabepuffer und Deklarationen dazu */
static BYTE     *OutputBuf;                     /* Der Puffer */
static WORD      OutputBufFill = 0;             /* Anzahl Zeichen im Puffer */

/* Handle auf das geschrieben wird */
static WORD     OutputHandle = 4;

/* Merker fuer die Flags des Printer-Devices */
WORD            PrinterFlags;


/****************************************************************************/
/*                                  Code                                    */
/****************************************************************************/



static BOOLEAN InitPrinter (BYTE * Buf, int BufSize, int Handle)
/* Stellt das Ausgabegeraet auf "binary" um und prueft gleichzeitig, ob das  */
/* Handle 4 ok (offen) ist. Der aktuelle Zustand wird gemerkt und bei Post   */
/* wiederhergestellt. Buf wird als Puffer fuer die Ausgabe verwendet und     */
/* muss PrintBufSize Bytes gross sein. ACHTUNG: Buf muss bis zum Aufruf von  */
/* ResetPrinter gueltig sein!                                                */
{
    /* Puffer und Handle uebernehmen */
    OutputBuf = Buf;
    OutputBufFill = 0;
#   ifdef __MSDOS__
      PrinterFlags = setmode (Handle,O_BINARY);
#   endif
    OutputHandle = Handle;
    return TRUE;
}


static void ResetPrinter (void)
/* Stellt den orginalen Zustand des Ausgabegeraets wieder her. */
{
#     ifdef __MSDOS__
	setmode (OutputHandle,PrinterFlags);
#     endif
}



static void Flush (void)
/* Schreibt den Ausgabepuffer leer. Muss am Ende eines Ausdrucks aufgerufen */
/* werden. Springt PrintAbortLabel an bei Fehlern.                          */
{
    if (OutputBufFill > 0) {
	if (write (OutputHandle, OutputBuf, OutputBufFill) != OutputBufFill) {
	    /* Fehler beim Schreiben, Fehleraussprung */
	    longjmp (PrintAborted, -2);
	}
	/* Puffer ist wieder leer */
	OutputBufFill = 0;

	memset (OutputBuf,0,PrintBufSize);
    }
}



static void  PrintByte (BYTE B)
/* Gibt ein Byte auf den Drucker (bzw. in den Ausgabepuffer) aus. */
/* Springt PrintAbortLabel an bei Fehlern.                        */
{
    /* Pruefen ob noch Platz im Puffer ist, wenn Nein Puffer leeren */
    if (OutputBufFill == PrintBufSize) {
	/* Puffer ist voll, leeren */
	Flush ();
    }
    /* Neues Byte in den Puffer schreiben */
    OutputBuf [OutputBufFill++] = B;
}



static void PrintData (BYTE * Data, unsigned Size)
/* Gibt Daten auf den Drucker (bzw. in den Ausgabepuffer) aus. */
/* Springt PrintAbortLabel an bei Fehlern.                     */
{
      int  i;
      for (i=0; i<Size; i++)
	  PrintByte (Data[i]);
}



static void PrintString (char * S)
/* Gibt einen Pascal-String auf den Drucker (bzw. in den Ausgabepuffer) aus. */
/* Springt PrintAbortLabel an bei Fehlern.                                   */
{
    if (S) PrintData ((unsigned char *) (S+1), *S);
}



static void PrintZString (char * S)
/* Gibt einen nullterminierten String auf den Drucker (bzw. in den */
/* Ausgabepuffer) aus. Springt PrintAbortLabel an bei Fehlern.     */
{
    PrintData ((unsigned char * ) S, strlen (S));
}


