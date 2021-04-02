/*
MIT License

Copyright (c) 2019-2021 Andre Seidelt <superilu@yahoo.com>

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

/** 
 * **Note: al3d module must be loaded by calling LoadLibrary("al3d") before using!**
 * 
 * @module a3d
 */

/**
 * polytype definition.
 * @property {*} POLYTYPE.FLAT A simple flat shaded polygon, taking the color from the `c' value of the first vertex.
 * @property {*} POLYTYPE.GCOL A single-color gouraud shaded polygon. The colors for each vertex are taken from the `c' value, and interpolated across the polygon.
 * @property {*} POLYTYPE.GRGB A gouraud shaded polygon which interpolates RGB triplets rather than a single color.
 * @property {*} POLYTYPE.ATEX An affine texture mapped polygon. This stretches the texture across the polygon with a simple 2d linear interpolation, which is fast but not mathematically correct. It can look OK if the polygon is fairly small or flat-on to the camera, but because it doesn't deal with perspective foreshortening, it can produce strange warping artifacts.
 * @property {*} POLYTYPE.PTEX A perspective-correct texture mapped polygon. This uses the `z' value from the vertex structure as well as the u/v coordinates, so textures are displayed correctly regardless of the angle they are viewed from.
 * @property {*} POLYTYPE.ATEX_MASK Like POLYTYPE_ATEX and POLYTYPE_PTEX, but zero texture map pixels are skipped, allowing parts of the texture map to be transparent. 
 * @property {*} POLYTYPE.PTEX_MASK Like POLYTYPE_ATEX and POLYTYPE_PTEX, but zero texture map pixels are skipped, allowing parts of the texture map to be transparent. 
 * @property {*} POLYTYPE.ATEX_LIT Like POLYTYPE_ATEX and POLYTYPE_PTEX, but the blender function is used to blend the texture with a light level taken from the `c' value in the vertex structure. 
 * @property {*} POLYTYPE.PTEX_LIT Like POLYTYPE_ATEX and POLYTYPE_PTEX, but the blender function is used to blend the texture with a light level taken from the `c' value in the vertex structure. 
 * @property {*} POLYTYPE.ATEX_MASK_LIT Like POLYTYPE_ATEX_LIT and POLYTYPE_PTEX_LIT, but zero texture map pixels are skipped, allowing parts of the texture map to be transparent. 
 * @property {*} POLYTYPE.PTEX_MASK_LIT Like POLYTYPE_ATEX_LIT and POLYTYPE_PTEX_LIT, but zero texture map pixels are skipped, allowing parts of the texture map to be transparent. 
 * @property {*} POLYTYPE.ATEX_TRANS Render translucent textures. All the general rules for drawing translucent things apply. 
 * @property {*} POLYTYPE.PTEX_TRANS Render translucent textures. All the general rules for drawing translucent things apply. 
 * @property {*} POLYTYPE.ATEX_MASK_TRANS Like POLYTYPE_ATEX_TRANS and POLYTYPE_PTEX_TRANS, but zero texture map pixels are skipped. 
 * @property {*} POLYTYPE.PTEX_MASK_TRANS Like POLYTYPE_ATEX_TRANS and POLYTYPE_PTEX_TRANS, but zero texture map pixels are skipped. 
 * @property {*} POLYTYPE.ZBUF OR this into POLYTYPE and the normal polygon3d(), polygon3d_f(), quad3d(), etc. functions will render z-buffered polygons.
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

/**
 * Use to create an empty matrix.
 * 
 * @returns an empty matrix and empty translation {@link Matrix}.
 */
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

/**
 * Return the identity matrix the 'do nothing' identity matrix. Multiplying by the identity matrix has no effect.
 * 
 * @returns a {@link Matrix}.
 */
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

/**
 * Constructs a translation matrix. When applied to the point (px, py, pz), this matrix will produce the point (px+x, py+y, pz+z). 
 * In other words, it moves things sideways.
 * 
 * @param {*} x x value or a vector as array.
 * @param {number} y y value.
 * @param {number} z y value.
 * 
 * @returns a {@link Matrix}.
 */
