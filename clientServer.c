#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ws2tcpip.h>
#include <winsock2.h>
#include <stdint.h>
#include <unistd.h>
#pragma comment(lib, "ws2_32.lib")

#define THREADS_NUMBER 4 
#define MAX_RAM_USAGE 1024*1024*1024
#define MAX_RAM_PER_THREAD MAX_RAM_USAGE / THREADS_NUMBER

char filePath[300];

char* readerBuffers[THREADS_NUMBER];
HANDLE threads[THREADS_NUMBER];
HANDLE readerEvents[THREADS_NUMBER];
long dataRead[THREADS_NUMBER];
HANDLE writerEvents[THREADS_NUMBER];

DWORD WINAPI ReaderThread(void* data){
   
    int ThreadNumber = *( (int*) data );
    free(data);
    FILE* file = fopen(filePath, "rb");
    int fd = _fileno(file);


    if(file == NULL)
        exit(0);

    // lseek64()
    
    long long  seekOffset = (long long)ThreadNumber* MAX_RAM_PER_THREAD;
    lseek64(fd, seekOffset, SEEK_CUR );
    // while(seekOffset > LONG_MAX)
    //     {
    //          fseek(file, LONG_MAX, SEEK_CUR);
    //          seekOffset-=LONG_MAX;
    //     }
    // printf("offset: %llu\n", ftell(file));


    int x = 0;

    while((x = fread(readerBuffers[ThreadNumber],sizeof(char), MAX_RAM_PER_THREAD, file))  == sizeof(char)*MAX_RAM_PER_THREAD )
    {
        printf("Thread %d I read %d , offset: %llu\n", ThreadNumber, x, ftell(file));
        dataRead[ThreadNumber] = x;
                 SetEvent(writerEvents[ThreadNumber]);
                    //  for(int j = 0; j < 3; j++)
                        lseek64(fd,MAX_RAM_PER_THREAD*3, SEEK_CUR);


                 WaitForSingleObject(readerEvents[ThreadNumber], INFINITE);
                 ResetEvent(readerEvents[ThreadNumber]);
    }
            printf("Thread %d I read %d , offset: %llu\n", ThreadNumber, x, ftell(file));
     dataRead[ThreadNumber] = x;
    SetEvent(writerEvents[ThreadNumber]);

    
    printf("Done:%d  i:%d\n", x,ThreadNumber);

    return 0;


}

void initialize(){
    for(int i =0 ; i < THREADS_NUMBER; i ++)
    {
          readerBuffers[i] = (char*)malloc(sizeof(char) *MAX_RAM_PER_THREAD);
          readerEvents[i]  = CreateEvent(
        NULL,   // Default security attributes
        FALSE,  // Manual reset event
        FALSE,   //UNSignaled
        NULL    // Unnamed event
    );

          writerEvents[i]  = CreateEvent(
        NULL,   // Default security attributes
        FALSE,  // Manual reset event
        FALSE,   //UNSignaled
        NULL    // Unnamed event
    );

    }
      
    
       

}

int main(int argc, char**argv)
{	initialize();
    char username[101];
    char ipString[100];
    if(argc != 3)
        exit(1);

    strcpy(ipString, argv[1]);
    strcpy(username, argv[2]);

    WSADATA wsa;
	
	//printf("\nInitialising Winsock...");
	if (WSAStartup(MAKEWORD(2,2),&wsa) != 0)
	{
		printf("Failed. Error Code : %d",WSAGetLastError());
		return 1;
	}

   // printf("\nCreating socket...");
    SOCKET s;
    	if((s = socket(AF_INET , SOCK_STREAM , 0 )) == INVALID_SOCKET)
	{
		printf("Could not create socket : %d" , WSAGetLastError());
	}

    struct sockaddr_in server,client;
 

        server.sin_addr.s_addr = INADDR_ANY;
        server.sin_family = AF_INET;
	    server.sin_port = htons( 1030 );

    if( bind(s ,(struct sockaddr *)&server , sizeof(server)) == SOCKET_ERROR)
	{
		printf("Bind failed with error code : %d" , WSAGetLastError());
	}
	
	//printf("\nBind done");

    listen(s , 1);

	puts("\nWaiting for incoming connections...");
	int c;
	c = sizeof(struct sockaddr_in);
	SOCKET new_socket;

    new_socket = accept(s , (struct sockaddr *)&client, &c);

    int bufferSize = 1024*1024*1024/4;
    setsockopt(new_socket, SOL_SOCKET, SO_SNDBUF, (const char*)&bufferSize, sizeof(bufferSize));
    printf("ACCEPTED!!\n");
	FILE *file;
	char path[300];
	do {
	printf("Path to file you want to send:\n");

	fgets(path, 300, stdin);
	path[strlen(path)-1] = '\0';
	file  = fopen((const char*) path, "rb");
	}
	while(file == NULL);
	printf("File found, sending..");

	fclose(file);

	char fileName[100];
	char* fileNameP;
	for(int i = 0; i <strlen(path); i++)
	{
		if(path[i] == '\\')
		{
			fileNameP = path + i + 1;
		}
	}




	strcpy(fileName, fileNameP);
	send(new_socket, (const char*) fileName, sizeof(char)*100, 0);


     strcpy(filePath, path);
        for(int i =0 ; i < THREADS_NUMBER; i++)
        {
            int *p = (int*)malloc(sizeof(int));
            *p = i;
			 threads[i] = CreateThread(NULL, 0, ReaderThread, p, 0, NULL);
        }

           // FILE* fileW =  fopen("ubuntu.iso", "wb");
    HANDLE hMainThread = GetCurrentThread();

    // Set the priority of the main thread to the highest priority
    BOOL success = SetThreadPriority(hMainThread, THREAD_PRIORITY_TIME_CRITICAL);

            int running = 1;
        int x;
            while(running)
            {
                 for(int i =0 ; i < THREADS_NUMBER; i++)
                {
                    WaitForSingleObject(writerEvents[i], INFINITE);
                    ResetEvent(writerEvents[i]);

                        printf("SPEED-A\n");
						x = send(new_socket, (const char*)readerBuffers[i], sizeof(char)* dataRead[i], 0);
                    printf("SPEED-B\n");
                   //      x = fwrite(readerBuffers[i], sizeof(char),dataRead[i], fileW);
                        if(x != MAX_RAM_PER_THREAD)
                            {
                              
                              running = 0;
                                 break;
                            }
                            else {
                                SetEvent(readerEvents[i]);
                            }   
                  
             }
            }
  

     WaitForMultipleObjects(THREADS_NUMBER, threads, TRUE, INFINITE);
  //  fclose(fileW);
    
        for(int i =0 ; i < THREADS_NUMBER; i ++)
            free(readerBuffers[i]);

	// char buffer[1024];
	// int readAmmount;
	// while(  (readAmmount = fread(buffer, sizeof(char), 1024, file))  > 0)
	// {
	// 	//send em
	// 	send(new_socket, (const char*) buffer, sizeof(char)*readAmmount, 0);
	// }

	// //send(new_socket, (const char*) buffer, sizeof(char)*readAmmount, 0);


closesocket(new_socket);
WSACleanup();
	

    exit(0);


}