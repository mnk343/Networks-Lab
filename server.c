#include "header_files.h"
#define MAX_LINE 1024
     
void driver( int connectionSocket )
{
	char buffer[MAX_LINE];
	int n;
	while(1){
		bzero(buffer , MAX_LINE);
		read ( connectionSocket , buffer , sizeof(buffer) ) ;
		printf("%s\n", buffer);
		bzero(buffer , MAX_LINE);
		n=0;
		while( (buffer[n++] = getchar() ) != '\n' );
			write( connectionSocket , buffer , sizeof(buffer) );
		if( strncmp( "exit" , buffer , 4 ) ==0 )
		{
			printf("Server Exit\n");
			break;
		}
	}
}

int main( int argc , char ** argv)
{
	int listenSocket , connectionSocket;
	pid_t childPid;

	socklen_t clientLength;
	struct sockaddr_in clientAddress , serverAddress;
	listenSocket = socket( AF_INET , SOCK_STREAM , 0 );
	bzero( &serverAddress , sizeof(serverAddress) );
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = htonl( INADDR_ANY );
	serverAddress.sin_port = htons( (int) atoi( argv[1] ) ) ;
	bind( listenSocket , (struct sockaddr *) &serverAddress , sizeof(serverAddress) );
	if(listen( listenSocket , 5 ) < 0  )
	{
		printf("Error in listening\n");
		exit(0);
	}

	while(1)
	{
		clientLength = sizeof(clientAddress);
		connectionSocket = accept( listenSocket , (struct sockaddr *) &clientAddress , &clientLength );
		if( connectionSocket < 0) 
		{
			printf("Error in creating connection socket\n");
			exit(0);
		}

		childPid = fork() ;
		if( childPid  == 0 )
		{
			if( close( listenSocket) < 0 )
			{
					printf("Error in closing listening socket\n");
					exit(0);
			}
			driver(connectionSocket);
			exit(0);
		}
		if(close( connectionSocket) < 0)
		{
			printf("Error in closing connection socket\n");
			exit(0);
		}

	}	
}











