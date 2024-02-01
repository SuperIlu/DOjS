#!/usr/bin/env python3

import socket

localIP     = "0.0.0.0"
localPort   = 20001
bufferSize  = 1024

msgFromServer       = "Hello UDP Client"
bytesToSend         = str.encode(msgFromServer)

# Create a datagram socket
UDPServerSocket = socket.socket(family=socket.AF_INET, type=socket.SOCK_DGRAM)

# Bind to address and ip
UDPServerSocket.bind((localIP, localPort))

print("UDP server up and listening on port {}".format(localPort))

# Listen for incoming datagrams
count=0
while(True):
	print("\n\n#{}: Waiting for data...".format(count))
	message, address = UDPServerSocket.recvfrom(bufferSize)
	clientMsg = "Message from Client:{}".format(message)
	clientIP  = "Client IP Address:{}".format(address)
	print(clientMsg)
	print(clientIP)

	# Sending a reply to client
	print("Sending {}".format(bytesToSend))
	UDPServerSocket.sendto(bytesToSend, address)
	count+=1
