/** 
 * Color definition.
 * @module 3dfx */

/**
* set a bit.
* 
* @param {number} i bit to set.
* @returns {number} number with given bit set.
*/
function FXBIT(i) { return (1 << (i)) };

/**
 * Parameters for vertex definition.
 * @property {number} XY
 * @property {number} ZW
 * @property {number} Q
 * @property {number} EXT
 * @property {number} A
 * @property {number} RGB
 * @property {number} PARG
 * @property {number} ST0
 * @property {number} ST1
 * @property {number} ST2
 * @property {number} Q0
 * @property {number} Q1
 * @property {number} Q2
 */
GR_PARAM = {
	XY: 0x01,
	Z: 0x02,
	W: 0x03,
	Q: 0x04,
	EXT: 0x05,
	A: 0x10,
	RGB: 0x20,
	PARGB: 0x30,
	ST0: 0x40,
	ST1: (0x40 + 1),
	ST2: (0x40 + 2),
	Q0: 0x50,
	Q1: (0x50 + 1),
	Q2: (0x50 + 2)
};

/**
 * typedef FxI32 GrCullMode_t;
 * 
 * @property {number} DISABLE
 * @property {number} NEGATIVE
 * @property {number} POSITIVE
 */
GR_CULL = {
	DISABLE: 0x0,
	NEGATIVE: 0x1,
	POSITIVE: 0x2
};

/**
 * typedef FxI32 GrCombineFunction_t;
 * @property {number} ZERO
 * @property {number} NONE
 * @property {number} LOCAL
 * @property {number} LOCAL_ALPHA
 * @property {number} SCALE_OTHER
 * @property {number} BLEND_OTHER
 * @property {number} SCALE_OTHER_ADD_LOCAL
 * @property {number} SCALE_OTHER_ADD_LOCAL_ALPHA
 * @property {number} SCALE_OTHER_MINUS_LOCAL
 * @property {number} SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL
 * @property {number} BLEND
 * @property {number} SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL_ALPHA
 * @property {number} SCALE_MINUS_LOCAL_ADD_LOCAL
 * @property {number} BLEND_LOCAL
 * @property {number} SCALE_MINUS_LOCAL_ADD_LOCAL_ALPHA
 */
GR_COMBINE_FUNCTION = {
	ZERO: 0x0,
	NONE: 0x0,
	LOCAL: 0x1,
	LOCAL_ALPHA: 0x2,
	SCALE_OTHER: 0x3,
	BLEND_OTHER: 0x3,
	SCALE_OTHER_ADD_LOCAL: 0x4,
	SCALE_OTHER_ADD_LOCAL_ALPHA: 0x5,
	SCALE_OTHER_MINUS_LOCAL: 0x6,
	SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL: 0x7,
	BLEND: 0x7,
	SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL_ALPHA: 0x8,
	SCALE_MINUS_LOCAL_ADD_LOCAL: 0x9,
	BLEND_LOCAL: 0x9,
	SCALE_MINUS_LOCAL_ADD_LOCAL_ALPHA: 0x10
};

/**
 * typedef FxI32 GrCombineFactor_t;
 * @property {number} ZERO
 * @property {number} NONE
 * @property {number} LOCAL
 * @property {number} OTHER_ALPHA
 * @property {number} LOCAL_ALPHA
 * @property {number} TEXTURE_ALPHA
 * @property {number} TEXTURE_RGB
 * @property {number} DETAIL_FACTOR
 * @property {number} LOD_FRACTION
 * @property {number} ONE
 * @property {number} ONE_MINUS_LOCAL
 * @property {number} ONE_MINUS_OTHER_ALPHA
 * @property {number} ONE_MINUS_LOCAL_ALPHA
 * @property {number} ONE_MINUS_TEXTURE_ALPHA
 * @property {number} ONE_MINUS_DETAIL_FACTOR
 * @property {number} ONE_MINUS_LOD_FRACTION
 */
GR_COMBINE_FACTOR = {
	ZERO: 0x0,
	NONE: 0x0,
	LOCAL: 0x1,
	OTHER_ALPHA: 0x2,
	LOCAL_ALPHA: 0x3,
	TEXTURE_ALPHA: 0x4,
	TEXTURE_RGB: 0x5,
	DETAIL_FACTOR: 0x4,
	LOD_FRACTION: 0x5,
	ONE: 0x8,
	ONE_MINUS_LOCAL: 0x9,
	ONE_MINUS_OTHER_ALPHA: 0xa,
	ONE_MINUS_LOCAL_ALPHA: 0xb,
	ONE_MINUS_TEXTURE_ALPHA: 0xc,
	ONE_MINUS_DETAIL_FACTOR: 0xc,
	ONE_MINUS_LOD_FRACTION: 0xd
};

