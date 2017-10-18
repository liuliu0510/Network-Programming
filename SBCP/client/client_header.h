#ifndef _CLIENT_HEADER
#define _CLIENT_HEADER

#define NAME_LEN 16
#define SERVERDOWN "#shutdown\n"
#define CLIENTDOWN "#exit\n"
#define MAXBUFSIZE 600
#define PROTOCOL_VER 3

struct SBCP_Msg_Header
{
	unsigned short vrsn9_type7;
	unsigned short length;
};

enum SBCP_Msg_Type
{
	JOIN = 2,
	FWD = 3,
	SEND = 4,
	NAK = 5,
	OFFLINE = 6,
	ACK = 7,
	ONLINE = 8,
	IDLE = 9
};

struct SBCP_Atr_Header
{
	unsigned short type;
	unsigned short length;
};

enum SBCP_Atr_Type
{
	Username = 2,
	Message = 4,
	Reason = 1,
	Client_Count = 3
};

int Packet_Send(char* msg, unsigned short msg_type);
int Packet_Recv(char* msg, int *num);

#endif