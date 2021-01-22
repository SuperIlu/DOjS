/**
 * original code from Steven Don
 * http://www.shdon.com/dos/gfx/api
 * 
 * It was modified to run with DOjS by Andre Seidelt <superilu@yahoo.com>.
 * Included with kind permission of Steven.
 */

/*
Kommen wir zu meinem Lieblingsfeature:
Wer um 1999/2000 herum auf einem PC gespielt hat der weiss vermutlich was das hier fuer eine Erweiterungskarte ist?

Es handelt sich dabei um eine 3dfx Voodoo1 3D Beschleunigerkarte, genaugenommen um eine Diamond Monster 3D.

3dfx war damals der Vorreiter wenn es um 3D Beschleunigung auf dem PC ging. Vor den Voodoo-Karten wurden die 3D Grafiken normalerweise auf der CPU berechnet
und die Spieleentwickler mussten sich immer zwischen Geschwindigkeit und Detailgrad entscheiden.

DOjS bringt eine direkten Zugriff auf die GLIDE Programmierschnittstelle fuer 3dfx Karten mit. Neben direktem Zugriff auf den Grafikspeicher sind Mechanismen wie
Texturen, Transparenz und Polygonrendering direkt ansprechbar.

Dank eines Treiber-Konzeptes ist die gesamte 3dfx Produkpalete verwendbar.

Das Ergebnis sehen wir hier: Dieser Wuerfel wird von einer Voodoo1 Karte in meinem Rechner gerendert und diese Textur wird zur Laufzeit dynamisch erzeugt und dargestellt.
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
	[0, 0, 0, 0, 0, 0, 0, 200, 0, 0],
	[0, 0, 0, 0, 0, 0, 0, 200, 0, 0],
	[0, 0, 0, 0, 0, 0, 0, 200, 0, 0],
	[0, 0, 0, 0, 0, 0, 0, 200, 0, 0],
];

var fallback = false;

// rotation states
var rotSteps = [
	[0, 0, 0],
	[0, Math.PI / 2, 0],
	[0, Math.PI, 0],
	[0, Math.PI / 2 * 3, 0],
	[3 * Math.PI / 2, 2 * Math.PI, Math.PI],
	[Math.PI / 2, 2 * Math.PI, 0],
];

var rotIdx = 0;					// current rotation
var txtFont;					// text rendering ont
var TEXSIZE = 256;				// texture size
var a2TextDest = 0;				// TMU memory destination for textures
var t1, t2, t3, t4, t5, a2;		// textures
var arcer2Bitmap;				// bitmaps

// arcer2 runtime data
var a2dat = {
	start: 0,
	r: TEXSIZE / 2,
	lastAI: null
};


var txt_3dfx = [
	"3dfx Interactive was a company",
	"headquartered in San Jose,",
	"California, founded in 1994,",
	"that specialized in the",
	"manufacturing of 3D graphics",
	"processing units and graphics",
	"cards. It was a pioneer in the",
	"field from the late 1990s",
	"until 2000.",
	"The company's flagship product",
	"was the Voodoo Graphics, an",
	"add-in card that accelerated",
	"3D graphics.",
];

var txt_cards = [
	"Card support:",
	"- Voodoo 1",
	"- Voodoo 2",
	"- Voodoo 3",
	"- Voodoo 4 [*]",
	"- Voodoo 5 [*]",
	"- Voodoo Rush",
	"- Voodoo Banshee",
	"",
	"[*] tested by @tunguska82"
];

var txt_features = [
	"Features:",
	"- Implements GLIDE3 API",
	"- Texture loading",
	"- Linear frame buffer access",
	"- Dynamic texture rendering",
	"- Uses DJGPP DXE mechanism",
	"- Only exposes Voodoo 1",
	"  functionality",
];

exports.prepare = function () {
	if (fxGetNumBoards() > 0) {
		fxInit();
		fxResetVertexLayout();
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
		txtFont = new Font(JSBOOTPATH + "fonts/cour14b.fnt");
		t1 = tex2tex("examples/dojs/m3d.3df", true);
		t2 = tex2tex("examples/dojs/3dfxlogo.3df", true);
		t3 = texttexture(txt_3dfx, Color(255, 128, 255));
		t4 = texttexture(txt_features, Color(128, 255, 255));
		t5 = texttexture(txt_cards, Color(255, 255, 128));

		arcer2Bitmap = new Bitmap(TEXSIZE, TEXSIZE);
		arcerInit();
	} else {
		Println("No 3dfx board found");
		fallback = true;
	}
}

function texttexture(t, c) {
	var bg = Color(32, 32, 32, 255);
	var render_bm = new Bitmap(TEXSIZE, TEXSIZE);
	SetRenderBitmap(render_bm);
	ClearScreen(bg);
	var yPos = 10;
	for (var l = 0; l < t.length; l++) {
		txtFont.DrawStringLeft(8, yPos, t[l], c, bg);
		yPos += txtFont.height + 2;
	}
	var tex = TexInfo(render_bm);
	SetRenderBitmap(null);
	tex.DownloadMipMap(GR_TMU.TMU0, FxTexMemGetStartAddress(GR_TMU.TMU0, tex), GR_MIPMAPLEVELMASK.BOTH);
	return tex;
}

function file2tex(tga, t) {
	var render_bm = new Bitmap(TEXSIZE, TEXSIZE);
	var file_bm = new Bitmap(tga);
	SetRenderBitmap(render_bm);
	ClearScreen(Color(32, 32, 32, 255));
	if (t) {
		file_bm.DrawTrans((render_bm.width - file_bm.width) / 2, (render_bm.height - file_bm.height) / 2);
	} else {
		file_bm.Draw((render_bm.width - file_bm.width) / 2, (render_bm.height - file_bm.height) / 2);
	}
	var tex = TexInfo(render_bm);
	SetRenderBitmap(null);
	tex.DownloadMipMap(GR_TMU.TMU0, FxTexMemGetStartAddress(GR_TMU.TMU0, tex), GR_MIPMAPLEVELMASK.BOTH);
	return tex;
}

function tex2tex(fname, t) {
	var tex = TexInfo(fname);
	tex.DownloadMipMap(GR_TMU.TMU0, FxTexMemGetStartAddress(GR_TMU.TMU0, tex), GR_MIPMAPLEVELMASK.BOTH);
	return tex;
}

function arcerInit() {
	// arcer 2
	a2dat.center = TEXSIZE / 2;
	a2dat.start = 0;
	a2dat.r = TEXSIZE / 2;
	a2dat.lastAI = null;
	SetRenderBitmap(arcer2Bitmap);
	ClearScreen(EGA.BLACK);
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

exports.present = function () {
	if (fallback) {
		background(0);
		textAlign(CENTER);
		textFont(fontHeader);
		fill(255);
		text("No Voodoo found", width / 2, height / 2);
	} else {
		arcer2Update();

		//Clear buffers
		fxBufferClear(0xFF101020, 0, 0);

		//Set coordinates to that of a cube
		var cubeSize = 70;
		y[0] = y[1] = y[2] = y[3] = -cubeSize;	// Top
		y[4] = y[5] = y[6] = y[7] = cubeSize;	// Bottom
		x[0] = x[3] = x[4] = x[7] = -cubeSize;	// Left
		x[1] = x[2] = x[5] = x[6] = cubeSize;	// Right
		z[0] = z[1] = z[4] = z[5] = -cubeSize;	// Back
		z[2] = z[3] = z[6] = z[7] = cubeSize;	// Front

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
		//Left
		t2.Source(GR_MIPMAPLEVELMASK.BOTH);
		DrawPoly(x[0], y[0], z[0], x[3], y[3], z[3],
			x[7], y[7], z[7], x[4], y[4], z[4]);
		//Right
		t4.Source(GR_MIPMAPLEVELMASK.BOTH);
		DrawPoly(x[2], y[2], z[2], x[1], y[1], z[1],
			x[5], y[5], z[5], x[6], y[6], z[6]);
		//Back
		t1.Source(GR_MIPMAPLEVELMASK.BOTH);
		DrawPoly(x[1], y[1], z[1], x[0], y[0], z[0],
			x[4], y[4], z[4], x[5], y[5], z[5]);
		//Front
		t3.Source(GR_MIPMAPLEVELMASK.BOTH);
		DrawPoly(x[3], y[3], z[3], x[2], y[2], z[2],
			x[6], y[6], z[6], x[7], y[7], z[7]);
		//The top and bottom using another texture

		//Top
		t5.Source(GR_MIPMAPLEVELMASK.BOTH);
		DrawPoly(x[0], y[0], z[0], x[1], y[1], z[1],
			x[2], y[2], z[2], x[3], y[3], z[3]);
		//Bottom
		a2.Source(GR_MIPMAPLEVELMASK.BOTH);
		DrawPoly(x[4], y[4], z[4], x[5], y[5], z[5],
			x[6], y[6], z[6], x[7], y[7], z[7]);

		// Set new angles
		if (xAngle < rotSteps[rotIdx][0]) {
			xAngle += 0.02;
		}
		if (xAngle > rotSteps[rotIdx][0]) {
			xAngle -= 0.02;
		}
		if (yAngle < rotSteps[rotIdx][1]) {
			yAngle += 0.02;
		}
		if (yAngle > rotSteps[rotIdx][1]) {
			yAngle -= 0.02;
		}
		if (zAngle < rotSteps[rotIdx][2]) {
			zAngle += 0.02;
		}
		if (zAngle > rotSteps[rotIdx][2]) {
			zAngle -= 0.02;
		}
	}
}

exports.keyHook = function (key) {
	if (key == KEY.Code.KEY_DOWN) {
		if (rotIdx < rotSteps.length - 1) {
			rotIdx++;
		}
	} else if (key == KEY.Code.KEY_UP) {
		if (rotIdx > 0) {
			rotIdx--;
		}
	}
};

exports.exitHook = function () {
	if (!fallback) {
		fxShutdown();
	}
};

/*
  This rotates point x,y,z around angles xa,ya,za
*/
function Rotate3D(x, y, z, xa, ya, za, idx) {
	var lx = x[idx], ly = y[idx], lz = z[idx];
	var xt, yt, zt;

	// Rotate around X-axis
	zt = (lz * Math.cos(xa) - ly * Math.sin(xa));
	yt = (lz * Math.sin(xa) + ly * Math.cos(xa));
	lz = zt; ly = yt;

	// Rotate around Z-axis
	xt = (lx * Math.cos(za) - ly * Math.sin(za));
	yt = (lx * Math.sin(za) + ly * Math.cos(za));
	lx = xt;

	// Rotate around Y-axis
	xt = (lx * Math.cos(ya) - lz * Math.sin(ya));
	zt = (lx * Math.sin(ya) + lz * Math.cos(ya));

	x[idx] = xt;
	y[idx] = yt;
	z[idx] = zt;
}

