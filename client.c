
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ws2tcpip.h>
#include <winsock2.h>
#include <stdint.h>
#pragma comment(lib, "ws2_32.lib")

HANDLE hMutex;
struct in_addr addrServer;
SOCKET globalServerSocket;
HANDLE threadGlobal;
#define MAX_USERNAME 100


char username[MAX_USERNAME]={};


    uint8_t nrEntries;
     char** people;
char** getConnectedPeople(SOCKET s){
  uint8_t code;
     code = 2;
     // WaitForSingleObject(hMutex, INFINITE);
     send(s, (const char*)&code, sizeof(code), 0);
     recv(s,  (char*)&nrEntries, sizeof(nrEntries)  ,0);
    
     for(uint8_t i =0; i < nrEntries; i++)
     {
       
        recv(s,  people[i], sizeof(char)*100  ,0);

     }

      //ReleaseMutex(hMutex);
    

}
BOOL CtrlHandler(DWORD fdwCtrlType) {
    switch (fdwCtrlType) {
        case CTRL_CLOSE_EVENT:
        {
          uint8_t  code = 3;
     send( globalServerSocket, (const char*)&code, sizeof(code), 0);

    WaitForSingleObject(threadGlobal,INFINITE);

    CloseHandle(&threadGlobal);
    CloseHandle(hMutex);
WSACleanup();

     closesocket( globalServerSocket);
     WSACleanup();
     exit(0);
        }
        
        default:
            return FALSE; 
    }
}
void initialize()
{
       SetConsoleCtrlHandler((PHANDLER_ROUTINE)CtrlHandler, TRUE);
        hMutex = CreateMutex(NULL, FALSE, NULL);
        people = (char**)malloc(sizeof(char*) * 10);
        for(int i = 0; i < 10; i++)
        people[i] = (char*)malloc(sizeof(char)* 100);

     
}

void showInformation(SOCKET s){

    system("cls");
    printf("Hello %s\n", username);
    printf("These are the available people to send files to:\n");
    getConnectedPeople(s);
    for(uint8_t i =0; i < nrEntries; i++)
    {
        printf("%d). %s\n", i+1, people[i]);
    }

    printf("Pick a number to send them a file.\n");

    printf("-1 - Quit\n");
   
}
DWORD WINAPI ReadFromServerThreadFunc (void* data){

    SOCKET s =  *((SOCKET*)data);
    uint8_t code;
    int running = 1;
    while(running){
         
        recv(s, (char* )&code, sizeof(code), 0);
       // WaitForSingleObject(hMutex, INFINITE);
        switch (code)
        {
        case 1:
        {   // ReleaseMutex(hMutex);
             showInformation(s);
        }
            break;

        case 3:
        {
        //    ReleaseMutex(hMutex);
            return 0;
        }
         break;
        case 4:
        {
                //char usernameRequested[101];
                struct in_addr ip;
               // recv(s, (char*)usernameRequested, sizeof(char)*100, 0);
                recv(s, (char*)&ip, sizeof(struct in_addr),0);
                //printf("Prietenul tau are ip-ul %s\n", inet_ntoa(ip));
              //Awaiting answer

                printf("Waiting to get an answer..\n");
                  WaitForSingleObject(hMutex, INFINITE);
                recv(s,(char* )&code, sizeof(code), 0);
                    ReleaseMutex(hMutex);
                if(code == 5)
                    {
                    

                        //Launch another process sweetheart
                        char commandCmd[150];
                strcpy(commandCmd, "");
                strcat(commandCmd,"start cmd /c clientServer.exe ");
                strcat(commandCmd, inet_ntoa(ip));
                strcat(commandCmd, " ");
                strcat(commandCmd, username);
                system((const char*) commandCmd);

                    }
                else {
                    printf("No accept! :(\n");
                }

              //  ReleaseMutex(hMutex);
        }
        break;
       case 7:{



        char receivedUsername[101];
        struct in_addr receivedIp;

        recv(s, (char*)receivedUsername, sizeof(char)*100, 0);
        recv(s, (char*)&receivedIp, sizeof(struct in_addr) , 0);


       uint8_t code2=6;
       FILE* fileCreation = fopen(".\\PIPE.axx", "w");
       if(fileCreation == NULL)
            {

        send(s, (const char*) &code2, sizeof(code2), 0);
        send(s, (const char*)receivedUsername, sizeof(char)*100, 0);
            }
    else 
    {
        fclose(fileCreation);
        char commandCmd[150];
        strcpy(commandCmd, "");
        strcat(commandCmd,"start cmd /c clientClient.exe ");
        strcat(commandCmd, inet_ntoa(receivedIp));
        strcat(commandCmd, " ");
        strcat(commandCmd, receivedUsername);
        system((const char*) commandCmd);

       FILE* PIPE = fopen(".\\PIPE.axx", "r");

       while(fread(&code2, sizeof(code2), 1, PIPE) == 0 )
       {
        //Wait for the user to accept or deny
        Sleep(50);
       }
       

        //From here force
        send(s, (const char*) &code2, sizeof(code2), 0);
        send(s, (const char*)receivedUsername, sizeof(char)*100, 0);

       // ReleaseMutex(hMutex);
    }
       
       }
       break;
        
        default:
            break;
        }
     
      
    }

 

}

