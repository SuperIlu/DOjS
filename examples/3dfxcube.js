/**
 * original code from Steven Don
 * http://www.shdon.com/dos/gfx/api
 * 
 * It was modified to run with DOjS by Andre Seidelt <superilu@yahoo.com>.
 * Included with kind permission of Steven.
 */

//Rotational angles for the rotating cube
var xAngle = 0, yAngle = 0, zAngle = 0;

//8 vertices
var x = [0, 0, 0, 0, 0, 0, 0, 0],
	y = [0, 0, 0, 0, 0, 0, 0, 0],
	z = [0, 0, 0, 0, 0, 0, 0, 0];

//   0, 1, 2, 3, 4, 5, 6, 7, 8, 9
//   X, Y, Z, Q, R, G, B, A, S, T
var vtx = [
	[0, 0, 0, 0, 0, 0, 0, 128, 0, 0],
	[0, 0, 0, 0, 0, 0, 0, 128, 0, 0],
	[0, 0, 0, 0, 0, 0, 0, 128, 0, 0],
	[0, 0, 0, 0, 0, 0, 0, 128, 0, 0]
];


var a2, a3, b;								// textures
var arcer2Bitmap, arcer3Bitmap, bBitmap;	// bitmaps

// TMU memory destination for textures
var a2TextDest = 0;
var a3TextDest = 0;
var bTextDest = 0;

var TEXSIZE = 256;	// texture size

// arcer2 runtime data
var a2dat = {
	start: 0,
	r: TEXSIZE / 2,
	lastAI: null
};

// arcer2 runtime data
var a3dat = {
	center: 0,
	arcs: null
};

function Setup() {
	fxInit();
	fxVertexLayout([GR_PARAM.XY, GR_PARAM.Z, GR_PARAM.Q, GR_PARAM.RGB, GR_PARAM.A, GR_PARAM.ST0]);

	//Set up render state
	fxTexCombine(
		GR_TMU.TMU0,
		GR_COMBINE_FUNCTION.LOCAL,
		GR_COMBINE_FACTOR.NONE,
		GR_COMBINE_FUNCTION.LOCAL,
		GR_COMBINE_FACTOR.NONE,
		false,
		false);
	fxColorCombine(
		GR_COMBINE_FUNCTION.SCALE_OTHER,
		GR_COMBINE_FACTOR.ONE,
		GR_COMBINE_LOCAL.NONE,
		GR_COMBINE_OTHER.TEXTURE,
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
		GR_BLEND.ONE,
		GR_BLEND.ZERO);
	fxAlphaTestFunction(GR_CMP.ALWAYS);

	//Enable z buffering
	fxDepthBufferMode(GR_DEPTHBUFFER.ZBUFFER);
	fxDepthBufferFunction(GR_CMP.GREATER);
	fxDepthMask(true);

	//Enable bilinear filtering, no mipmapping
	fxTexFilterMode(
		GR_TMU.TMU0,
		GR_TEXTUREFILTER.BILINEAR,
		GR_TEXTUREFILTER.BILINEAR);
	fxTexMipMapMode(
		GR_TMU.TMU0,
		GR_MIPMAP.NEAREST,
		false);

	//Enable fogging to black
	fxFogColorValue(0xff000000);
	var fTable = fxFogGenerateExp(.005);
	fxFogMode(GR_FOG.WITH_TABLE_ON_FOGCOORD_EXT);
	fxFogTable(fTable);

	//Load the textures
	FxTexMemInit(GR_TMU.TMU0);

	arcer2Bitmap = new Bitmap(TEXSIZE, TEXSIZE);
	arcer3Bitmap = new Bitmap(TEXSIZE, TEXSIZE);
	bBitmap = new Bitmap(TEXSIZE, TEXSIZE);
	arcerInit();
}

function arcerInit() {
	// arcer 3
	a3dat.center = TEXSIZE / 2;
	a3dat.arcs = [];
	var steps = TEXSIZE / 20;
	var maxSize = Math.sqrt(TEXSIZE * TEXSIZE + TEXSIZE * TEXSIZE) / 2;
	for (var i = 0; i < maxSize; i += steps) {
		a3dat.arcs.push({
			start: RandomInt(0, 3600),
			length: RandomInt(0, 3600),
			col: HSBColor(RandomInt(0, 255), 255, 255),
			r: i
		});
	}

	// arcer 2
	a2dat.center = TEXSIZE / 2;
	a2dat.start = 0;
	a2dat.r = TEXSIZE / 2;
	a2dat.lastAI = null;
	SetRenderBitmap(arcer2Bitmap);
	ClearScreen(EGA.BLACK);
	SetRenderBitmap(null);
}