function DrawPoly(x1, y1, z1, x2, y2, z2, x3, y3, z3, x4, y4, z4) {
	var xCenter = 320;
	var yCenter = 240;

	// Top left vertex
	vtx[0][0] = x1 + xCenter;	// x coordinate. on screen
	vtx[0][1] = y1 + yCenter;	// y coordinate, on screen
	vtx[0][2] = 65535 / z1;		// one-over-z coordinate
	vtx[0][3] = 1 / z1;			// one-over-w "homogenous" coordinate
	vtx[0][8] = 255 / z1;		// texture coordinates, being 0, 0 here
	vtx[0][9] = 0 / z1;

	// Top right vertex
	vtx[1][0] = x2 + xCenter;
	vtx[1][1] = y2 + yCenter;
	vtx[1][2] = 65535 / z2;
	vtx[1][3] = 1 / z2;
	vtx[1][8] = 0 / z2;
	vtx[1][9] = 0;

	// Bottom right vertex
	vtx[2][0] = x3 + xCenter;
	vtx[2][1] = y3 + yCenter;
	vtx[2][2] = 65535 / z3;
	vtx[2][3] = 1 / z3;
	vtx[2][8] = 0 / z3;
	vtx[2][9] = 255 / z3;

	// Bottom left vertex
	vtx[3][0] = x4 + xCenter;
	vtx[3][1] = y4 + yCenter;
	vtx[3][2] = 65535 / z4;
	vtx[3][3] = 1 / z4;
	vtx[3][8] = 255 / z4;
	vtx[3][9] = 255 / z4;

	// Draw 2 triangles to get a four-sided polygon
	fxDrawTriangle(vtx[0], vtx[1], vtx[2]);
	fxDrawTriangle(vtx[0], vtx[2], vtx[3]);
}
