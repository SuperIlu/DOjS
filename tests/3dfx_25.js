Include('tests/3dfx_tst.js');
Println("anti-aliased triangle test");

// TODO: texturing triangle does not work at all

var wrange;

var mode = true;
var angle = 0;

var lines = false;            /* Draw lines instead of triangles */
var blend = false;            /* Blend the triangle over the background */
var texturing = false;        /* Texture the tiangle */
var antialias = true;         /* Antialias the triangle? */
var bilinear = true;          /* Perform bilinear filtering on the texture? */
var render = true;            /* Draw? */
var backbuffer = true;        /* Draw to backbuffer? */
var background = true;        /* Draw background? */


var localVerts = [{}, {}, {}];
var texVerts = [];

var scrWidth = 640, scrHeight = 480;
var minColor = 10;            /* Vertex min color */
var maxColor = 245;           /* Vertex max color */
var alpha = 192.0;             /* Alpha for blending tringle over background */
var y_angle = 0.0;             /* rotation amount */

var ccFnc = GR_COLORCOMBINE.ITRGB; /* Start of w/ Gouraud shading */


var VERT_COUNT = 3;

/*
** This function is called once when the script is started.
*/
function Setup() {
	fxInit();
	fxOrigin(GR_ORIGIN.LOWER_LEFT);
	fxVertexLayout([GR_PARAM.XY, GR_PARAM.RGB, GR_PARAM.A]);
	createVertices();

	wrange = fxGetWDepthMinMax();

	localVerts[0].x = 0;
	localVerts[0].y = 0.75;
	localVerts[0].z = 0.0;
	localVerts[0].r = maxColor;
	localVerts[0].g = minColor;
	localVerts[0].b = minColor;
	localVerts[0].a = 255;
	localVerts[0].sow = 255;
	localVerts[0].tow = 255;
	localVerts[0].oow = 1;

	localVerts[1].x = -0.75;
	localVerts[1].y = -0.75;
	localVerts[1].z = 0.0;
	localVerts[1].sow = 0;
	localVerts[1].tow = 255;
	localVerts[1].oow = 1;
	localVerts[1].r = minColor;
	localVerts[1].g = maxColor;
	localVerts[1].b = minColor;
	localVerts[1].a = 255;

	localVerts[2].x = 0.75;
	localVerts[2].y = -0.75;
	localVerts[2].z = 0.0;
	localVerts[2].sow = 255;
	localVerts[2].tow = 0;
	localVerts[2].oow = 1;
	localVerts[2].r = minColor;
	localVerts[2].g = minColor;
	localVerts[2].b = maxColor;
	localVerts[2].a = 255;

	texVerts.push([0, 0, 255, 1, 0, 255]);
	texVerts.push([scrWidth, 0, 255, 1, 255, 255]);
	texVerts.push([scrWidth, scrHeight, 255, 1, 255, 0]);
	texVerts.push([0, scrHeight, 255, 1, 0, 0]);

	FxTexMemInit(GR_TMU.TMU0);
	bgDecal = new TexInfo("tests/miro.3df");
	triDecal = new TexInfo("tests/matt1.3df");

	bgDecal.DownloadMipMap(GR_TMU.TMU0, FxTexMemGetStartAddress(GR_TMU.TMU0, bgDecal), GR_MIPMAPLEVELMASK.BOTH);
	triDecal.DownloadMipMap(GR_TMU.TMU0, FxTexMemGetStartAddress(GR_TMU.TMU0, triDecal), GR_MIPMAPLEVELMASK.BOTH);

	fxTexMipMapMode(GR_TMU.TMU0, GR_MIPMAP.NEAREST, true);
	fxTexClampMode(GR_TMU.TMU0, GR_TEXTURECLAMP.WRAP, GR_TEXTURECLAMP.WRAP);
	fxTexFilterMode(GR_TMU.TMU0, GR_TEXTUREFILTER.BILINEAR, GR_TEXTUREFILTER.BILINEAR);

	fxTexCombine(
		GR_TMU.TMU0,
		GR_COMBINE_FUNCTION.LOCAL, GR_COMBINE_FACTOR.NONE,
		GR_COMBINE_FUNCTION.LOCAL, GR_COMBINE_FACTOR.NONE,
		false, false);
	fxRenderBuffer(backbuffer ? GR_BUFFER.BACKBUFFER : GR_BUFFER.FRONTBUFFER);

	/* Set up alpha blending for AA and compositing... */
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
}

