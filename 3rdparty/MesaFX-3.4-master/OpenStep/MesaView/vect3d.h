/*
 * vect3d.h - the definitions for vect3d.c functions.
 */

extern void	error(char *);
extern void	diff3(GLdouble [3], GLdouble [3], GLdouble [3]);
extern void	add3(GLdouble [3], GLdouble [3], GLdouble [3]);
extern void	scalarmult(GLdouble, GLdouble [3], GLdouble [3]);
extern GLdouble	dot3(GLdouble [3], GLdouble [3]);
extern GLdouble	length3(GLdouble [3]);
extern GLdouble	dist3(GLdouble [3], GLdouble [3]);
extern void	copy3(GLdouble [3], GLdouble [3]);
extern void	crossprod(GLdouble [3], GLdouble [3], GLdouble [3]);
extern void	normalize(GLdouble [3]);
extern void	print3(GLdouble [3]);
extern void	printmat3(GLdouble [3][3]);
extern void	identifymat3(GLdouble [3][3]);
extern void	copymat3(GLdouble *, GLdouble *);
extern void	xformvec3(GLdouble [3], GLdouble [3][3], GLdouble [3]);
extern long	samepoint(GLdouble p1[3], GLdouble p2[3]);
extern void	perpnorm(GLdouble p1[3], GLdouble p2[3],
                         GLdouble p3[3], GLdouble n[3]);
