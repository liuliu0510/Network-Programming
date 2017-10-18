#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <netinet/in.h>
#include "client_header.h"

int Packet_Send(char* msg, unsigned short msg_type)
{
	unsigned short vrsn9_type7 = (PROTOCOL_VER << 7) | msg_type;
	unsigned short msg_length;
	SBCP_Msg_Header msg_header;
	SBCP_Atr_Header atr_header;
	char* buffer = new char[MAXBUFSIZE]();
	char *p = buffer, *head = buffer;
	char *temp;

	msg_header.vrsn9_type7 = vrsn9_type7;
	msg_header.vrsn9_type7 = htons(msg_header.vrsn9_type7);
	p += sizeof(struct SBCP_Msg_Header);// reserve space for message header

	if (msg_type == JOIN || msg_type == SEND || msg_type == IDLE)
	{
		int temp_len = 0;
		temp = p;// Head pointer of atr_header
		p += sizeof(struct SBCP_Atr_Header);//reserve space for attribute header
		atr_header.type = msg_type == JOIN? Username : Message;
		
		atr_header.type = htons(atr_header.type);
		if (msg_type != IDLE)
		{
			strcpy(p, msg);
			temp_len = strlen(msg);
		}
		
		atr_header.length = sizeof(atr_header) + temp_len + 1;
		
		msg_header.length = sizeof(msg_header) + atr_header.length;
		atr_header.length = htons(atr_header.length);
		memcpy(temp, &atr_header, sizeof(atr_header));// copy info of attribute header to buffer
		

		int len = msg_header.length;
		msg_header.length = htons(msg_header.length);
		memcpy(head, &msg_header, sizeof(msg_header));//copy info of message header to buffer
		memcpy(msg, buffer, MAXBUFSIZE);//pack the packet back to msg
		return len;
		
	}

	return -1;
}

int Packet_Recv(char* msg, int *num)
{
	SBCP_Msg_Header msg_header;
	SBCP_Atr_Header atr_header;
	char* buffer = new char[MAXBUFSIZE]();
	int type = 0;

	//extract the message header from message
	memcpy(&msg_header, msg, sizeof(msg_header));
	msg_header.vrsn9_type7 = ntohs(msg_header.vrsn9_type7);
	msg_header.length = ntohs(msg_header.length);
	//extract the attribute header from message
	memcpy(&atr_header, msg + sizeof(msg_header), sizeof(atr_header));
	atr_header.type = ntohs(atr_header.type);
	atr_header.length = ntohs(atr_header.length);

	/* Check if the SBCP message header is ACK, NAK, ONLINE, 
		OFFLINE or IDLE, and if the relative attribute header
		correct*/
	type = msg_header.vrsn9_type7 & 0x7f;
	if (msg_header.vrsn9_type7 >> 7 != PROTOCOL_VER ||
		(type != ACK || atr_header.type != Client_Count) &&
		(type != NAK || atr_header.type != Reason) &&
		(type != ONLINE || atr_header.type != Username) &&
		(type != OFFLINE || atr_header.type != Username) &&
		(type != IDLE || atr_header.type != Username) &&
		(type != FWD || atr_header.type != Message))
	{
		return -1;
	}

	if (type == ACK)
	{
		memcpy(num, msg + sizeof(msg_header) + sizeof(atr_header), sizeof(int));
		*num = ntohl(*num);
		memcpy(buffer, msg + sizeof(msg_header) + sizeof(atr_header) * 2 + sizeof(int),
				MAXBUFSIZE - sizeof(msg_header) - sizeof(atr_header) * 2 + sizeof(int));
		memcpy(msg, buffer, MAXBUFSIZE);
	}
	else
	{
		memcpy(buffer, msg + sizeof(msg_header) + sizeof(atr_header),
			atr_header.length - sizeof(atr_header));
		memcpy(msg, buffer, MAXBUFSIZE);

	}
	return type;
}