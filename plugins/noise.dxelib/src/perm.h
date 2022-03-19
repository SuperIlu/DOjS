// noise1234
//
// Author: Stefan Gustavson, 2003-2005
// Contact: stefan.gustavson@liu.se
//
// This code was GPL licensed until February 2011.
// As the original author of this code, I hereby
// release it into the public domain.
// Please feel free to use it for whatever you want.
// Credit is appreciated where appropriate, and I also
// appreciate being told where this code finds any use,
// but you may do as you like.

extern const unsigned char perm[512];

extern float grad1(int hash, float x);
extern float grad2(int hash, float x, float y);
extern float grad3(int hash, float x, float y, float z);
extern float grad4(int hash, float x, float y, float z, float t);
