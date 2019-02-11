/*****************************************************************************/
/*                                                                           */
/*  grxprn00.h : Output of graphics on printer from GRX graphic library      */
/*  Version 0.5 (beta)   98/01/26   Andris Pavenis (pavenis@acad.latnet.lv)  */
/*                                                                           */
/*  Version 0.66         98/05/07   H.Schirmer                               */
/*         - made cpp # start on first column                                */
/*                                                                           */
/*  Version 0.68         98/05/13   H.Schirmer                               */
/*        - clean source for better portability / ANSI-C conformance         */
/*                                                                           */
/*  Version 0.7          98/05/20   A.Pavenis                                */
/*        - removed definitions of procedures internal to grxprint.c         */
/*          (they are now declared static inside grxprint.c)                 */
/*                                                                           */
/*  This code is port of part of printer BGI driver                          */
/*  (C) 1990-1995 Ullrich von Bassewitz (see copying.uz).                    */
/*                                                                           */
/*  Full version of printer BGI driver version 4.0 can be found              */
/*  at URL   ftp://ftp.musoftware.com/pub/uz/                                */
/*  An alternate URL is http://www.lanet.lv/~pavenis/printerbgi+src.zip      */
/*                                                                           */
/*****************************************************************************/

#ifndef __GRXPRN00_H
#define __GRXPRN00_H

#include <setjmp.h>

#ifndef __GRX20_H_INCLUDED__
#include <grx20.h>
#endif

/*                                                                           */
/* Datentyp BOOLEAN                                                          */
/*                                                                           */
typedef unsigned char BOOLEAN;
#define TRUE     1
#define OK       TRUE
#define FALSE    0

/*                                                                           */
/* Diverse andere Datentypen mit festen Bit-Groessen                         */
/*                                                                           */
/*
>>>>> These types should be mapped to the GRX internal types GR_int..u <<<<<
*/
typedef unsigned int  WORD;
typedef unsigned char BYTE;
typedef unsigned long DWORD;


/****************************************************************************/
/*                                                                          */
/*                                Macros                                    */
/*                                                                          */
/****************************************************************************/

/* NULL Macro */
#ifndef NULL
#define NULL    0
#endif

/* Die Farbtabelle wird (da nicht verwendet) als Moeglichkeit zur Einstellung*/
/* von diversen Daten von aussen benutzt. Sie heisst daher nicht ColorTable  */
/* (wie im SVGA-Treiber), sondern Settings (fuer die aktuelle Tabelle) und   */
/* DefaultSettings (fuer die Default-Einstellungen).                         */

/* Defines zum Zugriff auf die Elemente von Settings und DefaultSettings     */
#define   djDefQuality      (DefaultSettings.Quality)
#define   djDefShingling    (DefaultSettings.Shingling)
#define   djDefDepletion    (DefaultSettings.Depletion)
#define   djDefMediaType    (DefaultSettings.MediaType)
#define   djQuality         (Settings.Quality)
#define   djShingling       (Settings.Shingling)
#define   djDepletion       (Settings.Depletion)
#define   djMediaType       (Settings.MediaType)


/* RGB Palette. */
struct RGBEntry {
    BYTE R, G, B;
};

/****************************************************************************/
/*                   Konstante fuer das Flags Byte von _DST                 */
/****************************************************************************/

#define  pfIsEpson         0x01     /* Epson-Modus */

/* Nadeldrucker-Flags */
#define  pfReverse         0x02     /* Nadelnummerierung umdrehen (EPSON) */

/* Deskjet-Flags */
#define  pfSeparateBlack   0x02     /* Separate Schwarz-Plane (Deskjet) */
#define  pfDoCompression   0x04     /* TIFF Pacbits Kompression durchfuehren */
#define  pfHasPalette      0x08     /* RGB Palette ja/nein */


/* Anzahl Druckermodi */
#define  MaxModes  43


/****************************************************************************/
/*                  Grund-Deskriptor fuer einen Druckermodus                */
/****************************************************************************/

/* Die folgende struct enthaelt die Grund-Werte bzw. Funktionen, die sich   */
/* nie aendern. Jeder Treiber kann spezielle Funktionen am Ende hinzufuegen.*/

struct _DST {

