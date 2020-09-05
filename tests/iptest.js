/*
** This function is called once when the script is started.
*/
function Setup() {
	SetFramerate(1);
}

/*
** This function is repeatedly until ESC is pressed or Stop() is called.
*/
function Loop() {
	//////
	// test generic functions
	//Println(JSON.stringify(WattIpDebug([1, 2, 3, 4])));
	Println("GetLocalIpAddress() = " + JSON.stringify(GetLocalIpAddress()));
	Println("GetNetworkMask()    = " + JSON.stringify(GetNetworkMask()));
	Println("GetDomainname()     = " + JSON.stringify(GetDomainname()));
	Println("GetHostname()       = " + JSON.stringify(GetHostname()));
	Println("Resolve()           = " + JSON.stringify(Resolve("www.heise.de")));
	Println("ResolveIp()         = " + JSON.stringify(ResolveIp([193, 99, 144, 80])));

	//////
	// test UDP socket send
	udp = UdpSocket([192, 168, 2, 8], 20001);
	udp.WriteString("WattTest 123");
	udp.WaitFlush();
	udp.WaitInput();
	str = udp.ReadLine();
	Println("UDP = " + str);
	udp.Close();

	//////
	// test tcp socket send/receive
	s = Socket([192, 168, 2, 8], 65432);
	Println("ESTABLISHED         = " + s.Established());
	s.Mode(SOCKET.ASCII);
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

/*
TCP python test code
-=-=-=-=-=-=-=-=-=-=-=

#!/usr/bin/env python3

import socket

HOST = '192.168.2.8'  # Standard loopback interface address (localhost)
PORT = 65432        # Port to listen on (non-privileged ports are > 1023)

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.bind((HOST, PORT))
    s.listen()
    while True:
        try:
            conn, addr = s.accept()
            with conn:
                print('Connected by', addr)
                while True:
                    data = conn.recv(1024)
                    if not data:
                        break
                    print(data)
                    conn.sendall(data)
        except:
            print("restarting")

*/

/*
UDP python test code
-=-=-=-=-=-=-=-=-=-=-=

import socket

localIP     = "192.168.2.8"
localPort   = 20001
bufferSize  = 1024

msgFromServer       = "Hello UDP Client"
bytesToSend         = str.encode(msgFromServer)

# Create a datagram socket
UDPServerSocket = socket.socket(family=socket.AF_INET, type=socket.SOCK_DGRAM)

# Bind to address and ip
UDPServerSocket.bind((localIP, localPort))

print("UDP server up and listening")

# Listen for incoming datagrams
while(True):
    bytesAddressPair = UDPServerSocket.recvfrom(bufferSize)
    message = bytesAddressPair[0]
    address = bytesAddressPair[1]
    clientMsg = "Message from Client:{}".format(message)
    clientIP  = "Client IP Address:{}".format(address)
    print(clientMsg)
    print(clientIP)

    # Sending a reply to client
    UDPServerSocket.sendto(bytesToSend, address)

*/
