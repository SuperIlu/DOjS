/*
MIT License

Copyright (c) 2019-2020 Andre Seidelt <superilu@yahoo.com>

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
Include('tests/3dfx_tst.js');

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
	//	bgImg.FxDrawLfb(0, 0, GR_BUFFER.BACKBUFFER, false);

	for (var ty = 1; ty < 8; ty++) {
		for (var tx = 1; tx < 8; tx++) {
			SetMatrix(Identity());
			MultMatrix(Translation(-1.125 + SPACING * tx, -1.125 + SPACING * ty, 2 + Math.sin(wob + 0.1 * tx + 0.1 * ty)));

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

/*
** This function is called once when the script is started.
*/
function AASetup() {
	fxInit();
	fxVertexLayout([GR_PARAM.XY, GR_PARAM.A]);
	createVertices();

	fxColorCombine(
		GR_COMBINE_FUNCTION.LOCAL,
		GR_COMBINE_FACTOR.NONE,
		GR_COMBINE_LOCAL.CONSTANT,
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
}

/*
** This function is repeatedly until ESC is pressed or Stop() is called.
*/
function AALoop() {
	fxBufferClear(FX_BLACK, 0, 0);

	v1[0] = scaleX(0.5); v1[1] = scaleY(0.1);
	v2[0] = scaleX(0.8); v2[1] = scaleY(0.9);
	v3[0] = scaleX(0.2); v3[1] = scaleY(0.9);
	v1[2] = v2[2] = v3[2] = 0xFF;

	fxConstantColorValue(FX_RED);
	fxDrawTriangle(v1, v2, v3);

	fxOrigin(GR_ORIGIN.LOWER_LEFT);

	v1[2] = 0x00;
	fxConstantColorValue(FX_BLUE);
	fxDrawTriangle(v1, v2, v3);

	fxOrigin(GR_ORIGIN.UPPER_LEFT);
}

var zrange;

var texture;

var DISABLE = 0, NEAREST = 1, TRILINEAR = 2;
var MAX_DIST = 2.5;
var MIN_DIST = 1.0;

var mipMapMode = DISABLE;
var distance = 1.0;
var dDelta = 0.01;


/*
** This function is called once when the script is started.
*/
function BBSetup() {
	fxInit();
	fxOrigin(GR_ORIGIN.LOWER_LEFT);
	fxVertexLayout([GR_PARAM.XY, GR_PARAM.Q, GR_PARAM.ST0]);
	createVertices();

	fxColorCombine(
		GR_COMBINE_FUNCTION.SCALE_OTHER,
		GR_COMBINE_FACTOR.ONE,
		GR_COMBINE_LOCAL.NONE,
		GR_COMBINE_OTHER.TEXTURE,
		false);
	fxTexFilterMode(GR_TMU.TMU0, GR_TEXTUREFILTER.BILINEAR, GR_TEXTUREFILTER.BILINEAR);

	/* Load texture data into system ram */
	FxTexMemInit(GR_TMU.TMU0);
	texture = new TexInfo("tests/decal1.3df");
	/* Download texture data to TMU */
	texture.DownloadMipMap(GR_TMU.TMU0, FxTexMemGetStartAddress(GR_TMU.TMU0, texture), GR_MIPMAPLEVELMASK.BOTH);
	/* Select Texture As Source of all texturing operations */
	texture.Source(GR_MIPMAPLEVELMASK.BOTH);

	zrange = fxGetZDepthMinMax();

	/* Initialize Source 3D data - Rectangle on X/Z Plane 
	   Centered about Y Axis

	   0--1  Z+
	   |  |  |
	   2--3   - X+

	 */
	srcVerts[0][0] = -0.5; srcVerts[0][1] = 0.0; srcVerts[0][2] = 0.5; srcVerts[0][3] = 1.0;
	srcVerts[1][0] = 0.5; srcVerts[1][1] = 0.0; srcVerts[1][2] = 0.5; srcVerts[1][3] = 1.0;
	srcVerts[2][0] = -0.5; srcVerts[2][1] = 0.0; srcVerts[2][2] = -0.5; srcVerts[2][3] = 1.0;
	srcVerts[3][0] = 0.5; srcVerts[3][1] = 0.0; srcVerts[3][2] = -0.5; srcVerts[3][3] = 1.0;

	srcVerts[0][4] = 0.0; srcVerts[0][5] = 0.0;
	srcVerts[1][4] = 1.0; srcVerts[1][5] = 0.0;
	srcVerts[2][4] = 0.0; srcVerts[2][5] = 1.0;
	srcVerts[3][4] = 1.0; srcVerts[3][5] = 1.0;
}

/*
** This function is repeatedly until ESC is pressed or Stop() is called.
*/
function BBLoop() {
	fxBufferClear(0x00404040, 0, zrange[1]);

	switch (mipMapMode) {
		case DISABLE:
			fxTexMipMapMode(
				GR_TMU.TMU0,
				GR_MIPMAP.DISABLE,
				false);
			fxTexCombine(
				GR_TMU.TMU0,
				GR_COMBINE_FUNCTION.LOCAL,
				GR_COMBINE_FACTOR.NONE,
				GR_COMBINE_FUNCTION.LOCAL,
				GR_COMBINE_FACTOR.NONE,
				false, false);
			break;
		case NEAREST:
			fxTexMipMapMode(
				GR_TMU.TMU0,
				GR_MIPMAP.NEAREST,
				false);
			fxTexCombine(
				GR_TMU.TMU0,
				GR_COMBINE_FUNCTION.LOCAL,
				GR_COMBINE_FACTOR.NONE,
				GR_COMBINE_FUNCTION.LOCAL,
				GR_COMBINE_FACTOR.NONE,
				false, false);
			break;
		case TRILINEAR:
			fxTexMipMapMode(
				GR_TMU.TMU0,
				GR_MIPMAP.NEAREST,
				true);
			fxTexCombine(
				GR_TMU.TMU0,
				GR_COMBINE_FUNCTION.BLEND_LOCAL,
				GR_COMBINE_FACTOR.LOD_FRACTION,
				GR_COMBINE_FUNCTION.BLEND_LOCAL,
				GR_COMBINE_FACTOR.LOD_FRACTION,
				false, false);
			break;
	}

	/*---- 
	  A-B
	  |\|
	  C-D
	  -----*/
	v1[2] = 1.0;
	v2[2] = 1.0;
	v3[2] = 1.0;
	v4[2] = 1.0;

	distance += dDelta;
	if (distance > MAX_DIST ||
		distance < MIN_DIST) {
		dDelta *= -1.0;
		distance += dDelta;
	}

	SetMatrix(Identity());
	MultMatrix(XRotation(-20.0));
	MultMatrix(Translation(0.0, -0.3, distance));

	var xfVerts = TransformVertices(srcVerts);
	var prjVerts = ProjectVertices(xfVerts);

	v1[0] = scaleX(prjVerts[0][0]);
	v1[1] = scaleY(prjVerts[0][1]);
	v1[2] = 1.0 / prjVerts[0][3];

	v2[0] = scaleX(prjVerts[1][0]);
	v2[1] = scaleY(prjVerts[1][1]);
	v2[2] = 1.0 / prjVerts[1][3];

	v3[0] = scaleX(prjVerts[2][0]);
	v3[1] = scaleY(prjVerts[2][1]);
	v3[2] = 1.0 / prjVerts[2][3];

	v4[0] = scaleX(prjVerts[3][0]);
	v4[1] = scaleY(prjVerts[3][1]);
	v4[2] = 1.0 / prjVerts[3][3];

	v1[3] = prjVerts[0][4] * 255.0 * v1[2];
	v1[4] = prjVerts[0][5] * 255.0 * v1[2];

	v2[3] = prjVerts[1][4] * 255.0 * v2[2];
	v2[4] = prjVerts[1][5] * 255.0 * v2[2];

	v3[3] = prjVerts[2][4] * 255.0 * v3[2];
	v3[4] = prjVerts[2][5] * 255.0 * v3[2];

	v4[3] = prjVerts[3][4] * 255.0 * v4[2];
	v4[4] = prjVerts[3][5] * 255.0 * v4[2];
	fxDrawTriangle(v1, v2, v4);
	fxDrawTriangle(v1, v4, v3);

	if (mipMapMode == TRILINEAR) {
		fxAlphaBlendFunction(
			GR_BLEND.ONE,
			GR_BLEND.ONE,
			GR_BLEND.ZERO,
			GR_BLEND.ZERO);
		fxTexCombine(
			GR_TMU.TMU0,
			GR_COMBINE_FUNCTION.BLEND_LOCAL,
			GR_COMBINE_FACTOR.ONE_MINUS_LOD_FRACTION,
			GR_COMBINE_FUNCTION.BLEND_LOCAL,
			GR_COMBINE_FACTOR.ONE_MINUS_LOD_FRACTION,
			false, false);
		fxDrawTriangle(v1, v2, v4);
		fxDrawTriangle(v1, v4, v3);
		fxAlphaBlendFunction(
			GR_BLEND.ONE,
			GR_BLEND.ZERO,
			GR_BLEND.ZERO,
			GR_BLEND.ZERO);
	}
}

function Input(e) {
	if (CompareKey(e.key, 'm')) {
		mipMapMode++;
		mipMapMode %= 3;
	}
}
