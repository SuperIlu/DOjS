/*
 *    Example ported to DOjS by Andre Seidelt <superilu@yahoo.com>
 * 
 *    Example program for the Allegro library, by Bertrand Coconnier.
 *
 *    This program demonstrates how to use Z-buffered polygons and
 *    floating point 3D math routines. It also provides a simple
 *    way to compute fps (frames per second) using a timer. After
 *    selecting a screen resolution through the standard GUI dialog,
 *    the example shows two 3D cubes rotating and intersecting each
 *    other. Rather than having full polygons incorrectly overlap
 *    other polygons due to per-polygon sorting, each pixel is drawn
 *    at the correct depth.
 */
LoadLibrary("al3d");

var cube1 =
	[
		[-32., -32., -32., 0., 0., 8],
		[-32., 32., -32., 0., 0., 16],
		[32., 32., -32., 0., 0., 31],
		[32., -32., -32., 0., 0., 24],
		[-32., -32., 32., 0., 0., 8],
		[-32., 32., 32., 0., 0., 16],
		[32., 32., 32., 0., 0., 31],
		[32., -32., 32., 0., 0., 24]
	];


var cube2 =
	[
		[-32., -32., -32., 0., 0., 8],
		[-32., 32., -32., 0., 0., 16],
		[32., 32., -32., 0., 0., 31],
		[32., -32., -32., 0., 0., 24],
		[-32., -32., 32., 0., 0., 8],
		[-32., 32., 32., 0., 0., 16],
		[32., 32., 32., 0., 0., 31],
		[32., -32., 32., 0., 0., 24]
	];


var faces =
	[
		[2, 1, 0, 3],
		[4, 5, 6, 7],
		[0, 1, 5, 4],
		[2, 3, 7, 6],
		[4, 7, 3, 0],
		[1, 2, 6, 5]
	];

var zbuf;
var matrix1, matrix2;

var rx1, ry1, rz1;		/* cube #1 rotations */
var drx1, dry1, drz1;	/* cube #1 rotation speed */
var rx2, ry2, rz2;		/* cube #2 rotations */
var drx2, dry2, drz2;	/* cube #1 rotation speed */
var tx = 16.;		/* x shift between cubes */
var tz1 = 100.;		/* cube #1 z coordinate */
var tz2 = 105.;		/* cube #2 z coordinate */

var cube_x1 = [
	[0, 0, 0, 0, 0, 0],
	[0, 0, 0, 0, 0, 0],
	[0, 0, 0, 0, 0, 0],
	[0, 0, 0, 0, 0, 0],
	[0, 0, 0, 0, 0, 0],
	[0, 0, 0, 0, 0, 0],
	[0, 0, 0, 0, 0, 0],
	[0, 0, 0, 0, 0, 0]
];

var cube_x2 = [
	[0, 0, 0, 0, 0, 0],
	[0, 0, 0, 0, 0, 0],
	[0, 0, 0, 0, 0, 0],
	[0, 0, 0, 0, 0, 0],
	[0, 0, 0, 0, 0, 0],
	[0, 0, 0, 0, 0, 0],
	[0, 0, 0, 0, 0, 0],
	[0, 0, 0, 0, 0, 0]
];

var blue = [];
var green = [];

var img = null;

function Setup() {
	img = new Bitmap("examples/DOjStex.bmp");

	/* create the Z-buffer */
	zbuf = new ZBuffer();
	zbuf.Set();

	/* set up the viewport for the perspective projection */
	SetProjectionViewport(0, 0, SizeX(), SizeY());

	/* compute rotations and speed rotation */
	rx1 = ry1 = rz1 = 0.;
	rx2 = ry2 = rz2 = 0.;

	drx1 = Math.PI / 16 * Math.random();
	dry1 = Math.PI / 16 * Math.random();
	drz1 = Math.PI / 16 * Math.random();

	drx2 = Math.PI / 16 * Math.random();
	dry2 = Math.PI / 16 * Math.random();
	drz2 = Math.PI / 16 * Math.random();

	/* set the transformation matrices */
	matrix1 = GetTransformationMatrix(1., rx1, ry1, rz1, tx, 0., tz1);
	matrix2 = GetTransformationMatrix(1., rx2, ry2, rz2, -tx, 0., tz2);

	/* make a blue gradient */
	for (var i = 0; i < 32; i++) {
		blue.push(Color(10, 10, 32 + i * 2));
	}

	/* make a green gradient */
	for (var i = 0; i < 32; i++) {
		green.push(Color(10, i * 2 + 32, 10));
	}
}

function Loop() {
	ClearScreen(EGA.BLACK);
	zbuf.Clear(0);

	AnimCube(matrix1, matrix2, cube_x1, cube_x2);
	DrawCube(cube_x1, cube_x2);

	/* update transformation matrices */
	rx1 += drx1;
	ry1 += dry1;
	rz1 += drz1;
	rx2 += drx2;
	ry2 += dry2;
	rz2 += drz2;

	matrix1 = GetTransformationMatrix(1., rx1, ry1, rz1, tx, 0., tz1);
	matrix2 = GetTransformationMatrix(1., rx2, ry2, rz2, -tx, 0., tz2);
}

/*
** This function is called on any input.
*/
function Input(event) {
}

/* update cube positions */
function AnimCube(matrix1, matrix2, x1, x2) {
	for (var i = 0; i < 8; i++) {
		x1[i] = ApplyMatrix(matrix1, cube1[i][0], cube1[i][1], cube1[i][2]);
		x2[i] = ApplyMatrix(matrix2, cube2[i][0], cube2[i][1], cube2[i][2]);
		var pp1 = PerspProject(x1[i][0], x1[i][1], x1[i][2]);
		x1[i][0] = pp1[0];
		x1[i][1] = pp1[1];
		var pp2 = PerspProject(x2[i][0], x2[i][1], x2[i][2]);
		x2[i][0] = pp2[0];
		x2[i][1] = pp2[1];

		x1[i][5] = blue[cube1[i][5]];
	}
}

/* cull backfaces and draw cubes */
function DrawCube(x1, x2) {
	for (var i = 0; i < 6; i++) {
		var vtx1, vtx2, vtx3, vtx4;

		vtx1 = x1[faces[i][0]];
		vtx2 = x1[faces[i][1]];
		vtx3 = x1[faces[i][2]];
		vtx4 = x1[faces[i][3]];

		if (PolygonZNormal(vtx1, vtx2, vtx3) > 0) {
			Quad3D(POLYTYPE.GCOL | POLYTYPE.ZBUF, null, vtx1, vtx2, vtx3, vtx4);
		}

		vtx1 = x2[faces[i][0]];
		vtx2 = x2[faces[i][1]];
		vtx3 = x2[faces[i][2]];
		vtx4 = x2[faces[i][3]];

		if (PolygonZNormal(vtx1, vtx2, vtx3) > 0) {
			vtx1[3] = 0;
			vtx1[4] = 0;

			vtx2[3] = 0;
			vtx2[4] = img.height;

			vtx3[3] = img.width;
			vtx3[4] = img.height;

			vtx4[3] = img.width;
			vtx4[4] = 0;

			Quad3D(POLYTYPE.ATEX | POLYTYPE.ZBUF, img, vtx1, vtx2, vtx3, vtx4);
		}
	}
}

// immitate rand()
function AL_RAND() {
	return Math.random() * 2147483647;
}

