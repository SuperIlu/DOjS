/*****************************************************************************/
/*                                                                           */
/*                                PRNDATA.CPP                                */
/*                                                                           */
/*  (C) 1994 by Ullrich von Bassewitz                                        */
/*              Zwehrenbuehlstrasse 33                                       */
/*              72070 Tuebingen                                              */
/*                                                                           */
/*  E-Mail:     uz@ibb.schwaben.de                                           */
/*                                                                           */
/*****************************************************************************/

/* Revision 1.4  1998/02/05  Andris Pavenis                                  */
/* Modified for use under DJGPP                                              */
/*                                                                           */
/* (C) 1994 Ullrich von Bassewitz                                            */
/*                                                                           */
/* $Id: prndata.cpp 1.3 1995/04/28 16:20:53 Uz Exp $                         */
/*                                                                           */
/* $Log: prndata.cpp $                                                       */
/* Revision 1.3  1995/04/28 16:20:53  Uz                                     */
/* Umstellung auf PCX-Treiber.                                               */
/*                                                                           */
/* Revision 1.2  95/04/22  17:31:52  Uz                                      */
/* Neuer Modus 41 fuer DeskJet 1200 C.                                       */
/*                                                                           */
/* Revision 1.1  94/09/08  09:33:42  Uz                                      */
/* Initial revision                                                          */
/*                                                                           */


#include "grxprn00.h"

/*************** Original values from printer BGI driver */
/*#define  NP_A4_SIZEX   11000*/
/*#define  NP_A4_SIZEY    8000*/
/*#define  LJ_A4_SIZEX   10334*/
/*#define  LJ_A4_SIZEY    7800*/
/*#define  NP_A3_SIZEX   15600*/
/*#define  NP_A3_SIZEY   11400*/
/*********************************************************/
#define  NP_A4_SIZEX   11000
#define  NP_A4_SIZEY    8000
#define  LJ_A4_SIZEX   10334
#define  LJ_A4_SIZEY    7760
#define  NP_A3_SIZEX   15600
#define  NP_A3_SIZEY   11400
/*********************************************************/

/****************************************************************************/
/*                        8-Nadel Modus 240 * 72 DPI                        */
/****************************************************************************/

static struct EpsonDST EPSON240x72 = {

    /* DPI in X-und Y-Richtung*/
  { 72, 240,

    /* Blattgroesse in X- und Y-Richtung in 1/1000 Inch*/
    NP_A4_SIZEX, NP_A4_SIZEY,

    /* Anzahl Farben und Bits pro Pixel*/
    2, 1,

    /* Flags*/
    pfIsEpson,

    /* Name des Modus*/
    "\x10""EPSON 240x72 DPI",       /* Name des Modus*/

    /* Druckeroutine*/
    EpsonPrint },

    /* Bytes pro Druckerspalte und Durchgaenge*/
    1, 1,

    /* Linefeed1, d.h. normaler Linefeed*/
    "\x04\x1B\x4A\x18\r",           /* ESC 'J' 24 cr*/

    /* LineFeed2, d.h. kleinster Linefeed*/
    NULL,                           /* Nicht vorhanden*/

    /* Einschalten der Grafik*/
    "\x05\x1B\x40\x1B\x4F\r",          /* ESC '@' ESC 'O' \r*/

    /* Ausschalten der Grafik*/
    "\x03\f\x1B\x40",               /* \f ESC '@'*/

    /* Code vor den Grafik-Bytes*/
    "\x05\x1B\x2A\x03\x80\x07",     /* ESC '*' 03 1920*/

    /* Code nach den Grafik-Bytes*/
    NULL                            /* Nicht vorhanden*/

};



/****************************************************************************/
/*                        8-Nadel Modus 240 * 216 DPI                       */
/****************************************************************************/

static struct EpsonDST EPSON240x216 = {

    /* DPI in X-und Y-Richtung*/
  { 216, 240,

    /* Blattgroesse in X- und Y-Richtung in 1/1000 Inch*/
    NP_A4_SIZEX, NP_A4_SIZEY,

    /* Anzahl Farben und Bits pro Pixel*/
    2, 1,

    /* Flags*/
    pfIsEpson,

    /* Name des Modus*/
    "\x11""EPSON 240x216 DPI",      /* Name des Modus*/

    /* Druckeroutine*/
    EpsonPrint },

    /* Bytes pro Druckerspalte und Durchgaenge*/
    1, 3,

    /* Linefeed1, d.h. normaler Linefeed*/
    "\x04\x1B\x4A\x16\r",           /* ESC 'J' 22 cr*/

    /* LineFeed2, d.h. kleinster Linefeed*/
    "\x04\x1B\x4A\x01\r",           /* ESC 'J' 1 cr*/

    /* Einschalten der Grafik*/
    "\x05\x1B\x40\x1B\x4F\r",          /* ESC '@' ESC 'O' \r*/

    /* Ausschalten der Grafik*/
    "\x03\f\x1B\x40",               /* \f ESC '@'*/

    /* Code vor den Grafik-Bytes*/
    "\x05\x1B\x2A\x03\x80\x07",     /* ESC '*' 03 1920*/

    /* Code nach den Grafik-Bytes*/
    NULL                            /* Nicht vorhanden*/

};



/****************************************************************************/
/*                       24-Nadel Modus 180 * 180 DPI                       */
/****************************************************************************/

static struct EpsonDST EPSON180x180 = {

    /* DPI in X-und Y-Richtung*/
  { 180, 180,

    /* Blattgroesse in X- und Y-Richtung in 1/1000 Inch*/
    NP_A4_SIZEX, NP_A4_SIZEY,

    /* Anzahl Farben und Bits pro Pixel*/
    2, 1,

    /* Flags*/
    pfIsEpson,

    /* Name des Modus*/
    "\x11""EPSON 180x180 DPI",      /* Name des Modus*/

    /* Druckeroutine*/
    EpsonPrint },

    /* Bytes pro Druckerspalte und Durchgaenge*/
    3, 1,

    /* Linefeed1, d.h. normaler Linefeed*/
    "\x04\x1B\x4A\x18\r",           /* ESC 'J' 24 cr*/

    /* LineFeed2, d.h. kleinster Linefeed*/
    NULL,                           /* Nicht vorhanden*/

    /* Einschalten der Grafik*/
    "\x05\x1B\x40\x1B\x4F\r",          /* ESC '@' ESC 'O' \r*/

    /* Ausschalten der Grafik*/
    "\x03\f\x1B\x40",               /* \f ESC '@'*/

    /* Code vor den Grafik-Bytes*/
    "\x05\x1B\x2A\x27\xA0\x05",     /* ESC '*' 39 1440*/

    /* Code nach den Grafik-Bytes*/
    NULL                            /* Nicht vorhanden*/

};



