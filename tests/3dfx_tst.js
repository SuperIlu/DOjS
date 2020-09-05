/*
 * helping functions for 3dfx_XX.js
 */

// predefined colors with full alpha
exports.FX_RED = 0xFFFF0000;
exports.FX_GREEN = 0xFF00FF00;
exports.FX_BLUE = 0xFF0000FF;
exports.FX_BLACK = 0xFF000000;
exports.FX_WHITE = 0xFFFFFFFF;
exports.FX_GREY = 0x00808080;

// predefined colors with transparent alpha
exports.FX_BLUE_TRANS = 0x800000FF;

/**
 * scale 0..1 coordinate to screen width
 */
exports.scaleX = function (Xin) {
	return FX_WIDTH * Xin;
}

/**
 * scale 0..1 coordinate to screen height
 */
exports.scaleY = function (Yin) {
	return FX_HEIGHT * Yin;
}

/**
 * create ready to use vertex arrays
 */
exports.createVertices = function () {
	v1 = FxEmptyVertex();
	v2 = FxEmptyVertex();
	v3 = FxEmptyVertex();
	v4 = FxEmptyVertex();
}

var DEGREE = (.01745328);
var VP_OFFSET = 1.0;
var VP_SCALE = 0.5;

var currentMatrix = [
	[0, 0, 0, 0],
	[0, 0, 0, 0],
	[0, 0, 0, 0],
	[0, 0, 0, 0]
];

var matrixStack = [];

exports.PushMatrix = function () {
	matrixStack.push([
		currentMatrix[0].slice(0),
		currentMatrix[1].slice(0),
		currentMatrix[2].slice(0),
		currentMatrix[3].slice(0)
	]);
};

exports.PopMatrix = function () {
	currentMatrix = matrixStack.pop();
};

/**
 * dump current matrix
 */
exports.DumpMatrix = function () {
	Println("m=" + JSON.stringify(currentMatrix));
}

/**
 * create identity matrix
 */
exports.Identity = function () {
	return [
		[1, 0, 0, 0],
		[0, 1, 0, 0],
		[0, 0, 1, 0],
		[0, 0, 0, 1]
	];
}

/**
 * create x-rotation matrix.
 */
exports.XRotation = function (deg) {
	var c = Math.cos(deg * DEGREE);
	var s = Math.sin(deg * DEGREE);
	return [
		[1, 0, 0, 0],
		[0, c, s, 0],
		[0, -s, c, 0],
		[0, 0, 0, 1]
	];
}

/**
 * create y-rotation matrix.
 */
exports.YRotation = function (deg) {
	var c = Math.cos(deg * DEGREE);
	var s = Math.sin(deg * DEGREE);
	return [
		[c, 0, -s, 0],
		[0, 1, 0, 0],
		[s, 0, c, 0],
		[0, 0, 0, 1]
	];
}

/**
 * create z-rotation matrix.
 */
exports.ZRotation = function (deg) {
	var c = Math.cos(deg * DEGREE);
	var s = Math.sin(deg * DEGREE);
	return [
		[c, s, 0, 0],
		[-s, c, 0, 0],
		[0, 0, 1, 0],
		[0, 0, 0, 1]
	];
}

/**
 * create translation matrix
 */
exports.Translation = function (x, y, z) {
	return [
		[1, 0, 0, 0],
		[0, 1, 0, 0],
		[0, 0, 1, 0],
		[x, y, z, 1]
	];
}

/**
 * set current matrix
 */
exports.SetMatrix = function (m) {
	currentMatrix = m;
}

/**
 * get current matrix
 */
exports.GetMatrix = function () {
	return currentMatrix;
}

/**
 * multiply current matrix with m
 */
exports.MultMatrix = function (m) {
	var result = [
		[0, 0, 0, 0],
		[0, 0, 0, 0],
		[0, 0, 0, 0],
		[0, 0, 0, 0]
	];
	for (var j = 0; j < 4; j++) {
		for (var i = 0; i < 4; i++) {
			result[j][i] =
				currentMatrix[j][0] * m[0][i] +
				currentMatrix[j][1] * m[1][i] +
				currentMatrix[j][2] * m[2][i] +
				currentMatrix[j][3] * m[3][i];
		}
	}

	currentMatrix = result;
}

/**
 * Multiply list of vertices w/ current matrix. Expects vertices of the format [x, y, z, w, s, t]
 */
exports.TransformVertices = function (srcVerts) {
	var m = currentMatrix;
	var ret = [];
	for (var i = 0; i < srcVerts.length; i++) {
		var v = srcVerts[i];
		ret.push([
			v[0] * m[0][0] + v[1] * m[1][0] + v[2] * m[2][0] + v[3] * m[3][0],
			v[0] * m[0][1] + v[1] * m[1][1] + v[2] * m[2][1] + v[3] * m[3][1],
			v[0] * m[0][2] + v[1] * m[1][2] + v[2] * m[2][2] + v[3] * m[3][2],
			v[0] * m[0][3] + v[1] * m[1][3] + v[2] * m[2][3] + v[3] * m[3][3],
			v[4],
			v[5]
		]);
	}
	return ret;
}

exports.ProjectVertices = function (srcVerts) {
	/* simplified perspective proj matrix assume unit clip volume */
	var m = [
		[1, 0, 0, 0],
		[0, 1, 0, 0],
		[0, 0, 1, 1],
		[0, 0, 0, 0]
	];

	var ret = [];
	for (var i = 0; i < srcVerts.length; i++) {
		var v = srcVerts[i];
		var tmp = [
			v[0] * m[0][0] + v[1] * m[1][0] + v[2] * m[2][0] + v[3] * m[3][0],
			v[0] * m[0][1] + v[1] * m[1][1] + v[2] * m[2][1] + v[3] * m[3][1],
			v[0] * m[0][2] + v[1] * m[1][2] + v[2] * m[2][2] + v[3] * m[3][2],
			v[0] * m[0][3] + v[1] * m[1][3] + v[2] * m[2][3] + v[3] * m[3][3],
			v[4],
			v[5]
		]

		tmp[0] /= tmp[3];
		tmp[1] /= tmp[3];
		tmp[2] /= tmp[3];
		tmp[0] += VP_OFFSET;
		tmp[0] *= VP_SCALE;
		tmp[1] += VP_OFFSET;
		tmp[1] *= VP_SCALE;

		ret.push(tmp);
	}
	return ret;
}
