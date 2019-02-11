/**
 ** vesainfo.c ---- test program to print VESA BIOS information
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
 ** Contributions by: (See "doc/credits.doc" for details)
 ** Hartmut Schirmer (hsc@xaphod.techfak.uni-kiel.d400.de)
 **/

#include "libgrx.h"
#include "vesa.h"

typedef struct {
    int    bit;
    char  *name;
#   define DEFBIT(bit)     { bit, #bit }
} bitdef;

bitdef modeattrbits[] = {
    DEFBIT(MODE_SUPPORTED),
    DEFBIT(MODE_EXTINFO),
    DEFBIT(MODE_SUPBIOS),
    DEFBIT(MODE_ISCOLOR),
    DEFBIT(MODE_ISGRAPHICS),
    /* ==== VESA 2.0 and later ==== */
    DEFBIT(MODE_NON_VGA),
    DEFBIT(MODE_NO_BANKING),
    DEFBIT(MODE_LIN_FRAME),
    { 0 }
};

bitdef winattrbits[] = {
    DEFBIT(WIN_SUPPORTED),
    DEFBIT(WIN_READABLE),
    DEFBIT(WIN_WRITABLE),
    { 0 }
};

bitdef memorymodels[] = {
    DEFBIT(MODEL_TEXT),
    DEFBIT(MODEL_CGA),
    DEFBIT(MODEL_HERC),
    DEFBIT(MODEL_4PLANE),
    DEFBIT(MODEL_PACKED),
    DEFBIT(MODEL_256_NC),
    DEFBIT(MODEL_DIRECT),
    { 0 }
};

bitdef capabilitiesbits[] = {
    DEFBIT(CAP_DAC_WIDTH),
    /* ==== VESA 2.0 and later ==== */
    DEFBIT(CAP_NON_VGA),
    DEFBIT(CAP_DAC_BLANK),
    { 0 }
};

void printbits(int value,bitdef *def)
{
	int prev = 0;

	while(def->bit != 0) {
	    if(value & def->bit) {
		if(prev) printf(" | ");
		printf(def->name);
		prev = 1;
	    }
	    def++;
	}
	if(!prev) printf("0");
	printf("\n");
}

void printinfo(VESAvgaInfoBlock *vgainfo)
{
	char  far *sp = vgainfo->OEMstringPtr;
	short far *mp = vgainfo->VideoModePtr;

	printf("VESASignature:\t\"%c%c%c%c\"\n",
	    vgainfo->VESAsignature[0],
	    vgainfo->VESAsignature[1],
	    vgainfo->VESAsignature[2],
	    vgainfo->VESAsignature[3]
	);
	printf("VESAVersion:\t%d.%d\n",
	    VESA_VERSION_MAJOR(vgainfo->VESAversion),
	    VESA_VERSION_MINOR(vgainfo->VESAversion)
	);
	printf("OEMStringPtr:\t\"");
	while(*sp != '\0') putchar(*sp++);
	printf("\"\nCapabilities:\t"),
	printbits((int)vgainfo->Capabilities,capabilitiesbits);
	printf("VideoModePtr:\t0x%08lx\n",(long)mp);
	printf("Video Modes:\t");
	while(*mp != (short)(-1)) printf("0x%x ",(unsigned short)(*mp++));
	printf("\n");
	if(vgainfo->VESAversion >= VESA_VERSION(1,2)) {
	    printf("Memory Size:\t%d*64KBytes\n",vgainfo->MemorySize);
	}
	printf("\n");

}

char *getmodelname(int model)
{
	static char temp[50];

	if(model < 0) return(sprintf(temp,"Invalid model [%d]",model),temp);
	if(model <= MODEL_DIRECT) return(memorymodels[model].name);
	if(model <= 15) return(sprintf(temp,"VESA model [0x%02x]",model),temp);
	return(sprintf(temp,"OEM model [%0x2x]",model),temp);
}

