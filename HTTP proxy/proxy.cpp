#include "date.h"
#include "web.h"

#define PORT "2025" 

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

// get the message infomation 
string Get_msginfo(string msg)
{
	if (msg[msg.size() - 1] != EOF)
		return msg;

	return msg.substr(0, msg.size() - 1);
}
 
int main(int argc, char **argv)
{ 
    int sockfd, newfd;  //listening and newly accepted socket discriptor 
    fd_set master;  //master file descriptor list
    fd_set temp_fds;  //temp file descriptor list for select()
    int fdmax;        // maximum file descriptor number
    
    struct sockaddr_storage remoteaddr; //client address
    socklen_t addr_len;
    
    struct addrinfo hints, *servinfo, *p;
    int rv,i;
    
    char s[INET6_ADDRSTRLEN];
    
    FD_ZERO(&master);   //clear the master and temp sets
    FD_ZERO(&temp_fds); 
    
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC; 
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use my IP	
	
	if (argc != 3)
	{
		cout << "Invalid number of parameters." << endl;
		cout << "Please use ./proxy <ip to bind> <port to bind>" << endl;
		exit(1);
	}
	
	if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) 
	{
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        exit(2);
    }
	
	for(p = servinfo; p != NULL; p = p->ai_next) {
        sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (sockfd < 0) { 
            continue;
        }
        
        int yes=1;
        // lose the pesky "address already in use" error message
        setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
        
        cout << "bind" << endl;
        if (bind(sockfd, p->ai_addr, p->ai_addrlen) < 0) {
            close(sockfd);
            continue;
        }

        break;
    }
    // if we got here, it means we didn't get bound
    if (p == NULL) 
    {
        fprintf(stderr, "failed to bind socket\n");
        exit(3);
    }

    freeaddrinfo(servinfo); // all done with this
    
    //listen
    cout << "listen" << endl;
    if (listen(sockfd,10) == -1){
    	perror("listen");
    	exit(4);
    }
    
    //add the listener to the master set 
    FD_SET(sockfd, &master);
    
    //keep track of the biggest file descriptor
    fdmax = sockfd;	

    //creates the cache
	Cache cache(15);

	//main loop
    for(;;)
    {   
        temp_fds = master; //copy it
        if(select(fdmax+1, &temp_fds, NULL, NULL, NULL) == -1)
        {
        	perror("select");
        	exit(5);
        }
      
        for(i=0; i<=fdmax; i++)
        {
        	if (FD_ISSET(i,&temp_fds)) // we got one!
        	{ 
        	   if (i == sockfd)  // handle new connection
        	   {    
                  	//create the new socket connection
	                addr_len = sizeof remoteaddr;
	                newfd = accept(sockfd,(struct sockaddr *)&remoteaddr, &addr_len);
	                if (newfd == -1) 
	                {
                        perror("accept");
                    }
                    else{
                    	FD_SET(newfd, &master); // add to master set
                         if (newfd> fdmax)
                         {    // keep track of the max
                            fdmax = newfd;
                          }
                 
                      	}	
                    }
        	   else
        	   {   	//handle data from a client 
        	   	string msg = "";
                bool value = true;
                char buf[buffer_size];  //buffer for client data
                memset(&buf, 0, sizeof(buf));
                int bytes_recv = recv(i, buf, sizeof(buf),0);
   
                if (bytes_recv <=0)
                {  //got error or closed by client 
                   if(bytes_recv == 0){
                    	//connection closed 
                     	printf("socket %d hung up\n", i);
                    }
                    else{
                 	    perror("recv");
                     }
                    close(i);
                    FD_CLR(i, &master);
                }
                else{
                 	while (value)
                    {
                        msg = "";
                        for (int j = 0; j < bytes_recv; j++)
                        msg += buf[j];
                        msg = Get_msginfo(msg);
                        value = msg[msg.size() - 1] == '*';
                    } 
                    string url = msg;
                    string page = cache.Get_Page(url);
                    string* data = new string();
                 	*(data) = page;
            
                 	int position = 0;

                 	//write bytes to the given client
 
                 	while (position < (data->size()))
                  	{ 
                		memset(&buf, 0, sizeof(buf));
                 		string msg = "";
                	    int nbytes=0; 
 
                 		for (int j = 0; j< buffer_size & position <(data->size()); j++)
                 		{
                			buf[j] = (*(data))[position];
                			msg = (*(data))[position];
                 			position++;
                 			nbytes++;
                  		}
                        int bytes_sent = send(i, buf, nbytes,0);
                     }	
                   }
        	   }
            } 
          }
        }
        
    close(sockfd);
    return 0;
}	
	
	
	
                     
	
	
	
	
	
	
	
	
	
 
 
	

 