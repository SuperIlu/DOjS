/*
PL_MPEG Example - extract all frames of an mpg file and store as PNG

Dominic Szablewski - https://phoboslab.org


-- LICENSE: The MIT License(MIT)

Copyright(c) 2019 Dominic Szablewski

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files(the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and / or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions :
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.


-- Usage

pl_mpeg_extract_frames <video-file.mpg>


-- About

This program demonstrates how to extract all video frames from an MPEG-PS file. 
Frames are saved as PNG via stb_image_write: https://github.com/nothings/stb

*/

#include <stdio.h>

#define PL_MPEG_IMPLEMENTATION
#include "pl_mpeg.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

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
	uint8_t *rgb_buffer = (uint8_t *)malloc(w * h * 3);
	
	char png_name[16];
	plm_frame_t *frame = NULL;

	for (int i = 1; frame = plm_decode_video(plm); i++) {
		plm_frame_to_rgb(frame, rgb_buffer, w * 3);

		sprintf(png_name, "%06d.png", i);
		printf("Writing %s\n", png_name);
		stbi_write_png(png_name, w, h, 3, rgb_buffer, w * 3);
	}
	
    return 0;
}

