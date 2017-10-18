#include "packets.h"
#define MYPORT "2025"


// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}


int main(int argc, char** argv)
{
    int sockfd, newfd;
    fd_set master; //master file descriptor list
    fd_set temp_fds; //temp file descriptor list for select()
    int fdmax;       // maximum file descriptor number
    
    struct addrinfo hints, *servinfo, *p;
    int rv,i;
    int nbytes;
    struct sockaddr_storage their_addr;
    char buf[buffer_size];
    socklen_t addr_len;
    char s[INET6_ADDRSTRLEN];
    
    FD_ZERO(&master);   //clear the master and temp sets
    FD_ZERO(&temp_fds); 
    
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC; 
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE; // use my IP	
	
	if (argc != 3)
	{
		cout << "Invalid number of parameters." << endl;
		cout << "Please use ./server IPAdrress port" << endl;
		exit(1);
	}
	
	if ((rv = getaddrinfo(NULL, MYPORT, &hints, &servinfo)) != 0) 
	{
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }
	
	// loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) 
    {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,p->ai_protocol)) == -1)
        {
            perror("socket");
            continue;
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1)
        {
            close(sockfd);
            perror("bind");
            continue;
        }

        break;
    }

    if (p == NULL) 
    {
        fprintf(stderr, "failed to bind socket\n");
        return 2;
    }

    freeaddrinfo(servinfo); // all done with this
    printf("waiting to recvfrom...\n");
    
    //add the listener to the master set 
    FD_SET(sockfd, &master);
    
    //keep track of the biggest file descriptor
    fdmax = sockfd;
    
    //main loop
    for(;;)
    {   
        temp_fds = master; //copy it
        if(select(fdmax+1, &temp_fds, NULL, NULL, NULL) == -1)
        {
        	perror("select");
        	exit(2);
        }
      
        for(i=0; i<=fdmax; i++)
        {
        	if (FD_ISSET(i,&temp_fds)) // we got one!
        	{ 
        	   if (i == sockfd)  // handle new connection
        	   {    
        	   	   
                  	//create the new socket connection
	                int newfd = socket(PF_INET, SOCK_DGRAM, 0);
                    
                    
                    // kill "Address already in use" error message
                    int tr=1;
                    if (setsockopt(newfd,SOL_SOCKET,SO_REUSEADDR,&tr,sizeof(int)) == -1)
                    {
                       perror("setsockopt");
                       exit(3);
                    }
                     close(sockfd);
                    if (bind(newfd, p->ai_addr, p->ai_addrlen) == -1)
                    {
                        close(newfd);
                        perror("bind");
                        exit(4);
                    }
                    
                    cout << "Socket = " << newfd << endl;
                    FD_SET(newfd,&master);  //add to master set
                    if(newfd > fdmax)
                    {
                       fdmax=newfd;	
                    }
                    
                     //receive data request from a client
                    memset(&buf, 0, sizeof(buf));	
                    addr_len = sizeof their_addr;
                    nbytes = recvfrom(newfd, buf, buffer_size-1 , 0, (struct sockaddr *)&their_addr, &addr_len);
                    if (nbytes == -1)
                    {
                       perror("recvfrom");
                       exit(5);
                    }
                    if (nbytes == 0) // connection closed 
                    {
                    	printf("socket %d hung up\n",newfd);
                    }
    	            printf("got packet from %s\n",
                    inet_ntop(their_addr.ss_family,get_in_addr((struct sockaddr *)&their_addr), s, sizeof s));
                    printf("packet is %d bytes long\n", nbytes);
        
                    //parse the request
	                ClientPacket  cli_packet = ClientPacket (buf);
	                
	                //open the desired file
                   	ifstream file;
                  	file.open(cli_packet.GetFileName().c_str());
	                
	                //if the file does not exist, send an error
                 	if(!file.is_open())
                 	{
	                   	char errorBuffer[5] = {0, 5, 0, 1, 0};
		                sendto(newfd, errorBuffer, 5, 0, (struct sockaddr *) &their_addr, addr_len);
		                close(newfd);
	            	    exit(6);
                	}
	            
	               	unsigned int blocknum = 1;
	                unsigned int formerBlocknum = -1; //used to detect wrap around
                 	unsigned int wrapnum = 0;

                	//continue until file transfer is finished
                	while(!file.eof())
                	{
	                 	memset(&buf, 0, sizeof(buf));
	                	//create the data packet
	                	DataPacket dataPacket(file, blocknum, wrapnum);
	                	//place the data packet in the buffer
	                	dataPacket.data_into_buffer(buf);
		                
	                	//send the data packet
	                	sendto(newfd, buf, dataPacket.packet_size(), 0, (struct sockaddr *)&their_addr, addr_len);
		
	                 	bool Value = false;
                		//wait for the received packet
                		while(!Value)
                 		{
	                		memset(&buf, 0, sizeof(buf));
	                        
	                		int bytesRev = recvfrom(newfd, buf, sizeof(buf), 0, (struct sockaddr *)&their_addr,&addr_len);
                 			//decode the received packet
                		    cli_packet = ClientPacket(buf);
                 			if(cli_packet.GetOPCODE() == ACK)
	             			Value = true;
	                	}
	                 	//increment to the next block of data
	                 	formerBlocknum = blocknum;
	                	blocknum = cli_packet.GetBlocknum()+1;
	                	//check for the block having been wrapped around
                		if(blocknum == 1 & formerBlocknum!= 1)
	                 	{
	                		wrapnum++;
	                 	}
                   	}
	
	                cout << "Socket " << newfd << " finished!" << endl;
	                //close(newfd);
	                sockfd = socket(p->ai_family, p->ai_socktype,p->ai_protocol);
	                bind(sockfd, p->ai_addr, p->ai_addrlen);
	                
        	   }
        	   else
        	   {
        	        //receive data request from a client
                    memset(&buf, 0, sizeof(buf));	
                    addr_len = sizeof their_addr;
                    nbytes = recvfrom(i, buf, buffer_size-1 , 0, (struct sockaddr *)&their_addr, &addr_len);
                    if (nbytes == -1)
                    {
                       perror("recvfrom");
                       exit(7);
                    }
                    if (nbytes == 0) // connection closed 
                    {
                    	printf("socket %d hung up\n",i);
                    }
    	            printf("got packet from %s\n",
                    inet_ntop(their_addr.ss_family,get_in_addr((struct sockaddr *)&their_addr), s, sizeof s));
                    printf("packet is %d bytes long\n", nbytes);
        
                    //parse the request
	                ClientPacket  cli_packet = ClientPacket (buf);
	                
	                //open the desired file
                   	ifstream file;
                  	file.open(cli_packet.GetFileName().c_str());
	                
	                //if the file does not exist, send an error
                 	if(!file.is_open())
                 	{
	                   	char errorBuffer[5] = {0, 5, 0, 1, 0};
		                sendto(i, errorBuffer, 5, 0, (struct sockaddr *) &their_addr, addr_len);
		                close(i);
	            	    exit(8);
                	}
	            
	               	unsigned int blocknum = 1;
	                unsigned int formerBlocknum = -1; //used to detect wrap around
                 	unsigned int wrapnum = 0;

                	//continue until file transfer is finished
                	while(!file.eof())
                	{
	                 	memset(&buf, 0, sizeof(buf));
	                	//create the data packet
	                	DataPacket dataPacket(file, blocknum, wrapnum);
	                	//place the data packet in the buffer
	                	dataPacket.data_into_buffer(buf);
		                
	                	//send the data packet
	                	sendto(i, buf, dataPacket.packet_size(), 0, (struct sockaddr *)&their_addr, addr_len);
		
	                 	bool Value = false;
                		//wait for the received packet
                		while(!Value)
                 		{
	                		memset(&buf, 0, sizeof(buf));
	                        
	                		int bytesRev = recvfrom(i, buf, sizeof(buf), 0, (struct sockaddr *)&their_addr,&addr_len);
                 			//decode the received packet
                		    cli_packet = ClientPacket(buf);
                 			if(cli_packet.GetOPCODE() == ACK)
	             			Value = true;
	                	}
	                 	//increment to the next block of data
	                 	formerBlocknum = blocknum;
	                	blocknum = cli_packet.GetBlocknum()+1;
	                	//check for the block having been wrapped around
                		if(blocknum == 1 & formerBlocknum!= 1)
	                 	{
	                		wrapnum++;
	                 	}
                   	}
	
	                cout << "Socket " << i << " finished!" << endl;

        	     }
            }  
    	
        }
    }
    
    close(sockfd);
    return 0;
}	
	
	

  
 