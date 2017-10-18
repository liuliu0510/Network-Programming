#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h> 
#include <sys/select.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <string.h>
#include <strings.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/errno.h>
#include <netdb.h>
#include <vector>
#include <bitset>

using namespace std;


//Opcodes in the packets server received from clients
enum OPCODE
{
    ACK,
    RRQ,
    ERROR
};

//full size of the buffer to send out 
const int buffer_size = 516;

//Packets received from client
class ClientPacket
{
private:
	OPCODE OpCode;
	string filename;
	string mode;
	unsigned int blockNum;
   
public:
	ClientPacket (char* buf); 
	//Get functions
	OPCODE GetOPCODE();
	string GetFileName();
	string GetMode();
	unsigned int GetBlocknum();
};

//Packet sent to client
class DataPacket
{
private:
	string Data;
	unsigned int blockNum;

public:
	DataPacket(ifstream &file, unsigned int blocknum, unsigned int wrapnum);
	void data_into_buffer(char* buf);
	int packet_size();
};
