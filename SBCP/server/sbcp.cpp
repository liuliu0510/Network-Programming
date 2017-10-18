#include <string.h>
#include <netinet/in.h>
#include "clients_list.h"
#include "common_header.h"


/*
there are two fucntions to pack and unpack a massage of server
*/

int Packet_Recv(char* msg)
{
	/* msg is a SBCP packet.
	the function unpack the packet, return the type of the packet.
	*/
	SBCP_Msg_Header msg_header;
	SBCP_Atr_Header atr_header;
	char* buffer = new char[MAXBUFSIZE]();

	memcpy(&msg_header, msg, sizeof(msg_header));
	msg_header.vrsn9_type7 = ntohs(msg_header.vrsn9_type7);
	msg_header.length = ntohs(msg_header.length);
	memcpy(&atr_header, msg + sizeof(msg_header), sizeof(atr_header));
	atr_header.type = ntohs(atr_header.type);
	atr_header.length = ntohs(atr_header.length);

	if ((msg_header.vrsn9_type7 & 0x7f) == IDLE)
	{
		memset(msg, '\0', MAXBUFSIZE);
		return IDLE;
	}
	else if ((msg_header.vrsn9_type7 >> 7 != PROTOCOL_VER) ||
		((msg_header.vrsn9_type7 & 0x7f != SEND || atr_header.type != Message) &&
		(msg_header.vrsn9_type7 & 0x7f != JOIN || atr_header.type != Username)))
	{
		return -1;
	}

	strncpy(buffer, msg + sizeof(msg_header) + sizeof(atr_header),
			atr_header.length - sizeof(atr_header));
	strncpy(msg, buffer, MAXBUFSIZE);

	return msg_header.vrsn9_type7 & 0x7f;
}

int Packet_Send(char* msg, unsigned short msg_type, struct Client_info *list)
{
	//first 9 bits are protocal version and next 7 bits are message type
	unsigned short vrsn9_type7 = (PROTOCOL_VER << 7) | msg_type;
	SBCP_Msg_Header msg_header;
	SBCP_Atr_Header atr_header;
	Client_info* client_pointer;
	char* buffer = new char[MAXBUFSIZE]();
	char* packet_pointer = buffer;

	msg_header.vrsn9_type7 = htons(vrsn9_type7);
	packet_pointer = packet_pointer + sizeof(struct SBCP_Msg_Header);// reserve place for message header

	if (msg_type == ACK && list != NULL) // sending ACK
	{
		atr_header.type = Client_Count;
		atr_header.type = htons(atr_header.type);
		atr_header.length = sizeof(atr_header) + sizeof(list->client_socket);
		unsigned short temp_length1 = atr_header.length;
		unsigned short temp_length2 = 0;
		atr_header.length = htons(atr_header.length);
		memcpy(packet_pointer, &atr_header, sizeof(atr_header));//copy attribute header
		packet_pointer = packet_pointer + sizeof(atr_header);//move pointer after atr_header
		
		int clist = htonl(list->client_socket);
		memcpy(packet_pointer, &clist, sizeof(clist));//copy number of clients to packet
		packet_pointer = packet_pointer + sizeof(clist);
		char* temp_p = packet_pointer;
		packet_pointer = packet_pointer + sizeof(atr_header);

		for (client_pointer = list->next; client_pointer != NULL; client_pointer = client_pointer->next)
		{//copy all clients' names
			strcpy(packet_pointer, client_pointer->client_name);
			packet_pointer += strlen(client_pointer->client_name);
			*packet_pointer++ = '\n';
			temp_length2 += strlen(client_pointer->client_name) + 1;
		}
		*--packet_pointer = '\0';

		atr_header.type = Username;
		atr_header.length = sizeof(atr_header) + temp_length2;
		temp_length2 = atr_header.length;
		atr_header.type = htons(atr_header.type);
		atr_header.length = htons(atr_header.length);
		memcpy(temp_p, &atr_header, sizeof(atr_header));
		unsigned short len = msg_header.length = sizeof(msg_header) + temp_length2 + temp_length1;
		msg_header.length = htons(msg_header.length);
		memcpy(buffer, &msg_header, sizeof(msg_header));
		memcpy(msg, buffer, MAXBUFSIZE);

		return len;


	}
	else if (msg_type == ONLINE || msg_type == OFFLINE || msg_type == NAK ||
				msg_type == FWD)
	{
		char* temp_p = packet_pointer;//store current position
		packet_pointer += sizeof(struct SBCP_Atr_Header);//find type position
		strcpy(packet_pointer, msg);

		switch (msg_type)
		{
			case FWD:
				atr_header.type = Message;
				break;
			case NAK:
				atr_header.type = Reason;
				break;
			case ONLINE:
			case OFFLINE:
				atr_header.type = Username;
				break;
			default:
				break;
		}

		atr_header.type = htons(atr_header.type);
		atr_header.length = sizeof(atr_header) + strlen(msg) + 1;
		int len = msg_header.length = sizeof(msg_header) + atr_header.length;
		atr_header.length = htons(atr_header.length);
		memcpy(temp_p, &atr_header, sizeof(atr_header));
		msg_header.length = htons(msg_header.length);
		memcpy(buffer, &msg_header, sizeof(msg_header));
		memcpy(msg, buffer, MAXBUFSIZE);

		return len;
	}
	else
	{
		return -1;
	}

}