/****************************************************************************/
/*                       24-Nadel Modus 360 * 180 DPI                       */
/****************************************************************************/

static struct EpsonDST EPSON360x180 = {

    /* DPI in X-und Y-Richtung*/
  { 180, 360,

    /* Blattgroesse in X- und Y-Richtung in 1/1000 Inch*/
    NP_A4_SIZEX, NP_A4_SIZEY,

    /* Anzahl Farben und Bits pro Pixel*/
    2, 1,

    /* Flags*/
    pfIsEpson,

    /* Name des Modus*/
    "\x11""EPSON 360x180 DPI",      /* Name des Modus*/

    /* Druckeroutine*/
    EpsonPrint },

    /* Bytes pro Druckerspalte und Durchgaenge*/
    3, 1,

    /* Linefeed1, d.h. normaler Linefeed*/
    "\x04\x1B\x4A\x18\r",           /* ESC 'J' 24 cr*/

    /* LineFeed2, d.h. kleinster Linefeed*/
    NULL,                           /* Nicht vorhanden*/

    /* Einschalten der Grafik*/
    "\x05\x1B\x40\x1B\x4F\r",          /* ESC '@' ESC 'O' \r*/

    /* Ausschalten der Grafik*/
    "\x03\f\x1B\x40",               /* \f ESC '@'*/

    /* Code vor den Grafik-Bytes*/
    "\x05\x1B\x2A\x28\x40\x0B",     /* ESC '*' 40 2880*/

    /* Code nach den Grafik-Bytes*/
    NULL                            /* Nicht vorhanden*/
};



/****************************************************************************/
/*                    24-Nadel Modus 360 * 360 DPI, EPSON                   */
/****************************************************************************/

static struct EpsonDST EPSON360x360 = {

    /* DPI in X-und Y-Richtung*/
  { 360, 360,

    /* Blattgroesse in X- und Y-Richtung in 1/1000 Inch*/
    NP_A4_SIZEX, NP_A4_SIZEY,

    /* Anzahl Farben und Bits pro Pixel*/
    2, 1,

    /* Flags*/
    pfIsEpson,

    /* Name des Modus*/
    "\x11""EPSON 360x360 DPI",      /* Name des Modus*/

    /* Druckeroutine*/
    EpsonPrint },

    /* Bytes pro Druckerspalte und Durchgaenge*/
    3, 2,

    /* Linefeed1, d.h. normaler Linefeed*/
    "\x05\x1B\x2B\x2F\x0D\x0A",     /* ESC '+' 47 cr lf*/

    /* LineFeed2, d.h. kleinster Linefeed*/
    "\x05\x1B\x2B\x01\x0D\x0A",     /* ESC '+' 01 cr lf*/

    /* Einschalten der Grafik*/
    "\x05\x1B\x40\x1B\x4F\r",          /* ESC '@' ESC 'O' \r*/

    /* Ausschalten der Grafik*/
    "\x03\f\x1B\x40",               /* \f ESC '@'*/

    /* Code vor den Grafik-Bytes*/
    "\x05\x1B\x2A\x28\x40\x0B",     /* ESC '*' 40 2880*/

    /* Code nach den Grafik-Bytes*/
    NULL                            /* Nicht vorhanden*/
};



/****************************************************************************/
/*                   24-Nadel Modus 360 * 360 DPI, NEC P6                   */
/****************************************************************************/

static struct EpsonDST NEC360x360 = {

    /* DPI in X-und Y-Richtung*/
  { 360, 360,

    /* Blattgroesse in X- und Y-Richtung in 1/1000 Inch*/
    NP_A4_SIZEX, NP_A4_SIZEY,

    /* Anzahl Farben und Bits pro Pixel*/
    2, 1,

    /* Flags*/
    pfIsEpson,

    /* Name des Modus*/
    "\x12""NEC P6 360x360 DPI",     /* Name des Modus*/

    /* Druckeroutine*/
    EpsonPrint },

    /* Bytes pro Druckerspalte und Durchgaenge*/
    3, 2,

    /* Linefeed1, d.h. normaler Linefeed*/
    "\x05\x1C\x33\x2F\x0D\x0A",     /* FS '3' 47 cr lf*/

    /* LineFeed2, d.h. kleinster Linefeed*/
    "\x05\x1C\x33\x01\x0D\x0A",     /* FS '3' 01 cr lf*/

    /* Einschalten der Grafik*/
    "\x05\x1B\x40\x1B\x4F\r",          /* ESC '@' ESC 'O' \r*/

    /* Ausschalten der Grafik*/
    "\x03\f\x1B\x40",               /* \f ESC '@'*/

    /* Code vor den Grafik-Bytes*/
    "\x05\x1B\x2A\x28\x40\x0B",     /* ESC '*' 40 2880*/

    /* Code nach den Grafik-Bytes*/
    NULL                            /* Nicht vorhanden*/
};



/****************************************************************************/
/*                     IBM Proprinter X24, 180 * 180 DPI                    */
/****************************************************************************/

static struct EpsonDST IBMPro180x180 = {

    /* DPI in X-und Y-Richtung*/
  { 180, 180,

    /* Blattgroesse in X- und Y-Richtung in 1/1000 Inch*/
    NP_A4_SIZEX, NP_A4_SIZEY,

    /* Anzahl Farben und Bits pro Pixel*/
    2, 1,

    /* Flags*/
    pfIsEpson,

    /* Name des Modus*/
    "\x1A""IBM Proprinter 180x180 DPI",

    /* Druckeroutine*/
    EpsonPrint },

    /* Bytes pro Druckerspalte und Durchgaenge*/
    3, 1,

    /* Linefeed1, d.h. normaler Linefeed*/
    "\x05\x1B\x33\x18\r\n",                       /* ESC '3' 24 cr lf*/

    /* LineFeed2, d.h. kleinster Linefeed*/
    NULL,                                         /* Nicht vorhanden*/

    /* Einschalten der Grafik*/
    "\x0C\x1B\x4F\x1B\x5B\x5C\x04\x00\x00\x00\x00\xB4\r",

    /* Ausschalten der Grafik*/
    "\x0C\x1B\x32\x1B\x5B\x5C\x04\x00\x00\x00\x00\xD8\f",

    /* Code vor den Grafik-Bytes*/
    "\x06\x1B\x5B\x67\xE1\x10\x0B",               /* ESC '[' 'g' 4321 11*/

    /* Code nach den Grafik-Bytes*/
    NULL                            /* Nicht vorhanden*/

};



/****************************************************************************/
/*                     IBM Proprinter X24, 360 * 180 DPI                    */
/****************************************************************************/

static struct EpsonDST IBMPro360x180 = {

