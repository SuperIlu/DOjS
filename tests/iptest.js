/*
uses udp-test.py and tcp-test.py
*/

// var TEST_HOST = 'nonpi.local';
// var TEST_HOST = [192, 168, 2, 8];
var TEST_HOST = "192.168.2.9";

var UDP_PORT = 20001;
var TCP_PORT = 65432;

/*
** This function is called once when the script is started.
*/
function Setup() {
	SetFramerate(1);
	DEBUG = true;
}

/*
** This function is repeatedly until ESC is pressed or Stop() is called.
*/
function Loop() {
	//////
	// test generic functions
	//Println(JSON.stringify(WattIpDebug([1, 2, 3, 4])));
	Println("GetNetworkInterfaces() = " + JSON.stringify(GetNetworkInterfaces()));
	Println("GetDomainname()        = " + JSON.stringify(GetDomainname()));
	Println("GetHostname()          = " + JSON.stringify(GetHostname()));
	Println("Resolve()              = " + JSON.stringify(Resolve("www.heise.de")));
	Println("ResolveIp()            = " + JSON.stringify(ResolveIp([193, 99, 144, 80])));
	Println("Resolve(TEST_HOST)     = " + JSON.stringify(Resolve(TEST_HOST)));

	//////
	// test UDP socket send
	Println("UDP: Opening");
	udp = UdpSocket(TEST_HOST, UDP_PORT);
	Println("UDP: Writing");
	udp.WriteString("WattTest 123");
	Println("UDP: DataReady=" + udp.DataReady());
	Println("UDP: Flush");
	udp.WaitFlush();
	Println("UDP: WaitInput");
	udp.WaitInput();
	Println("UDP: DataReady=" + udp.DataReady());
	Println("UDP: ReadLine");
	str = udp.ReadLine();
	Println("UDP: " + str);
	udp.Close();
	Println("UDP: Done");

	//////
	// test tcp socket send/receive
	s = Socket(TEST_HOST, TCP_PORT);
	Println("ESTABLISHED         = " + s.Established());
	// s.Mode(SOCKET.ASCII);
	s.WriteByte(32);
	s.Flush();
	s.WaitFlush();
	Println("DataReady ch        = " + s.DataReady());
	sp = s.ReadByte();
	Println("SPACE               = " + sp);

	s.WriteString("This is a test of the emergency broadcast system!\r\n");
	s.WaitFlush();
	s.WaitInput();
	Println("DataReady str       = " + s.DataReady());
	str = s.ReadLine();
	Println("STRING              = " + str);

	Println("local port          = " + s.GetLocalPort());
	Println("remote port         = " + s.GetRemotePort());
	Println("remote host         = " + JSON.stringify(s.GetRemoteHost()));

	s.Close();

	//////
	// HTTP test
	Println("Trying http_get()");
	var http = http_get("http://192.168.2.8/index.html");
	Println(JSON.stringify(http));
	Println(http_string_content(http));
	Println(JSON.stringify(http_headers(http)));


	//////
	// server socket
	var count = 0;
	var ssocket = ServerSocket(SOCKET.SERVER.ANY, 4711);
	while (!ssocket.Established()) {
		Println("No connection yet:" + count);
		Sleep(1000);
		count++;
		if (count > 20) {
			break;
		}
	}
	Println("Connection from " + JSON.stringify(ssocket.GetRemoteHost()) + ":" + ssocket.GetRemotePort());
	while (ssocket.Established() || ssocket.DataReady()) {
		var ready = ssocket.DataReady();
		var line = ssocket.ReadLine();

		if (line) {
			Println("[" + ready + "] " + line);
			break;
		} else {
			Sleep(1000);
		}
	}
	ssocket.Close();

	Println("Exiting");
	Stop();
}

function Input(e) {
}
