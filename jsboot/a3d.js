/*
MIT License

Copyright (c) 2019 Andre Seidelt <superilu@yahoo.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

/** @module a3d */

/**
 * polytype definition.
 */
POLYTYPE = {
	FLAT: 0,
	GCOL: 1,
	GRGB: 2,
	ATEX: 3,
	PTEX: 4,
	ATEX_MASK: 5,
	PTEX_MASK: 6,
	ATEX_LIT: 7,
	PTEX_LIT: 8,
	ATEX_MASK_LIT: 9,
	PTEX_MASK_LIT: 10,
	ATEX_TRANS: 11,
	PTEX_TRANS: 12,
	ATEX_MASK_TRANS: 13,
	PTEX_MASK_TRANS: 14,
	MAX: 15,
	ZBUF: 16
};

function GetEmptyMatrix() {
	return {
		v: [
			/* 3x3 */
			[0, 0, 0],
			[0, 0, 0],
			[0, 0, 0]
		],

		/* zero translation */
		t: [0, 0, 0]
	};
}

function GetIdentityMatrix() {
	return {
		v: [
			/* 3x3 identity */
			[1.0, 0.0, 0.0],
			[0.0, 1.0, 0.0],
			[0.0, 0.0, 1.0]
		],

		/* zero translation */
		t: [0.0, 0.0, 0.0]
	};
}

function GetTranslationMatrix(x, y, z) {
	var m = GetIdentityMatrix();

	m.t[0] = x;
	m.t[1] = y;
	m.t[2] = z;

	return m;
}

function GetScalingMatrix(x, y, z) {
	var m = GetIdentityMatrix();

	m.v[0][0] = x;
	m.v[1][1] = y;
	m.v[2][2] = z;

	return m;
}

function GetXRotatateMatrix(r) {
	var c = Math.cos(r);
	var s = Math.sin(r);

	m = GetIdentityMatrix();

	m.v[1][1] = c;
	m.v[1][2] = -s;

	m.v[2][1] = s;
	m.v[2][2] = c;

	return m;
}

function GetYRotatateMatrix(r) {
	var c = Math.cos(r);
	var s = Math.sin(r);

	m = GetIdentityMatrix();

	m.v[0][0] = c;
	m.v[0][2] = s;

	m.v[2][0] = -s;
	m.v[2][2] = c;

	return m;
}

function GetZRotatateMatrix(r) {
	var c = Math.cos(r);
	var s = Math.sin(r);

	m = GetIdentityMatrix();

	m.v[0][0] = c;
	m.v[0][1] = -s;

	m.v[1][0] = s;
	m.v[1][1] = c;

	return m;
}

function GetRotationMatrix(x, y, z) {
	var sin_x = Math.sin(x);
	var cos_x = Math.cos(x);

	var sin_y = Math.sin(y);
	var cos_y = Math.cos(y);

	var sin_z = Math.sin(z);
	var cos_z = Math.cos(z);

	var sinx_siny = sin_x * sin_y;
	var cosx_siny = cos_x * sin_y;

	m.v[0][0] = cos_y * cos_z;
	m.v[0][1] = cos_y * sin_z;
	m.v[0][2] = -sin_y;

	m.v[1][0] = (sinx_siny * cos_z) - (cos_x * sin_z);
	m.v[1][1] = (sinx_siny * sin_z) + (cos_x * cos_z);
	m.v[1][2] = (cosx_siny * sin_z) - (sin_x * cos_z);

	m.v[2][0] = (cosx_siny * cos_z) + (sin_x * sin_z);
	m.v[2][1] = sin_x * cos_y;
	m.v[2][2] = cos_x * cos_y;

	m.t[0] = m.t[1] = m.t[2] = 0;

	return m;
}

function GetAlignMatrix(m, xfront, yfront, zfront, xup, yup, zup) {
	xfront = -xfront;
	yfront = -yfront;
	zfront = -zfront;

	var nfront = NormalizeVector(xfront, yfront, zfront);
	var right = CrossProduct(xup, yup, zup, nfront[0], nfront[1], nfront[2]);
	var nright = NormalizeVector(right[0], right[1], right[2]);
	var up = CrossProduct(nfront[0], nfront[1], nfront[2], nright[0], nright[1], nright[2]);
	/* No need to normalize up here, since right and front are perpendicular and normalized. */

	m.v[0][0] = nright[0];
	m.v[0][1] = up[0];
	m.v[0][2] = nfront[0];

	m.v[1][0] = nright[1];
	m.v[1][1] = up[1];
	m.v[1][2] = nfront[1];

	m.v[2][0] = nright[2];
	m.v[2][1] = up[2];
	m.v[2][2] = nfront[2];

	m.t[0] = m.t[1] = m.t[2] = 0;
}

function GetVectorRotationMatrix(m, x, y, z, a) {
	var aTmp = a * Math.PI / 128.0;
	var s = Math.sin(aTmp);
	var c = Math.cos(aTmp);
	var cc = 1 - c;
	var norm = NormalizeVector(x, y, z);
	x = norm[0];
	y = norm[1];
	z = norm[2];

	m.v[0][0] = (cc * x * x) + c;
	m.v[0][1] = (cc * x * y) + (z * s);
	m.v[0][2] = (cc * x * z) - (y * s);

	m.v[1][0] = (cc * x * y) - (z * s);
	m.v[1][1] = (cc * y * y) + c;
	m.v[1][2] = (cc * z * y) + (x * s);

	m.v[2][0] = (cc * x * z) + (y * s);
	m.v[2][1] = (cc * y * z) - (x * s);
	m.v[2][2] = (cc * z * z) + c;

	m.t[0] = m.t[1] = m.t[2] = 0;
}

