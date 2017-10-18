You can see how to run and compile the program in the makefile. 

The program is invoked using ./server IPAddress Portnumber (./server 127.0.0.1 2025)
 
I used tftp on ubuntu to test our program. (tftp 127.0.0.1 2025)

The program includes 3 files: sever.cpp, packets.cpp, packets.h.

packets.h contains all the header files we need for the whole program,some common variables of the two .cpp files,and statements of classes and functions in the packets.cpp file. 

packets.cpp contains functions of 2 classes: Clientpacket and DataPacket. 

Clientpacket is used to deal with packets received from clients, which are: RRQ, ACK and ERROR here. 

And DataPacket is sent from server to the client, in response to the RRQ packet. 


The program transfers data to the client, and sends an error if the file does not exist. 

The server is implented using select() approach to deal with the multiplexing I/O. The server receives a connection, and a request for a file in a tftp format. when the server finds that there is a new connection, it would then create a new socket and  bind this new socket with its address and port number. Then the new socket will be used to do the communication and documment transmission. 

There is a bug in our program, when we bind the new sock