function GetTranslationMatrix(x, y, z) {
	if (x instanceof Array) {
		z = x[2] || 0;
		y = x[1] || 0;
		x = x[0] || 0;
	}

	var m = GetIdentityMatrix();

	m.t[0] = x;
	m.t[1] = y;
	m.t[2] = z;

	return m;
}

/**
 * Constructs a scaling matrix. When applied to the point (px, py, pz), this matrix will produce the point (px*x, py*y, pz*z). 
 * In other words, it stretches or shrinks things. 
 * 
 * @param {*} x x value or a vector as array.
 * @param {number} y y value.
 * @param {number} z y value.
 * 
 * @returns a {@link Matrix}.
 */
function GetScalingMatrix(x, y, z) {
	if (x instanceof Array) {
		z = x[2] || 0;
		y = x[1] || 0;
		x = x[0] || 0;
	}
	var m = GetIdentityMatrix();

	m.v[0][0] = x;
	m.v[1][1] = y;
	m.v[2][2] = z;

	return m;
}

/**
 * Construct X axis rotation matrices. When applied to a point, these matrices will rotate it about the X axis by the specified angle (given in radians).
 * 
 * @param {number} r rotation in radians.
 * 
 * @returns a {@link Matrix}.
 */
function NGetXRotateMatrix(r) {
	var c = Math.cos(r);
	var s = Math.sin(r);

	m = GetIdentityMatrix();

	m.v[1][1] = c;
	m.v[1][2] = -s;

	m.v[2][1] = s;
	m.v[2][2] = c;

	return m;
}

/**
 * Construct Y axis rotation matrices. When applied to a point, these matrices will rotate it about the Y axis by the specified angle (given in radians).
 * 
 * @param {number} r rotation in radians.
 * 
 * @returns a {@link Matrix}.
 */
function NGetYRotateMatrix(r) {
	var c = Math.cos(r);
	var s = Math.sin(r);

	m = GetIdentityMatrix();

	m.v[0][0] = c;
	m.v[0][2] = s;

	m.v[2][0] = -s;
	m.v[2][2] = c;

	return m;
}

/**
 * Construct Z axis rotation matrices. When applied to a point, these matrices will rotate it about the Z axis by the specified angle (given in radians).
 * 
 * @param {number} r rotation in radians.
 * 
 * @returns a {@link Matrix}.
 */
function NGetZRotateMatrix(r) {
	var c = Math.cos(r);
	var s = Math.sin(r);

	m = GetIdentityMatrix();

	m.v[0][0] = c;
	m.v[0][1] = -s;

	m.v[1][0] = s;
	m.v[1][1] = c;

	return m;
}

/**
 * Constructs a transformation matrix which will rotate points around all three axes by the specified amounts (given in radians). 
 * The direction of rotation can simply be found out with the right-hand rule: Point the dumb of your right hand towards the origin along the axis of rotation, and the fingers will curl in the positive direction of rotation. 
 * E.g. if you rotate around the y axis, and look at the scene from above, a positive angle will rotate in clockwise direction. 
 * 
 * @param {*} x x value or a vector as array.
 * @param {number} y y value.
 * @param {number} z y value.
 * 
 * @returns a {@link Matrix}.
 */
function NGetRotationMatrix(x, y, z) {
	if (x instanceof Array) {
		z = x[2] || 0;
		y = x[1] || 0;
		x = x[0] || 0;
	}

	var sin_x = Math.sin(x);
	var cos_x = Math.cos(x);

	var sin_y = Math.sin(y);
	var cos_y = Math.cos(y);

	var sin_z = Math.sin(z);
	var cos_z = Math.cos(z);

	var sinx_siny = sin_x * sin_y;
	var cosx_siny = cos_x * sin_y;

	m = GetIdentityMatrix();

	m.v[0][0] = cos_y * cos_z;
	m.v[0][1] = cos_y * sin_z;
	m.v[0][2] = -sin_y;

	m.v[1][0] = (sinx_siny * cos_z) - (cos_x * sin_z);
	m.v[1][1] = (sinx_siny * sin_z) + (cos_x * cos_z);
	m.v[1][2] = (sin_x * cos_y);

	m.v[2][0] = (cosx_siny * cos_z) + (sin_x * sin_z);
	m.v[2][1] = (cosx_siny * sin_z) - (sin_x * cos_z);
	m.v[2][2] = cos_x * cos_y;

	m.t[0] = m.t[1] = m.t[2] = 0;

	return m;
}