    /* DPI in X-und Y-Richtung*/
  { 180, 360,

    /* Blattgroesse in X- und Y-Richtung in 1/1000 Inch*/
    NP_A4_SIZEX, NP_A4_SIZEY,

    /* Anzahl Farben und Bits pro Pixel*/
    2, 1,

    /* Flags*/
    pfIsEpson,

    /* Name des Modus*/
    "\x1A""IBM Proprinter 360x180 DPI",

    /* Druckeroutine*/
    EpsonPrint },

    /* Bytes pro Druckerspalte und Durchgaenge*/
    3, 1,

    /* Linefeed1, d.h. normaler Linefeed*/
    "\x05\x1B\x33\x18\r\n",                       /* ESC '3' 24 cr lf*/

    /* LineFeed2, d.h. kleinster Linefeed*/
    NULL,                                         /* Nicht vorhanden*/

    /* Einschalten der Grafik*/
    "\x0C\x1B\x4F\x1B\x5B\x5C\x04\x00\x00\x00\x00\xB4\r",

    /* Ausschalten der Grafik*/
    "\x0C\x1B\x32\x1B\x5B\x5C\x04\x00\x00\x00\x00\xD8\f",

    /* Code vor den Grafik-Bytes*/
    "\x06\x1B\x5B\x67\xC1\x21\x0C",               /* ESC '[' 'g' 8641 12*/

    /* Code nach den Grafik-Bytes*/
    NULL                            /* Nicht vorhanden*/

};



/****************************************************************************/
/*                 24-Nadel Modus 180 * 180 DPI EPSON Farbe                 */
/****************************************************************************/

static struct EpsonDST EPSON180x180C = {

    /* DPI in X-und Y-Richtung*/
  { 180, 180,

    /* Blattgroesse in X- und Y-Richtung in 1/1000 Inch*/
    NP_A4_SIZEX, NP_A4_SIZEY,

    /* Anzahl Farben und Bits pro Pixel*/
    9, 4,

    /* Flags*/
    pfIsEpson,

    /* Name des Modus*/
    "\x18""EPSON 180x180 DPI Color",    /* Name des Modus*/

    /* Druckeroutine*/
    EpsonPrint },

    /* Bytes pro Druckerspalte und Durchgaenge*/
    3, 1,

    /* Linefeed1, d.h. normaler Linefeed*/
    "\x04\x1B\x4A\x18\r",               /* ESC 'J' 24 cr*/

    /* LineFeed2, d.h. kleinster Linefeed*/
    NULL,                               /* Nicht vorhanden*/

    /* Einschalten der Grafik*/
    "\x05\x1B\x40\x1B\x4F\r",           /* ESC '@' ESC 'O' \r*/

    /* Ausschalten der Grafik*/
    "\x03\f\x1B\x40",                   /* \f ESC '@'*/

    /* Code vor den Grafik-Bytes*/
    "\x05\x1B\x2A\x27\xA0\x05",         /* ESC '*' 39 1440*/

    /* Code nach den Grafik-Bytes*/
    NULL                                /* Nicht vorhanden*/

};



/****************************************************************************/
/*                 24-Nadel Modus 360 * 180 DPI EPSON Farbe                 */
/****************************************************************************/

static struct EpsonDST EPSON360x180C = {

    /* DPI in X-und Y-Richtung*/
  { 180, 360,

    /* Blattgroesse in X- und Y-Richtung in 1/1000 Inch*/
    NP_A4_SIZEX, NP_A4_SIZEY,

    /* Anzahl Farben und Bits pro Pixel*/
    9, 4,

    /* Flags*/
    pfIsEpson,

    /* Name des Modus*/
    "\x17""EPSON 360x180 DPI Color",    /* Name des Modus*/

    /* Druckeroutine*/
    EpsonPrint },

    /* Bytes pro Druckerspalte und Durchgaenge*/
    3, 1,

    /* Linefeed1, d.h. normaler Linefeed*/
    "\x04\x1B\x4A\x18\r",           /* ESC 'J' 24 cr*/

    /* LineFeed2, d.h. kleinster Linefeed*/
    NULL,                           /* Nicht vorhanden*/

    /* Einschalten der Grafik*/
    "\x05\x1B\x40\x1B\x4F\r",          /* ESC '@' ESC 'O' \r*/

    /* Ausschalten der Grafik*/
    "\x03\f\x1B\x40",               /* \f ESC '@'*/

    /* Code vor den Grafik-Bytes*/
    "\x05\x1B\x2A\x28\x40\x0B",     /* ESC '*' 40 2880*/

    /* Code nach den Grafik-Bytes*/
    NULL                            /* Nicht vorhanden*/
};



/****************************************************************************/
/*                 24-Nadel Modus 360 * 360 DPI, EPSON Farbe                */
/****************************************************************************/

static struct EpsonDST EPSON360x360C = {

    /* DPI in X-und Y-Richtung*/
  { 360, 360,

    /* Blattgroesse in X- und Y-Richtung in 1/1000 Inch*/
    NP_A4_SIZEX, NP_A4_SIZEY,

    /* Anzahl Farben und Bits pro Pixel*/
    9, 4,

    /* Flags*/
    pfIsEpson,

    /* Name des Modus*/
    "\x17""EPSON 360x360 DPI Color",    /* Name des Modus*/

    /* Druckeroutine*/
    EpsonPrint },

    /* Bytes pro Druckerspalte und Durchgaenge*/
    3, 2,

    /* Linefeed1, d.h. normaler Linefeed*/
    "\x05\x1B\x2B\x2F\x0D\x0A",     /* ESC '+' 47 cr lf*/

    /* LineFeed2, d.h. kleinster Linefeed*/
    "\x05\x1B\x2B\x01\x0D\x0A",     /* ESC '+' 01 cr lf*/

    /* Einschalten der Grafik*/
    "\x05\x1B\x40\x1B\x4F\r",          /* ESC '@' ESC 'O' \r*/

    /* Ausschalten der Grafik*/
    "\x03\f\x1B\x40",               /* \f ESC '@'*/

    /* Code vor den Grafik-Bytes*/
    "\x05\x1B\x2A\x28\x40\x0B",     /* ESC '*' 40 2880*/

    /* Code nach den Grafik-Bytes*/
    NULL                            /* Nicht vorhanden*/
};



/****************************************************************************/
/*                24-Nadel Modus 360 * 360 DPI, NEC P6 Farbe                */
/****************************************************************************/

static struct EpsonDST NEC360x360C = {

    /* DPI in X-und Y-Richtung*/
  { 360, 360,

    /* Blattgroesse in X- und Y-Richtung in 1/1000 Inch*/
    NP_A4_SIZEX, NP_A4_SIZEY,

    /* Anzahl Farben und Bits pro Pixel*/
    9, 4,

    /* Flags*/
    pfIsEpson,

    /* Name des Modus*/
    "\x18""NEC P6 360x360 DPI Color",   /* Name des Modus*/

    /* Druckeroutine*/
    EpsonPrint },

