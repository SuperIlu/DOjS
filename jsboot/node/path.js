// https://nodejs.org/docs/latest/api/path.html#pathbasenamepath-suffix
exports.basename = function (fname, ext) {
	var lastSlash = fname.lastIndexOf("/");
	var ret;
	if (lastSlash == -1) {
		ret = fname;
	} else {
		ret = fname.substring(lastSlash + 1);
	}

	if (ext) {
		if (ret.endsWith(ext)) {
			ret = ret.split('.').slice(0, -1).join('.')
		}
	}

	return ret;
};

// https://nodejs.org/docs/latest/api/path.html#pathdelimiter
exports.delimiter = ";";

// https://nodejs.org/docs/latest/api/path.html#pathdirnamepath
exports.dirname = function (fname) {
	var lastSlash = fname.lastIndexOf("/");
	if (lastSlash == -1) {
		return fname;
	} else {
		return fname.substring(0, lastSlash);
	}
};

// https://nodejs.org/docs/latest/api/path.html#pathextnamepath
exports.extname = function (fname) {
	var lastDot = fname.lastIndexOf(".");
	if ((lastDot == -1) || (lastDot == 0)) {
		return "";
	} else {
		return fname.substring(lastDot);
	}
};

// https://nodejs.org/docs/latest/api/path.html#pathformatpathobject
exports.format = function () { throw "TBD"; }

// https://nodejs.org/docs/latest/api/path.html#pathmatchesglobpath-pattern
exports.matchesGlob = function () { throw "TBD"; }

// https://nodejs.org/docs/latest/api/path.html#pathisabsolutepath
exports.isAbsolute = function (path) {
	return path[1] == ":" && path[2] == "/"
}

// https://nodejs.org/docs/latest/api/path.html#pathjoinpaths
exports.join = function () {
	var ret = "";
	for (var i = 0; i < arguments.length; i++) {
		if ((i != 0) && !ret.endsWith('/')) {
			ret += "/";
		}
		ret += arguments[i];
	}
	return ret;
};

// https://nodejs.org/docs/latest/api/path.html#pathnormalizepath
exports.normalize = exports.resolve;

// https://nodejs.org/docs/latest/api/path.html#pathparsepath
exports.parse = function () { throw "TBD"; }

// https://nodejs.org/docs/latest/api/path.html#pathposix
exports.posix = null;

// https://nodejs.org/docs/latest/api/path.html#pathrelativefrom-to
exports.relative = function () { throw "TBD"; }

// https://nodejs.org/docs/latest/api/path.html#pathresolvepaths
exports.resolve = function () {
	var ret = "";
	for (var i = 0; i < arguments.length; i++) {
		if ((i != 0) && !ret.endsWith('/')) {
			ret += "/";
		}
		ret += arguments[i];
	}
	return RealPath(ret);
};

// https://nodejs.org/docs/latest/api/path.html#pathsep
exports.sep = "/";

// https://nodejs.org/docs/latest/api/path.html#pathtonamespacedpathpath
exports.toNamespacedPath = function (path) { return path; }

// https://nodejs.org/docs/latest/api/path.html#pathwin32
exports.win32 = null;