/**
 * Rotates a matrix so that it is aligned along the specified coordinate vectors (they need not be normalized or perpendicular, but the up and front must not be equal). 
 * A front vector of 0,0,-1 and up vector of 0,1,0 will return the identity matrix.
 * 
 * @param {number} xfront 
 * @param {number} yfront 
 * @param {number} zfront 
 * @param {number} xup 
 * @param {number} yup 
 * @param {number} zup 
 * 
 * @returns a {@link Matrix}.
 */
function GetAlignMatrix(xfront, yfront, zfront, xup, yup, zup) {
	xfront = -xfront;
	yfront = -yfront;
	zfront = -zfront;

	var nfront = NormalizeVector(xfront, yfront, zfront);
	var right = CrossProduct(xup, yup, zup, nfront[0], nfront[1], nfront[2]);
	var nright = NormalizeVector(right[0], right[1], right[2]);
	var up = CrossProduct(nfront[0], nfront[1], nfront[2], nright[0], nright[1], nright[2]);
	/* No need to normalize up here, since right and front are perpendicular and normalized. */

	m = GetIdentityMatrix();

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

	return m;
}

/**
 * Constructs a transformation matrix which will rotate points around the specified x,y,z vector by the specified angle (given in radians). 
 * 
 * @param {*} x x value or a vector as array.
 * @param {number} y y value.
 * @param {number} z y value.
 * @param {number} a rotation value.
 * 
 * @returns a {@link Matrix}.
 */
function GetVectorRotationMatrix(x, y, z, a) {
	if (x instanceof Array) {
		a = y || 0;

		z = x[2] || 0;
		y = x[1] || 0;
		x = x[0] || 0;
	}

	var s = Math.sin(a);
	var c = Math.cos(a);
	var cc = 1 - c;
	var norm = NormalizeVector(x, y, z);
	x = norm[0];
	y = norm[1];
	z = norm[2];

	m = GetIdentityMatrix();

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

	return m;
}

/**
 * Constructs a transformation matrix which will rotate points around all three axes by the specified amounts (given in radians), scale the result by the specified amount (pass 1 for no change of scale), and then translate to the requested x, y, z position.
 * 
 * @param {number} scale scaling value.
 * @param {number} xrot x-rotation value.
 * @param {number} yrot y-rotation value.
 * @param {number} zrot z-rotation value.
 * @param {number} x x value.
 * @param {number} y y value.
 * @param {number} z y value.
 * 
 * @returns a {@link Matrix}.
 */
function NGetTransformationMatrix(scale, xrot, yrot, zrot, x, y, z) {
	var sin_x = Math.sin(xrot);
	var cos_x = Math.cos(xrot);

	var sin_y = Math.sin(yrot);
	var cos_y = Math.cos(yrot);

	var sin_z = Math.sin(zrot);
	var cos_z = Math.cos(zrot);

	var sinx_siny = sin_x * sin_y;
	var cosx_siny = cos_x * sin_y;

	m = GetIdentityMatrix();

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

	return m;
}

/**
 * Constructs a camera matrix for translating world-space objects into a normalised view space, ready for the perspective projection. 
 * The x, y, and z parameters specify the camera position, xfront, yfront, and zfront are the 'in front' vector specifying which way the camera is facing (this can be any length: normalisation is not required), and xup, yup, and zup are the 'up' direction vector.
 * The fov parameter specifies the field of view (ie. width of the camera focus) in radians. 
 * For typical projections, a field of view in the region 32-48 will work well. 64 (90°) applies no extra scaling - so something which is one unit away from the viewer will be directly scaled to the viewport. 
 * A bigger FOV moves you closer to the viewing plane, so more objects will appear. A smaller FOV moves you away from the viewing plane, which means you see a smaller part of the world.
 * Finally, the aspect ratio is used to scale the Y dimensions of the image relative to the X axis, so you can use it to adjust the proportions of the output image (set it to 1 for no scaling - but keep in mind that the projection also performs scaling according to the viewport size). 
 * Typically, you will pass (float)w/(float)h, where w and h are the parameters you passed to set_projection_viewport. 
 * 
 * @param {number} x x camera position.
 * @param {number} y y camera position.
 * @param {number} z y camera position.
 * @param {number} xfront x camera facing.
 * @param {number} yfront y camera facing.
 * @param {number} zfront z camera facing.
 * @param {number} xup x of 'up direction'.
 * @param {number} yup y of 'up direction'.
 * @param {number} zup z of 'up direction'.
 * @param {number} fov field of view in radians.
 * @param {number} aspect aspect ratio.
 * 
 * @returns {Matrix} a {@link Matrix}.
 */
