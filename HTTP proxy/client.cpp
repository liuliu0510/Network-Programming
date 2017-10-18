#include "headers.h"
 
#define PORT "2025" 

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}


string Get_msginfo(string msg)
{
	if (msg[msg.size() - 1] != EOF)
		return msg;

	return msg.substr(0, msg.size() - 1);
}

int main(int argc, char **argv) 
{ 
    int clientsock;  
    char buf[buffer_size];
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN];
     
   //test the number of arguments
	if (argc != 4)
	{
		cout << "Invalid number of parameters." << endl;
		cout << "Please use ./client <proxy address> <proxy port> <URL to retrieve>" << endl;
		exit(1);
	} 
    
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
	
    if ((rv = getaddrinfo(argv[1], PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and connect to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((clientsock = socket(p->ai_family, p->ai_socktype,  p->ai_protocol)) == -1) {
            perror("client: socket");
            continue;
        }

        if (connect(clientsock, p->ai_addr, p->ai_addrlen) == -1) {
            close(clientsock);
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
    printf("client: connecting to %s\n", s);

    freeaddrinfo(servinfo); // all done with this structure
    
    string url = argv[3];
	url += EOF;
	
    memset(&buf, 0, sizeof(buf));	
	for(int i = 0; i < url.size(); i++)
		buf[i] = url[i];
	int bytes_sent = send(clientsock, buf, url.size(),0);

	cout << "Information received:" << endl;
	int bytes_recv = 1;
	string msg = "0";
	bool value = true;
	while (value)
	{
		memset(&buf, 0, sizeof(buf));
		bytes_recv = recv(clientsock, buf, sizeof(buf),0);
		msg = "";
		for (int i = 0; i < bytes_recv; i++)
			msg += buf[i];

		value = msg[msg.size() - 1] != EOF;
		msg = Get_msginfo(msg);
		cout << msg;
	}
	cout << endl << "Complete!" << endl;
	close(clientsock);
	return 0;
}