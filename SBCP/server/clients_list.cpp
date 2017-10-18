#include <stdlib.h>
#include <string.h>
#include "clients_list.h"


//Clients list Initial
struct Client_info* Init_list(void)
{
	Client_info* list = new Client_info();
	if (list == NULL) 
	{
		return NULL;
	}

	list->client_name = new char[NAME_LEN + 1];
	if (list->client_name == NULL)
	{
		free(list);
		return NULL;
	}

	strcpy(list->client_name, SERVERDOWN);
	list->next = NULL;
	list->client_socket = 0;

	return list;
}

int Add_client(struct Client_info* list, const int socket, const char* name)
{//add a new client
	Client_info* node1;
	Client_info* node2;

	node1 = list;
	//check if there is a duplicate of name
	while (node1 != NULL)
	{
		if (strcmp(node1->client_name, name) == 0)
		{
			return NAME_DUPLICATE;
		}
		node2 = node1;
		node1 = node1->next;
	}
	//there is no duplicate and node2->next will be the space for next client
	node2->next = new Client_info();
	if (node2->next == NULL)
	{
		return LACK_OF_MEMORY;
	}
	node1 = node2;
	node2 = node2->next;
	node2->client_name = new char[NAME_LEN];
	if (node2->client_name == NULL)
	{
		free(node2);
		node1->next = NULL;
		return LACK_OF_MEMORY;
	}
	//append the info of new client
	strcpy(node2->client_name, name);
	node2->client_socket = socket;
	node2->next = NULL;
	list->client_socket++;

	return ADD_SUCCESS;
}

const char* Del_client(struct Client_info* list, const int socket)
{//delete a client
	if (list == NULL)
	{
		return NULL;
	}

	Client_info* node1;
	Client_info* node2;

	node1 = list->next;
	node2 = list;
	while (node1 !=NULL)
	{
		if (node1->client_socket == socket)
		{
			char* name = new char[NAME_LEN]();
			strcpy(name, node1->client_name);
			node2->next = node1->next;
			free(node1->client_name);
			free(node1);
			list->client_socket--;
			return name;
		}
		node2 = node1;
		node1 = node1->next;
	}

	return NULL;
}

bool Find_Client(struct Client_info* list, const int socket, char* name)
{
	Client_info* cur = list->next;
	while (cur != NULL)
	{
		if (cur->client_socket == socket)
		{
			strcpy(name, cur->client_name);
			return true;
		}
		cur = cur->next;
	}
	return false;
}
