/*
** This function is called once when the script is started.
*/
function Setup() {
	var i = new IniFile('tests/test.ini');
	Println(i.filename);
	Println("1    := " + i.Get('foo'));
	Println("2    := " + i.Get('global', 'bar'));
	Println('NULL := ' + i.Get('global', 'p1'));
	Println('gna  := ' + i.Get('p2'));
	Println('NULL := ' + i.Get('hurz'));
	Stop();
}

/*
** This function is repeatedly until ESC is pressed or Stop() is called.
*/
function Loop() {
}

/*
** This function is called on any input.
*/
function Input(e) {
}
