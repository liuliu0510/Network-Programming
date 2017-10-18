#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "client_header.h"

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char const *argv[])
{
	int sockfd;
	int send_len, sock_send = 0, sock_recv;
	int num_of_clients;
	int msg_type;
	int fd_max;
	bool shutdown = false;
    char* msg = new char[MAXBUFSIZE]();
    char* buffer = new char[MAXBUFSIZE]();
    struct addrinfo hints, *serv_info, *p;
    fd_set read_fds, write_fds;
    char s[INET6_ADDRSTRLEN];
    FILE* stream = stdin;
    struct timeval tv;//count IDLE

    if (argc != 4) {
        fprintf(stderr,"usage: ./client name host_ip host_port\n");
        exit(1);
    }

    if (strlen(argv[1]) >= NAME_LEN)
    {
    	printf("client name %s is too long, it should be at most 15 characters", argv[1]);
    	exit(1);
    }

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((getaddrinfo(argv[2], argv[3], &hints, &serv_info)) != 0) {
        printf("Fail to get server address\n");
        return 1;
    }

    for(p = serv_info; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("client: socket");
            continue;
        }

        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("client: connect");
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "client: failed to connect\n");
        return 2;
    }

    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), s, sizeof s);
    printf("Client: connecting to %s\n", s);
    fflush(stdout);

    freeaddrinfo(serv_info); // all done with this structure

    strcpy(msg, argv[1]);
    if ( (send_len = Packet_Send(msg, JOIN)) == -1)
    {
    	if ((sock_send = send(sockfd, CLIENTDOWN, send_len, 0)) == -1) {
	    	perror("Wrong packet!");
		}
    	perror("Fail to join!");
    	close(sockfd);
    	exit(1);
    }
    
    if ((sock_send = send(sockfd, msg, send_len, 0)) == -1) {
    	perror("Unkown message sent");
    	perror("Fail to join!");
    	close(sockfd);
    	exit(1);
	}
	if ((sock_recv = recv(sockfd, buffer, MAXBUFSIZE-1, 0)) == -1) {
        perror("Receive error, fail to join!");
        exit(1);
    }
    
    buffer[sock_recv] = '\0';
    msg_type = Packet_Recv(buffer, &num_of_clients);//get the message type
	if (msg_type == -1)
	{
		printf("Wrong ACK or NAK packet received!\n");
		fflush(stdout);
		exit(1);
	}
	if (msg_type == ACK)
	{
		if (num_of_clients == 1)
		{
			printf("There is only %d client:\n%s\n", num_of_clients, buffer);
			fflush(stdout);
		} else {
			printf("There are %d clients:\n%s\n", num_of_clients, buffer);
			fflush(stdout);
		}
		printf("*********************\n\n");
	}
	else if (msg_type == NAK)
	{
		printf("%s\n", buffer);
		fflush(stdout);
		close(sockfd);
		exit(1);
	}
	else
	{
		printf("Error in receiving ACK or NAK packet\n");
		close(sockfd);
		exit(1);
	}

	FD_ZERO(&read_fds);
	fd_max = sockfd > fileno(stream) ? sockfd : fileno(stream);
	tv.tv_sec = 10;
	tv.tv_usec = 0;

	while(1)
	{
		FD_SET(sockfd, &read_fds);  // read from sockfd
        FD_SET(fileno(stream), &read_fds);  // use standard input to write
        memset(msg, '\0', MAXBUFSIZE);// clear the message buffer

		if (select(fd_max+1, &read_fds, NULL, NULL, &tv) == -1) {   // select from standard input and socket
            perror("select");
            exit(1);
        }
        if (FD_ISSET(sockfd, &read_fds))  // read from sockfd)//receve message forwarded by server
        {
        	sock_recv = recv(sockfd, msg, MAXBUFSIZE-1, 0);
        	msg_type = Packet_Recv(msg, &num_of_clients);
        	if (msg_type == ONLINE)
        	{
        		printf("*************************************\n");
        		printf("A new client name \"%s\" come online \n", msg);
        		printf("*************************************\n\n");
        		fflush(stdout);

        	}
        	else if (msg_type == OFFLINE)
        	{
        		if (strcmp(msg,SERVERDOWN) == 0)
        		{
        			printf("Server has been shutdown!\n");
        			shutdown = true;
        			continue;
        		}
        		printf("*************************************\n");
        		printf("Client \"%s\" offline\n", msg);
        		printf("*************************************\n\n");
        	}
        	else if (msg_type == FWD)
        	{
        		
        		printf("%s\n", msg);
        	}
        }
        //if input a meesage
        else if (FD_ISSET(fileno(stdin), &read_fds))
        {
        	fgets(msg, MAXBUFSIZE, stdin);
        	if (strcmp(msg, CLIENTDOWN) == 0)//check if the client want to leave
        	{
        		send_len = Packet_Send(msg, SEND);
        		send(sockfd, msg, send_len, 0);
        		printf("You are offline!\n");
        		printf("*****************\n");
        		fflush(stdout);
        		break;
        	}
        	//send nomal message
        	printf("\n");
        	send_len = Packet_Send(msg, SEND);
        	send(sockfd, msg, send_len, 0);
        	tv.tv_sec = 10;
        }
        else
        {
        	printf("*************************************\n");
    		printf("Send IDLE!\n");
    		printf("*************************************\n\n");	
        	send_len = Packet_Send(msg, IDLE);
        	send(sockfd, msg, send_len, 0);
        	tv.tv_sec = 10;
        }
        FD_ZERO(&read_fds);
        if (shutdown)
        {
        	break;
        }

	}
	return 0;
}