int main(int argc, char ** argv)
{


    printf("Pick a username: ");
    fgets(username, MAX_USERNAME, stdin);
    username[strlen(username)-1]= '\0';

    initialize();



    struct hostent* net; 
    WSADATA wsaData;
    int error;
        if ((error = WSAStartup(MAKEWORD(2, 2), &wsaData)) !=0)
        {      
            printf("Error %d in WSAStartup, result will fail\n",error);
        }   
     net = gethostbyname("axxpro360.tplinkdns.com");     
        memcpy(&addrServer, net->h_addr, sizeof(struct in_addr));
      //  printf("IP Address: %s\n", inet_ntoa(addrServer));
    
    SOCKET s;
    	if((s = socket(AF_INET , SOCK_STREAM , 0 )) == INVALID_SOCKET)
	{
		printf("Could not create socket : %d" , WSAGetLastError());
	}
    globalServerSocket = s;
	struct sockaddr_in server;
    server.sin_addr = addrServer;
    server.sin_port = htons(1026);
    server.sin_family = AF_INET;

    	if (connect(s , (struct sockaddr *)&server , sizeof(server)) < 0)
	{
		puts("connect error");
		return 1;
	}
	
	

  uint8_t code;
     code = 1;
     send(s, (const char*)&code, sizeof(code), 0);
    send(s, username, sizeof(char)*100, 0 );

    

    int running = 1;
    showInformation(s);
    HANDLE thread = CreateThread(NULL, 0, ReadFromServerThreadFunc, &s, 0, NULL);
    threadGlobal = thread;
    while(running){
    int choice;
 
    scanf("%d", &choice);
    WaitForSingleObject(hMutex, INFINITE);
   

    //WaitForSingleObject(hMutex, INFINITE);
             
    if((choice > 0 && choice <= nrEntries) || choice == -1)
    {switch (choice)
        {
     case -1:
        { code = 3;
        send(s, (const char*)&code, sizeof(code), 0);
        running = 0;

        }
    break;
    
    default:
    {
        //One of the connected clients
        uint8_t code = 4;
        send(s, (const char*)&code, sizeof(code), 0);
        send(s, (const char*) people[choice-1], sizeof(char)* 100, 0);


    }
        break;
    }

    }
     ReleaseMutex(hMutex);
  
    }
    WaitForSingleObject(thread,INFINITE);
     closesocket(s);
    CloseHandle(&thread);
    CloseHandle(hMutex);
WSACleanup();

exit(0);



}