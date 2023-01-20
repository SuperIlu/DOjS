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
********
Example package index:
{
	pkgtype:"dojs",
	index_date:"2022-02-26T10:22:33.000Z",
	packages: [
		{name:"foo", version:1, url:"http://192.168.2.8/dojs_dpm/foo.js", depends:["bar"]},
		{name:"bar", version:2, url:"http://192.168.2.8/dojs_dpm/bar.js", depends:[]}
	]
}
*/

Include("console");
LoadLibrary("curl");

var JS_SUFFIX = ".js";

var con;
var https;
var index = null;

var indexUrl = "https://raw.githubusercontent.com/SuperIlu/DOjSHPackages/master/dojs/index.json";
//var indexUrl = "http://192.168.2.8/dojs_dpm/dojs_index.json";

function commandFunc(c) {
	var inp = c.GetInput().trim();
	var linp = inp.toLowerCase();
	if (inp.length > 0) {
		cmd(inp);
		if (linp === "help") {
			CmdHelp();
		} else if ((linp === "quit") || (linp === "exit")) {
			Stop();
		} else if (linp === "fetch") {
			CmdFetch();
		} else if (linp === "list") {
			CmdList();
		} else if (linp === "installed") {
			CmdAvailable();
		} else if (linp.startsWith("remove ")) {
			CmdRemove(inp);
		} else if (linp.startsWith("install ")) {
			CmdInstall(inp);
		} else if (linp.startsWith("setindex ")) {
			CmdSetIndex(inp);
		} else {
			err("Unknown command: '" + inp + "'");
		}
		con.SetInput("");
		msg("");
	}
}

/**
 * extract the version for a module.
 * 
 * @param {string} m name of the module
 * 
 * @returns the module version or 0 if there was no version.
 */
function GetModuleVersion(m) {
	try {
		var pkg = Require(e);			// try to load module
		if (pkg.__VERSION__) {			// check if __VERSION__ is provided
			return pkg.__VERSION__;
		}
	} catch (e) { }

	return 0; // fallback if there is no version info
}

/**
 * COMMAND: show a list of installed packages
 */
function CmdAvailable() {
	try {
		var pkgs = GetInstalledPackages();
		msg("Installed packages:");
		pkgs.forEach(function (e) {
			msg("  " + e + "{" + GetModuleVersion(e) + "}");
		});
	} catch (e) {
		err(e);
	}
}

/**
 * COMMAND: remove an installed package.
 * 
 * @param {string} inp 
 */
function CmdRemove(inp) {
	var param = GetParameter(inp);

	var found = GetInstalledPackages().indexOf(param) != -1;

	if (found) {
		var jsboot = new Zip(JSBOOT_ZIP, ZIPFILE.DELETE);
		jsboot.DeleteFile(PACKAGE_DIR + param + JS_SUFFIX);
		jsboot.Close();
	} else {
		err("Package '" + param + "' not found.");
	}
}

/**
 * COMMAND: show help.
 */
function CmdHelp() {
	msg("installed - list installed packages.");
	msg("remove    - remove package."); // TODO:
	msg("fetch     - fetch package index from server.");
	msg("install   - install a package (and its dependencies) from package index.");
	msg("list      - list available packages in index.");
	msg("setindex  - set index URL (HTTP or HTTPS).");
	msg("help      - this help.");
	msg("quit      - exit dpm.");
	msg("");
	msg("Current package index URL:");
	msg("  " + indexUrl);
}

/**
 * COMMAND: set/change remote index URL.
 * 
 * @param {string} inp the command line input.
 */
function CmdSetIndex(inp) {
	var param = GetParameter(inp);
	if (param.toLowerCase().startsWith("http://") || param.toLowerCase().startsWith("https://")) {
		indexUrl = param;
	} else {
		err("Illegal URL (does not start with http or https): '" + param + "'");
	}
}

/**
 * COMMAND: fetch package index from remote.
 */
function CmdFetch() {
	try {
		var resp = https.DoRequest(indexUrl);
		if (resp[2] === 200) {
			var content = "exports.idx = " + resp[0].ToString() + ";";	// build JS
			var exports = { "idx": null };								// build object to import into
			NamedFunction('exports', content, "idx")(exports);			// evaluate JS

			if (exports.idx.pkgtype === "dojs") {
				index = exports.idx;

				msg("Loaded " + resp[0].length + " bytes from index with " + index.packages.length + " packages:");
				msg("  " + indexUrl);
				msg("Index date: " + index.index_date);
			} else {
				err("Index fomat error.");
			}
		} else {
			err("Download failed: " + resp[2]);
		}
	} catch (e) {
		err(e);
	}
}

/**
 * COMMAND: list installable packages to console.
 */
function CmdList() {
	if (index) {
		msg("Packages in index:");
		index.packages.forEach(function (e) {
			msg(e.name + " / V" + e.version + " / dep: " + JSON.stringify(e.depends));
		});
	} else {
		err("Index is not loaded. Use 'fetch' first!");
	}
}

