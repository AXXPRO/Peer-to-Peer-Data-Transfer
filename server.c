
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <winsock2.h>
#include <stdint.h>

#pragma comment(lib, "ws2_32.lib")
struct threadData {
struct sockaddr_in clientData;
SOCKET clientSocket;
HANDLE mutex;
int index;
char username[100];
};
struct in_addr addrServer;

struct threadData* clients[10];
void initializeClients(){

for(int i =0; i < 10; i++)
{
	clients[i] = NULL;
}
}
int firstFree(){
	for(int i = 0; i < 10; i++)
		if(clients[i] == NULL)
			return i;
	return -1;
}
void markFree(int index){

	closesocket( clients[index]->clientSocket);
	free(clients[index]);

	clients[index] = NULL;
}
void addClient(struct threadData* el){
	clients[el->index] = el;
}
int numberClients(){
	int k = 0;
	for(int i = 0; i < 10; i++)
		if(clients[i] != NULL)
			k++;
	return k;
}

void sendEveryoneRefreshNotice(char* username){
uint8_t code = 1;
for(int i =0; i < 10; i++)
{
	if(clients[i]!=NULL && strcmp(username, clients[i]->username) != 0)
		{
			send(clients[i]->clientSocket, (const char*)&code, sizeof(code), 0);
		}
}
	
}

void sendUsernameIpToSocket(SOCKET s, char* username){

	
	for(int i =0 ; i<10; i++)
	{
	
		if(strcmp(clients[i]->username, username)==0)
			{
				
				uint8_t code=4;
				send(s, (const char*)&code, sizeof(code), 0);
				//send(s, (const char*)username,sizeof(char)*100,  0 );
				struct in_addr ip;
				ip = clients[i]->clientData.sin_addr;
				send(s,  (const char*)&ip,  sizeof(struct in_addr), 0 );
				break;
			}
	}

}
void getConnectionCode(char* usernameAsked, struct threadData* source){

	uint8_t code=7;
	for(int i =0; i<10; i++)
		{
			if(clients[i]!= NULL && strcmp(usernameAsked, clients[i]->username) == 0)
				{
					//Found it
					send(clients[i]->clientSocket, (const char*)&code ,sizeof(code), 0);
					send(clients[i]->clientSocket, (const char*)source->username ,sizeof(char)*100, 0);
					send(clients[i]->clientSocket, (const char*)&source->clientData.sin_addr,sizeof(struct in_addr), 0);

					
				}
		}


}
DWORD WINAPI NewClientThreadFunc(void* data) {

	struct threadData* currentClient = (struct threadData*) data;
    char ipv4[100];
	
	strcpy(ipv4,   inet_ntoa(currentClient->clientData.sin_addr) );
printf("User connected with ipv4: %s\n", ipv4);
uint8_t code;
char username[101];



while(recv(currentClient->clientSocket,  (char*)&code, sizeof(code)  ,0) == sizeof(code) )
	{
		printf("%d\n", code);

		switch(code)
		{
		case 1:
			{   
				//Always first thing sent when connecting, then never again
				recv(currentClient->clientSocket, (char* )username, sizeof(char)*100, 0);
				strcpy(currentClient->username , username);
				sendEveryoneRefreshNotice(username);

			}
			break;
		case 2:
			{
				uint8_t nr = numberClients()-1;
				send(currentClient->clientSocket, (const char*)&nr, sizeof(nr), 0);				
				for(int i = 0; i < 10; i++)
					{

						if(clients[i]!=NULL && strcmp(clients[i]->username, username)!=0)
							{
									send(currentClient->clientSocket, (const char*)clients[i]->username, sizeof(char)*100, 0);	
							}

					}
				
			}
			break;

		case 3:
		{
			//Only once per client
			uint8_t code = 3;
			send(currentClient->clientSocket,(const char*) &code, sizeof(code), 0 );
			markFree(currentClient->index);
			sendEveryoneRefreshNotice("");


			return 0;
		}
		break;

		case 4:
		{
			char usernameAsked[101];
			recv(currentClient->clientSocket, (char* )usernameAsked, sizeof(char)*100, 0);		
			sendUsernameIpToSocket(currentClient->clientSocket, usernameAsked);
		    getConnectionCode(usernameAsked, currentClient);

		

		}
		break;
		case 5:
		{		uint8_t code2 = 5;
				char receiveUsername[101];
				recv(currentClient->clientSocket,  (char*)receiveUsername, sizeof(char)*100, 0);
				for(int i =0; i<10; i++)
					{
						if(clients[i]!= NULL && strcmp(receiveUsername, clients[i]->username) == 0)
							{
							//Found it
							send(clients[i]->clientSocket, (const char*)&code2 ,sizeof(code2), 0);					
							}
					}

		}
		break;
		case 6:
		{
				uint8_t code2 = 6;
				char receiveUsername[101];
				recv(currentClient->clientSocket,  (char*)receiveUsername, sizeof(char)*100, 0);
				for(int i =0; i<10; i++)
					{
						if(clients[i]!= NULL && strcmp(receiveUsername, clients[i]->username) == 0)
							{
							//Found it
							send(clients[i]->clientSocket, (const char*)&code2 ,sizeof(code2), 0);					
							}
					}
		}
		break;

		default:
			printf("SHOULD NEVER HAPPEN!:%d\n", code);
		}

		

	}






	markFree(currentClient->index);
	  return 0;
}

int main(int argc, char ** argv)
{	
	initializeClients();

    WSADATA wsa;
	
	printf("\nInitialising Winsock...");
	if (WSAStartup(MAKEWORD(2,2),&wsa) != 0)
	{
		printf("Failed. Error Code : %d",WSAGetLastError());
		return 1;
	}

    printf("\nCreating socket...");
    SOCKET s;
    	if((s = socket(AF_INET , SOCK_STREAM , 0 )) == INVALID_SOCKET)
	{
		printf("Could not create socket : %d" , WSAGetLastError());
	}

    struct sockaddr_in server,client;
        struct hostent* net; 
         net = gethostbyname("axxpro360.tplinkdns.com");     
        memcpy(&addrServer, net->h_addr, sizeof(struct in_addr));

        server.sin_addr.s_addr = INADDR_ANY;
        server.sin_family = AF_INET;
	    server.sin_port = htons( 1026 );

    if( bind(s ,(struct sockaddr *)&server , sizeof(server)) == SOCKET_ERROR)
	{
		printf("Bind failed with error code : %d" , WSAGetLastError());
	}
	
	printf("\nBind done");

    listen(s , 5);

	puts("\nWaiting for incoming connections...");
	int c;
	c = sizeof(struct sockaddr_in);
	SOCKET new_socket;
	int currIndex;
	while( (new_socket = accept(s , (struct sockaddr *)&client, &c)) != INVALID_SOCKET )
	{
		
		currIndex = firstFree();
		if(currIndex!= -1)
		{	
			struct threadData* newClient = (struct threadData*)malloc(sizeof(struct threadData));
			newClient->index = currIndex;
			newClient->clientData = client;
			newClient->clientSocket = new_socket;
			newClient->mutex = CreateMutex(NULL, FALSE, NULL);
			strcpy(newClient->username ,"");
			addClient(newClient);

			HANDLE thread = CreateThread(NULL, 0, NewClientThreadFunc, newClient, 0, NULL);
		}
		else {
			closesocket(new_socket);
		}
	}
	
	if (new_socket == INVALID_SOCKET)
	{
		printf("accept failed with error code : %d" , WSAGetLastError());
		return 1;
	}
	closesocket(s);
	WSACleanup();

}