#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ws2tcpip.h>
#include <winsock2.h>
#include <stdint.h>
#pragma comment(lib, "ws2_32.lib")

long long totalRead = 0;
long secondsPassed = 0;
DWORD WINAPI speedThread(void* data){

    while(1){
     Sleep(1000);
    secondsPassed+=1;
    printf("SPEED no-write: %dMB/s\n",totalRead/ (1024*1024 * secondsPassed));
    }


}
int main(int argc, char**argv)
{
    char username[101];
    char ipString[100];
    if(argc != 3)
        exit(1);
    FILE* PIPE = fopen(".\\PIPE.axx", "w");
    if(PIPE == NULL)
        exit(1);
    strcpy(ipString, argv[1]);
    strcpy(username, argv[2]);

    printf("Do you want to accept a connection from %s? ip:%s\nyes - no\n", username, ipString);
    char answer[5];
    fgets(answer, 5, stdin);
    answer[strlen(answer)-1]= '\0';
    uint8_t code;
    if(strcmp(answer, "no") == 0)
        {
            code = 6;
            fwrite(&code, sizeof(code), 1, PIPE);
            fclose(PIPE);
            exit(0);

        }

    //Yes otherwise
    code = 5;
    fwrite(&code, sizeof(code), 1, PIPE);
    fclose(PIPE);



    WSADATA wsaData;
    int error;
        if ((error = WSAStartup(MAKEWORD(2, 2), &wsaData)) !=0)
        {      
            printf("Error %d in WSAStartup, result will fail\n",error);
        }   

      //  printf("IP Address: %s\n", inet_ntoa(addrServer));
    
    SOCKET s;
    	if((s = socket(AF_INET , SOCK_STREAM , 0 )) == INVALID_SOCKET)
	{
		printf("Could not create socket : %d" , WSAGetLastError());
	}

	struct sockaddr_in server;
    server.sin_addr.s_addr = inet_addr(ipString);
    server.sin_port = htons(1030);
    server.sin_family = AF_INET;

    	while (connect(s , (struct sockaddr *)&server , sizeof(server)) < 0)
	{
		printf("Waiting for connection...\n");
        Sleep(1000);
	}


       int bufferSize = 1024*1024*1024/4;;
    setsockopt(s, SOL_SOCKET, SO_SNDBUF, (const char*)&bufferSize, sizeof(bufferSize));
    printf("Connected! \n");

    char * buffer = (char*)malloc(sizeof(char)*1024*1024*1024/4);
	int readAmmount;
    char fileName[100];
    recv(s,  fileName, sizeof(char)*100, 0);
    //Make it work
    FILE* file = fopen(fileName, "wb");
  HANDLE st = CreateThread(NULL, 0, speedThread, NULL, 0, NULL);
    HANDLE hMainThread = GetCurrentThread();
    // Set the priority of the main thread to the highest priority
    BOOL success = SetThreadPriority(hMainThread, THREAD_PRIORITY_TIME_CRITICAL);

	while(  (readAmmount = recv(s,  buffer, sizeof(char)*1024*1024*1024 / 4, 0))  > 0)
	{
        printf("Read: %d  bytes\n", readAmmount);
		fwrite(buffer, sizeof(char),readAmmount, file);
        totalRead += readAmmount;
		
	}
    fwrite(buffer, sizeof(char),readAmmount, file);

    free(buffer);



    fclose(file);
closesocket(s);
WSACleanup();
	
        exit(0);



}