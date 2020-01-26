Include('tests/3dfx_tst.js');
Println("Clear screen to blue");

/*
** This function is called once when the script is started.
*/
function Setup() {
	fxInit();
}

/*
** This function is repeatedly until ESC is pressed or Stop() is called.
*/
function Loop() {
	fxBufferClear(FX_BLUE, 0, 0);
}

function Input(e) { }
