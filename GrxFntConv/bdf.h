#ifndef __BDF_H_
#define __BDF_H_

#include <stdbool.h>

#include "libgrx.h"

int bdf_width(int ch);
int bdf_bitmap(int ch, int w, int h, char *buffer);
bool bdf_read_data(char *fname, GrFontHeader *h, unsigned int size);

#endif  // __BDF_H_