    /* Bytes pro Druckerspalte und Durchgaenge*/
    3, 2,

    /* Linefeed1, d.h. normaler Linefeed*/
    "\x05\x1C\x33\x2F\x0D\x0A",     /* FS '3' 47 cr lf*/

    /* LineFeed2, d.h. kleinster Linefeed*/
    "\x05\x1C\x33\x01\x0D\x0A",     /* FS '3' 01 cr lf*/

    /* Einschalten der Grafik*/
    "\x05\x1B\x40\x1B\x4F\r",          /* ESC '@' ESC 'O' \r*/

    /* Ausschalten der Grafik*/
    "\x03\f\x1B\x40",               /* \f ESC '@'*/

    /* Code vor den Grafik-Bytes*/
    "\x05\x1B\x2A\x28\x40\x0B",     /* ESC '*' 40 2880*/

    /* Code nach den Grafik-Bytes*/
    NULL                            /* Nicht vorhanden*/
};



/****************************************************************************/
/*                         HP-Laserdrucker 75*75 DPI                        */
/****************************************************************************/

static struct LJDST HPLJ75 = {

    /* DPI in X-und Y-Richtung*/
  { 75, 75,

    /* Blattgroesse in X- und Y-Richtung in 1/1000 Inch*/
    LJ_A4_SIZEX, LJ_A4_SIZEY,

    /* Anzahl Farben und Bits pro Pixel*/
    2, 1,

    /* Flags*/
    pfDoCompression,

    /* Name des Modus*/
    "\x0E""HPLJ 75x75 DPI",         /* Name des Modus*/

    /* Druckeroutine*/
    LaserPrint },

    /* Festlegen der Grafik*/
    "\x1B*t75R"                     /* ESC * t 75 R*/

};



/****************************************************************************/
/*                        HP-Laserdrucker 100*100 DPI                       */
/****************************************************************************/

static struct LJDST HPLJ100 = {

    /* DPI in X-und Y-Richtung*/
  { 100, 100,

    /* Blattgroesse in X- und Y-Richtung in 1/1000 Inch*/
    LJ_A4_SIZEX, LJ_A4_SIZEY,

    /* Anzahl Farben und Bits pro Pixel*/
    2, 1,

    /* Flags*/
    pfDoCompression,

    /* Name des Modus*/
    "\x10""HPLJ 100x100 DPI",       /* Name des Modus*/

    /* Druckeroutine*/
    LaserPrint },

    /* Festlegen der Grafik*/
    "\x1B*t100R"                    /* ESC * t 100 R*/

};



/****************************************************************************/
/*                        HP-Laserdrucker 150*150 DPI                       */
/****************************************************************************/

static struct LJDST HPLJ150 = {

    /* DPI in X-und Y-Richtung*/
  { 150, 150,

    /* Blattgroesse in X- und Y-Richtung in 1/1000 Inch*/
    LJ_A4_SIZEX, LJ_A4_SIZEY,

    /* Anzahl Farben und Bits pro Pixel*/
    2, 1,

    /* Flags*/
    pfDoCompression,

    /* Name des Modus*/
    "\x10""HPLJ 150x150 DPI",       /* Name des Modus*/

    /* Druckeroutine*/
    LaserPrint },

    /* Festlegen der Grafik*/
    "\x1B*t150R"                    /* ESC * t 150 R*/

};



/****************************************************************************/
/*                        HP-Laserdrucker 300*300 DPI                       */
/****************************************************************************/

static struct LJDST HPLJ300 = {

    /* DPI in X-und Y-Richtung*/
  { 300, 300,

    /* Blattgroesse in X- und Y-Richtung in 1/1000 Inch*/
    LJ_A4_SIZEX, LJ_A4_SIZEY,

    /* Anzahl Farben und Bits pro Pixel*/
    2, 1,

    /* Flags*/
    pfDoCompression,

    /* Name des Modus*/
    "\x10""HPLJ 300x300 DPI",       /* Name des Modus*/

    /* Druckeroutine*/
    LaserPrint },

    /* Festlegen der Grafik*/
    "\x1B*t300R"                    /* ESC * t 300 R*/

};



/****************************************************************************/
/*                HP-Laserdrucker 75*75 DPI ohne Kompression                */
/****************************************************************************/

static struct LJDST HPLJ75O = {

    /* DPI in X-und Y-Richtung*/
  { 75, 75,

    /* Blattgroesse in X- und Y-Richtung in 1/1000 Inch*/
    LJ_A4_SIZEX, LJ_A4_SIZEY,

    /* Anzahl Farben und Bits pro Pixel*/
    2, 1,

    /* Flags*/
    0,

    /* Name des Modus*/
    "\x0E""HPLJ 75x75 DPI",         /* Name des Modus*/

    /* Druckeroutine*/
    LaserPrint },

    /* Festlegen der Grafik*/
    "\x1B*t75R"                     /* ESC * t 75 R*/

};



/****************************************************************************/
/*               HP-Laserdrucker 100*100 DPI ohne Kompression               */
/****************************************************************************/

static struct LJDST HPLJ100O = {

    /* DPI in X-und Y-Richtung*/
  { 100, 100,

    /* Blattgroesse in X- und Y-Richtung in 1/1000 Inch*/
    LJ_A4_SIZEX, LJ_A4_SIZEY,

    /* Anzahl Farben und Bits pro Pixel*/
    2, 1,

    /* Flags*/
    0,

    /* Name des Modus*/
    "\x10""HPLJ 100x100 DPI",       /* Name des Modus*/

    /* Druckeroutine*/
    LaserPrint },

    /* Festlegen der Grafik*/
    "\x1B*t100R"                    /* ESC * t 100 R*/

};



/****************************************************************************/
/*               HP-Laserdrucker 150*150 DPI ohne Kompression               */
/****************************************************************************/

static struct LJDST HPLJ150O = {

    /* DPI in X-und Y-Richtung*/
  { 150, 150,

    /* Blattgroesse in X- und Y-Richtung in 1/1000 Inch*/
    LJ_A4_SIZEX, LJ_A4_SIZEY,

    /* Anzahl Farben und Bits pro Pixel*/
    2, 1,

    /* Flags*/
    0,

    /* Name des Modus*/
    "\x10""HPLJ 150x150 DPI",       /* Name des Modus*/

    /* Druckeroutine*/
    LaserPrint },

    /* Festlegen der Grafik*/
    "\x1B*t150R"                    /* ESC * t 150 R*/

};



/****************************************************************************/
/*               HP-Laserdrucker 300*300 DPI ohne Kompression               */
/****************************************************************************/

static struct LJDST HPLJ300O = {

