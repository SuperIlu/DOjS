Include('tests/3dfx_tst.js');
Println("texture filter modes test");

var zrange;

var texture;

var minify = false;
var bilerp = false;
var clamping = 1;

/*
** This function is called once when the script is started.
*/
function Setup() {
	fxInit();
	fxVertexLayout([GR_PARAM.XY, GR_PARAM.Q, GR_PARAM.ST0]);
	createVertices();

	fxColorCombine(
		GR_COMBINE_FUNCTION.SCALE_OTHER,
		GR_COMBINE_FACTOR.ONE,
		GR_COMBINE_LOCAL.NONE,
		GR_COMBINE_OTHER.TEXTURE,
		false);
	fxTexCombine(
		GR_TMU.TMU0,
		GR_COMBINE_FUNCTION.LOCAL,
		GR_COMBINE_FACTOR.NONE,
		GR_COMBINE_FUNCTION.LOCAL,
		GR_COMBINE_FACTOR.NONE,
		false, false);
	fxTexMipMapMode(
		GR_TMU.TMU0,
		GR_MIPMAP.DISABLE,
		false);

	/* Load texture data into system ram */
	FxTexMemInit(GR_TMU.TMU0);
	texture = new TexInfo("tests/miro.3df");
	/* Download texture data to TMU */
	texture.DownloadMipMap(GR_TMU.TMU0, FxTexMemGetStartAddress(GR_TMU.TMU0, texture), GR_MIPMAPLEVELMASK.BOTH);
	/* Select Texture As Source of all texturing operations */
	texture.Source(GR_MIPMAPLEVELMASK.BOTH);

	zrange = fxGetZDepthMinMax();
}

/*
** This function is repeatedly until ESC is pressed or Stop() is called.
*/
function Loop() {
	fxBufferClear(FX_BLACK, 0, zrange[1]);

	/*---- 
	  A-B
	  |\|
	  C-D
	  -----*/
	v1[2] = 1.0;
	v2[2] = 1.0;
	v3[2] = 1.0;
	v4[2] = 1.0;

	if (minify) {
		v1[0] = v3[0] = scaleX(0.0);
		v2[0] = v4[0] = scaleX(1.0);
		v1[1] = v2[1] = scaleY(0.0);
		v3[1] = v4[1] = scaleY(1.0);
	} else { /* magnify */
		v1[0] = v3[0] = scaleX(0.45);
		v2[0] = v4[0] = scaleX(0.55);
		v1[1] = v2[1] = scaleY(0.45);
		v3[1] = v4[1] = scaleY(0.55);
	}

	v1[3] = v3[3] = 0.0;
	v2[3] = v4[3] = 255.0;
	v1[4] = v2[4] = 0.0;
	v3[4] = v4[4] = 255.0;

	if (bilerp) {
		fxTexFilterMode(
			GR_TMU.TMU0,
			GR_TEXTUREFILTER.BILINEAR,
			GR_TEXTUREFILTER.BILINEAR);
	} else {
		fxTexFilterMode(
			GR_TMU.TMU0,
			GR_TEXTUREFILTER.POINT_SAMPLED,
			GR_TEXTUREFILTER.POINT_SAMPLED);
	}

	switch (clamping) {
		case 0:
			v2[3] = v4[3] = 1023.0;
			v3[4] = v4[4] = 1023.0;
			fxTexClampMode(GR_TMU.TMU0, GR_TEXTURECLAMP.WRAP, GR_TEXTURECLAMP.WRAP);
			break;
		case 1:
			v2[3] = v4[3] = 255.0;
			v3[4] = v4[4] = 255.0;
			fxTexClampMode(GR_TMU.TMU0, GR_TEXTURECLAMP.CLAMP, GR_TEXTURECLAMP.CLAMP);
			break;
		case 2:
			v2[3] = v4[3] = 1023.0;
			v3[4] = v4[4] = 1023.0;
			fxTexClampMode(GR_TMU.TMU0, GR_TEXTURECLAMP.MIRROR_EXT, GR_TEXTURECLAMP.MIRROR_EXT);
			break;
		case 3:
			v2[3] = v4[3] = 1023.0;
			v3[4] = v4[4] = 1023.0;
			fxTexClampMode(GR_TMU.TMU0, GR_TEXTURECLAMP.MIRROR_EXT, GR_TEXTURECLAMP.WRAP);
			break;
		case 4:
			v2[3] = v4[3] = 1023.0;
			v3[4] = v4[4] = 1023.0;
			fxTexClampMode(GR_TMU.TMU0, GR_TEXTURECLAMP.WRAP, GR_TEXTURECLAMP.MIRROR_EXT);
			break;
		case 5:
			v2[3] = v4[3] = 1023.0;
			v3[4] = v4[4] = 1023.0;
			fxTexClampMode(GR_TMU.TMU0, GR_TEXTURECLAMP.MIRROR_EXT, GR_TEXTURECLAMP.CLAMP);
			break;
		case 6:
			v2[3] = v4[3] = 1023.0;
			v3[4] = v4[4] = 1023.0;
			fxTexClampMode(GR_TMU.TMU0, GR_TEXTURECLAMP.CLAMP, GR_TEXTURECLAMP.MIRROR_EXT);
			break;
	}

	fxDrawTriangle(v1, v4, v3);
	fxDrawTriangle(v1, v2, v4);
}

function Input(e) {
	if (CompareKey(e.key, 'm')) {
		minify = !minify;
	} else if (CompareKey(e.key, 'f')) {
		bilerp = !bilerp;
	} else if (CompareKey(e.key, 'c')) {
		clamping++;
		clamping = clamping % 7;
	}
}
