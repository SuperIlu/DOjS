Include('tests/3dfx_tst.js');
Println("renders two interpenetrating triangles with z-buffering");

var zrange;

/*
** This function is called once when the script is started.
*/
function Setup() {
	fxInit();
	fxVertexLayout([GR_PARAM.XY, GR_PARAM.Z]);
	createVertices();

	fxColorCombine(
		GR_COMBINE_FUNCTION.LOCAL,
		GR_COMBINE_FACTOR.NONE,
		GR_COMBINE_LOCAL.CONSTANT,
		GR_COMBINE_OTHER.NONE,
		false);

	zrange = fxGetZDepthMinMax();

	fxDepthBufferMode(GR_DEPTHBUFFER.ZBUFFER);
	fxDepthBufferFunction(GR_CMP.GREATER);
	fxDepthMask(true);
}

/*
** This function is repeatedly until ESC is pressed or Stop() is called.
*/
function Loop() {
	fxBufferClear(FX_BLACK, 0, zrange[1]);

	v1[0] = scaleX(0.25); v1[1] = scaleY(0.21);
	v2[0] = scaleX(0.75); v2[1] = scaleY(0.21);
	v3[0] = scaleX(0.50); v3[1] = scaleY(0.79);

    /*----------------------------------------------------------- 
      Depth values should be scaled from reciprocated Depth Value 
      then scaled from 0 to 65535.0.
      
      ooz = ( 1.0f / Z ) * 65535.0f = 65535.0f / Z
      -----------------------------------------------------------*/
	var zDist = 10.0;
	v1[2] = v2[2] = v3[2] = (65535.0 / zDist);

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
	zDist = 12.5;
	v1[2] = v2[2] = (65535.0 / zDist);
	zDist = 7.5;
	v3[2] = (65535.0 / zDist);

	fxConstantColorValue(FX_GREEN);

	fxDrawTriangle(v1, v2, v3);
}

function Input(e) { }