    /* DPI in X-und Y-Richtung*/
  { 300, 300,

    /* Blattgroesse in X- und Y-Richtung in 1/1000 Inch*/
    LJ_A4_SIZEX, LJ_A4_SIZEY,

    /* Anzahl Farben und Bits pro Pixel*/
    2, 1,

    /* Flags*/
    0,

    /* Name des Modus*/
    "\x10""HPLJ 300x300 DPI",       /* Name des Modus*/

    /* Druckeroutine*/
    LaserPrint },

    /* Festlegen der Grafik*/
    "\x1B*t300R"                    /* ESC * t 300 R*/

};



/****************************************************************************/
/*                        HP-Laserdrucker 600*600 DPI                       */
/****************************************************************************/

static struct LJDST HPLJ600 = {

    /* DPI in X-und Y-Richtung*/
  { 600, 600,

    /* Blattgroesse in X- und Y-Richtung in 1/1000 Inch*/
    LJ_A4_SIZEX, LJ_A4_SIZEY,

    /* Anzahl Farben und Bits pro Pixel*/
    2, 1,

    /* Flags*/
    pfDoCompression,

    /* Name des Modus*/
    "\x10""HPLJ 600x600 DPI",       /* Name des Modus*/

    /* Druckeroutine*/
    LaserPrint },

    /* Festlegen der Grafik*/
    "\x1B*t600R"                    /* ESC * t 300 R*/

};



/****************************************************************************/
/*                          75*75 DPI HP-DJ, Farbe                          */
/****************************************************************************/

static struct LJDST HPDJ75 = {

    /* DPI in X-und Y-Richtung*/
  { 75, 75,

    /* Blattgroesse in X- und Y-Richtung in 1/1000 Inch*/
    LJ_A4_SIZEX, LJ_A4_SIZEY,

    /* Anzahl Farben und Bits pro Pixel*/
    8, 3,

    /* Flags*/
    pfDoCompression,

    /* Name des Modus*/
    "\x0F""HPDJ500C 75 DPI",        /* Name des Modus*/

    /* Druckeroutine*/
    LaserPrint },

    /* Einschalten der Grafik*/
    "\x1B*t75R"                     /* ESC * t 75 R*/

};


/****************************************************************************/
/*                         100*100 DPI HP-DJ, Farbe                         */
/****************************************************************************/

static struct LJDST HPDJ100 = {

    /* DPI in X-und Y-Richtung*/
  { 100, 100,

    /* Blattgroesse in X- und Y-Richtung in 1/1000 Inch*/
    LJ_A4_SIZEX, LJ_A4_SIZEY,

    /* Anzahl Farben und Bits pro Pixel*/
    8, 3,

    /* Flags*/
    pfDoCompression,

    /* Name des Modus*/
    "\x10""HPDJ500C 100 DPI",       /* Name des Modus*/

    /* Druckeroutine */
    LaserPrint },

    /* Einschalten der Grafik*/
    "\x1B*t100R"                    /* ESC * t 100 R*/

};


/****************************************************************************/
/*                         150*150 DPI HP-DJ, Farbe                         */
/****************************************************************************/

static struct LJDST HPDJ150 = {

    /* DPI in X-und Y-Richtung*/
  { 150, 150,

    /* Blattgroesse in X- und Y-Richtung in 1/1000 Inch*/
    LJ_A4_SIZEX, LJ_A4_SIZEY,

    /* Anzahl Farben und Bits pro Pixel*/
    8, 3,

    /* Flags*/
    pfDoCompression,

    /* Name des Modus*/
    "\x10""HPDJ500C 150 DPI",       /* Name des Modus*/

    /* Druckeroutine */
    LaserPrint },

    /* Einschalten der Grafik*/
    "\x1B*t150R"                    /* ESC * t 150 R*/

};


/****************************************************************************/
/*                         300*300 DPI HP-DJ, Farbe                         */
/****************************************************************************/

static struct LJDST HPDJ300 = {

    /* DPI in X-und Y-Richtung*/
  { 300, 300,

    /* Blattgroesse in X- und Y-Richtung in 1/1000 Inch*/
    LJ_A4_SIZEX, LJ_A4_SIZEY,

    /* Anzahl Farben und Bits pro Pixel*/
    8, 3,

    /* Flags*/
    pfDoCompression,

    /* Name des Modus*/
    "\x10""HPDJ500C 300 DPI",       /* Name des Modus*/

    /* Druckeroutine */
    LaserPrint },

    /* Einschalten der Grafik*/
    "\x1B*t300R"                    /* ESC * t 300 R*/

};


/****************************************************************************/
/*                 75*75 DPI HP-DJ, Farbe, Schwarzabtrennung                */
/****************************************************************************/

static struct LJDST HPDJ75S = {

    /* DPI in X-und Y-Richtung*/
  { 75, 75,

    /* Blattgroesse in X- und Y-Richtung in 1/1000 Inch*/
    LJ_A4_SIZEX, LJ_A4_SIZEY,

    /* Anzahl Farben und Bits pro Pixel*/
    8, 3,

    /* Flags*/
    pfDoCompression | pfSeparateBlack,

    /* Name des Modus*/
    "\x0F""HPDJ500C 75 DPI",        /* Name des Modus*/

    /* Druckeroutine*/
    LaserPrint },

    /* Einschalten der Grafik*/
    "\x1B*t75R"                     /* ESC * t 75 R*/

};


/****************************************************************************/
/*                100*100 DPI HP-DJ, Farbe, Schwarzabtrennung               */
/****************************************************************************/

static struct LJDST HPDJ100S = {

    /* DPI in X-und Y-Richtung*/
  { 100, 100,

    /* Blattgroesse in X- und Y-Richtung in 1/1000 Inch*/
    LJ_A4_SIZEX, LJ_A4_SIZEY,

    /* Anzahl Farben und Bits pro Pixel*/
    8, 3,

    /* Flags*/
    pfDoCompression | pfSeparateBlack,

    /* Name des Modus*/
    "\x10""HPDJ500C 100 DPI",       /* Name des Modus*/

    /* Druckeroutine */
    LaserPrint },

    /* Einschalten der Grafik*/
    "\x1B*t100R"                    /* ESC * t 100 R*/

};


/****************************************************************************/
/*                150*150 DPI HP-DJ, Farbe, Schwarabtrennung                */
/****************************************************************************/

static struct LJDST HPDJ150S = {

    /* DPI in X-und Y-Richtung*/
  { 150, 150,

    /* Blattgroesse in X- und Y-Richtung in 1/1000 Inch*/
    LJ_A4_SIZEX, LJ_A4_SIZEY,

    /* Anzahl Farben und Bits pro Pixel*/
    8, 3,

    /* Flags*/
    pfDoCompression | pfSeparateBlack,

    /* Name des Modus*/
    "\x10""HPDJ500C 150 DPI",       /* Name des Modus*/

    /* Druckeroutine */
    LaserPrint },