function arcer3Update() {
	SetRenderBitmap(arcer3Bitmap);
	ClearScreen(EGA.BLACK);
	for (var i = 0; i < a3dat.arcs.length; i++) {
		var ca = a3dat.arcs[i];
		var ai = CircleArc(a3dat.center, a3dat.center, ca.r, ca.start, ca.start + ca.length, ca.col);
		if (i % 2) {
			ca.start++;
			if (ca.start > 3600) {
				ca.start = 0;
			}
		} else {
			ca.start--;
			if (ca.start < 0) {
				ca.start = 3600;
			}
		}
	}
	a3 = new TexInfo(arcer3Bitmap);
	// allocate texture memory only once
	if (a3TextDest == 0) {
		a3TextDest = FxTexMemGetStartAddress(GR_TMU.TMU0, a3);
	}
	a3.DownloadMipMap(GR_TMU.TMU0, a3TextDest, GR_MIPMAPLEVELMASK.BOTH);
	SetRenderBitmap(null);
}

function arcer2Update() {
	SetRenderBitmap(arcer2Bitmap);
	var size = RandomInt(0, 255);
	var c = RandomInt(0, 255);
	var cCol = HSBColor(c, 255, 255);
	var lCol = HSBColor(c, 128, 255);

	var ai = CircleArc(a2dat.center, a2dat.center, a2dat.r, a2dat.start, a2dat.start + size, cCol);
	if (a2dat.lastAI) {
		Line(a2dat.lastAI.endX, a2dat.lastAI.endY, ai.startX, ai.startY, lCol);
	}

	a2dat.start += size;
	if (a2dat.start > 255) {
		a2dat.start -= 255;
	}
	a2dat.lastAI = ai;
	a2dat.r -= 5;
	if (a2dat.r <= 0) {
		ClearScreen(EGA.BLACK);
		a2dat.r = TEXSIZE / 2;
		a2dat.lastAI = null;
	}
	a2 = new TexInfo(arcer2Bitmap);
	// allocate texture memory only once
	if (a2TextDest == 0) {
		a2TextDest = FxTexMemGetStartAddress(GR_TMU.TMU0, a2);
	}
	a2.DownloadMipMap(GR_TMU.TMU0, a2TextDest, GR_MIPMAPLEVELMASK.BOTH);
	SetRenderBitmap(null);
}

function boxlineUpdate() {
	var m = TEXSIZE / 2;
	SetRenderBitmap(bBitmap);
	FilledBox(0, 0, TEXSIZE, TEXSIZE, Color(0, 0, 0, 8));

	//fill(random(32, 224));

	var h = RandomInt(5, 40);
	var w = RandomInt(5, 110);
	var p = RandomInt(0, TEXSIZE - h / 2);

	Box(m - w, p, m + w, p + h, EGA.WHITE);
	Line(m, 0, m, TEXSIZE, EGA.WHITE);

	b = new TexInfo(bBitmap);
	// allocate texture memory only once
	if (bTextDest == 0) {
		bTextDest = FxTexMemGetStartAddress(GR_TMU.TMU0, b);
	}
	b.DownloadMipMap(GR_TMU.TMU0, bTextDest, GR_MIPMAPLEVELMASK.BOTH);
	SetRenderBitmap(null);
}

