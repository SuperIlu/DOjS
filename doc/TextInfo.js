/**
 * create new texture from 3df file or a Bitmap.
 * @class
 * 
 * @param {(string|Bitmap)} src 3df file to load as texture or Bitmap to convert to texture
 */
function TexInfo(src) {
	/**
	 * filename
	 * @member {string}
	 */
	this.filename = null;
	/**
	 * large LOD
	 * @member {number}
	 */
	this.largeLod = 0;
	/**
	 * small LOD
	 * @member {number}
	 */
	this.smallLod = 0;
	/**
	 * aspect ratio
	 * @member {number}
	 */
	this.aspectRatio = 0;
	/**
	 * texture format
	 * @member {number}
	 */
	this.format = 0;
	/**
	 * table type
	 * @member {number}
	 */
	this.tableType = 0;
	/**
	 * size of texture
	 * @member {number}
	 */
	this.textureSize = 0;
	/**
	 * texture mem address (if downloaded)
	 * @member {number}
	 */
	this.address = 0;
	/**
	 * TMU (if downloaded)
	 * @member {number}
	 */
	this.tmu = 0;
};

/**
 * download the MIP map to texture memory.
 * @param {GR_TMU} tmu the TMU unit to download to.
 * @param {number} address destination memory address in texture memory.
 * @param {GR_MIPMAPLEVELMASK} evenOdd one of GR_MIPMAPLEVELMASK.
 */
TexInfo.prototype.DownloadMipMap = function (tmu, address, evenOdd) { };
/**
 * mark the texture as 'not downloaded' again.
 */
TexInfo.prototype.MarkUnused = function () { };
/**
 * specify this TexInfo as the current texture source for rendering.
 * @param {GR_MIPMAPLEVELMASK} evenOdd one of GR_MIPMAPLEVELMASK.
 */
TexInfo.prototype.Source = function (evenOdd) { };
/**
 * return the texture memory consumed by a texture
 * @param {GR_MIPMAPLEVELMASK} evenOdd one of GR_MIPMAPLEVELMASK.
 * @returns {number} size of texture in bytes.
 */
TexInfo.prototype.MemRequired = function (evenOdd) { };
