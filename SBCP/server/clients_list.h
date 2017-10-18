#ifndef _CLIENTS_LIST
#define _CLIENTS_LIST

#define NAME_LEN 16
#define SERVERDOWN "#shutdown\n"
#define CLIENTDOWN "#exit\n"
#define MAXBUFSIZE 600
#define PROTOCOL_VER 3

enum COMMOND
{
	SERVER_SHUTDOWN,
	NAME_DUPLICATE,
	LACK_OF_MEMORY,
	ADD_SUCCESS
};

struct Client_info        //Node for a user's information
{
	char *client_name; //User's name
	int client_socket;             //User's Socket Number
	struct Client_info* next;
};

struct Client_info* Init_list(void);
int Add_client(struct Client_info* list, const int socket, const char* name);
const char* Del_client(struct Client_info* list, const int socket);
bool Find_Client(struct Client_info* list, const int socket, char* name);

#endif