/**
 * typedef FxI32 GrCombineLocal_t;
 * @property {number} ITERATED
 * @property {number} CONSTANT
 * @property {number} NONE
 * @property {number} DEPTH
 */
GR_COMBINE_LOCAL = {
	ITERATED: 0x0,
	CONSTANT: 0x1,
	NONE: 0x1,
	DEPTH: 0x2
};

/**
 * typedef FxI32 GrCombineOther_t;
 * @property {number} ITERATED
 * @property {number} TEXTURE
 * @property {number} CONSTANT
 * @property {number} NONE
 */
GR_COMBINE_OTHER = {
	ITERATED: 0x0,
	TEXTURE: 0x1,
	CONSTANT: 0x2,
	NONE: 0x2
};

/**
 * typedef FxI32 GrAlphaBlendFnc_t;
 * @property {number} ZERO
 * @property {number} SRC_ALPHA
 * @property {number} SRC_COLOR
 * @property {number} DST_COLOR
 * @property {number} DST_ALPHA
 * @property {number} ONE
 * @property {number} ONE_MINUS_SRC_ALPHA
 * @property {number} ONE_MINUS_SRC_COLOR
 * @property {number} ONE_MINUS_DST_COLOR
 * @property {number} ONE_MINUS_DST_ALPHA
 * @property {number} RESERVED_8
 * @property {number} RESERVED_9
 * @property {number} RESERVED_A
 * @property {number} RESERVED_B
 * @property {number} RESERVED_C
 * @property {number} RESERVED_D
 * @property {number} RESERVED_E
 * @property {number} ALPHA_SATURATE
 * @property {number} PREFOG_COLOR
 */
GR_BLEND = {
	ZERO: 0x0,
	SRC_ALPHA: 0x1,
	SRC_COLOR: 0x2,
	DST_COLOR: 0x2,
	DST_ALPHA: 0x3,
	ONE: 0x4,
	ONE_MINUS_SRC_ALPHA: 0x5,
	ONE_MINUS_SRC_COLOR: 0x6,
	ONE_MINUS_DST_COLOR: 0x6,
	ONE_MINUS_DST_ALPHA: 0x7,
	RESERVED_8: 0x8,
	RESERVED_9: 0x9,
	RESERVED_A: 0xa,
	RESERVED_B: 0xb,
	RESERVED_C: 0xc,
	RESERVED_D: 0xd,
	RESERVED_E: 0xe,
	ALPHA_SATURATE: 0xf,
	PREFOG_COLOR: 0xf
};

/**
 * grDrawVertexArray/grDrawVertexArrayContiguous primitive type
 * @property {number} POINTS
 * @property {number} LINE_STRIP
 * @property {number} LINES
 * @property {number} POLYGON
 * @property {number} TRIANGLE_STRIP
 * @property {number} TRIANGLE_FAN
 * @property {number} TRIANGLES
 * @property {number} TRIANGLE_STRIP_CONTINUE
 * @property {number} TRIANGLE_FAN_CONTINUE
 */
GR_VERTEX = {
	POINTS: 0,
	LINE_STRIP: 1,
	LINES: 2,
	POLYGON: 3,
	TRIANGLE_STRIP: 4,
	TRIANGLE_FAN: 5,
	TRIANGLES: 6,
	TRIANGLE_STRIP_CONTINUE: 7,
	TRIANGLE_FAN_CONTINUE: 8
};

/**
 * typedef FxU32 GrEnableMode_t;
 * @property {number} AA_ORDERED
 * @property {number} ALLOW_MIPMAP_DITHER
 * @property {number} PASSTHRU
 * @property {number} SHAMELESS_PLUG
 * @property {number} VIDEO_SMOOTHING
 */
GR_ENABLE = {
	AA_ORDERED: 0x01,
	ALLOW_MIPMAP_DITHER: 0x02,
	PASSTHRU: 0x03,
	SHAMELESS_PLUG: 0x04,
	VIDEO_SMOOTHING: 0x05
};

/**
 * typedef FxI32 GrDitherMode_t;
 * @property {number} DISABLE
 * @property {number} D2x2
 * @property {number} D4x4
 */
