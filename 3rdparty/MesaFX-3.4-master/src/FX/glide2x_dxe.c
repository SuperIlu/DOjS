/* C source for generating a dummy glide2x.dxe to be used when
 * generating an import library for a dxe depending on glide2x.
 *
 * gcc -Wall -c glide2x.c -o glide2x.o
 * dxe3gen -o glide2x.dxe -E _gr -E _gu -E _ConvertAndDownloadRle -U glide2x.o
 */

/* common gr and gu symbols exported by all
 * sst1, sst96, cvg, and h3 glide2x.dxe:
 */
void ConvertAndDownloadRle () {}
void grAADrawLine () {}
void grAADrawPoint () {}
void grAADrawPolygon () {}
void grAADrawPolygonVertexList () {}
void grAADrawTriangle () {}
void grAlphaBlendFunction () {}
void grAlphaCombine () {}
void grAlphaControlsITRGBLighting () {}
void grAlphaTestFunction () {}
void grAlphaTestReferenceValue () {}
void grBufferClear () {}
void grBufferNumPending () {}
void grBufferSwap () {}
void grCheckForRoom () {}
void grChromakeyMode () {}
void grChromakeyValue () {}
void grClipWindow () {}
void grColorCombine () {}
void grColorMask () {}
void grConstantColorValue () {}
void grConstantColorValue4 () {}
void grCullMode () {}
void grDepthBiasLevel () {}
void grDepthBufferFunction () {}
void grDepthBufferMode () {}
void grDepthMask () {}
void grDisableAllEffects () {}
void grDitherMode () {}
void grDrawLine () {}
void grDrawPlanarPolygon () {}
void grDrawPlanarPolygonVertexList () {}
void grDrawPoint () {}
void grDrawPolygon () {}
void grDrawPolygonVertexList () {}
void grDrawTriangle () {}
void grErrorSetCallback () {}
void grFogColorValue () {}
void grFogMode () {}
void grFogTable () {}
void grGammaCorrectionValue () {}
void grGlideGetState () {}
void grGlideGetVersion () {}
void grGlideInit () {}
void grGlideSetState () {}
void grGlideShamelessPlug () {}
void grGlideShutdown () {}
void grHints () {}
void grLfbConstantAlpha () {}
void grLfbConstantDepth () {}
void grLfbLock () {}
void grLfbReadRegion () {}
void grLfbUnlock () {}
void grLfbWriteColorFormat () {}
void grLfbWriteColorSwizzle () {}
void grLfbWriteRegion () {}
void grRenderBuffer () {}
void grResetTriStats () {}
void grSplash () {}
void grSstConfigPipeline () {}
void grSstControl () {}
void grSstIdle () {}
void grSstIsBusy () {}
void grSstOrigin () {}
void grSstPerfStats () {}
void grSstQueryBoards () {}
void grSstQueryHardware () {}
void grSstResetPerfStats () {}
void grSstScreenHeight () {}
void grSstScreenWidth () {}
void grSstSelect () {}
void grSstStatus () {}
void grSstVideoLine () {}
void grSstVidMode () {}
void grSstVRetraceOn () {}
void grSstWinClose () {}
void grSstWinOpen () {}
void grTexCalcMemRequired () {}
void grTexClampMode () {}
void grTexCombine () {}
void grTexCombineFunction () {}
void grTexDetailControl () {}
void grTexDownloadMipMap () {}
void grTexDownloadMipMapLevel () {}
void grTexDownloadMipMapLevelPartial () {}
void grTexDownloadTable () {}
void grTexDownloadTablePartial () {}
void grTexFilterMode () {}
void grTexLodBiasValue () {}
void grTexMaxAddress () {}
void grTexMinAddress () {}
void grTexMipMapMode () {}
void grTexMultibase () {}
void grTexMultibaseAddress () {}
void grTexNCCTable () {}
void grTexSource () {}
void grTexTextureMemRequired () {}
void grTriStats () {}
void gu3dfGetInfo () {}
void gu3dfLoad () {}
void guAADrawTriangleWithClip () {}
void guAlphaSource () {}
void guColorCombineFunction () {}
void guDrawPolygonVertexListWithClip () {}
void guDrawTriangleWithClip () {}
void guEncodeRLE16 () {}
void guEndianSwapBytes () {}
void guEndianSwapWords () {}
void guFbReadRegion () {}
void guFbWriteRegion () {}
void guFogGenerateExp () {}
void guFogGenerateExp2 () {}
void guFogGenerateLinear () {}
void guFogTableIndexToW () {}
void guMovieSetName () {}
void guMovieStart () {}
void guMovieStop () {}
void guMPDrawTriangle () {}
void guMPInit () {}
void guMPTexCombineFunction () {}
void guMPTexSource () {}
void guTexAllocateMemory () {}
void guTexChangeAttributes () {}
void guTexCombineFunction () {}
void guTexCreateColorMipMap () {}
void guTexDownloadMipMap () {}
void guTexDownloadMipMapLevel () {}
void guTexGetCurrentMipMap () {}
void guTexGetMipMapInfo () {}
void guTexMemQueryAvail () {}
void guTexMemReset () {}
void guTexSource () {}