void printmodeinfo(int mode,int version,VESAmodeInfoBlock *modeinfo)
{
	printf("Mode 0x%x is supported\n",mode);
	printf("  ModeAttributes:   ");
	printbits(modeinfo->ModeAttributes,modeattrbits);
	printf("  WinAAttributes:   ");
	printbits(modeinfo->WinAAttributes,winattrbits);
	printf("  WinBAttributes:   ");
	printbits(modeinfo->WinBAttributes,winattrbits);
	printf("  WinGranularity:   %d\n",modeinfo->WinGranularity);
	printf("  WinSize:          %d\n",modeinfo->WinSize);
	printf("  WinASegment:      0x%04x\n",(unsigned short)modeinfo->WinASegment);
	printf("  WinBSegment:      0x%04x\n",(unsigned short)modeinfo->WinBSegment);
	printf("  WinFuncPtr:       0x%08lx\n",(long)modeinfo->WinFuncPtr);
	printf("  BytesPerScanLine: %d\n",modeinfo->BytesPerScanLine);
	if(!(modeinfo->ModeAttributes & MODE_EXTINFO)) return;
	printf("  XResolution:      %d\n",modeinfo->XResolution);
	printf("  YResolution:      %d\n",modeinfo->YResolution);
	printf("  XCharSize:        %d\n",modeinfo->XCharSize);
	printf("  YCharSize:        %d\n",modeinfo->YCharSize);
	printf("  NumberOfPlanes:   %d\n",modeinfo->NumberOfPlanes);
	printf("  BitsPerPixel:     %d\n",modeinfo->BitsPerPixel);
	printf("  NumberOfBanks:    %d\n",modeinfo->NumberOfBanks);
	printf("  MemoryModel:      %d (%s)\n",modeinfo->MemoryModel,getmodelname(modeinfo->MemoryModel));
	printf("  BankSize:         %d\n",modeinfo->BankSize);
	printf("  NumImagePages     %d\n",modeinfo->NumImagePages);
	if(version < VESA_VERSION(1,2)) return;
	printf("  RedMaskSize:      %d\n",modeinfo->RedMaskSize);
	printf("  RedMaskPos:       %d\n",modeinfo->RedMaskPos);
	printf("  GreenMaskSize:    %d\n",modeinfo->GreenMaskSize);
	printf("  GreenMaskPos:     %d\n",modeinfo->GreenMaskPos);
	printf("  BlueMaskSize:     %d\n",modeinfo->BlueMaskSize);
	printf("  BlueMaskPos:      %d\n",modeinfo->BlueMaskPos);
	printf("  ReservedMaskSize: %d\n",modeinfo->ReservedMaskSize);
	printf("  ReservedMaskPos:  %d\n",modeinfo->ReservedMaskPos);
	printf("  DirectScreenMode: %d\n",modeinfo->DirectScreenMode);
	if(version < VESA_VERSION(2,0)) return;
	printf("  LinearFrameBuffer:0x%08lx\n", modeinfo->LinearFrameBuffer);
	printf("  StartOffScreenMem:0x%08lx\n", modeinfo->StartOffScreenMem);
	printf("  OffScreenMemSize: %d kb\n", modeinfo->OffScreenMemSize);
}

#define PTR_ADD(p,o) ((void *) ((char*)(p)+(o)) )

void printpminfo(VESApmInfoBlock *pb) {
	unsigned short *st = (unsigned short *) PTR_ADD(pb,pb->SubTable_off+VESApmInfoBlock_BASEOFF);

	printf("VESA bios includes protected mode support:\n");
	printf("  PM info table start:\t\t%04x:%04x\n", pb->RealMode_SEG, pb->RealMode_OFF);
	printf("  PM info table length:\t\t0x%04x\n", pb->PhysicalLength);
	printf("  set window offset:\t\t%04x (%04x:%04x)\n", pb->SetWindow_off, pb->RealMode_SEG, pb->RealMode_OFF+pb->SetWindow_off);
	printf("  set display start offset:\t%04x (%04x:%04x)\n", pb->DisplStart_off, pb->RealMode_SEG, pb->RealMode_OFF+pb->DisplStart_off);
	printf("  set primary palette offset:\t%04x (%04x:%04x)\n", pb->PPalette_off, pb->RealMode_SEG, pb->RealMode_OFF+pb->PPalette_off);
	printf("  resource table offset:\t%04x (%04x:%04x)\n", pb->SubTable_off, pb->RealMode_SEG, pb->RealMode_OFF+pb->SubTable_off);
	if (pb->SubTable_off != 0) {
	  int first;
	  printf("  required ports:\t\t");
	  first = 1;
	  while (*st != 0xffff) {
	    printf("%s%04x", (first?"":", "), *st);
	    ++st;
	  }
	  printf("\n  required memory areas:\t");
	  first = 1;
	  while (*st != 0xffff) {
	    unsigned long start = *st++;
	    unsigned long end   = *st++;
	    start <<= 4;
	    end += start;
	    printf("%s%06lx-%06lx", (first?"":", "), start, end-1);
	  }
	  printf("\n");
	}
	printf("\n");
}

int main(void)
{
	VESApmInfoBlock  *pb;
	VESAvgaInfoBlock  vb;
	VESAmodeInfoBlock mb;
	if(_GrViDrvVESAgetVGAinfo(&vb)) {
	    short far *mp = vb.VideoModePtr;
	    printinfo(&vb);
	    if((pb=_GrViDrvVESAgetPMinfo()) != NULL)
	      printpminfo(pb);
	    while(*mp != (short)(-1)) {
		if(_GrViDrvVESAgetModeInfo(*mp,&mb)) {
		    printmodeinfo(*mp,vb.VESAversion,&mb);
		}
		else {
		    printf("Mode 0x%x IS NOT SUPPORTED!\n",*mp);
		}
		mp++;
	    }
	}
	else printf("VESA BIOS extensions not found\n");
	return 0;
}