GR_DITHER = {
	DISABLE: 0x0,
	D2x2: 0x1,
	D4x4: 0x2
};

/**
 * typedef FxI32 GrDepthBufferMode_t;
 * @property {number} DISABLE
 * @property {number} ZBUFFER
 * @property {number} WBUFFER
 * @property {number} ZBUFFER_COMPARE_TO_BIAS
 * @property {number} WBUFFER_COMPARE_TO_BIAS
 */
GR_DEPTHBUFFER = {
	DISABLE: 0x0,
	ZBUFFER: 0x1,
	WBUFFER: 0x2,
	ZBUFFER_COMPARE_TO_BIAS: 0x3,
	WBUFFER_COMPARE_TO_BIAS: 0x4
};

/**
 * typedef FxI32 GrCmpFnc_t;
 * @property {number} NEVER
 * @property {number} LESS
 * @property {number} EQUAL
 * @property {number} LEQUAL
 * @property {number} GREATER
 * @property {number} NOTEQUAL
 * @property {number} GEQUAL
 * @property {number} ALWAYS
 */
GR_CMP = {
	NEVER: 0x0,
	LESS: 0x1,
	EQUAL: 0x2,
	LEQUAL: 0x3,
	GREATER: 0x4,
	NOTEQUAL: 0x5,
	GEQUAL: 0x6,
	ALWAYS: 0x7
};

/**
 * typedef FxI32 GrFogMode_t;
 * @property {number} DISABLE
 * @property {number} WITH_TABLE_ON_FOGCOORD_EXT
 * @property {number} WITH_TABLE_ON_Q
 * @property {number} WITH_TABLE_ON_W
 * @property {number} WITH_ITERATED_Z
 * @property {number} MULT2
 * @property {number} ADD2
 */
GR_FOG = {
	DISABLE: 0x0,
	WITH_TABLE_ON_FOGCOORD_EXT: 0x1,
	WITH_TABLE_ON_Q: 0x2,
	WITH_TABLE_ON_W: 0x2,
	WITH_ITERATED_Z: 0x3,
	MULT2: 0x100,
	ADD2: 0x200
};

/**
 * typedef FxI32 GrChipID_t;
 * @property {number} TMU0
 * @property {number} TMU1
 * @property {number} TMU2
 */
GR_TMU = {
	TMU0: 0x0,
	TMU1: 0x1,
	TMU2: 0x2
};

/**
 * typedef FxI32 GrTextureFilterMode_t;
 * @property {number} POINT_SAMPLED
 * @property {number} BILINEAR
 */
GR_TEXTUREFILTER = {
	POINT_SAMPLED: 0x0,
	BILINEAR: 0x1
};

/**
 * typedef FxI32 GrTextureClampMode_t;
 * @property {number} WRAP
 * @property {number} CLAMP
 * @property {number} MIRROR_EXT
 */
GR_TEXTURECLAMP = {
	WRAP: 0x0,
	CLAMP: 0x1,
	MIRROR_EXT: 0x2
};

/**
 * typedef FxI32 GrMipMapMode_t;
 * @property {number} DISABLE
 * @property {number} NEAREST
 * @property {number} NEAREST_DITHER
 */
GR_MIPMAP = {
	DISABLE: 0x0, /* no mip mapping  */
	NEAREST: 0x1, /* use nearest mipmap */
	NEAREST_DITHER: 0x2 /* GR_MIPMAP_NEAREST + LOD dith */
};


/**
 * typedef FxI32 GrOriginLocation_t;
 * @property {number} UPPER_LEFT
 * @property {number} LOWER_LEFT
 * @property {number} ANY
 */
GR_ORIGIN = {
	UPPER_LEFT: 0x0,
	LOWER_LEFT: 0x1,
	ANY: 0xFF
};

/**
 * typedef FxI32 GrAspectRatio_t;
 * @property {number} LOG2_8x1
 * @property {number} LOG2_4x1
 * @property {number} LOG2_2x1
 * @property {number} LOG2_1x1
 * @property {number} LOG2_1x2
 * @property {number} LOG2_1x4
 * @property {number} LOG2_1x8
 */