function GetTransformationMatrix(m, scale, xrot, yrot, zrot, x, y, z) {
	var sin_x = Math.sin(x);
	var cos_x = Math.cos(x);

	var sin_y = Math.sin(y);
	var cos_y = Math.cos(y);

	var sin_z = Math.sin(z);
	var cos_z = Math.cos(z);

	var sinx_siny = sin_x * sin_y;
	var cosx_siny = cos_x * sin_y;

	m.v[0][0] = (cos_y * cos_z) * scale;
	m.v[0][1] = (cos_y * sin_z) * scale;
	m.v[0][2] = (-sin_y) * scale;

	m.v[1][0] = ((sinx_siny * cos_z) - (cos_x * sin_z)) * scale;
	m.v[1][1] = ((sinx_siny * sin_z) + (cos_x * cos_z)) * scale;
	m.v[1][2] = (sin_x * cos_y) * scale;

	m.v[2][0] = ((cosx_siny * cos_z) + (sin_x * sin_z)) * scale;
	m.v[2][1] = ((cosx_siny * sin_z) - (sin_x * cos_z)) * scale;
	m.v[2][2] = (cos_x * cos_y) * scale;

	m.t[0] = x;
	m.t[1] = y;
	m.t[2] = z;
}

function GetCameraMatrix(m, x, y, z, xfront, yfront, zfront, xup, yup, zup, fov, aspect) {
	/* make 'in-front' into a unit vector, and negate it */
	var nfront = NormalizeVector(xfront, yfront, zfront);
	xfront = -nfront[0];
	yfront = -nfront[1];
	zfront = -nfront[2];

	/* make sure 'up' is at right angles to 'in-front', and normalize */
	d = DotProduct(xup, yup, zup, xfront, yfront, zfront);
	xup -= d * xfront;
	yup -= d * yfront;
	zup -= d * zfront;
	var nup = NormalizeVector(xup, yup, zup);

	/* calculate the 'sideways' vector */
	var side = CrossProduct(nup[0], nup[1], nup[2], xfront, yfront, zfront);

	/* set matrix rotation parameters */
	var camera = GetEmptyMatrix();
	camera.v[0][0] = side[0];
	camera.v[0][1] = side[1];
	camera.v[0][2] = side[2];

	camera.v[1][0] = nup[0];
	camera.v[1][1] = nup[1];
	camera.v[1][2] = nup[2]

	camera.v[2][0] = nfront[0];
	camera.v[2][1] = nfront[1];
	camera.v[2][2] = nfront[2];

	/* set matrix translation parameters */
	camera.t[0] = -(x * side[0] + y * side[1] + z * side[2]);
	camera.t[1] = -(x * up[0] + y * up[1] + z * up[2]);
	camera.t[2] = -(x * nfront[0] + y * nfront[1] + z * nfront[2]);

	/* construct a scaling matrix to deal with aspect ratio and FOV */
	var width = Math.tan(64.0 - fov / 2);
	var scale = GetScalingMatrix(width, -aspect * width, -1.0);

	/* combine the camera and scaling matrices */
	return MatrixMul(camera, scale);
}

function QTranslateMatrix(m, x, y, z) {
	m.t[0] += x;
	m.t[1] += y;
	m.t[2] += z;
}

function QScaleMatrix(m, scale) {
	for (var i = 0; i < 3; i++) {
		for (var j = 0; j < 3; j++) {
			m.v[i][j] *= scale;
		}
	}
}

function MatrixMul(m1, m2) {
	var out = GetEmptyMatrix();

	for (var i = 0; i < 3; i++) {
		for (var j = 0; j < 3; j++) {
			out.v[i][j] = (m1.v[0][j] * m2.v[i][0]) +
				(m1.v[1][j] * m2.v[i][1]) +
				(m1.v[2][j] * m2.v[i][2]);
		}

		out.t[i] = (m1.t[0] * m2.v[i][0]) +
			(m1.t[1] * m2.v[i][1]) +
			(m1.t[2] * m2.v[i][2]) +
			m2.t[i];
	}
	return out;
}

function VectorLength_f(x, y, z) {
	return Math.sqrt(x * x + y * y + z * z);
}

function NormalizeVector(x, y, z) {
	var length = 1.0 / VectorLength(x, y, z);

	x *= length;
	y *= length;
	z *= length;

	return [x, y, z, 0, 0, 0];
}

function DotProduct(x1, y1, z1, x2, y2, z2) {
	return (x1 * x2) + (y1 * y2) + (z1 * z2);
}

function CrossProduct(x1, y1, z1, x2, y2, z2) {
	var xout = (y1 * z2) - (z1 * y2);
	var yout = (z1 * x2) - (x1 * z2);
	var zout = (x1 * y2) - (y1 * x2);

	return [xout, yout, zout, 0, 0, 0];
}

function PolygonZNormal(v1, v2, v3) {
	return ((v2[0] - v1[0]) * (v3[1] - v2[1])) - ((v3[0] - v2[0]) * (v2[1] - v1[1]));
}

function ApplyMatrix(m, x, y, z) {
	var xout = (x * m.v[0][0] + y * m.v[0][1] + z * m.v[0][2] + m.t[0]);
	var yout = (x * m.v[1][0] + y * m.v[1][1] + z * m.v[1][2] + m.t[1]);
	var zout = (x * m.v[2][0] + y * m.v[2][1] + z * m.v[2][2] + m.t[2]);

	return [xout, yout, zout, 0, 0, 0];
}