    /* Einschalten der Grafik*/
    "\x1B*t150R"                    /* ESC * t 150 R*/

};


/****************************************************************************/
/*                300*300 DPI HP-DJ, Farbe, Schwarzabtrennung               */
/****************************************************************************/

static struct LJDST HPDJ300S = {

    /* DPI in X-und Y-Richtung*/
  { 300, 300,

    /* Blattgroesse in X- und Y-Richtung in 1/1000 Inch*/
    LJ_A4_SIZEX, LJ_A4_SIZEY,

    /* Anzahl Farben und Bits pro Pixel*/
    8, 3,

    /* Flags*/
    pfDoCompression | pfSeparateBlack,

    /* Name des Modus*/
    "\x10""HPDJ500C 300 DPI",       /* Name des Modus*/

    /* Druckeroutine */
    LaserPrint },

    /* Einschalten der Grafik*/
    "\x1B*t300R"                        /* ESC * t 300 R*/

};


/****************************************************************************/
/*                   24-Nadel Modus 180 * 180 DPI, DIN A3                   */
/****************************************************************************/

/* Achtung: Die Anzahl der Pixel in X- sowie Y-Richtung muss durch 8 teilbar*/
/* sein. Aus diesem Grund sind beim Druckerstring vor den Grafikbytes 2048*/
/* Bytes anstelle (der sich aus Aufloesung/Blattgroesse ergebenden) 2052*/
/* eingetragen.*/

static struct EpsonDST EPSON180x180_A3 = {

    /* DPI in X-und Y-Richtung*/
  { 180, 180,

    /* Blattgroesse in X- und Y-Richtung in 1/1000 Inch*/
    NP_A4_SIZEX, NP_A4_SIZEY,

    /* Anzahl Farben und Bits pro Pixel*/
    2, 1,

    /* Flags*/
    pfIsEpson,

    /* Name des Modus*/
    "\x14""EPSON 180x180 DPI A3",           /* Name des Modus*/

    /* Druckeroutine*/
    EpsonPrint },

    /* Bytes pro Druckerspalte und Durchgaenge*/
    3, 1,

    /* Linefeed1, d.h. normaler Linefeed*/
    "\x04\x1B\x4A\x18\r",               /* ESC 'J' 24 cr*/

    /* LineFeed2, d.h. kleinster Linefeed*/
    NULL,                               /* Nicht vorhanden*/

    /* Einschalten der Grafik*/
    "\x09\x1B\x40\x1B\x4F\x1B\x43\x00\x10\r",   /* ESC '@' ESC 'O' ESC 'C' 0 16 \r*/

    /* Ausschalten der Grafik*/
    "\x03\f\x1B\x40",                   /* \f ESC '@'*/

    /* Code vor den Grafik-Bytes*/
    "\x05\x1B\x2A\x27\x00\x08",         /* ESC '*' 39 2048*/

    /* Code nach den Grafik-Bytes*/
    NULL                                /* Nicht vorhanden*/

};


/****************************************************************************/
/*                   24-Nadel Modus 360 * 180 DPI, DIN A3                   */
/****************************************************************************/

static struct EpsonDST EPSON360x180_A3 = {

    /* DPI in X-und Y-Richtung*/
  { 180, 360,

    /* Blattgroesse in X- und Y-Richtung in 1/1000 Inch*/
    NP_A4_SIZEX, NP_A4_SIZEY,

    /* Anzahl Farben und Bits pro Pixel*/
    2, 1,

    /* Flags*/
    pfIsEpson,

    /* Name des Modus*/
    "\x14""EPSON 360x180 DPI A3",           /* Name des Modus*/

    /* Druckeroutine*/
    EpsonPrint },

    /* Bytes pro Druckerspalte und Durchgaenge*/
    3, 1,

    /* Linefeed1, d.h. normaler Linefeed*/
    "\x04\x1B\x4A\x18\r",                   /* ESC 'J' 24 cr*/

    /* LineFeed2, d.h. kleinster Linefeed*/
    NULL,                                   /* Nicht vorhanden*/

    /* Einschalten der Grafik*/
    "\x09\x1B\x40\x1B\x4F\x1B\x43\x00\x10\r",   /* ESC '@' ESC 'O' ESC 'C' 0 16 \r*/

    /* Ausschalten der Grafik*/
    "\x03\f\x1B\x40",                       /* \f ESC '@'*/

    /* Code vor den Grafik-Bytes*/
    "\x05\x1B\x2A\x28\x08\x10",             /* ESC '*' 40 4104*/

    /* Code nach den Grafik-Bytes*/
    NULL                                    /* Nicht vorhanden*/
};


/****************************************************************************/
/*                24-Nadel Modus 360 * 360 DPI, EPSON, DIN A3               */
/****************************************************************************/

static struct EpsonDST EPSON360x360_A3 = {

    /* DPI in X-und Y-Richtung*/
  { 360, 360,

    /* Blattgroesse in X- und Y-Richtung in 1/1000 Inch*/
    NP_A4_SIZEX, NP_A4_SIZEY,

    /* Anzahl Farben und Bits pro Pixel*/
    2, 1,

    /* Flags*/
    pfIsEpson,

    /* Name des Modus*/
    "\x14""EPSON 360x360 DPI A3",               /* Name des Modus*/

    /* Druckeroutine*/
    EpsonPrint },

    /* Bytes pro Druckerspalte und Durchgaenge*/
    3, 2,

    /* Linefeed1, d.h. normaler Linefeed*/
    "\x05\x1B\x2B\x2F\x0D\x0A",         /* ESC '+' 47 cr lf*/

    /* LineFeed2, d.h. kleinster Linefeed*/
    "\x05\x1B\x2B\x01\x0D\x0A",         /* ESC '+' 01 cr lf*/

    /* Einschalten der Grafik*/
    "\x09\x1B\x40\x1B\x4F\x1B\x43\x00\x10\r",   /* ESC '@' ESC 'O' ESC 'C' 0 16 \r*/

    /* Ausschalten der Grafik*/
    "\x03\f\x1B\x40",                   /* \f ESC '@'*/

    /* Code vor den Grafik-Bytes*/
    "\x05\x1B\x2A\x28\x08\x10",         /* ESC '*' 40 4104*/

    /* Code nach den Grafik-Bytes*/
    NULL                                /* Nicht vorhanden*/
};



/****************************************************************************/
/*               24-Nadel Modus 360 * 360 DPI, NEC P7, DIN A3               */
/****************************************************************************/

static struct EpsonDST NEC360x360_A3 = {