/**
 * COMMAND: install a package
 * 
 * @param {string} inp the command line input.
 */
function CmdInstall(inp) {
	var param = GetParameter(inp);

	if (!index) {
		err("Index is not loaded. Use 'fetch' first!");
		return;
	}

	pkg = FindInIndex(param, index.packages);

	if (!pkg) {
		err("Package not found: '" + param + "'");
		return;
	}

	var toInstall = ResolveDependencies(pkg);

	msg("Installation candidates:");
	toInstall.forEach(function (e) {
		msg("  " + e.name);
	});

	toInstall.forEach(InstallPackage);
}

/**
 * find a package entry in the index with given name.
 * 
 * @param {string} name name of the package
 * @param {object[]} idx an array with package entries.
 * 
 * @returns a package entry.
 */
function FindInIndex(name, idx) {
	// search package in index
	var pkg = null;
	idx.forEach(function (e) {
		if (e.name === name) {
			pkg = e;
		}
	});
	return pkg;
}

/**
 * build a list of packages to install from the package dependencies
 * 
 * @param {object} pkg a package description
 * 
 * @returns {object{}} a list of package descriptions to install
 */
function ResolveDependencies(pkg) {
	var todo = [pkg];
	var candidates = [];

	while (todo.length > 0) {							// while there are still unhandled packages
		var cur = todo.shift();							// get first unhandled package
		candidates.push(cur);							// push it to the installation candidates
		cur.depends.forEach(function (e) {
			if (!FindInIndex(e, candidates)) {			// check all dependencies if they were already handled
				var dpkg = FindInIndex(e, index.packages);
				if (!pkg) {
					throw new Error("Dependency '" + e + "' not found in index");
				} else {
					todo.push(dpkg);
				}
			}
		});
	}

	return candidates;
}

/**
 * Install a package.
 * 
 * @param {*} pkg a package object
 */
function InstallPackage(pkg) {
	if (GetInstalledPackages().indexOf(pkg.name) != -1) {
		msg("Package '" + pkg.name + "' is already installed.");
	} else {
		msg("Downloading package '" + pkg.name + "' from '" + pkg.url + "'");
		try {
			var resp = https.DoRequest(pkg.url);
			if (resp[2] === 200) {
				msg("Downloaded " + resp[0].length + " bytes");
				var jsboot = new Zip(JSBOOT_ZIP, ZIPFILE.APPEND);
				jsboot.WriteInts(PACKAGE_DIR + pkg.name + JS_SUFFIX, resp[0]);
				jsboot.Close();
				msg("Package '" + pkg.name + "' installed.");
			} else {
				throw new Error("Download failed: " + resp[2]);
			}
		} catch (e) {
			err(e);
		}
	}
}

/**
 * get an alphabetically sorted list of installed packages (basenames).
 * 
 * @returns {string[]} a list of package names.
 */
function GetInstalledPackages() {
	var ret = [];

	var jsboot = new Zip(JSBOOT_ZIP, ZIPFILE.READ);
	jsboot.GetEntries().forEach(function (e) {
		if (e.name.toUpperCase().startsWith(PACKAGE_DIR) && !e.is_directory) {
			ret.push(e.name.substring(PACKAGE_DIR.length).slice(0, -JS_SUFFIX.length));
		}
	});
	jsboot.Close();

	ret.sort();

	return ret;
}

/**
 * extract the command parameter.
 * 
 * @param {string} inp the command line.
 * 
 * @returns the part of the command lien after the first " " or null if no SPACE in string.
 */
function GetParameter(inp) {
	var idx = inp.indexOf(" ");
	if (idx >= 0) {
		return inp.substring(idx).trim();
	} else {
		return null;
	}
}

/**
 * print a message.
 * @param {string} s the message
 */
function msg(s) {
	con.Log(s, EGA.GREEN);
	Println(s);
}

/**
 * print a command line.
 * @param {string} s the message
 */
function cmd(s) {
	con.Log(s, EGA.YELLOW);
	Println(s);
}

/**
 * print an error.
 * @param {string} s the error
 */
function err(s) {
	con.Log(s, EGA.RED);
	Println(s);
}

function Setup() {
	MouseShowCursor(false);
	https = new Curl();
	con = new Console(EGA.GREEN, EGA.BLACK, commandFunc);
	msg("DOjS Packet Manager (DPM) V0.2 starting up.");
	msg("Local IP address : " + JSON.stringify(GetLocalIpAddress()));
	msg("Network mask     : " + JSON.stringify(GetNetworkMask()));
	msg("Type 'help' for command help.")
	msg("Current package index URL:");
	msg("  " + indexUrl);
}
function Loop() {
	con.Draw();
}
function Input(e) { con.HandleInput(e); }
