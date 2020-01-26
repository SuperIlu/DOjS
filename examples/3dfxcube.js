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
	t1 = new TexInfo("examples/v1text.3df");
	t2 = new TexInfo("examples/6x86text.3df");
	/* Download texture data to TMU */
	t1.DownloadMipMap(GR_TMU.TMU0, FxTexMemGetStartAddress(GR_TMU.TMU0, t1), GR_MIPMAPLEVELMASK.BOTH);
	t2.DownloadMipMap(GR_TMU.TMU0, FxTexMemGetStartAddress(GR_TMU.TMU0, t2), GR_MIPMAPLEVELMASK.BOTH);

}

function Loop() {
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
	t1.Source(GR_MIPMAPLEVELMASK.BOTH)
	//Left
	DrawPoly(x[0], y[0], z[0], x[3], y[3], z[3],
		x[7], y[7], z[7], x[4], y[4], z[4]);
	//Right
	DrawPoly(x[2], y[2], z[2], x[1], y[1], z[1],
		x[5], y[5], z[5], x[6], y[6], z[6]);
	//Back
	DrawPoly(x[1], y[1], z[1], x[0], y[0], z[0],
		x[4], y[4], z[4], x[5], y[5], z[5]);
	//Front
	DrawPoly(x[3], y[3], z[3], x[2], y[2], z[2],
		x[6], y[6], z[6], x[7], y[7], z[7]);
	//The top and bottom using another texture

	t2.Source(GR_MIPMAPLEVELMASK.BOTH)
	//Top
	DrawPoly(x[0], y[0], z[0], x[1], y[1], z[1],
		x[2], y[2], z[2], x[3], y[3], z[3]);
	//Bottom
	DrawPoly(x[4], y[4], z[4], x[5], y[5], z[5],
		x[6], y[6], z[6], x[7], y[7], z[7]);

	//Set new angles
	xAngle += 0.01; yAngle += 0.02; zAngle += 0.03;
}


function Input(e) { }

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