    /* DPI in X-und Y-Richtung*/
  { 360, 360,

    /* Blattgroesse in X- und Y-Richtung in 1/1000 Inch*/
    NP_A4_SIZEX, NP_A4_SIZEY,

    /* Anzahl Farben und Bits pro Pixel*/
    2, 1,

    /* Flags*/
    pfIsEpson,

    /* Name des Modus*/
    "\x15""NEC P7 360x360 DPI A3",      /* Name des Modus*/

    /* Druckeroutine*/
    EpsonPrint },

    /* Bytes pro Druckerspalte und Durchgaenge*/
    3, 2,

    /* Linefeed1, d.h. normaler Linefeed*/
    "\x05\x1C\x33\x2F\x0D\x0A",         /* FS '3' 47 cr lf*/

    /* LineFeed2, d.h. kleinster Linefeed*/
    "\x05\x1C\x33\x01\x0D\x0A",         /* FS '3' 01 cr lf*/

    /* Einschalten der Grafik*/
    "\x09\x1B\x40\x1B\x4F\x1B\x43\x00\x10\r",   /* ESC '@' ESC 'O' ESC 'C' 0 16 \r*/

    /* Ausschalten der Grafik*/
    "\x03\f\x1B\x40",                   /* \f ESC '@'*/

    /* Code vor den Grafik-Bytes*/
    "\x05\x1B\x2A\x28\x08\x10",         /* ESC '*' 40 4104*/

    /* Code nach den Grafik-Bytes*/
    NULL                                /* Nicht vorhanden*/
};



/****************************************************************************/
/*                24-Nadel Modus 180 * 180 DPI, DIN A3, Farbe               */
/****************************************************************************/

/* Achtung: Die Anzahl der Pixel in X- sowie Y-Richtung muss durch 8 teilbar */
/* sein. Aus diesem Grund sind beim Druckerstring vor den Grafikbytes 2048  */
/* Bytes anstelle (der sich aus Aufloesung/Blattgroesse ergebenden) 2052      */
/* eingetragen.                                                             */

static struct EpsonDST EPSON180x180C_A3 = {

    /* DPI in X-und Y-Richtung*/
  { 180, 180,

    /* Blattgroesse in X- und Y-Richtung in 1/1000 Inch*/
    NP_A4_SIZEX, NP_A4_SIZEY,

    /* Anzahl Farben und Bits pro Pixel*/
    9, 4,

    /* Flags*/
    pfIsEpson,

    /* Name des Modus*/
    "\x1A""EPSON 180x180 DPI A3 Color",         /* Name des Modus*/

    /* Druckeroutine*/
    EpsonPrint },

    /* Bytes pro Druckerspalte und Durchgaenge*/
    3, 1,

    /* Linefeed1, d.h. normaler Linefeed*/
    "\x04\x1B\x4A\x18\r",               /* ESC 'J' 24 cr*/

    /* LineFeed2, d.h. kleinster Linefeed*/
    NULL,                               /* Nicht vorhanden*/

    /* Einschalten der Grafik*/
    "\x09\x1B\x40\x1B\x4F\x1B\x43\x00\x10\r",   /* ESC '@' ESC 'O' ESC 'C' 0 16 \r*/

    /* Ausschalten der Grafik*/
    "\x03\f\x1B\x40",                   /* \f ESC '@'*/

    /* Code vor den Grafik-Bytes*/
    "\x05\x1B\x2A\x27\x00\x08",         /* ESC '*' 39 2048*/

    /* Code nach den Grafik-Bytes*/
    NULL                                /* Nicht vorhanden*/

};


/****************************************************************************/
/*                24-Nadel Modus 360 * 180 DPI, DIN A3, Farbe               */
/****************************************************************************/

static struct EpsonDST EPSON360x180C_A3 = {

    /* DPI in X-und Y-Richtung*/
  { 180, 360,

    /* Blattgroesse in X- und Y-Richtung in 1/1000 Inch*/
    NP_A4_SIZEX, NP_A4_SIZEY,

    /* Anzahl Farben und Bits pro Pixel*/
    9, 4,

    /* Flags*/
    pfIsEpson,

    /* Name des Modus*/
    "\x1A""EPSON 360x180 DPI A3 Color",         /* Name des Modus*/

    /* Druckeroutine*/
    EpsonPrint },

    /* Bytes pro Druckerspalte und Durchgaenge*/
    3, 1,

    /* Linefeed1, d.h. normaler Linefeed*/
    "\x04\x1B\x4A\x18\r",                   /* ESC 'J' 24 cr*/

    /* LineFeed2, d.h. kleinster Linefeed*/
    NULL,                                   /* Nicht vorhanden*/

    /* Einschalten der Grafik*/
    "\x09\x1B\x40\x1B\x4F\x1B\x43\x00\x10\r",   /* ESC '@' ESC 'O' ESC 'C' 0 16 \r*/

    /* Ausschalten der Grafik*/
    "\x03\f\x1B\x40",                       /* \f ESC '@'*/

    /* Code vor den Grafik-Bytes*/
    "\x05\x1B\x2A\x28\x08\x10",             /* ESC '*' 40 4104*/

    /* Code nach den Grafik-Bytes*/
    NULL                                    /* Nicht vorhanden*/
};


/****************************************************************************/
/*            24-Nadel Modus 360 * 360 DPI, EPSON, DIN A3, Farbe            */
/****************************************************************************/

static struct EpsonDST EPSON360x360C_A3 = {

    /* DPI in X-und Y-Richtung*/
  { 360, 360,

    /* Blattgroesse in X- und Y-Richtung in 1/1000 Inch*/
    NP_A4_SIZEX, NP_A4_SIZEY,

    /* Anzahl Farben und Bits pro Pixel*/
    9, 4,

    /* Flags*/
    pfIsEpson,

    /* Name des Modus*/
    "\x1A""EPSON 360x360 DPI A3 Color",         /* Name des Modus*/

    /* Druckeroutine*/
    EpsonPrint },

    /* Bytes pro Druckerspalte und Durchgaenge*/
    3, 2,

    /* Linefeed1, d.h. normaler Linefeed*/
    "\x05\x1B\x2B\x2F\x0D\x0A",         /* ESC '+' 47 cr lf*/

    /* LineFeed2, d.h. kleinster Linefeed*/
    "\x05\x1B\x2B\x01\x0D\x0A",         /* ESC '+' 01 cr lf*/

    /* Einschalten der Grafik*/
    "\x09\x1B\x40\x1B\x4F\x1B\x43\x00\x10\r",   /* ESC '@' ESC 'O' ESC 'C' 0 16 \r*/

    /* Ausschalten der Grafik*/
    "\x03\f\x1B\x40",                   /* \f ESC '@'*/

    /* Code vor den Grafik-Bytes*/
    "\x05\x1B\x2A\x28\x08\x10",         /* ESC '*' 40 4104*/

    /* Code nach den Grafik-Bytes*/
    NULL                                /* Nicht vorhanden*/
};



