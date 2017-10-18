#include "packets.h"


ClientPacket ::	ClientPacket (char* buf)
{
	//get the opcode from the buffer
	int opcode = buf[0];
	opcode = (opcode<<8) + buf[1];
		
	//set the opcode
	if(opcode == 1)
		OpCode = RRQ;
	else if(opcode == 4)
		OpCode = ACK;
	else
		OpCode = ERROR;
		
	filename = "";
	mode = "";
		
	if(OpCode == RRQ) // read request 
	{
		//set the filename
		int position = 2; 
		while(buf[position] != 0)
		{
			filename += buf[position];
			position ++;
		}
		position ++;
			
		//set the mode
		while(buf[position] != 0)
		{
			mode += buf[position];
			position ++;
		}		
		blockNum = -1;  //this isn't an Ack, so set the block number -1
								 
	}	
	else if(OpCode == ACK) 
	{
	   //extract the block number from the ack statement
		unsigned int high_byte = buf[2];
		high_byte = high_byte & 255;
		high_byte = high_byte << 8;
		unsigned int low_byte = buf[3];
		low_byte = low_byte & 255;
		blockNum = high_byte + low_byte;
	}
}

OPCODE ClientPacket :: GetOPCODE()
{
    return OpCode; 
}

string ClientPacket :: GetFileName() 
{
    return filename; 
}

string ClientPacket :: GetMode()
{ 
    return mode;
}

unsigned int ClientPacket :: GetBlocknum()
{ 
    return blockNum; 
}

//takes the files, the block number, and wrap number to extract the needed bits from the file
//the wrap number is the number of times the requested block has been "wrapped around"
DataPacket::DataPacket(ifstream& file, unsigned int blocknum, unsigned int wrapnum) 
		: blockNum(blocknum)
{		
	int position = 0;
	char* readByte= new char(); //byte to be read in 
	Data = "";
		
	file.seekg((wrapnum*65536 + (blockNum-1))*512); //go to the current position in the file
	//loop until either we reach the end of the file or 512 bytes
	while(!file.eof() & position < buffer_size - 4)
	{
		if(file.peek() != EOF)
		{
			//read the next byte, and add it to our string of data
			file.read(readByte, 1);
			Data += *readByte;
			position ++;
		}
	}
}
	
//this is called to atually take the string of data, and place it into a character buffer
void DataPacket:: data_into_buffer(char* buf)
{		
	//make the header for the data packet
	string packet_header = "";
	//add the opcode
	packet_header += (char)0;
	packet_header += (char)3;
	//add the block number
	packet_header += (char)(blockNum>>8 & 0xFF);
	packet_header += (char)(blockNum & 0xFF);
		
	//place the header info into the buffer
	for(int i = 0; i < packet_header.size(); i++)
		*(buf + i) = packet_header[i];
		
	//place the data into the buffer
	for(int i = packet_header.size(); i < Data.size() + packet_header.size(); i++)
		*(buf + i) = Data[i -packet_header.size()];
}	
	
int DataPacket:: packet_size()
{ 
    return Data.size() + 4; 
}