      WORD        XDPI;           /* Aufloesung in X-Richtung*/
      WORD        YDPI;           /* Aufloesung in Y-Richtung*/
      WORD        XInch;          /* Groesse X in Inch * 1000*/
      WORD        YInch;          /* Groesse Y in Inch * 1000*/

      WORD        ColorCount;     /* Anzahl Farben des Modus*/
      BYTE        ColorBits;      /* Anzahl Bit in denen ein Pixel codiert ist */
      BYTE        Flags;          /* Diverse bitmapped Flags */

      char        *Name;          /* Name des Modes */

      void        (*Print) ( struct _DST * , int );
      /*void        (*Print) (); *//* Druck-Routine */

      /* Member functions, die obige Flags auswerten, alle inline!          */
      /*BOOLEAN IsEpson () const        { return Flags & pfIsEpson; }       */
      /*BOOLEAN Reverse () const        { return Flags & pfReverse; }       */
      /*BOOLEAN SeparateBlack () const  { return Flags & pfSeparateBlack; } */
      /*BOOLEAN DoCompression () const  { return Flags & pfDoCompression; } */
      /*BOOLEAN HasPalette () const     { return Flags & pfHasPalette; }    */

  };

#define   IsEpson(x)          (x->Flags & pfIsEpson)
#define   Reverse(x)          (x->Flags & pfReverse)
#define   hasSeparateBlack(x) (x->Flags & pfSeparateBlack)
#define   DoCompression(x)    (x->Flags & pfDoCompression)
#define   HasPalette(x)       (x->Flags & pfHasPalette)

/* Der Zeiger auf die aktuelle DST*/
/* extern struct _DST  *DSTPtr; */
/* Die Tabelle mit den Zeigern auf die Modi*/
extern struct _DST  *DSTTable [MaxModes];


/* Groesse des Ausgabepuffers*/
#define  PrintBufSize   1024


/*****************************************************************************/
/*  Dot matrix printers related procedures                                   */
/*****************************************************************************/
/*                                                                           */
/*  Die folgende Struktur enthaelt einen Device-Status-Block. Die ersten     */
/*  Werte muessen immer dieselben sein, da sie vom Grafik-Modul so erwartet  */
/*  werden. Im Anschluss kommen eigene Variablen, die von Drucker zu Drucker */
/*  bzw. besser von Treiber zu Treiber verschieden sein koennen.             */
/*                                                                           */
/*****************************************************************************/


struct EpsonDST {

    struct _DST     DST;              /* Orginal-DST */

    BYTE            ColBytes;         /* Anzahl Bytes / Druckerspalte */
    BYTE            PassCount;        /* Wie oft ueberdrucken */

    char            *LineFeed1;       /* Normaler Linefeed */
    char            *LineFeed2;       /* Linefeed zwischen Ueberdrucken */

    char            *GraphicsOn;      /* Grafik einschalten (mit Init) */
    char            *GraphicsOff;     /* Grafik ausschalten */

    char            *PreBytes;        /* String vor Grafik-Daten  */
    char            *PostBytes;       /* String nach Grafik-Daten */

};

/****************************************************************************/
/*                                                                          */
/* Universelle Ausgaberoutine fuer Nadeldrucker.                            */
/*                                                                          */
/* Parameter:                                                               */
/*   (keine)                                                                */
/*                                                                          */
/* Ergebnisse:                                                              */
/*  (keine)    bzw. hoffentlich ein Ausdruck...                             */
/*                                                                          */
/****************************************************************************/

void  EpsonPrint ( struct _DST * DSTPtr , int PRNHandle );
/* Universelle Drucker-Routine fuer Nadeldrucker */


/****************************************************************************/
/*  Laser and DeskJet printers related procedures                           */
/****************************************************************************/
/*                                                                          */
/*  Die folgende Struktur enthaelt fuer den LaserJet angepassten Device-    */
/*  Status Block.                                                           */
/*                                                                          */
/****************************************************************************/


struct LJDST {

    struct _DST  DST;            /* Orginal-DST */
    char        *GraphicsOn;     /* Grafik einschalten (mit Init) */

};




void  LaserPrint ( struct _DST * DSTPtr , int PRNHandle );


#endif