GR_ASPECT = {
	LOG2_8x1: 3,    /* 8W x 1H */
	LOG2_4x1: 2,    /* 4W x 1H */
	LOG2_2x1: 1,    /* 2W x 1H */
	LOG2_1x1: 0,    /* 1W x 1H */
	LOG2_1x2: - 1,  /* 1W x 2H */
	LOG2_1x4: - 2,	/* 1W x 4H */
	LOG2_1x8: - 3   /* 1W x 8H */
};

/**
 * typedef FxI32 GrLOD_t;
 * @property {number} LOG2_256
 * @property {number} LOG2_128
 * @property {number} LOG2_64
 * @property {number} LOG2_32
 * @property {number} LOG2_16
 * @property {number} LOG2_8
 * @property {number} LOG2_4
 * @property {number} LOG2_2
 * @property {number} LOG2_1
 */
GR_LOD = {
	LOG2_256: 0x8,
	LOG2_128: 0x7,
	LOG2_64: 0x6,
	LOG2_32: 0x5,
	LOG2_16: 0x4,
	LOG2_8: 0x3,
	LOG2_4: 0x2,
	LOG2_2: 0x1,
	LOG2_1: 0x0,
};

/**
 * typedef FxI32 GrTextureFormat_t;
 * @property {number} BIT8
 * @property {number} RGB_332
 * @property {number} YIQ_422
 * @property {number} ALPHA_8
 * @property {number} INTENSITY_8
 * @property {number} ALPHA_INTENSITY_44
 * @property {number} P_8
 * @property {number} RSVD0
 * @property {number} P_8_6666
 * @property {number} P_8_6666_EXT
 * @property {number} RSVD1
 * @property {number} BIT16
 * @property {number} ARGB_8332
 * @property {number} AYIQ_8422
 * @property {number} RGB_565
 * @property {number} ARGB_1555
 * @property {number} ARGB_4444
 * @property {number} ALPHA_INTENSITY_88
 * @property {number} AP_88
 * @property {number} RSVD2
 * @property {number} RSVD4
 */
GR_TEXFMT = {
	BIT8: 0x0,
	RGB_332: 0x0,
	YIQ_422: 0x1,
	ALPHA_8: 0x2, /* (0..0xFF) alpha     */
	INTENSITY_8: 0x3, /* (0..0xFF) intensity */
	ALPHA_INTENSITY_44: 0x4,
	P_8: 0x5, /* 8-bit palette */
	RSVD0: 0x6, /* GR_TEXFMT_P_8_RGBA */
	P_8_6666: 0x6,
	P_8_6666_EXT: 0x6,
	RSVD1: 0x7,
	BIT16: 0x8,
	ARGB_8332: 0x8,
	AYIQ_8422: 0x9,
	RGB_565: 0xa,
	ARGB_1555: 0xb,
	ARGB_4444: 0xc,
	ALPHA_INTENSITY_88: 0xd,
	AP_88: 0xe, /* 8-bit alpha 8-bit palette */
	RSVD2: 0xf,
	RSVD4: 0xf
};

/**
 * typedef FxU32 GrTexTable_t;
 * @property {number} NCC0
 * @property {number} NCC1
 * @property {number} PALETTE
 * @property {number} PALETTE_6666_EXT
 */
GR_TEXTABLE = {
	NCC0: 0x0,
	NCC1: 0x1,
	PALETTE: 0x2,
	PALETTE_6666_EXT: 0x3
};

/**
 * GR_MIPMAPLEVELMASK
 * @property {number} EVEN
 * @property {number} ODD
 * @property {number} BOTH
 */
GR_MIPMAPLEVELMASK = {
	EVEN: FXBIT(0),
	ODD: FXBIT(1),
	BOTH: (FXBIT(0) | FXBIT(1))
};

/**
 * typedef FxI32 GrBuffer_t;
 * @property {number} FRONTBUFFER
 * @property {number} BACKBUFFER
 * @property {number} AUXBUFFER
 * @property {number} DEPTHBUFFER
 * @property {number} ALPHABUFFER
 * @property {number} TRIPLEBUFFER
 */
GR_BUFFER = {
	FRONTBUFFER: 0x0,
	BACKBUFFER: 0x1,
	AUXBUFFER: 0x2,
	DEPTHBUFFER: 0x3,
	ALPHABUFFER: 0x4,
	TRIPLEBUFFER: 0x5
};


