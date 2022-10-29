/**
 * DIN A3 page width
 */
PDF_A3_WIDTH = 0;
/**
 * DIN A3 page height
 */
PDF_A3_HEIGHT = 0;
/**
 * DIN A4 page width
 */
PDF_A4_WIDTH = 0;
/**
 * DIN A4 page height
 */
PDF_A4_HEIGHT = 0;
/**
 * LETTER page width
 */
PDF_LETTER_WIDTH = 0;
/**
 * LETTER page height
 */
PDF_LETTER_HEIGHT = 0;

/**
 * align text left
 */
PDF_ALIGN_LEFT = 0;
/**
 * align text right
 */
PDF_ALIGN_RIGHT = 0;
/**
 * align text center
 */
PDF_ALIGN_CENTER = 0;
/**
 * align text in the center, with padding to fill the available space
 */
PDF_ALIGN_JUSTIFY = 0;
/**
 * Like PDF_ALIGN_JUSTIFY, except even short lines will be fully justified
 */
PDF_ALIGN_JUSTIFY_ALL = 0;
/**
 * Fake alignment for only checking wrap height with no writes
 */
PDF_ALIGN_NO_WRITE = 0;

/**
 * Produce code-128A style barcodes
 */
PDF_BARCODE_128A = 0;
/**
 * Produce code-39 style barcodes
 */
PDF_BARCODE_39 = 0;

/**
 * id for top-level bookmark.
 */
PDF_TOPLEVEL_BOOKMARK = 0;

/**
 * PDf font.
 */
PDF_COURIER = null;
/**
 * PDf font.
 */
PDF_COURIER_BOLD = null;
/**
 * PDf font.
 */
PDF_COURIER_BOLD_OBLIQUE = null;
/**
 * PDf font.
 */
PDF_COURIER_OBLIQUE = null;
/**
 * PDf font.
 */
PDF_HELVETICA = null;
/**
 * PDf font.
 */
PDF_HELVETICA_BOLD = null;
/**
 * PDf font.
 */
PDF_HELVETICA_BOLD_OBLIQUE = null;
/**
 * PDf font.
 */
PDF_HELVETICA_OBLIQUE = null;
/**
 * PDf font.
 */
PDF_TIMES = null;
/**
 * PDf font.
 */
PDF_TIMES_BOLD = null;
/**
 * PDf font.
 */
PDF_TIMES_BOLD_OBLIQUE = null;
/**
 * PDf font.
 */
PDF_TIMES_OBLIQUE = null;
/**
 * PDf font.
 */
PDF_SYMBOL = null;
/**
 * PDf font.
 */
PDF_ZAPF_DINGBATS = null;


/**
 * render a PDF.
 * 
 * **Note: PDFGen module must be loaded by calling LoadLibrary("pdfgen") before using!**
 * 
 * @see LoadLibrary()
 * 
 * @class
 * 
 * @param {number} width page width.
 * @param {number} height page height.
 */
function PDFGen(width, height) {
	/**
	 * page width
	 * @member {number}
	 */
	this.width = 0;
	/**
	 * page height
	 * @member {number}
	 */
	this.height = 0;
}

/**
 * Save the given pdf document to the given FILE output
 * @param {string} filename Name of the file to store the PDF into
 */
PDFGen.prototype.Save = function (filename) { }
/**
 * Add a new page to the given pdf
 */
PDFGen.prototype.AppendPage = function () { }
/**
 * Sets the font to use for text objects. Default value is Times-Roman if
 * this function is not called.
 * Note: The font selection should be done before text is output,
 * and will remain until pdf_set_font is called again.
 * @param {string} font New font to use. This must be one of the standard PDF fonts
 */
PDFGen.prototype.SetFont = function (font) { }
/**
 * 
 * @param {number} x1 X offset of start of line
 * @param {number} y1 Y offset of start of line
 * @param {number} x2 X offset of end of line
 * @param {number} y2 Y offset of end of line
 * @param {number} width Width of the line
 * @param {Color} col Colour to draw the line
 */
