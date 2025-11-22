/*
PL_MPEG Example - extract all frames of an mpg file and store as BMP
SPDX-License-Identifier: MIT

Dominic Szablewski - https://phoboslab.org


-- Usage

pl_mpeg_extract_frames <video-file.mpg>

Images named 000001.bmp, 000002.bmp... will be created in the current directory 


-- About

This program demonstrates how to extract all video frames from an MPEG-PS file. 
Frames are saved as BMP.

*/

#include <stdio.h>

#define PL_MPEG_IMPLEMENTATION
#include "pl_mpeg.h"

int write_bmp(const char *path, int width, int height, uint8_t *pixels) {
	FILE *fh = fopen(path, "wb");
	if (!fh) {
		return 0;
	}

	int padded_width = (width * 3 + 3) & (~3);
	int padding = padded_width - (width * 3);
	int data_size = padded_width * height;
	int file_size = 54 + data_size;

	fwrite("BM", 1, 2, fh);
	fwrite(&file_size, 1, 4, fh);
	fwrite("\x00\x00\x00\x00\x36\x00\x00\x00\x28\x00\x00\x00", 1, 12, fh);
	fwrite(&width, 1, 4, fh);
	fwrite(&height, 1, 4, fh);
	fwrite("\x01\x00\x18\x00\x00\x00\x00\x00", 1, 8, fh); // planes, bpp, compression
	fwrite(&data_size, 1, 4, fh);
	fwrite("\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 1, 16, fh);

	for (int y = height - 1; y >= 0; y--) {
		fwrite(pixels + y * width * 3, 3, width, fh);
		fwrite("\x00\x00\x00\x00", 1, padding, fh);
	}
	fclose(fh);
	return file_size;
}

int main(int argc, char *argv[]) {
	if (argc < 2) {
		printf("Usage: pl_mpeg_extract_frames <file.mpg>\n");
		return 1;
	}

	plm_t *plm = plm_create_with_filename(argv[1]);
	if (!plm) {
		printf("Couldn't open file %s\n", argv[1]);
		return 1;
	}

	plm_set_audio_enabled(plm, FALSE);
	
	int w = plm_get_width(plm);
	int h = plm_get_height(plm);
	uint8_t *pixels = (uint8_t *)malloc(w * h * 3);
	
	char bmp_name[16];
	plm_frame_t *frame = NULL;

	for (int i = 1; frame = plm_decode_video(plm); i++) {
		plm_frame_to_bgr(frame, pixels, w * 3); // BMP expects BGR ordering

		sprintf(bmp_name, "%06d.bmp", i);
		printf("Writing %s\n", bmp_name);
		write_bmp(bmp_name, w, h, pixels);
	}
	
    return 0;
}

