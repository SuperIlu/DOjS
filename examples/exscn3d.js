/*
 *    Example ported to DOjS by Andre Seidelt <superilu@yahoo.com>
 *
 *    Example program for the Allegro library, by Bertrand Coconnier.
 *
 *    This program demonstrates how to use scanline sorting algorithm
 *    in Allegro (create_scene, clear_scene, ... functions). It
 *    also provides an example of how to use the 3D clipping
 *    function. The example consists of a flyby through a lot of
 *    rotating 3d cubes.
 *
 */
LoadLibrary("al3d");

var MAX_CUBES = 4;

var Xvertex = [
	[-10., -10., -10., 0., 0., 8],
	[-10., 10., -10., 0., 0., 16],
	[10., 10., -10., 0., 0., 31],
	[10., -10., -10., 0., 0., 24],
	[-10., -10., 10., 0., 0., 8],
	[-10., 10., 10., 0., 0., 16],
	[10., 10., 10., 0., 0., 31],
	[10., -10., 10., 0., 0., 24]
];

var vertex = [
	[-10., -10., -10., 0., 0., Color(10, 10, 32 + 8 * 2)],
	[-10., 10., -10., 0., 0., Color(10, 10, 32 + 16 * 2)],
	[10., 10., -10., 0., 0., Color(10, 10, 32 + 31 * 2)],
	[10., -10., -10., 0., 0., Color(10, 10, 32 + 24 * 2)],
	[-10., -10., 10., 0., 0., Color(10, 10, 32 + 8 * 2)],
	[-10., 10., 10., 0., 0., Color(10, 10, 32 + 16 * 2)],
	[10., 10., 10., 0., 0., Color(10, 10, 32 + 31 * 2)],
	[10., -10., 10., 0., 0., Color(10, 10, 32 + 24 * 2)]
];

var cube = [
	[2, 1, 0, 3],
	[4, 5, 6, 7],
	[0, 1, 5, 4],
	[2, 3, 7, 6],
	[4, 7, 3, 0],
	[1, 2, 6, 5]
];

//var blue = [];

var vout;
var v = [[], [], [], []];

var rx = 0;
var ry = 0;
var tz = 40;
var rot = 0;
var inc = (Math.PI / 180); // 1deg
var rotx = 6 * (Math.PI / 180); // 6 deg
var roty = rotx;
var deg35 = 35 * (Math.PI / 180); // 35 deg
var translat = MAX_CUBES * 20 + 20;

function Setup() {
	CreateScene(24 * MAX_CUBES * MAX_CUBES * MAX_CUBES, 6 * MAX_CUBES * MAX_CUBES * MAX_CUBES);
	SetProjectionViewport(0, 0, SizeX(), SizeY());
}

function Loop() {
	ClearScreen(EGA.BLACK);
	ClearScene();

	// matrix2: rotates cube
	var matrix2 = GetRotationMatrix(rx, ry, 0);
	// matrix3: turns head right/left
	var matrix3 = GetRotationMatrix(0, rot, 0);

	for (var k = MAX_CUBES - 1; k >= 0; k--) {
		for (var j = 0; j < MAX_CUBES; j++) {
			for (var i = 0; i < MAX_CUBES; i++) {
				// matrix1: locates cubes
				var matrix1 = GetTranslationMatrix(j * 40 - translat, i * 40 - translat, tz + k * 40);

				// matrix: rotates cube THEN locates cube THEN turns head right/left
				DrawCube(MatrixMul(MatrixMul(matrix2, matrix1), matrix3), 6);
			}
		}
	}

	// sorts and renders polys
	RenderScene();

	// manage cubes movement
	tz -= 2;
	if (!tz) { tz = 40; }
	rx += rotx;
	ry += roty;
	rot += inc;
	if ((rot >= deg35) || (rot <= -deg35)) { inc = -inc; }
}

/*
** This function is called on any input.
*/
function Input(event) {
}

function DrawCube(matrix, num_poly) {
	for (var i = 0; i < num_poly; i++) {
		for (var j = 0; j < 4; j++) {
			v[j] = ApplyMatrix(matrix, vertex[cube[i][j]]);
		}

		var vout = Clip3D(POLYTYPE.GCOL, 0.1, 1000., v);
		if (vout.length) {
			for (var j = 0; j < vout.length; j++) {
				vout[j] = PerspProject(vout[j]);
			}

			if (PolygonZNormal(vout[0], vout[1], vout[2]) > 0) {
				ScenePolygon3D(POLYTYPE.GCOL, null, vout);
			}
		}
	}
}
