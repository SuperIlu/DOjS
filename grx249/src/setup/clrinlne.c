/**
 ** clrinlne.c ---- the color inline functions
 **
 ** Copyright (c) 1995 Csaba Biegl, 820 Stirrup Dr, Nashville, TN 37221
 ** [e-mail: csaba@vuse.vanderbilt.edu]
 **
 ** This file is part of the GRX graphics library.
 **
 ** The GRX graphics library is free software; you can redistribute it
 ** and/or modify it under some conditions; see the "copying.grx" file
 ** for details.
 **
 ** This library is distributed in the hope that it will be useful,
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 **
 **/

#include "libgrx.h"

GrColor (GrColorValue)(GrColor c)
{
	return(GrColorValue(c));
}

GrColor (GrColorMode)(GrColor c)
{
	return(GrColorMode(c));
}

GrColor (GrWriteModeColor)(GrColor c)
{
	return(GrWriteModeColor(c));
}

GrColor (GrXorModeColor)(GrColor c)
{
	return(GrXorModeColor(c));
}

GrColor (GrOrModeColor)(GrColor c)
{
	return(GrOrModeColor(c));
}
GrColor (GrAndModeColor)(GrColor c)
{
	return(GrAndModeColor(c));
}

GrColor (GrImageModeColor)(GrColor c)
{
	return(GrImageModeColor(c));
}

GrColor (GrNumColors)(void)
{
	return(GrNumColors());
}

GrColor (GrNumFreeColors)(void)
{
	return(GrNumFreeColors());
}

GrColor (GrBuildRGBcolorT)(int r,int g,int b)
{
	return(GrBuildRGBcolorT(r,g,b));
}

GrColor (GrBuildRGBcolorR)(int r,int g,int b)
{
	return(GrBuildRGBcolorR(r,g,b));
}

int (GrRGBcolorRed)(GrColor c)
{
	return(GrRGBcolorRed(c));
}

int (GrRGBcolorGreen)(GrColor c)
{
	return(GrRGBcolorGreen(c));
}

int (GrRGBcolorBlue)(GrColor c)
{
	return(GrRGBcolorBlue(c));
}

GrColor (GrAllocColorID)(int r,int g,int b)
{
	return(GrAllocColorID(r,g,b));
}

GrColor (GrAllocColor2)(long hcolor)
{
        return(GrAllocColor2(hcolor));
}

GrColor (GrAllocColor2ID)(long hcolor)
{
        return(GrAllocColor2ID(hcolor));
}

void (GrQueryColorID)(GrColor c,int *r,int *g,int *b)
{
	GrQueryColorID(c,r,g,b);
}

void (GrQueryColor2ID)(GrColor c,long *hcolor)
{
        GrQueryColor2ID(c,hcolor);
}

