Include('tests/3dfx_tst.js');
Println("renders two interpenetrating triangles with w-buffering");

var wrange;

/*
** This function is called once when the script is started.
*/
function Setup() {
	fxInit();
	fxVertexLayout([GR_PARAM.XY, GR_PARAM.Q]);
	createVertices();

	fxColorCombine(
		GR_COMBINE_FUNCTION.LOCAL,
		GR_COMBINE_FACTOR.NONE,
		GR_COMBINE_LOCAL.CONSTANT,
		GR_COMBINE_OTHER.NONE,
		false);

	wrange = fxGetWDepthMinMax();

	fxDepthBufferMode(GR_DEPTHBUFFER.WBUFFER);
	fxDepthBufferFunction(GR_CMP.LESS);
	fxDepthMask(true);
}

/*
** This function is repeatedly until ESC is pressed or Stop() is called.
*/
function Loop() {
	fxBufferClear(FX_BLACK, 0, wrange[1]);

	v1[0] = scaleX(0.25); v1[1] = scaleY(0.21);
	v2[0] = scaleX(0.75); v2[1] = scaleY(0.21);
	v3[0] = scaleX(0.50); v3[1] = scaleY(0.79);

	/*----------------------------------------------------------- 
	  OOW Values are in the range (1,1/65535)

	  This can be the exact same computed 1/W that you use
	  for texture mapping.  This saves on host computation and 
	  vertex data transferred across the PCI bus.
	  -----------------------------------------------------------*/
	var wDist = 10.0;
	v1[2] = v2[2] = v3[2] = (1.0 / wDist);

	fxConstantColorValue(FX_GREY);

	fxDrawTriangle(v1, v2, v3);

	v1[0] = scaleX(0.86); v1[1] = scaleY(0.21);
	v2[0] = scaleX(0.86); v2[1] = scaleY(0.79);
	v3[0] = scaleX(0.14); v3[1] = scaleY(0.5);

    /*----------------------------------------------------------- 
      Depth values should be scaled from reciprocated Depth Value 
      then scaled to ( 0, 65535 )
      
      ooz = ( 1.0f / Z ) * 65535.0f = 65535.0f / Z
      -----------------------------------------------------------*/
	wDist = 12.5;
	v1[2] = v2[2] = (1.0 / wDist);
	wDist = 7.5;
	v3[2] = (1.0 / wDist);

	fxConstantColorValue(FX_GREEN);

	fxDrawTriangle(v1, v2, v3);
}

function Input(e) { }
