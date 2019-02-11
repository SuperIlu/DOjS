/**
 ** BCC2GRX  -  Interfacing Borland based graphics programs to LIBGRX
 ** Copyright (C) 1993-97 by Hartmut Schirmer
 **
 **
 ** Contact :                Hartmut Schirmer
 **                          Feldstrasse 118
 **                  D-24105 Kiel
 **                          Germany
 **
 ** e-mail : hsc@techfak.uni-kiel.de
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

#include "bccgrx00.h"

char *getdrivername( void )
{
  _DO_INIT_CHECK_RV(NULL);
#ifdef GRX_VERSION
{
  static char *grxname = NULL;
  const GrVideoDriver *vd = GrCurrentVideoDriver();
  if (vd != NULL) {
    grxname = realloc(grxname, 16+strlen(vd->name));
    if (grxname != NULL) {
      strcpy(grxname, "GRX driver \"");
      strcat(grxname, vd->name);
      strcat(grxname, "\"");
      return grxname;
    }
  }
}
#else
  switch (__gr_ADAPTER) {
    case GR_VGA         : return "VGA driver";
    case GR_EGA         : return "EGA driver";
    case GR_HERC        : return "Hercules mono driver";
    case GR_8514A       : return "8514A driver";
    case GR_S3          : return "S3 graphics accelerator driver";
  }
#endif
  return "unknown graphics driver";
}