function Loop() {
	arcer2Update();
	arcer3Update();
	boxlineUpdate();

	//Clear buffers
	fxBufferClear(0xFF101020, 0, 0);

	//Set coordinates to that of a cube
	y[0] = y[1] = y[2] = y[3] = -50; //Top
	y[4] = y[5] = y[6] = y[7] = 50; //Bottom
	x[0] = x[3] = x[4] = x[7] = -50; //Left
	x[1] = x[2] = x[5] = x[6] = 50; //Right
	z[0] = z[1] = z[4] = z[5] = -50; //Back
	z[2] = z[3] = z[6] = z[7] = 50; //Front

	//Rotate over the angles
	for (var i = 0; i < 8; i++)
		Rotate3D(x, y, z, xAngle, yAngle, zAngle, i);

	//Perform the perspective transformation
	for (var i = 0; i < 8; i++) {
		z[i] += 200;
		x[i] *= 400; x[i] /= z[i];
		y[i] *= 400; y[i] /= z[i];
	}

	//Draw the cube, the sides use the brick texture
	a2.Source(GR_MIPMAPLEVELMASK.BOTH)
	//Left
	DrawPoly(x[0], y[0], z[0], x[3], y[3], z[3],
		x[7], y[7], z[7], x[4], y[4], z[4]);
	//Right
	DrawPoly(x[2], y[2], z[2], x[1], y[1], z[1],
		x[5], y[5], z[5], x[6], y[6], z[6]);
	a3.Source(GR_MIPMAPLEVELMASK.BOTH)
	//Back
	DrawPoly(x[1], y[1], z[1], x[0], y[0], z[0],
		x[4], y[4], z[4], x[5], y[5], z[5]);
	//Front
	DrawPoly(x[3], y[3], z[3], x[2], y[2], z[2],
		x[6], y[6], z[6], x[7], y[7], z[7]);
	//The top and bottom using another texture

	b.Source(GR_MIPMAPLEVELMASK.BOTH)
	//Top
	DrawPoly(x[0], y[0], z[0], x[1], y[1], z[1],
		x[2], y[2], z[2], x[3], y[3], z[3]);
	//Bottom
	DrawPoly(x[4], y[4], z[4], x[5], y[5], z[5],
		x[6], y[6], z[6], x[7], y[7], z[7]);

	//Set new angles
	xAngle += 0.01; yAngle += 0.02; zAngle += 0.03;
}

var idx = 0;

function Input(e) {
	if (CompareKey(e.key, 's')) {
		var img = new Bitmap(0, 0, 640, 480, GR_BUFFER.FRONTBUFFER);
		img.SaveBmpImage("3dcube" + idx + ".bmp");
		idx++;
	}
}

/*
  This rotates point x,y,z around angles xa,ya,za
*/
function Rotate3D(x, y, z, xa, ya, za, idx) {
	var lx = x[idx], ly = y[idx], lz = z[idx];
	var xt, yt, zt;

	//Rotate around X-axis
	zt = (lz * Math.cos(xa) - ly * Math.sin(xa));
	yt = (lz * Math.sin(xa) + ly * Math.cos(xa));
	lz = zt; ly = yt;

	//Rotate around Z-axis
	xt = (lx * Math.cos(za) - ly * Math.sin(za));
	yt = (lx * Math.sin(za) + ly * Math.cos(za));
	lx = xt;

	//Rotate around Y-axis
	xt = (lx * Math.cos(ya) - lz * Math.sin(ya));
	zt = (lx * Math.sin(ya) + lz * Math.cos(ya));

	x[idx] = xt;
	y[idx] = yt;
	z[idx] = zt;
}

function DrawPoly(x1, y1, z1, x2, y2, z2, x3, y3, z3, x4, y4, z4) {
	//Convert coordinates to vertices

	//Top left vertex
	vtx[0][0] = x1 + 320.0;		//x coordinate. on screen
	vtx[0][1] = y1 + 240.0;		//y coordinate, on screen
	vtx[0][2] = 65535.0 / z1;	//one-over-z coordinate
	vtx[0][3] = 1.0 / z1;		//one-over-w "homogenous" coordinate
	vtx[0][8] = 0.0;			//texture coordinates, being 0, 0 here
	vtx[0][9] = 0.0;

	//Top right vertex
	vtx[1][0] = x2 + 320.0;
	vtx[1][1] = y2 + 240.0;
	vtx[1][2] = 65535.0 / z2;
	vtx[1][3] = 1.0 / z2;
	vtx[1][8] = 255.0 / z2;
	vtx[1][9] = 0.0;

	//Bottom right vertex
	vtx[2][0] = x3 + 320.0;
	vtx[2][1] = y3 + 240.0;
	vtx[2][2] = 65535.0 / z3;
	vtx[2][3] = 1.0 / z3;
	vtx[2][8] = 255.0 / z3;
	vtx[2][9] = 255.0 / z3;

	//Bottom left vertex
	vtx[3][0] = x4 + 320.0;
	vtx[3][1] = y4 + 240.0;
	vtx[3][2] = 65535.0 / z4;
	vtx[3][3] = 1.0 / z4;
	vtx[3][8] = 0.0;
	vtx[3][9] = 255.0 / z4;

	//Draw 2 triangles to get a four-sided polygon
	fxDrawTriangle(vtx[0], vtx[1], vtx[2]);
	fxDrawTriangle(vtx[0], vtx[2], vtx[3]);
}
