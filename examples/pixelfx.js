/*
MIT License

Copyright (c) 2019-2021 Andre Seidelt <superilu@yahoo.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
Include('examples/3dfx_tst.js');

var srcVerts = [
	[0, 0, 0, 0, 0, 0],	// x, y, z, w, s, t
	[0, 0, 0, 0, 0, 0],
	[0, 0, 0, 0, 0, 0],
	[0, 0, 0, 0, 0, 0]
];

var RECT_SIZE = 0.1;
var WOBBLE = 0.1;
var SPACING = 0.25;

var bgImg;

function Setup() {
	bgImg = new Bitmap("examples/3dfx.tga");

	fxInit();
	fxOrigin(GR_ORIGIN.LOWER_LEFT);
	fxVertexLayout([GR_PARAM.XY, GR_PARAM.RGB, GR_PARAM.A]);
	createVertices();

	fxColorCombine(
		GR_COMBINE_FUNCTION.LOCAL,
		GR_COMBINE_FACTOR.NONE,
		GR_COMBINE_LOCAL.ITERATED,
		GR_COMBINE_OTHER.NONE,
		false);
	fxAlphaCombine(
		GR_COMBINE_FUNCTION.LOCAL,
		GR_COMBINE_FACTOR.NONE,
		GR_COMBINE_LOCAL.ITERATED,
		GR_COMBINE_OTHER.NONE,
		false);
	fxAlphaBlendFunction(
		GR_BLEND.SRC_ALPHA,
		GR_BLEND.ONE_MINUS_SRC_ALPHA,
		GR_BLEND.ZERO,
		GR_BLEND.ZERO);

	/* Initialize Source 3D data - Centered Rectangle on X/Y Plane 
	w, s and t are unused because we have no texture
   0--1  Z+
   |  |  |
   2--3   - X+

 */
	srcVerts[0][0] = -RECT_SIZE; srcVerts[0][1] = -RECT_SIZE; srcVerts[0][2] = 0; srcVerts[0][3] = 1.0;
	srcVerts[1][0] = -RECT_SIZE; srcVerts[1][1] = RECT_SIZE; srcVerts[1][2] = 0; srcVerts[1][3] = 1.0;
	srcVerts[2][0] = RECT_SIZE; srcVerts[2][1] = -RECT_SIZE; srcVerts[2][2] = 0; srcVerts[2][3] = 1.0;
	srcVerts[3][0] = RECT_SIZE; srcVerts[3][1] = RECT_SIZE; srcVerts[3][2] = 0; srcVerts[3][3] = 1.0;

	wob = 0;

	// initialize vertex colors
	v1[2] = 0xFF; // R
	v1[3] = 0x00; // G
	v1[4] = 0x00; // B
	v1[5] = 0x00; // A

	v2[2] = 0x00; // R
	v2[3] = 0xFF; // G
	v2[4] = 0x00; // B
	v2[5] = 0xFF; // A

	v3[2] = 0x00; // R
	v3[3] = 0x00; // G
	v3[4] = 0xFF; // B
	v3[5] = 0xFF; // A

	v4[2] = 0xFF; // R
	v4[3] = 0xFF; // G
	v4[4] = 0xFF; // B
	v4[5] = 0xFF; // A
}

function Loop() {
	fxBufferClear(FX_BLACK, 0, 0);
	//bgImg.FxDrawLfb(0, 0, GR_BUFFER.BACKBUFFER, false);

	for (var ty = 1; ty < 8; ty++) {
		for (var tx = 1; tx < 8; tx++) {
			SetMatrix(Identity());
			MultMatrix(Translation(-1.125 + SPACING * tx, 1 + SPACING * ty, 2 + Math.sin(wob + 0.1 * tx + 0.1 * ty)));
			MultMatrix(XRotation(60));

			var xfVerts = TransformVertices(srcVerts);
			var prjVerts = ProjectVertices(xfVerts);

			v1[0] = scaleX(prjVerts[0][0]);
			v1[1] = scaleY(prjVerts[0][1]);

			v2[0] = scaleX(prjVerts[1][0]);
			v2[1] = scaleY(prjVerts[1][1]);

			v3[0] = scaleX(prjVerts[2][0]);
			v3[1] = scaleY(prjVerts[2][1]);

			v4[0] = scaleX(prjVerts[3][0]);
			v4[1] = scaleY(prjVerts[3][1]);

			fxDrawTriangle(v1, v2, v4);
			fxDrawTriangle(v1, v4, v3);
		}
		PopMatrix();
	}

	wob += WOBBLE;
}

function Input(e) {
	if (CompareKey(e.key, 'm')) {
		mipMapMode++;
		mipMapMode %= 3;
	}
}
