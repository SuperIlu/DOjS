# livedojs README

Live coding extension for VSCode and [DOjS](https://github.com/SuperIlu/DOjS).

## Features

This enables live creative coding using VSCode and an instance of DOjS that have network access.

## Requirements
* A computer running MS-DOS, Win95, Win98 or FreeDOS
* Network card and a matching packet driver for the DOS computer
* [DOjS](https://github.com/SuperIlu/DOjS) running the sketch "examples\websvr.js"

## Extension packaging
* change current directory of your command line to `vscode/livedojs`
* You need NodeJS, npm and vsce in your path (see [here](https://code.visualstudio.com/api/working-with-extensions/publishing-extension))
* Run `vsce package` to create a vsix
