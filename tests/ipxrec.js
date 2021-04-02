LoadLibrary("ipx");
Include("console");

var con;
var sock_num = 0x1234;

function Setup() {
	IpxSocketOpen(sock_num);
	MouseShowCursor(false);

	con = new Console();
	con.Log("Waiting for data on socket " + sock_num);
}

function Loop() {
	con.Draw();

	if (IpxCheckPacket()) {
		var pck = IpxGetPacket();
		con.Log(JSON.stringify(pck));
	}
}

function Input(e) { }