/****************************************************************************/
/*            24-Nadel Modus 360 * 360 DPI, NEC P7, DIN A3, Farbe           */
/****************************************************************************/

static struct EpsonDST NEC360x360C_A3 = {

    /* DPI in X-und Y-Richtung*/
  { 360, 360,

    /* Blattgroesse in X- und Y-Richtung in 1/1000 Inch*/
    NP_A4_SIZEX, NP_A4_SIZEY,

    /* Anzahl Farben und Bits pro Pixel*/
    9, 4,

    /* Flags*/
    pfIsEpson,

    /* Name des Modus*/
    "\x1B""NEC P7 360x360 DPI A3 Color",        /* Name des Modus*/

    /* Druckeroutine*/
    EpsonPrint },

    /* Bytes pro Druckerspalte und Durchgaenge*/
    3, 2,

    /* Linefeed1, d.h. normaler Linefeed*/
    "\x05\x1C\x33\x2F\x0D\x0A",         /* FS '3' 47 cr lf*/

    /* LineFeed2, d.h. kleinster Linefeed*/
    "\x05\x1C\x33\x01\x0D\x0A",         /* FS '3' 01 cr lf*/

    /* Einschalten der Grafik*/
    "\x09\x1B\x40\x1B\x4F\x1B\x43\x00\x10\r",   /* ESC '@' ESC 'O' ESC 'C' 0 16 \r*/

    /* Ausschalten der Grafik*/
    "\x03\f\x1B\x40",                   /* \f ESC '@'*/

    /* Code vor den Grafik-Bytes*/
    "\x05\x1B\x2A\x28\x08\x10",         /* ESC '*' 40 4104*/

    /* Code nach den Grafik-Bytes*/
    NULL                                /* Nicht vorhanden*/
};



/****************************************************************************/
/*                  150*150 DPI HP-DJ 1200C, 256 Farben, A4                 */
/****************************************************************************/

static struct LJDST HPDJ1200C150 = {

    /* DPI in X-und Y-Richtung*/
  { 150, 150,

    /* Blattgroesse in X- und Y-Richtung in 1/1000 Inch*/
    LJ_A4_SIZEX, LJ_A4_SIZEY,

    /* Anzahl Farben und Bits pro Pixel*/
    256, 8,

    /* Flags*/
    pfDoCompression | pfHasPalette,

    /* Name des Modus*/
    "\x11""HPDJ1200C 150 DPI",       /* Name des Modus*/

    /* Druckeroutine */
    LaserPrint },

    /* Einschalten der Grafik*/
    "\x1B*t150R"                    /* ESC * t 150 R*/

};

/****************************************************************************/
/*          HP-Laserdrucker 1200*1200 DPI                                   */
/****************************************************************************/

static struct LJDST HPLJ1200 = {

    /* DPI in X-und Y-Richtung*/
  { 1200, 1200,

    /* Blattgroesse in X- und Y-Richtung in 1/1000 Inch*/
    LJ_A4_SIZEX, LJ_A4_SIZEY,

    /* Anzahl Farben und Bits pro Pixel*/
    2, 1,

    /* Flags*/
    pfDoCompression,

    /* Name des Modus*/
    "\x12""HPLJ 1200x1200 DPI",       /* Name des Modus*/

    /* Druckeroutine*/
    LaserPrint },

    /* Festlegen der Grafik*/
    "\x1B*t1200R"                    /* ESC * t 300 R*/

};


/****************************************************************************/
/*                      Das Array Zeigern auf die DST's                     */
/****************************************************************************/

struct _DST * DSTTable [MaxModes] = {

    (struct _DST*) &EPSON240x72,               /*  0*/
    (struct _DST*) &EPSON240x216,              /*  1*/
    (struct _DST*) &EPSON180x180,              /*  2*/
    (struct _DST*) &EPSON360x180,              /*  3*/
    (struct _DST*) &EPSON360x360,              /*  4*/
    (struct _DST*) &NEC360x360,                /*  5*/
    (struct _DST*) &IBMPro180x180,             /*  6*/
    (struct _DST*) &IBMPro360x180,             /*  7*/
    (struct _DST*) &EPSON180x180C,             /*  8*/
    (struct _DST*) &EPSON360x180C,             /*  9*/
    (struct _DST*) &EPSON360x360C,             /* 11*/
    (struct _DST*) &NEC360x360C,               /* 10*/
    (struct _DST*) NULL,                       /* 12*/
    (struct _DST*) NULL,                       /* 13 Benutzerdefinierter Modus 1*/
    (struct _DST*) NULL,                       /* 14 Benutzerdefinierter Modus 2*/
    (struct _DST*) NULL,                       /* 15 Benutzerdefinierter Modus 3*/
    (struct _DST*) &HPLJ75,                    /* 16 LaserJet-Modi*/
    (struct _DST*) &HPLJ100,                   /* 17*/
    (struct _DST*) &HPLJ150,                   /* 18*/
    (struct _DST*) &HPLJ300,                   /* 19*/
    (struct _DST*) &HPLJ75O,                   /* 20 Nochmals LJ, ohne Kompression*/
    (struct _DST*) &HPLJ100O,                  /* 21*/
    (struct _DST*) &HPLJ150O,                  /* 22*/
    (struct _DST*) &HPLJ300O,                  /* 23*/
    (struct _DST*) &HPDJ75,                    /* 24 Farbmodi des DeskJet*/
    (struct _DST*) &HPDJ100,                   /* 25*/
    (struct _DST*) &HPDJ150,                   /* 26*/
    (struct _DST*) &HPDJ300,                   /* 27*/
    (struct _DST*) &HPDJ75S,                   /* 28 Farbmodi, Deskjet mit Schwarzabtrennung*/
    (struct _DST*) &HPDJ100S,                  /* 29*/
    (struct _DST*) &HPDJ150S,                  /* 30*/
    (struct _DST*) &HPDJ300S,                  /* 31*/
    (struct _DST*) &HPLJ600,                   /* 32*/
    (struct _DST*) &EPSON180x180_A3,           /* 33   A3 Modi*/
    (struct _DST*) &EPSON360x180_A3,           /* 34*/
    (struct _DST*) &EPSON360x360_A3,           /* 35*/
    (struct _DST*) &NEC360x360_A3,             /* 36*/
    (struct _DST*) &EPSON180x180C_A3,          /* 37   A3 Modi, Farbe*/
    (struct _DST*) &EPSON360x180C_A3,          /* 38*/
    (struct _DST*) &EPSON360x360C_A3,          /* 39*/
    (struct _DST*) &NEC360x360C_A3,            /* 40*/
    (struct _DST*) &HPDJ1200C150,              /* 41   Deskjet 1200C, 256 Farben*/
    (struct _DST*) &HPLJ1200,                  /* 42   HP LJ4000, 1200 * 1200 DPI*/

};



