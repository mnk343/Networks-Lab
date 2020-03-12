#include "header_files.h"

#define MAX_LINE 1024

void driver(int clientSocket) 
{ 
    char buffer[MAX_LINE]; 
    int size; 
    while(1) 
    { 
        bzero(buffer, MAX_LINE); 
        printf("Enter the request (RequestType UFC Number ): "); 
        size = 0; 
        while ((buffer[size++] = getchar()) != '\n');
		write(clientSocket, buffer, sizeof(buffer)); 
        
        bzero(buffer, MAX_LINE); 
        read(clientSocket, buffer, sizeof(buffer)); 
        printf("From Server : %s\n", buffer); 
        if ((strncmp(buffer, "exit", 4)) == 0) { 
            printf("Client Exit...\n"); 
            break; 
        } 
    } 
} 

int main( int argc , char ** argv) 
{
	int clientSocket;
	struct sockaddr_in serverAddress;

	if( argc != 3)
	{
		printf("Please enter correct number of arguments!!");
		exit(0);
	}

	clientSocket = socket(AF_INET , SOCK_STREAM , 0 );
	if( clientSocket < 0 )
	{
		printf("Error while creating the client socket\n");
		exit(0);
	}
	bzero (&serverAddress , sizeof(serverAddress) );

	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons( (int)atoi( argv[2] ) );
	if( inet_pton( AF_INET , argv[1] , &serverAddress.sin_addr ) <= 0 )
	{
		printf("INET PTON error!! \n");
		exit(0);
	}

	if (connect( clientSocket , (struct sockaddr *) &serverAddress , sizeof(serverAddress)) < 0)
	{
		printf("Connect Error\n");
		exit(0);
	}
	
	driver(clientSocket);	
	exit(0);

	return 0;
}