function GetCameraMatrix(x, y, z, xfront, yfront, zfront, xup, yup, zup, fov, aspect) {
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
	var width = Math.tan(Math.PI / 2 - fov); // TODO: richtig?
	var scale = GetScalingMatrix(width, -aspect * width, -1.0);

	/* combine the camera and scaling matrices */
	return MatrixMul(camera, scale);
}

/**
 * Optimised routine for translating an already generated matrix: this simply adds in the translation offset, so there is no need to build two temporary matrices and then multiply them together. 
 * 
 * @param {number} m a {@link Matrix}.
 * @param {number} x x-offset or a vector as array.
 * @param {number} y y-offset.
 * @param {number} z z-offset.
 */
function QTranslateMatrix(m, x, y, z) {
	if (x instanceof Array) {
		z = x[2] || 0;
		y = x[1] || 0;
		x = x[0] || 0;
	}

	m.t[0] += x;
	m.t[1] += y;
	m.t[2] += z;
}

/**
 * Optimised routine for scaling an already generated matrix: this simply adds in the scale factor, so there is no need to build two temporary matrices and then multiply them together.
 * 
 * @param {number} m a {@link Matrix}.
 * @param {number} scale scale factor
 */
function QScaleMatrix(m, scale) {
	for (var i = 0; i < 3; i++) {
		for (var j = 0; j < 3; j++) {
			m.v[i][j] *= scale;
		}
	}
}

/**
 * Multiplies two matrices. 
 * The resulting matrix will have the same effect as the combination of m1 and m2, ie. when applied to a point p, (p * out) = ((p * m1) * m2). 
 * Any number of transformations can be concatenated in this way. 
 * Note that matrix multiplication is not commutative, ie. matrix_mul(m1, m2) != matrix_mul(m2, m1). 
 * 
 * @param {Matrix} m1 first {@link Matrix}.
 * @param {Matrix} m2 second {@link Matrix}.
 * 
 * @returns a new {@link Matrix}.
 */
