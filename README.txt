ECEN602 Assignment Programming Assignment2
----------------------------------

Team Number: 10
Member 1 # Sama, Avani
Member 2 # Li, Wan
---------------------------------------

Design:
--------------------
For the tftp_server.c
Used socket() to create a socket and then get the File Descriptor.
Used bind() to associate that socket with a port.
Used select() function to choose which client it will work with and then send the client file.
Implement the single stop and wait machenism.
The server store the sliding window packet in the buffer and will resend the packet after the 
timeout.The data acket size is 512 bytes.
Timeout is kept as 1 second
   Server is able to accept clients, get their RRQ and send the file to client.
---------------------------------------

Description/Comments:
--------------------
1. This server can provide a simple TFTP service for client and server based on socket programming.
   This package contains three files: tftp_server.c,README and makefile,
   to generate object files, use: "make -f makefile" in the path of these files in a linux environment.
2. To use the service, first start server by ./server SERVER_PORT
   SERVER_IP is the IP address of server, SERVER_PORT is the port of server.
3. Test cases performed are as follows-
	a)multiple clients can connect
	b)file size 512 can be transferred
	c)File sizes consistent on both cient and server machines 


Unix command for starting server:
------------------------------------------
./server.out SERVER_PORT
