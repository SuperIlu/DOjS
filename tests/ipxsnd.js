LoadLibrary("ipx");
Include("console");

var con;
var sock_num = 0x1234;
var counter = 0;

function Setup() {
	SetFramerate(1);
	IpxSocketOpen(sock_num);
	MouseShowCursor(false);

	con = new Console();
	con.Log("Sending data on socket " + sock_num);
}

function Loop() {
	con.Draw();

	var txt = "" + counter;
	con.Log("Sent " + txt);
	IpxSend(txt, IPX.BROADCAST);
	counter++;
}

function Input(e) { }