/**
 * typedef FxI32 GrColorCombineFnc_t;
 * @property {number} ZERO
 * @property {number} CCRGB
 * @property {number} ITRGB
 * @property {number} ITRGB_DELTA0
 * @property {number} DECAL_TEXTURE
 * @property {number} TEXTURE_TIMES_CCRGB
 * @property {number} TEXTURE_TIMES_ITRGB
 * @property {number} TEXTURE_TIMES_ITRGB_DELTA0
 * @property {number} TEXTURE_TIMES_ITRGB_ADD_ALPHA
 * @property {number} TEXTURE_TIMES_ALPHA
 * @property {number} TEXTURE_TIMES_ALPHA_ADD_ITRGB
 * @property {number} TEXTURE_ADD_ITRGB
 * @property {number} TEXTURE_SUB_ITRGB
 * @property {number} CCRGB_BLEND_ITRGB_ON_TEXALPHA
 * @property {number} DIFF_SPEC_A
 * @property {number} DIFF_SPEC_B
 * @property {number} ONE
 */
GR_COLORCOMBINE = {
	ZERO: 0x0,
	CCRGB: 0x1,
	ITRGB: 0x2,
	ITRGB_DELTA0: 0x3,
	DECAL_TEXTURE: 0x4,
	TEXTURE_TIMES_CCRGB: 0x5,
	TEXTURE_TIMES_ITRGB: 0x6,
	TEXTURE_TIMES_ITRGB_DELTA0: 0x7,
	TEXTURE_TIMES_ITRGB_ADD_ALPHA: 0x8,
	TEXTURE_TIMES_ALPHA: 0x9,
	TEXTURE_TIMES_ALPHA_ADD_ITRGB: 0xa,
	TEXTURE_ADD_ITRGB: 0xb,
	TEXTURE_SUB_ITRGB: 0xc,
	CCRGB_BLEND_ITRGB_ON_TEXALPHA: 0xd,
	DIFF_SPEC_A: 0xe,
	DIFF_SPEC_B: 0xf,
	ONE: 0x10
};

/** hard edge when assigning texture memory at 2MiB */
var TEXMEM_2MB_EDGE = 2097152;

/** TMU memory start address/next address per TMU */
var _nextTexture = [0, 0, 0];
/** TMU memory end address per TMU */
var _lastTexture = [0, 0, 0];

/**
 * (re)initialize simple texture memory management for given TMU.
 * 
 * @param {GR_TMU} tmu the tmu to initialize.
 */
function FxTexMemInit(tmu) {
	_nextTexture[tmu] = fxTexMinAddress(tmu);
	_lastTexture[tmu] = fxTexMaxAddress(tmu);

	Debug("TMU[" + tmu + "] memory 0x" + _nextTexture[tmu].toString(16) + "..0x" + _lastTexture[tmu].toString(16));
}

/**
 * try to find the next available memory for texture.
 * 
 * @param {GR_TMU} tmu the TMU where this texture shall be used.
 * @param {TexInfo} info the texture.
 * 
 * @returns {number} a start address or null.
 */
function FxTexMemGetStartAddress(tmu, info) {
	start = _nextTexture[tmu];
	/* check for 2MB edge and space past it if necessary */
	if ((start < TEXMEM_2MB_EDGE) && (start + info.textureSize > TEXMEM_2MB_EDGE)) {
		start = TEXMEM_2MB_EDGE;
	}
	_nextTexture[tmu] += info.textureSize;
	if (_nextTexture[tmu] <= _lastTexture[tmu]) {
		Debug("TMU[" + tmu + "] found texture memory at 0x" + start.toString(16) + " for texture of size " + info.textureSize + ". Remaining texture memory is " + (_lastTexture[tmu] - _nextTexture[tmu]) + " bytes.");
		return start;
	} else {
		_nextTexture[tmu] = start;
		Debug("TMU[" + tmu + "] found NO texture memory for texture of size " + info.textureSize);
		return null;
	}
}

/**
 * create empty vertex.
 * @returns {number[]} an empty vertex.
 */
function FxEmptyVertex() {
	var ret = [];
	var len = fxGetVertexSize();
	for (var i = 0; i < len; i++) {
		ret.push(0);
	}
	return ret;
}

/**
 * split up RGB value and store in vertex.
 * 
 * @param {number[]} v the vertex to store in.
 * @param {number} idx start index of RGB values.
 * @param {number} rgb the RGB value.
 */
function FxRGB2Vertex(v, idx, rgb) {
	v[idx + 0] = (rgb >> 16) & 0xFF;
	v[idx + 1] = (rgb >> 8) & 0xFF;
	v[idx + 2] = rgb & 0xFF;
}