function PointMatMult(v) {
	var m = GetMatrix();
	var ptmp = {};
	ptmp.a = v.a;
	ptmp.r = v.r;
	ptmp.g = v.g;
	ptmp.b = v.b;
	ptmp.oow = v.oow;
	ptmp.sow = v.sow;
	ptmp.tow = v.tow;

	ptmp.x = (v.x * m[0][0]) + (v.y * m[1][0]) + (v.z * m[2][0]) + m[3][0];
	ptmp.y = (v.x * m[0][1]) + (v.y * m[1][1]) + (v.z * m[2][1]) + m[3][1];
	ptmp.z = (v.x * m[0][2]) + (v.y * m[1][2]) + (v.z * m[2][2]) + m[3][2];
	w = (v.x * m[0][3]) + (v.y * m[1][3]) + (v.z * m[2][3]) + m[3][3];
	if (w != 0) {
		ptmp.x /= w; ptmp.y /= w; ptmp.z /= w;
	}
	return ptmp;
}

function toVert(v) {
	return [v.x, v.y, v.r, v.g, v.b, v.a, v.oow, v.sow, v.tow];
}

/*
** This function is repeatedly until ESC is pressed or Stop() is called.
*/
function Loop() {
	SetMatrix(Identity());
	MultMatrix(YRotation(y_angle));

	var xformedVerts = [{}, {}, {}];
	for (var i = 0; i < VERT_COUNT; i++) {
		xformedVerts[i] = PointMatMult(localVerts[i]);
		xformedVerts[i].x = xformedVerts[i].x / (xformedVerts[i].z + 2.0);
		xformedVerts[i].y = xformedVerts[i].y / (xformedVerts[i].z + 2.0);
		xformedVerts[i].x *= scrWidth / 2.0;
		xformedVerts[i].y *= scrHeight / 2.0;
		xformedVerts[i].x += scrWidth / 2.0;
		xformedVerts[i].y += scrHeight / 2.0;
		xformedVerts[i].oow = 1 / ((xformedVerts[i].z + 2) * scrHeight);
		xformedVerts[i].sow *= xformedVerts[i].oow;
		xformedVerts[i].tow *= xformedVerts[i].oow;
	}

	switch (ccFnc) {
		case GR_COLORCOMBINE.ITRGB:
			fxColorCombine(
				GR_COMBINE_FUNCTION.LOCAL,
				GR_COMBINE_FACTOR.NONE,
				GR_COMBINE_LOCAL.ITERATED,
				GR_COMBINE_OTHER.NONE,
				false);
			break;

		case GR_COLORCOMBINE.DECAL_TEXTURE:
			fxColorCombine(
				GR_COMBINE_FUNCTION.SCALE_OTHER,
				GR_COMBINE_FACTOR.ONE,
				GR_COMBINE_LOCAL.NONE,
				GR_COMBINE_OTHER.TEXTURE,
				false);
			break;

		case GR_COLORCOMBINE.TEXTURE_TIMES_ITRGB:
			fxColorCombine(
				GR_COMBINE_FUNCTION.SCALE_OTHER,
				GR_COMBINE_FACTOR.LOCAL,
				GR_COMBINE_LOCAL.ITERATED,
				GR_COMBINE_OTHER.TEXTURE,
				false);
			break;
	}

	if (render) {
		fxBufferClear(0xffffffff, 0, wrange[1]);
		if (background) {
			oldState = new FxState();

			fxResetVertexLayout();
			fxVertexLayout([GR_PARAM.XY, GR_PARAM.A, GR_PARAM.Q, GR_PARAM.ST0]);

			fxAlphaBlendFunction(
				GR_BLEND.ONE, GR_BLEND.ZERO,
				GR_BLEND.ONE, GR_BLEND.ZERO);

			fxColorCombine(
				GR_COMBINE_FUNCTION.SCALE_OTHER,
				GR_COMBINE_FACTOR.ONE,
				GR_COMBINE_LOCAL.NONE,
				GR_COMBINE_OTHER.TEXTURE,
				false);

			bgDecal.Source(GR_MIPMAPLEVELMASK.BOTH);

			fxDrawTriangle(texVerts[0], texVerts[1], texVerts[2]);
			fxDrawTriangle(texVerts[2], texVerts[3], texVerts[0]);

			oldState.Set();
			fxClipWindow(0, 0, scrWidth, scrHeight);
		}

		if (texturing)
			triDecal.Source(GR_MIPMAPLEVELMASK.BOTH);

		if (antialias) {
			fxEnable(GR_ENABLE.AA_ORDERED);
			if (lines) {
				fxDrawLine(toVert(xformedVerts[0]), toVert(xformedVerts[1]));
				fxDrawLine(toVert(xformedVerts[1]), toVert(xformedVerts[2]));
				fxDrawLine(toVert(xformedVerts[2]), toVert(xformedVerts[0]));
			} else {
				fxAADrawTriangle(toVert(xformedVerts[0]), toVert(xformedVerts[1]), toVert(xformedVerts[2]), true, true, true);
			}
		} else {
			fxDisable(GR_ENABLE.AA_ORDERED);
			if (lines) {
				fxDrawLine(toVert(xformedVerts[0]), toVert(xformedVerts[1]));
				fxDrawLine(toVert(xformedVerts[1]), toVert(xformedVerts[2]));
				fxDrawLine(toVert(xformedVerts[2]), toVert(xformedVerts[0]));
			} else {
				fxDrawTriangle(toVert(xformedVerts[0]), toVert(xformedVerts[1]), toVert(xformedVerts[2]));
			}
		}

		y_angle += 2;
		if (y_angle > 360.0) {
			y_angle = 0;
		}
	}
}