PDFGen.prototype.AddLine = function (x1, y1, x2, y2, width, col) { }
/**
 * 
 * @param {number} x X offset of the center of the circle
 * @param {number} y Y offset of the center of the circle
 * @param {number} r Radius of the circle
 * @param {number} width Width of the circle outline stroke
 * @param {Color} col Colour to draw the circle outline stroke
 * @param {Color} fill_col Colour to fill the circle
 */
PDFGen.prototype.AddCircle = function (x, y, r, width, col, fill_col) { }
/**
 * 
 * @param {number} x X offset of the center of the ellipse
 * @param {number} y Y offset of the center of the ellipse
 * @param {number} xr Radius of the ellipse in the X axis
 * @param {number} yr Radius of the ellipse in the Y axis
 * @param {number} width Width of the ellipse outline stroke
 * @param {Color} col Colour to draw the ellipse outline stroke
 * @param {Color} fill_col Colour to fill the ellipse
 */
PDFGen.prototype.AddEllipse = function (x, y, xr, yr, width, col, fill_col) { }
/**
 * Add an outline rectangle to the document
 * @param {number} x X offset to start rectangle at
 * @param {number} y Y offset to start rectangle at
 * @param {number} w Width of rectangle
 * @param {number} h Height of rectangle
 * @param {number} width Width of rectangle border
 * @param {Color} col Colour to draw the rectangle
 */
PDFGen.prototype.AddRectangle = function (x, y, w, h, width, col) { }
/**
 * Add a filled rectangle to the document
 * @param {number} x X offset to start rectangle at
 * @param {number} y Y offset to start rectangle at
 * @param {number} w Width of rectangle
 * @param {number} h Height of rectangle
 * @param {number} width Width of rectangle border
 * @param {Color} col Colour to draw the rectangle
 */
PDFGen.prototype.AddFilledRectangle = function (x, y, w, h, width, col) { }

/**
 * Add a text string to the document
 * @param {string} txt String to display
 * @param {number} size Point size of the font
 * @param {number} x X location to put it in
 * @param {number} y Y location to put it in
 * @param {Color} col Colour to draw the text
 */
PDFGen.prototype.AddText = function (txt, size, x, y, col) { }
/**
 * Add a text string to the document, making it wrap if it is too
 * long
 * @param {string} txt String to display
 * @param {number} size Point size of the font
 * @param {number} x X location to put it in
 * @param {number} y Y location to put it in
 * @param {Color} col Colour to draw the text
 * @param {number} wwidth Width at which to wrap the text
 * @param {number} align Text alignment (see PDF_ALIGN_xxx)
 * @returns {number} the final height of the wrapped text here
 */
PDFGen.prototype.AddTextWrap = function (txt, size, x, y, col, wwidth, align) { }

/**
 * Add an image file as an image to the document.
 * Support image formats: JPEG, PNG, BMP & PPM
 * @param {number} x X offset to put BMP at
 * @param {number} y Y offset to put BMP at
 * @param {number} w Displayed width of image
 * @param {number} h Displayed height of image
 * @param {string} fname Filename of image file to display
 */
PDFGen.prototype.AddImageFile = function (x, y, w, h, fname) { }
/**
 * Adjust the width/height of current page
 * @param {number} w Width of the page in points
 * @param {number} h Height of the page in points
 */
PDFGen.prototype.PageSetSize = function (w, h) { }
/**
 * Calculate the width of a given string in the current font
 * @param {string} font Name of the font to get the width of.
 * @param {string} txt Text to determine width of
 * @param {number} size Size of the text, in points
 * @returns {number} text width
 */
PDFGen.prototype.GetFontTextWidth = function (font, txt, size) { }
/**
 * 
 * @param {number} parent ID of a previously created bookmark that is the parent of this one. PDF_TOPLEVEL_BOOKMARK if this should be a top-level bookmark.
 * @param {string} txt String to associate with the bookmark
 * @returns {number} 
 */
