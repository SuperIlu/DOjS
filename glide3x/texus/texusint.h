/*
** Copyright (c) 1995, 3Dfx Interactive, Inc.
** All Rights Reserved.
**
** This is UNPUBLISHED PROPRIETARY SOURCE CODE of 3Dfx Interactive, Inc.;
** the contents of this file may not be disclosed to third parties, copied or
** duplicated in any form, in whole or in part, without the prior written
** permission of 3Dfx Interactive, Inc.
**
** RESTRICTED RIGHTS LEGEND:
** Use, duplication or disclosure by the Government is subject to restrictions
** as set forth in subdivision (c)(1)(ii) of the Rights in Technical Data
** and Computer Software clause at DFARS 252.227-7013, and/or in similar or
** successor clauses in the FAR, DOD or NASA FAR Supplement. Unpublished  -
** rights reserved under the Copyright Laws of the United States.
*/

#ifndef __TEXUSINT_H__
#define __TEXUSINT_H__

#include "texus.h"

int txBitsPerPixel(GrTextureFormat_t format);

#define GR_TEXFMT_SIZE(x) txBitsPerPixel(x)

#define TX_OFORMAT_3DF                                  10
#define TX_OFORMAT_TGA                                  11
#define TX_OFORMAT_PPM                                  12

#ifndef MAX
#define MAX(x,y)        ((x) > (y) ? (x) : (y))
#endif
#ifndef MAX
#define MIN(x,y)        ((x) < (y) ? (x) : (y))
#endif

#define TX_MAX_LEVEL       16
typedef struct  _TxMip {
    int             format;                         /* GR_TEXFMT_... */
    int             width;                          /* pixels */
    int             height;                         /* pixels */
    int             depth;                          /* mipmap levels */
    int             size;                           /* bytes */
        void    *data[TX_MAX_LEVEL];
        FxU32   pal[256];
} TxMip;

FxBool  txMipRead(TxMip *txMip, const char *filename, int preferredFormat);
FxBool  txMipReadFromFP(TxMip *txMip, const char *debug_filename, FILE *file, int preferredFormat);
void    txMipWrite(TxMip *txMip, char *file, char *ext, int split);
void    txMipResample(TxMip *destMip, TxMip *srcMip);
void    txMipClamp( TxMip *dstMip, TxMip *srcMip );
void    txMipMipmap(TxMip *txMip);

FX_ENTRY void FX_CALL txMipQuantize(TxMip *pxMip, TxMip *txMip, int fmt, FxU32 d, FxU32 comp);
void    txMipNcc(TxMip *pxMip, TxMip *txMip, int fmt, FxU32 dither, FxU32 comp);
void    txMipNccNNet(TxMip *pxMip, TxMip *txMip, int fmt, FxU32 dither, FxU32 comp);
int     txMipPal256(TxMip *pxMip, TxMip *txMip, int fmt, FxU32 dither, FxU32 comp);
int     txMipPal6666(TxMip *pxMip, TxMip *txMip, int fmt, FxU32 dither, FxU32 comp);

void    txMipDequantize(TxMip *txMip, TxMip *pxMip);
void    txMipView(TxMip *txMip, char *filename, int wait, int bgcolor);
void    txViewClose();

int             txLog2(int n);
int     txFloorPow2(int n);
int             txCeilPow2(int n);
int     txGCD(int a, int b);
int             txAspectRatio(int w, int h);
void    txPanic(char *);
void    txError(char *);
void    txYABtoPal256(int *palette, const int* yabTable);
void    txRectCopy(FxU8 *dst, int dstStride, const FxU8 *src, int srcStride,
                        int width, int height);
FxBool  txMipAlloc(TxMip *txMip);
FxBool  txMipSetMipPointers(TxMip *txMip);
int             txMemRequired(TxMip *txMip);
void    txBasename(const char *name, char *base);
void    txPathAndBasename(const char *name, char* pathbase);
void    txExtension(const char *name, char *ext);

void txMipFree( TxMip *mip );
void txMipTrueToFixedPal( TxMip *outputMip, TxMip *trueColorMip, const FxU32 *pal,
                          FxU32 flags );


extern  int txVerbose;
extern  int *explode3;
#define DISTANCE(ar, ag, ab, br, bg, bb) \
                ((explode3[(ar)-(br)] << 1) + (explode3[(ag)-(bg)]<<2) + explode3[(ab)-(bb)])

void    txDiffuseIndex(TxMip *pxMip, TxMip *txMip, int pixsize, 
                const FxU32 *palette, int       ncolors);
int             txNearestColor(int ir, int ig, int ib, const FxU32 *pal, int npal);

FxBool _txReadTGAHeader( FILE *stream, FxU32 cookie, TxMip *info);
FxBool _txReadTGAData( FILE *stream, TxMip *info);