function Input(e) {
	if (CompareKey(e.key, 'a')) {
		antialias = !antialias;
	} else if (CompareKey(e.key, 'b')) {
		if (bilinear) {
			bilinear = false;
			fxTexFilterMode(GR_TMU.TMU0, GR_TEXTUREFILTER.POINT_SAMPLED, GR_TEXTUREFILTER.POINT_SAMPLED);
		} else {
			fxTexFilterMode(GR_TMU.TMU0, GR_TEXTUREFILTER.BILINEAR, GR_TEXTUREFILTER.BILINEAR);
			bilinear = true;
		}
	} else if (CompareKey(e.key, 'c')) {
		if (blend) {
			blend = false;
			for (var i = 0; i < VERT_COUNT; i++) { localVerts[i].a = 255.0; }
		} else {
			blend = true;
			for (var i = 0; i < VERT_COUNT; i++) { localVerts[i].a = alpha; }
		}
	} else if (CompareKey(e.key, 'f')) {
		if (backbuffer) {
			backbuffer = false;
			fxRenderBuffer(GR_BUFFER.FRONTBUFFER);
		} else {
			backbuffer = true;
			fxRenderBuffer(GR_BUFFER.BACKBUFFER);
		}
	} else if (CompareKey(e.key, 'i')) {
		background = !background;
	} else if (CompareKey(e.key, 'l')) {
		lines = !lines;
	} else if (CompareKey(e.key, 'o')) {
		if (origin == GR_ORIGIN.LOWER_LEFT) {
			origin = GR_ORIGIN.UPPER_LEFT;
		} else {
			origin = GR_ORIGIN.LOWER_LEFT;
		}
		fxOrigin(origin);
	} else if (CompareKey(e.key, 'p')) {
		render = !render;
	} else if (CompareKey(e.key, 's')) {
		if (cullMode == GR_CULL.DISABLE) {
			cullMode = GR_CULL.NEGATIVE;
		} else {
			cullMode = GR_CULL.DISABLE;
		}
		fxCullMode(cullMode);
	} else if (CompareKey(e.key, 't')) {
		if (texturing) {
			ccFnc = GR_COLORCOMBINE.ITRGB;
			texturing = false;
			fxResetVertexLayout();
			fxVertexLayout([GR_PARAM.XY, GR_PARAM.RGB, GR_PARAM.A]);
		} else {
			ccFnc = GR_COLORCOMBINE.DECAL_TEXTURE;
			texturing = true;
			fxResetVertexLayout();
			fxVertexLayout([GR_PARAM.XY, GR_PARAM.RGB, GR_PARAM.A, GR_PARAM.Q, GR_PARAM.ST0]);
		}
	} else if (CompareKey(e.key, 'm')) {
		ccFnc = GR_COLORCOMBINE.TEXTURE_TIMES_ITRGB;
	}
}
