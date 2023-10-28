// The module 'vscode' contains the VS Code extensibility API
// Import the module and reference it with the alias vscode in your code below
const vscode = require('vscode');

// import http
const http = require('http')

const TOKEN = "// livedojs";

var hostName = null;
var dojsStatusBar = null;

/**
 * @param {vscode.ExtensionContext} context
 */
function activate(context) {
	console.log('>>> "livedojs" is now active!');

	//////
	// create save hook
	let saveHook = vscode.workspace.onDidSaveTextDocument((doc) => {
		const data = doc.getText();
		if (doc.fileName.endsWith(".js") && data.startsWith(TOKEN)) {
			do_upload(data);
		}
	});
	context.subscriptions.push(saveHook);

	//////
	// CMD: reset connection
	let reset = vscode.commands.registerCommand('livedojs.reset', function () {
		if (hostName) {
			http.get("http://" + hostName + "/livecode", (res) => {
				const { statusCode } = res;
				let error;
				// Any 2xx status code signals a successful response but
				// here we're only checking for 200.
				if (statusCode !== 200) {
					error = new Error('Request Failed.\n' +
						`Status Code: ${statusCode}`);
				}
				if (error) {
					vscode.window.showErrorMessage(error);
					console.log(error);
				} else {
					vscode.window.showInformationMessage('DOjS reset');
				}
				res.resume();
			});
		} else {
			vscode.window.showErrorMessage("DOjS hostname is not set, use 'Set DOjS hostname'");
		}
	});
	context.subscriptions.push(reset);

	//////
	// CMD: set hostname 
	let cmdHostname = 'livedojs.hostname';
	let hname = vscode.commands.registerCommand(cmdHostname, function () {
		let input = vscode.window.showInputBox();
		input.then(function (hn) {
			hostName = hn;
			updateStatusBar();
		});
	});
	context.subscriptions.push(hname);

	//////
	// CMD: upload 
	let upload = vscode.commands.registerCommand('livedojs.upload', function () {
		const data = vscode.window.activeTextEditor.document.getText();

		if (vscode.window.activeTextEditor.document.fileName.endsWith(".js") && data.startsWith("// livedojs")) {
			do_upload(data);
		} else {
			vscode.window.showErrorMessage("Current editor must contain a '.js' file and first line must be '" + TOKEN + "'");
		}
	});
	context.subscriptions.push(upload);

	//////
	// CMD: logfile
	const logfile_name = 'JSLOG.TXT';
	let logfile = vscode.commands.registerCommand('livedojs.logfile', function () {
		if (hostName) {
			http.get("http://" + hostName + "/livelog", (res) => {
				const { statusCode } = res;
				let error;
				// Any 2xx status code signals a successful response but
				// here we're only checking for 200.
				if (statusCode !== 200) {
					error = new Error('Request Failed.\n' +
						`Status Code: ${statusCode}`);
				}
				if (error) {
					vscode.window.showErrorMessage(error);
					console.log(error);
					res.resume();
					return;
				}

				res.setEncoding('utf8');
				let rawData = '';
				res.on('data', (chunk) => { rawData += chunk; });
				res.on('end', () => {
					const newFile = vscode.Uri.parse('untitled:' + logfile_name);
					vscode.workspace.openTextDocument(newFile).then(document => {
						const edit = new vscode.WorkspaceEdit();
						return vscode.workspace.applyEdit(edit).then(success => {
							if (success) {
								vscode.window.showTextDocument(document).then((editor) => {
									editor.edit(editBuilder => {
										var firstLine = editor.document.lineAt(0);
										var lastLine = editor.document.lineAt(editor.document.lineCount - 1);
										var textRange = new vscode.Range(firstLine.range.start, lastLine.range.end);
										editBuilder.replace(textRange, rawData);
									});
								});
							} else {
								vscode.window.showInformationMessage('Error!');
							}
						});
					});
				});

			});
		} else {
			vscode.window.showErrorMessage("DOjS hostname is not set, use 'Set DOjS hostname'");
		}
	});
	context.subscriptions.push(logfile);

	// create a new status bar item that we can now manage
	dojsStatusBar = vscode.window.createStatusBarItem(vscode.StatusBarAlignment.Right, 100);
	dojsStatusBar.command = cmdHostname;
	context.subscriptions.push(dojsStatusBar);

	// update status bar item once at start
	updateStatusBar();
}

function do_upload(data) {
	if (hostName) {
		const options = {
			hostname: hostName,
			port: 80,
			path: '/livecode',
			method: 'PUT',
			timeout: 1000,
			headers: {
				'Content-Type': 'text/plain',
				'Content-Length': data.length
			}
		}

		const req = http.request(options, res => {
			console.log(`statusCode: ${res.statusCode}`)

			res.on('data', d => {
				console.log(d);
			});

			if (res.statusCode === 200) {
				vscode.window.showInformationMessage('DOjS upload OK');
			}
		});

		req.on('error', error => {
			console.log(error);
			vscode.window.showErrorMessage(error);
			req.end();
			return;
		});

		// use its "timeout" event to abort the request
		req.on('timeout', () => {
			req.destroy();
			vscode.window.showErrorMessage("Can't connect to " + hostName);
		});

		req.write(data);
		req.end();
	} else {
		vscode.window.showErrorMessage("DOjS hostname is not set, use 'Set DOjS hostname'");
	}
}

/**
 * update status bar with hostname
 */
function updateStatusBar() {
	if (hostName) {
		dojsStatusBar.text = 'DOjS host: ' + hostName;
	} else {
		dojsStatusBar.text = 'DOjS host: UNSET';
	}
	dojsStatusBar.show();
}

// this method is called when your extension is deactivated
function deactivate() { }

module.exports = {
	activate,
	deactivate
}
