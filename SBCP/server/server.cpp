#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

#include "clients_list.h"
#include "common_header.h"

#define MSG_BUF 600

void failure_send(char* msg, unsigned short msg_type, struct Client_info *list, int socket);
void broadcast_send(struct Client_info* clients_list, char* msg, int socket, int len);

int main(int argc, char const *argv[])
{
	unsigned short port; // port number
	int sock_listener; // lis
	int yes = 1;
	int cur_clients = 0;
	int max_sock;
	int new_socket; // newly accept()ed socket descriptor
	char msg[MSG_BUF]; //buffer
	bool server_status = false;
	struct addrinfo hints, *server_info, *p_serv;
	struct Client_info* clients_list = NULL; // clients list
	fd_set master_list, ready_list;
	socklen_t addr_len;
	struct sockaddr_storage remote_addr; // client address

	if (argc != 4)
	{
		printf("Error: wrong input parameters\n");
		printf("Usage: ./exe_name server_ip port_number max_client_number\n");
		fflush(stdout);
		exit(1);
	}
	else
	{
		port = atoi(argv[2]);
		if (port < 1024)
		{
			printf("Illegal Port Number: %d\n", port);
			fflush(stdout);
			printf("Users port number should be at least larger than 1023");
			fflush(stdout);
			exit(1);
		}
	}
	const int max_clients = atoi(argv[3]); // max number of clients


	memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC; // Both IPv4 and IPv6 are ok
    hints.ai_socktype = SOCK_STREAM; // TCP stream sockets
    hints.ai_flags = AI_PASSIVE; // fill in my IP

    if (getaddrinfo(argv[1], argv[ 2 ], &hints, &server_info) != 0)
	{
		printf("Error in getting IP address\n");
    	return 1;
	}

	// loop through all the result and bind to the first we can
	for (p_serv = server_info; p_serv != NULL; p_serv = p_serv->ai_next)
	{
		if ((sock_listener = socket(p_serv->ai_family, p_serv->ai_socktype, p_serv->ai_protocol)) == -1)
		{
			continue;
		}
		if (setsockopt(sock_listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
		{
			printf("Set Socket failure\n");
			exit(1);
		}
		if (bind(sock_listener, p_serv->ai_addr, p_serv->ai_addrlen) == -1)
		{
			close(sock_listener);
			continue;
		}
		break;
	}

	freeaddrinfo(server_info);

	if (p_serv == NULL)  {
        fprintf(stderr, "server: failed to bind\n");
        fflush(stdout);
        exit(1);
    }

    if(listen(sock_listener, max_clients) == -1)
	{
		printf("Error in listening!");
		fflush(stdout);
		system("pause");
		close(sock_listener);
		exit(1);
	}

	FD_ZERO(&master_list);    // clear the master and temp sets
    FD_ZERO(&ready_list);
	FD_SET(sock_listener, &master_list);     // add the listener to the master set
	FD_SET(fileno(stdin), &master_list);
	max_sock = sock_listener;
	
	clients_list = Init_list(); //create clients' list
	if(clients_list == NULL)
	{
		printf("Lack of Memory! Unable to set a user list\n");
		fflush(stdout);
		system("pause");
		close(sock_listener);
		exit(1);
	}
	printf("Server has been created...\n\n");
	printf("waiting for clients on port %d\n", port);
	printf("Limit Number of Clients: %d\n\n", max_clients);
	fflush(stdout);
	printf("Waiting...\n\n");
	
	while (1)
	{
		ready_list = master_list;
		fflush(stdout);
		fflush(stdin);

		if (select(max_sock + 1, &ready_list, NULL, NULL, NULL) == -1)
		{
			printf("Error Select\n");
			exit(1);
		}
		//traverse all the existing connections looking for data to read
		for (int active = 0; active <= max_sock; active++)
		{
			if (FD_ISSET(active, &ready_list)) // We got one active socket
			{
				if (active == sock_listener)//handle new connections
				{
					addr_len = sizeof(remote_addr);
					new_socket = accept(sock_listener, (struct sockaddr*)&remote_addr,&addr_len);
					
					if (new_socket == -1)
					{
						printf("Error accepting new socket\n");
						fflush(stdout);
						continue;
					}
					// A new TCP connection has established
					int test = recv(new_socket, msg, MSG_BUF, 0);

					int status = Packet_Recv(msg);
					//printf("%d\n", status);
					int len = 0;
					if (cur_clients == max_clients)
					{
						memset(msg, '\0', MSG_BUF);
						strcpy(msg, "The number of Clients has reached its limit\n");
						failure_send(msg, NAK, NULL, new_socket);
						continue;
					}
					// Wrong Packet
					if (status != JOIN)
					{
						printf("Socket->%d format error\n", new_socket);
						fflush(stdout);
						memset(msg, '\0', MSG_BUF);
						strcpy(msg,"Packet format error\n");
						failure_send(msg, NAK, NULL, new_socket);
						continue;
					}
					//ready to communicate
					int add_status = Add_client(clients_list, new_socket, msg);
					//printf("here\n");
					//printf("%d\n", add_status);
					if (add_status == NAME_DUPLICATE || strcmp(msg, SERVERDOWN) == 0)
					{
						printf("Fail to connect, reason: Duplicate Name->%s\n", msg);
						fflush(stdout);
						strcpy(msg, "Duplicate Name");
						failure_send(msg, NAK, NULL, new_socket);
						continue; 
					}
					if (add_status == LACK_OF_MEMORY)
					{
						printf("Lack of Memory. Disconnectiong User\n");
						fflush(stdout);
						strcpy(msg, "Fail to connect, reason: lack of memory");
						failure_send(msg, NAK, NULL, new_socket);
						continue;
					}
					printf("New Client Accepted!\n");
					fflush(stdout);
					FD_SET(new_socket, &master_list);
					max_sock = new_socket > max_sock ? new_socket : max_sock;
					cur_clients++;
					printf("New Connection from %s on socket %d\n", msg, new_socket);
					fflush(stdout);
					len = Packet_Send(msg, ONLINE, NULL);
					broadcast_send(clients_list, msg, new_socket, len);
					len = Packet_Send(msg, ACK, clients_list);
					send(new_socket, msg, len + 1, 0);

					//server_status = true;
				}
				else if (active != sock_listener && active != fileno(stdin))//handle exist connections
				{
					char* name = new char[MAXBUFSIZE]();
					memset(msg, '\0', MSG_BUF);
					recv(active, msg, sizeof(char)*MSG_BUF, 0);
					int status = Packet_Recv(msg);
					if (status != SEND && status != IDLE)
					{
						printf("The Connection from socket %d is terminated by the client ", active);
						fflush(stdout);
						
						strcpy(name, Del_client(clients_list, active));//Delete the client and return the name
						printf("%s\n", name);
						int len = Packet_Send(name, OFFLINE, NULL);
						cur_clients--;
						broadcast_send(clients_list, name, active, len);//send to all clients
						FD_CLR(active, &master_list);
						close(active);
						continue;
					}
					if (strcmp(msg, CLIENTDOWN) == 0) {
						printf("The Connection from socket %d is terminated by the client ", active);
						fflush(stdout);
						
						strcpy(name, Del_client(clients_list, active));//Delete the client and return the name
						printf("%s\n", name);
						int len = Packet_Send(name, OFFLINE, NULL);
						cur_clients--;
						broadcast_send(clients_list, name, active, len);//send to all clients
						FD_CLR(active, &master_list);
						close(active);
						continue;
					}
					Find_Client(clients_list, active, name);
					if (status == IDLE)
					{
						strcpy(msg, "Client \"");
						strcat(msg, name);
						strcat(msg, "\" is IDLE!");
						memset(name, '\0',MAXBUFSIZE);
						strncpy(name, msg, MAXBUFSIZE);
					}
					else
					{
						strcat(name, ": ");
						strcat(name, msg);
					}
					
					int len = Packet_Send(name, FWD, NULL);
					broadcast_send(clients_list, name, active, len);
				}
				else //server shutdown
				{
					memset(msg, '\0', MAXBUFSIZE);
					fgets(msg, MAXBUFSIZE, stdin);
					if (strcmp(msg, SERVERDOWN) == 0)
					{
						int len = Packet_Send(msg, OFFLINE, NULL);
						broadcast_send(clients_list, msg, 0, len);
						fflush(stdin);
						server_status = true;
						break;
					}
				}
			}
		}
		if (server_status)
		{
			printf("Server Shut Down\n");
			close(sock_listener);
			break;
		}
	}

	return 0;
}

void failure_send(char* msg, unsigned short msg_type, struct Client_info *list, int socket)
{
	int len = Packet_Send(msg, msg_type, list);
	send(socket, msg, len + 1, 0);
	close(socket);
	return;
}

void broadcast_send(struct Client_info* clients_list, char* msg, int socket, int len)
{
	struct Client_info* cur = clients_list->next;

	//printf("send %s to", msg + 8);
	fflush(stdout);
	while (cur != NULL)//send message to all clients
	{
		if (cur->client_socket == socket)
		{
			cur = cur->next;
			continue;
		}
		//printf("%s\n", cur->client_name);
		fflush(stdout);
		send(cur->client_socket, msg, len + 1, 0);
		cur = cur->next;
	}
}