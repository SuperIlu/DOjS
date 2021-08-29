/*
    This file was derived from the p5.js source code at
    https://github.com/processing/p5.js

    Copyright (c) the p5.js contributors and Andre Seidelt <superilu@yahoo.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

var PI = Math.PI;

exports.HALF_PI = PI / 2;
exports.PI = PI;
exports.QUARTER_PI = PI / 4;
exports.TAU = PI * 2;
exports.TWO_PI = PI * 2;
exports.DEG_TO_RAD = PI / 180.0;
exports.RAD_TO_DEG = 180.0 / PI;

exports.DEGREES = 'degrees';
exports.RADIANS = 'radians';

exports.LEFT = 'left';
exports.CENTER = 'center';
exports.RIGHT = 'right';
exports.TOP = 'top';
exports.BOTTOM = 'bottom';
exports.BASELINE = exports.BOTTOM;

exports.CLOSE = 'close';
exports.POINTS = 'points';
exports.LINES = 'lines';
exports.TRIANGLES = 'triangles';

exports.OPEN = 'open';
exports.CHORD = 'chord';
exports.PIE = 'pie';

exports.MIDDLE = -10;

exports.CORNER = 'CORNER';
exports.CORNERS = 'CORNERS';
exports.CENTER = 'center';
exports.RADIUS = 'radius';

exports.RGB = 'rgb';
exports.HSB = 'hsb';
exports.HSL = 'hsl';

// blend modes
exports.BLEND = 'source-over';
exports.REMOVE = 'destination-out';
exports.ADD = 'lighter';
exports.DARKEST = 'darken';
exports.LIGHTEST = 'lighten';
exports.DIFFERENCE = 'difference';
exports.SUBTRACT = 'subtract';
exports.EXCLUSION = 'exclusion';
exports.MULTIPLY = 'multiply';
exports.SCREEN = 'screen';
exports.REPLACE = 'copy';
exports.OVERLAY = 'overlay';
exports.HARD_LIGHT = 'hard-light';
exports.SOFT_LIGHT = 'soft-light';
exports.DODGE = 'color-dodge';
exports.BURN = 'color-burn';
