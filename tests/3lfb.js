Include('tests/3dfx_tst.js');
Println("test linear frame buffer write");

var zrange;

var dynBitmap;
var genTextDest = 0;
var circSize = 32;
var circDir = 1;

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
	SetRenderBitmap(null);
}

/*
** This function is repeatedly until ESC is pressed or Stop() is called.
*/
function Loop() {
	updateTexture();

	fxBufferClear(FX_BLACK, 0, zrange[1]);
	fxLfbConstantDepth(zrange[0]);

	dynBitmap.FxDrawLfb(100, 100, GR_BUFFER.BACKBUFFER, false);
	//fxLfbTest();
}

function Input(e) {
}
