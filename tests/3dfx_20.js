Include('tests/3dfx_tst.js');
Println("mipmap modes");

var zrange;

var texture;

var DISABLE = 0, NEAREST = 1, TRILINEAR = 2;
var MAX_DIST = 2.5;
var MIN_DIST = 1.0;

var mipMapMode = DISABLE;
var distance = 1.0;
var dDelta = 0.01;

var srcVerts = [
	[0, 0, 0, 0, 0, 0],	// x, y, z, w, s, t
	[0, 0, 0, 0, 0, 0],
	[0, 0, 0, 0, 0, 0],
	[0, 0, 0, 0, 0, 0]
];

/*
** This function is called once when the script is started.
*/
function Setup() {
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
function Loop() {
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
