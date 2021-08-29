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
LoadLibrary("png");
Include('examples/3dfx_tst.js');

var zrange;

var tx;

var MAX_DIST = 2.5;
var MIN_DIST = 1.0;

var distance = 1.0;
var dDelta = 0.01;

var RECT_SIZE = 0.2;
var WOBBLE = 0.1;
var SPACING = 0.5;


var srcVerts = [
	[0, 0, 0, 0, 0, 0],	// x, y, z, w, s, t
	[0, 0, 0, 0, 0, 0],
	[0, 0, 0, 0, 0, 0],
	[0, 0, 0, 0, 0, 0]
];

var rose;
var alpha = 200;
var TEXSIZE = 256;
var blendModes = [
	BLEND.REPLACE,
	BLEND.ALPHA,
	BLEND.ADD,
	BLEND.DARKEST,
	BLEND.LIGHTEST,
	BLEND.DIFFERENCE,
	BLEND.EXCLUSION,
	BLEND.MULTIPLY,
	BLEND.SCREEN,
	BLEND.OVERLAY,
	BLEND.HARD_LIGHT,
	BLEND.DOGE,
	BLEND.BURN,
];

function createTexture(bmode) {
	var bm = new Bitmap(TEXSIZE, TEXSIZE);
	SetRenderBitmap(bm);
	ClearScreen(EGA.BLACK);

	TransparencyEnabled(BLEND.REPLACE);
	rose.Draw(0, 0);

	TransparencyEnabled(bmode);
	FilledCircle(TEXSIZE / 2, TEXSIZE / 3, TEXSIZE / 4, Color(255, 0, 0, alpha));
	FilledCircle(TEXSIZE / 3, (TEXSIZE / 3) * 2, TEXSIZE / 4, Color(0, 255, 0, alpha));
	FilledCircle((TEXSIZE / 3) * 2, (TEXSIZE / 3) * 2, TEXSIZE / 4, Color(0, 0, 255, alpha));

	var ti = new TexInfo(bm);
	var txDest = FxTexMemGetStartAddress(GR_TMU.TMU0, ti);
	ti.DownloadMipMap(GR_TMU.TMU0, txDest, GR_MIPMAPLEVELMASK.BOTH);
	SetRenderBitmap(null);

	return ti;
}

function Setup() {
	rose = new Bitmap("examples/rose.png");

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

	/* Load texture data into system ram */
	FxTexMemInit(GR_TMU.TMU0);
	tx = [];
	for (var i = 0; i < blendModes.length; i++) {
		tx.push(createTexture(blendModes[i]));
	}

	zrange = fxGetZDepthMinMax();

	/* Initialize Source 3D data - Rectangle on X/Z Plane 
	   Centered about Y Axis

	   0--1  Z+
	   |  |  |
	   2--3   - X+

	 */
	srcVerts[0][0] = -RECT_SIZE; srcVerts[0][1] = -RECT_SIZE; srcVerts[0][2] = 0; srcVerts[0][3] = 1.0;
	srcVerts[1][0] = -RECT_SIZE; srcVerts[1][1] = RECT_SIZE; srcVerts[1][2] = 0; srcVerts[1][3] = 1.0;
	srcVerts[2][0] = RECT_SIZE; srcVerts[2][1] = -RECT_SIZE; srcVerts[2][2] = 0; srcVerts[2][3] = 1.0;
	srcVerts[3][0] = RECT_SIZE; srcVerts[3][1] = RECT_SIZE; srcVerts[3][2] = 0; srcVerts[3][3] = 1.0;

	srcVerts[0][4] = 0.0; srcVerts[0][5] = 0.0;
	srcVerts[1][4] = 1.0; srcVerts[1][5] = 0.0;
	srcVerts[2][4] = 0.0; srcVerts[2][5] = 1.0;
	srcVerts[3][4] = 1.0; srcVerts[3][5] = 1.0;

	wob = 0;
}

function Loop() {
	fxBufferClear(0x00404040, 0, zrange[1]);

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

	var txIdx = 0;
	for (y = 0; y < 4; y++) {
		for (x = 0; x < 4; x++) {
			if (txIdx < tx.length) {
				SetMatrix(Identity());
				MultMatrix(Translation(-0.75 + SPACING * x, -0.75 + SPACING * y, 2 + Math.sin(wob + 0.1 * x + 0.1 * y)));
				//MultMatrix(XRotation(50));

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

				/* Select Texture */
				tx[txIdx].Source(GR_MIPMAPLEVELMASK.BOTH);
				txIdx++;

				fxDrawTriangle(v1, v2, v4);
				fxDrawTriangle(v1, v4, v3);
			}
		}
	}
	wob += WOBBLE;
}

function Input(e) {
}