FxBool _txReadRGTHeader( FILE *stream, FxU32 cookie, TxMip *info);
FxBool _txReadRGTData( FILE *stream, TxMip *info);

FxBool _txReadSBIHeader( FILE *stream, FxU32 cookie, TxMip *info);
FxBool _txReadSBIData( FILE *stream, TxMip *info);

FxBool _txReadPPMHeader( FILE *stream, FxU32 cookie, TxMip *info);
FxBool _txReadPPMData( FILE *stream, TxMip *info);

FxBool _txRead3DFHeader( FILE *stream, FxU32 cookie, TxMip *info);
FxBool _txRead3DFData( FILE *stream, TxMip *info);

int _txReadHeader( FILE *stream, TxMip *info );

FX_ENTRY void FX_CALL txPalToNcc( GuNccTable *ncc_table, const FxU32 *pal );
void txNccToPal( FxU32 *pal, const GuNccTable *ncc_table );

#define MAX_TEXWIDTH    2048
#define MAX_TEXWIDTH_3DF 256
#define MAX_TEXWIDTH_TXS 2048


extern TxErrorCallbackFnc_t _txErrorCallback;

extern  char *Format_Name[];


/* General read functions for 3DF/TXS files */


/*FxBool _txRead3DFNCCTable (FILE* stream, FxI32* ncc_table); */
/*FxBool _txRead3DFPalTable (FILE* stream, FxI32* pal); */

/* General write functions for 3DF/TXS files */

/*FxBool _txWrite3dfNCCTable (FILE *stream, FxU32 *yab); */
/*FxBool _txWrite3dfPalTable (FILE *stream, FxU32 *pal); */

/* TXS constants */

#define TXS_COOKIE             "TXSF"
#define TXS_COOKIE_SIZE        5
#define TXS_VERSION            1.0f
#define TXS_READ_NUM_FIELDS    7
#define TXS_WRITE_OFFSET_SIZE  9



/* TXS file header

   New fields can be added at any time, but they should ALWAYS be added after the data_offset
   field.  This keeps backwards compatibility in place if the new fields aren't mandatory.

   Field Definitions:

   cookie			- used to identify a true TXS file
   version			- TXS version for the file.  For backwards compatible changes (i.e. the file
					  can still be loaded and used by earlier versions of TXS/Texus ), only the
					  minor revision number should be changed.  For non-backwards compatible
					  changes, the major revision number must be changed.
   format			- texture format of the data
   width			- width of the texture
   height			- height of the texture
   mipmap_levels	- number of mipmap levels contained in the data
   data_offset		- offset in bytes from the start of the file to the actual texture data
*/

typedef struct
{
	char  cookie[TXS_COOKIE_SIZE];
	float version;
	FxU16 format;
	FxU16 width;
	FxU16 height;
	FxU16 mipmap_levels;
    FxU32 data_offset;  /* in bytes */
} TXSHeader;

/* TXS info structure

   Contains the general information needed by program & utility procedures.  This is similar to
   the TXS file header structure, but doesn't contain the cookie, data_offset, or any other data
   only needed for the file itself.

   Special versions of this structure can easily be made for specific APIs like Glide or D3D if
   necessary.
*/

typedef struct
{
	float version;
	FxU16 format;
	FxU16 width;
	FxU16 height;
	FxU16 mipmap_levels;
	FxU32 data_size;
	union
	{
		void *palette;
		void *nccTable;
	} table;
	void  *data;
} TXSInfo;

typedef struct
{
	FxBool new_mipmaps;
	FxBool blend_alpha;
	FxBool force_alpha;
	FxU8   alpha_value;
	int    fxt1_format;
	FxU32  dither;
	FxU32  compression;
} TXSConvertOptions;

void initTXSInfo (TXSInfo *info);
FxU32 calcTXSMemRequired (TXSInfo *info);
void freeTXS (TXSInfo *info);
FxBool getTXSInfo (const char *filename, TXSInfo *info);
FxBool loadTXS (const char *filename, TXSInfo *info);
FxBool writeTXS (const char *filename, TXSInfo *info);
void initTXSConvertOptions (TXSConvertOptions *opt);
FxBool convertTXS (TXSInfo *src, TXSInfo *dest, TXSConvertOptions *opt);
FxBool convertTXSFile (const char *filename, TXSInfo *dest, TXSConvertOptions *opt);

FxBool _txReadTXSHeader( FILE *stream, TxMip *info, FxBool output_info );
FxBool _txReadTXSData ( FILE *stream, TxMip *txMip );

FxBool txWriteTXS( FILE *stream, TxMip *txMip);


#endif  /* __TEXUSINT_H__ */
