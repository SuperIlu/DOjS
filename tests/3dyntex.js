Include('tests/3dfx_tst.js');
Println("dynamic texturing example");

var zrange;

var dynBitmap;
var genTextDest = 0;
var circSize = 32;
var circDir = 1;
var tg;

/*
** This function is called once when the script is started.
*/
function Setup() {
	fxInit();
	fxVertexLayout([GR_PARAM.XY]);
	createVertices();

	fxTexCombine(
		GR_TMU.TMU0,
		GR_COMBINE_FUNCTION.LOCAL,
		GR_COMBINE_FACTOR.NONE,
		GR_COMBINE_FUNCTION.NONE,
		GR_COMBINE_FACTOR.NONE,
		false, false);

	/* Load texture data into system ram */
	FxTexMemInit(GR_TMU.TMU0);

	/* Enable Bilinear Filtering + Mipmapping */
	fxTexFilterMode(GR_TMU.TMU0, GR_TEXTUREFILTER.BILINEAR, GR_TEXTUREFILTER.BILINEAR);
	fxTexMipMapMode(GR_TMU.TMU0, GR_MIPMAP.NEAREST, false);

	// set texture mode
	fxResetVertexLayout();
	fxVertexLayout([GR_PARAM.XY, GR_PARAM.Q, GR_PARAM.ST0]);
	createVertices();
	fxColorCombine(
		GR_COMBINE_FUNCTION.SCALE_OTHER,
		GR_COMBINE_FACTOR.ONE,
		GR_COMBINE_LOCAL.NONE,
		GR_COMBINE_OTHER.TEXTURE,
		false);

	zrange = fxGetZDepthMinMax();

	dynBitmap = new Bitmap(256, 256);
}

function updateTexture() {
	circSize += circDir;
	if (circSize > 64) {
		circDir = -1;
	} else if (circSize < 16) {
		circDir = 1;
	}

	// dynamically generate texture
	SetRenderBitmap(dynBitmap);
	ClearScreen(EGA.BLACK);
	CustomLine(0, 0, SizeX(), SizeY(), 5, EGA.RED);
	CustomLine(0, SizeY(), SizeX(), 0, 5, EGA.GREEN);
	Circle(SizeX() / 2, SizeY() / 2, circSize, EGA.YELLOW);
	tg = new TexInfo(dynBitmap);
	SetRenderBitmap(null);

	// allocate texture memory only once
	if (genTextDest == 0) {
		genTextDest = FxTexMemGetStartAddress(GR_TMU.TMU0, tg);
	}
	tg.DownloadMipMap(GR_TMU.TMU0, genTextDest, GR_MIPMAPLEVELMASK.BOTH);
}

/*
** This function is repeatedly until ESC is pressed or Stop() is called.
*/
function Loop() {
	updateTexture();

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

	v1[0] = v3[0] = scaleX(0.2);
	v2[0] = v4[0] = scaleX(0.8);
	v1[1] = v2[1] = scaleY(0.2);
	v3[1] = v4[1] = scaleY(0.8);

	v1[3] = v3[3] = 0.0;
	v2[3] = v4[3] = 255.0;
	v1[4] = v2[4] = 0.0;
	v3[4] = v4[4] = 255.0;

	tg.Source(GR_MIPMAPLEVELMASK.BOTH)
	fxDrawTriangle(v1, v4, v3);
	fxDrawTriangle(v1, v2, v4);

	fxConstantColorValue(FX_BLUE);
}

function Input(e) {
}