PDFGen.prototype.AddBookmark = function (parent, txt) { }
/**
 * Add a quadratic bezier curve to the document
 * @param {number} x1 X offset of the initial point of the curve
 * @param {number} y1 Y offset of the initial point of the curve
 * @param {number} x2 X offset of the final point of the curve
 * @param {number} y2 Y offset of the final point of the curve
 * @param {number} xq X offset of the control point of the curve
 * @param {number} yq Y offset of the control point of the curve
 * @param {number} width Width of the curve
 * @param {Color} col Colour to draw the curve
 */
PDFGen.prototype.AddQuadraticBezier = function (x1, y1, x2, y2, xq, yq, width, col) { }
/**
 * Add a cubic bezier curve to the document
 * @param {number} x1 X offset of the initial point of the curve
 * @param {number} y1 Y offset of the initial point of the curve
 * @param {number} x2 X offset of the final point of the curve
 * @param {number} y2 Y offset of the final point of the curve
 * @param {number} xq1 X offset of the first control point of the curve
 * @param {number} yq1 Y offset of the first control point of the curve
 * @param {number} xq2 X offset of the second control of the curve
 * @param {number} yq2 Y offset of the second control of the curve
 * @param {number} width Width of the curve
 * @param {Color} col Colour to draw the curve
 */
PDFGen.prototype.AddCubicBezier = function (x1, y1, x2, y2, xq1, yq1, xq2, yq2, width, col) { }
/**
 * Add a barcode to the document
 * @param {number} type PDF_BARCODE_128A or PDF_BARCODE_39
 * @param {number} x X offset to put barcode at
 * @param {number} y Y offset to put barcode at
 * @param {number} w Width of barcode
 * @param {number} h Height of barcode
 * @param {string} txt Barcode contents
 * @param {Color} col Colour to draw barcode
 */
PDFGen.prototype.AddBarcode = function (type, x, y, w, h, txt, col) { }
/**
 * Add a Bitmap as an image to the document
 * @param {number} x X offset to put BMP at
 * @param {number} y Y offset to put BMP at
 * @param {number} w Displayed width of image
 * @param {number} h Displayed height of image
 * @param {Bitmap} bm Bitmap to add.
 */
PDFGen.prototype.AddRgb = function (x, y, w, h, bm) { }
/**
 * Add an outline polygon to the document
 * @param {number[][]} points an array of 2-number arrays with the points of the polygon (e.g. [[10,10], [20,20], [30,30]]).
 * @param {number} width Width of polygon border
 * @param {Color} col Colour to draw the polygon
 */
PDFGen.prototype.AddPolygon = function (points, width, col) { }
/**
 * Add a filled polygon to the document
 * @param {number[][]} points an array of 2-number arrays with the points of the polygon (e.g. [[10,10], [20,20], [30,30]]).
 * @param {number} width Width of polygon border
 * @param {Color} col Colour to draw the polygon
 */
PDFGen.prototype.AddFilledPolygon = function (points, width, col) { }
/**
 * Add a custom path to the document.<BR/>
 * <BR/>
 * Each entry in the ops array must consist of an array with 7 values. Unused values have to be 0.<BR/>
 * value 1: op Operation command. Possible operators are: m = move to, l = line to, c = cubic bezier curve with two control points, v = cubic bezier curve with one control point fixed at first point, y = cubic bezier curve with one control point fixed at second point, h = close path<BR/>
 * value 2: X offset of the first point. Used with: m, l, c, v, y<BR/>
 * value 3: Y offset of the first point. Used with: m, l, c, v, y<BR/>
 * value 4: X offset of the second point. Used with: c, v, y<BR/>
 * value 5: Y offset of the second point. Used with: c, v, y<BR/>
 * value 6: X offset of the third point. Used with: c<BR/>
 * value 7: Y offset of the third point. Used with: c<BR/>
 * <BR/>
 * @param {number[][]} ops an array of 7 - value arrays with the ops of the path.
 * @param {number} width Width of polygon border
 * @param {Color} col Colour to draw the polygon
 * @param {Color} fill_col Colour to fill the path
 */
PDFGen.prototype.AddCustomPath = function (ops, width, col, fill_col) { }