function NMatrixMul(m1, m2) {
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

/**
 * Calculates the length of the vector (x, y, z), using that good 'ole Pythagoras theorem.
 * 
 * @param {*} x x value or vector as array.
 * @param {number} y y value.
 * @param {number} z y value.
 * 
 * @returns {number} vector length.
 */
function VectorLength(x, y, z) {
	if (x instanceof Array) {
		z = x[2] || 0;
		y = x[1] || 0;
		x = x[0] || 0;
	}

	return Math.sqrt(x * x + y * y + z * z);
}

/**
 * Converts the vector (x, y, z) to a unit vector. 
 * This points in the same direction as the original vector, but has a length of one. 
 * 
 * @param {*} x x value or vector as array.
 * @param {number} y y value.
 * @param {number} z y value.
 * 
 * @returns {number[]} a new vector.
 */
function NormalizeVector(x, y, z) {
	var u = 0;
	var v = 0;
	var c = 0;
	if (x instanceof Array) {
		c = x[5] || 0;
		v = x[4] || 0;
		u = x[3] || 0;
		z = x[2] || 0;
		y = x[1] || 0;
		x = x[0] || 0;
	}

	var length = 1.0 / VectorLength(x, y, z);

	x *= length;
	y *= length;
	z *= length;

	return [x, y, z, u, v, c];
}

/**
 * Calculates the dot product (x1, y1, z1) . (x2, y2, z2), returning the result.
 * 
 * @param {*} x1 x value or first vector as array.
 * @param {*} y1 y value or second vector as array.
 * @param {number} z1 z value.
 * @param {number} x2 x value.
 * @param {number} y2 y value.
 * @param {number} z2 z value.
 * 
 * @returns {number} dot product.
 */
function DotProduct(x1, y1, z1, x2, y2, z2) {
	if (x1 instanceof Array) {
		z1 = x1[2] || 0;
		y1 = x1[1] || 0;
		x1 = x1[0] || 0;

		z2 = y1[2] || 0;
		y2 = y1[1] || 0;
		x2 = y1[0] || 0;
	}

	return (x1 * x2) + (y1 * y2) + (z1 * z2);
}

/**
 * Calculates the cross product (x1, y1, z1) x (x2, y2, z2). 
 * The cross product is perpendicular to both of the input vectors, so it can be used to generate polygon normals. 
 * 
 * @param {*} x1 x value or first vector as array.
 * @param {*} y1 y value or second vector as array.
 * @param {number} z1 z value.
 * @param {number} x2 x value.
 * @param {number} y2 y value.
 * @param {number} z2 z value.
 * 
 * @returns {number[]} a new vector.
 */
function CrossProduct(x1, y1, z1, x2, y2, z2) {
	if (x1 instanceof Array) {
		z1 = x1[2] || 0;
		y1 = x1[1] || 0;
		x1 = x1[0] || 0;

		z2 = y1[2] || 0;
		y2 = y1[1] || 0;
		x2 = y1[0] || 0;
	}

	var xout = (y1 * z2) - (z1 * y2);
	var yout = (z1 * x2) - (x1 * z2);
	var zout = (x1 * y2) - (y1 * x2);

	return [xout, yout, zout, 0, 0, 0];
}

/**
 * Finds the Z component of the normal vector to the specified three vertices (which must be part of a convex polygon). 
 * This is used mainly in back-face culling. The back-faces of closed polyhedra are never visible to the viewer, therefore they never need to be drawn. 
 * This can cull on average half the polygons from a scene. 
 * If the normal is negative the polygon can safely be culled. If it is zero, the polygon is perpendicular to the screen.
 * However, this method of culling back-faces must only be used once the X and Y coordinates have been projected into screen space using PerspProject() (or if an orthographic (isometric) projection is being used). 
 * Note that this function will fail if the three vertices are co-linear (they lie on the same line) in 3D space. 
 * 
 * @param {number[]} v1 first vector.
 * @param {number[]} v2 second vector.
 * @param {number[]} v3 third vector.
 * 
 * @returns {number} z component.
 */
function NPolygonZNormal(v1, v2, v3) {
	return ((v2[0] - v1[0]) * (v3[1] - v2[1])) - ((v3[0] - v2[0]) * (v2[1] - v1[1]));
}

/**
 * Multiplies the point (x, y, z) by the transformation matrix m.
 * 
 * @param {Matrix} m the matrix
 * @param {*} x x value or vector as array.
 * @param {number} y y value.
 * @param {number} z y value.
 * 
 * @returns {number[]} a new vector.
 */
function NApplyMatrix(m, x, y, z) {
	var u = 0;
	var v = 0;
	var c = 0;
	if (x instanceof Array) {
		c = x[5] || 0;
		v = x[4] || 0;
		u = x[3] || 0;
		z = x[2] || 0;
		y = x[1] || 0;
		x = x[0] || 0;
	}

	var xout = (x * m.v[0][0] + y * m.v[0][1] + z * m.v[0][2] + m.t[0]);
	var yout = (x * m.v[1][0] + y * m.v[1][1] + z * m.v[1][2] + m.t[1]);
	var zout = (x * m.v[2][0] + y * m.v[2][1] + z * m.v[2][2] + m.t[2]);

	return [xout, yout, zout, u, v, c];
}

/** scaling factor x for the perspective projection */
var _persp_xscale_f = 160.0;
/** scaling factor y for the perspective projection */
var _persp_yscale_f = 100.0;
/** scaling factor x offset for the perspective projection */
var _persp_xoffset_f = 160.0;
/** scaling factor y offset for the perspective projection */
var _persp_yoffset_f = 100.0;

/**
 * Sets the viewport used to scale the output of the PerspProject() function. 
 * Pass the dimensions of the screen area you want to draw onto, which will typically be 0, 0, SizeX(), and SizeY(). 
 * Also don't forget to pass an appropriate aspect ratio to GetCameraMatrix() later. 
 * The width and height you specify here will determine how big your viewport is in 3d space. 
 * So if an object in your 3D space is w units wide, it will fill the complete screen when you run into it (i.e., if it has a distance of 1.0 after the camera matrix was applied. 
 * The fov and aspect-ratio parameters to get_camera_matrix also apply some scaling though, so this isn't always completely true). 
 * If you pass -1/-1/2/2 as parameters, no extra scaling will be performed by the projection. 
 */
function SetProjectionViewport(x, y, w, h) {
	_persp_xscale_f = w / 2;
	_persp_yscale_f = h / 2;
	_persp_xoffset_f = x + w / 2;
	_persp_yoffset_f = y + h / 2;
	_SetProjectionViewport(x, y, w, h);
}

/*
 * Projects the 3d point (x, y, z) into 2d screen space and using the scaling parameters previously set by calling SetProjectionViewport(). 
 * This function projects from the normalized viewing pyramid, which has a camera at the origin and facing along the positive z axis. 
 * The x axis runs left/right, y runs up/down, and z increases with depth into the screen. 
 * The camera has a 90 degree field of view, ie. points on the planes x=z and -x=z will map onto the left and right edges of the screen, and the planes y=z and -y=z map to the top and bottom of the screen. 
 * If you want a different field of view or camera location, you should transform all your objects with an appropriate viewing matrix, eg. to get the effect of panning the camera 10 degrees to the left, rotate all your objects 10 degrees to the right. 
 * 
 * @param {*} x x value or vector as array.
 * @param {number} y y value.
 * @param {number} z y value.
 */
function NPerspProject(x, y, z) {
	var u = 0;
	var v = 0;
	var c = 0;
	if (x instanceof Array) {
		c = x[5] || 0;
		v = x[4] || 0;
		u = x[3] || 0;
		z = x[2] || 0;
		y = x[1] || 0;
		x = x[0] || 0;
	}

	var z1 = 1.0 / z;
	var xout = ((x * z1) * _persp_xscale_f) + _persp_xoffset_f;
	var yout = ((y * z1) * _persp_yscale_f) + _persp_yoffset_f;

	return [xout, yout, z, u, v, c];
}

/**
 * Allocates memory for a scene, `nedge' and `npoly' are your estimates of how many edges and how many polygons you will render (you cannot get over the limit specified here).
 * 
 * @param {number} nedge max number of edges.
 * @param {number} npoly max number of polygons.
 */
function CreateScene(nedge, npoly) {
	_bitmap_array = [];
	_CreateScene(nedge, npoly);
}

/**
 * Puts a polygon in the rendering list. Nothing is really rendered at this moment. Should be called between ClearScene() and RenderScene().
 * 
 * Arguments are the same as for Polygon3D().
 * 
 * Unlike Polygon3D(), the polygon may be concave or self-intersecting. Shapes that penetrate one another may look OK, but they are not really handled by this code. 
 * 
 * @param {*} type 
 * @param {*} texture 
 * @param {*} vtx 
 */
function ScenePolygon3D(type, texture, vtx) {
	_bitmap_array.push(texture);	// make sure texture bitmap is not garbage collected until RenderScene() has been called!
	_ScenePolygon3D(type, texture, vtx);
}

/**
 * Renders all the specified ScenePolygon3D()'s on the current bitmap. Rendering is done one scanline at a time, with no pixel being processed more than once.
 * 
 * Note that between ClearScene() and RenderScene() you shouldn't change the clip rectangle of the destination bitmap. For speed reasons, you should set the clip rectangle to the minimum. 
 */
function RenderScene() {
	_RenderScene();
	_bitmap_array = [];
}

/**
 * Initializes a scene.
 */
function ClearScene() {
	_ClearScene();
	_bitmap_array = [];
}

/**
 * Deallocate memory previously allocated by CreateScene. Use this to avoid memory leaks in your program.
 */
function DestroyScene() {
	_DestroyScene();
	_bitmap_array = [];
}