/* external libc symbols required by all of
 * sst1, sst96, cvg, and h3 glide2x.dxe:
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <dos.h>
#include <dpmi.h>
#include <sys/nearptr.h>
#include <setjmp.h>
#include <crt0.h>
long EXTSYM0000 = (long) &_crt0_startup_flags;
long EXTSYM0001 = (long) &__dj_assert;
long EXTSYM0002 = (long) &__djgpp_base_address;
long EXTSYM0003 = (long) &__djgpp_nearptr_disable;
long EXTSYM0004 = (long) &__djgpp_nearptr_enable;
long EXTSYM0005 = (long) &__dj_stderr;
long EXTSYM0006 = (long) &__dj_stdout;
long EXTSYM0007 = (long) &__dpmi_free_physical_address_mapping;
long EXTSYM0008 = (long) &__dpmi_physical_address_mapping;
long EXTSYM0009 = (long) &atof;
long EXTSYM0010 = (long) &atoi;
long EXTSYM0011 = (long) &atol;
long EXTSYM0012 = (long) &clock;
long EXTSYM0013 = (long) &exit;
long EXTSYM0014 = (long) &fclose;
long EXTSYM0015 = (long) &fflush;
long EXTSYM0016 = (long) &fgetc;
long EXTSYM0017 = (long) &fgets;
long EXTSYM0018 = (long) &fopen;
long EXTSYM0019 = (long) &fprintf;
long EXTSYM0020 = (long) &fread;
long EXTSYM0021 = (long) &free;
long EXTSYM0022 = (long) &fwrite;
long EXTSYM0023 = (long) &getc;
long EXTSYM0024 = (long) &getenv;
long EXTSYM0025 = (long) &int86;
long EXTSYM0026 = (long) &longjmp;
long EXTSYM0027 = (long) &malloc;
long EXTSYM0028 = (long) &memcpy;
long EXTSYM0029 = (long) &memset;
long EXTSYM0030 = (long) &pow;
long EXTSYM0031 = (long) &printf;
long EXTSYM0032 = (long) &putenv;
long EXTSYM0033 = (long) &puts;
long EXTSYM0034 = (long) &setjmp;
long EXTSYM0035 = (long) &signal;
long EXTSYM0036 = (long) &sprintf;
long EXTSYM0037 = (long) &sscanf;
long EXTSYM0038 = (long) &strcat;
long EXTSYM0039 = (long) &strchr;
long EXTSYM0040 = (long) &strcmp;
long EXTSYM0041 = (long) &strcpy;
long EXTSYM0042 = (long) &strncat;
long EXTSYM0043 = (long) &strncpy;
long EXTSYM0044 = (long) &strtok;
long EXTSYM0045 = (long) &strtoul;
long EXTSYM0046 = (long) &usleep;
long EXTSYM0047 = (long) &vfprintf;
long EXTSYM0048 = (long) &vsprintf;

