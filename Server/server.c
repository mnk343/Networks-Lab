#include "header_files.h"
 #include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MAX_LINE 1024         // Maximum size of buffer.   
#define MAX_CLIENTS 100       // Max number of clients allowed for the server.
int clients[MAX_CLIENTS];     // Array to save SOCK_NUM for each client.
int totalCost[MAX_CLIENTS];   // Array to store total cost for each client.

// This function handles the Ctrl-C signal. It will be called when Ctrl-C is pressed on console from server's side.
void ctrlCHandler( int num )
{
	char buffer[MAX_LINE];
	signal(SIGINT, ctrlCHandler);	// This sets the signal to invoke ctrlCHandler function whenever Ctrl-C is pressed.
	for(int i=MAX_CLIENTS-1;i>=0;i--){	// Loop to iterate over all sockets and close each one of them.

		if(clients[i] != -1){
			bzero(buffer , MAX_LINE);	// Message is written in buffer to show on client's side.
			char *t = "2 Server Down";
			int bufferLength = 0;
			for(int i = 0; t[i]; i++){
				buffer[ bufferLength++ ] = t[i];
			}
			buffer [ bufferLength ] = '\0';
			write(clients[i] , buffer , sizeof(buffer));

			if(close(clients[i]) < 0){
				printf("Socket Number %d could not be released\n",clients[i]);
			}
			clients[i] = -1;	
		}
	}
	printf("Server Exited Gracefully\n" );
	exit(0);
}

// This function is used to reverse a string.
void reverse(char str[], int length) 
{ 
    int start = 0; 
    int end = length -1; 
    while (start < end) 
    { 
        char temp  = *(str+start);
        *(str+start) = *(str+end);
        *(str+end) = temp;
        start++; 
        end--; 
    } 
} 
  
// This is the implementation of itoa() to convert an interger to a string. 
// It takes interger, char * and the base to convert integer as input.
void itoa(int num, char* str, int base) 
{ 
    int i = 0; 
    bool isNegative = false; 
  
    /* Handle 0 explicitely, otherwise empty string is printed for 0 */
    if (num == 0) 
    { 
        str[i++] = '0'; 
        str[i] = '\0'; 
    } 
  
    // In standard itoa(), negative numbers are handled only with  
    // base 10. Otherwise numbers are considered unsigned. 
    if (num < 0 && base == 10) 
    { 
        isNegative = true; 
        num = -num; 
    } 
  
    // Process individual digits 
    while (num != 0) 
    { 
        int rem = num % base; 
        str[i++] = (rem > 9)? (rem-10) + 'a' : rem + '0'; 
        num = num/base; 
    } 
  
    // If number is negative, append '-' 
    if (isNegative) 
        str[i++] = '-'; 
  
    str[i] = '\0'; // Append string terminator 
  
    // Reverse the string 
    reverse(str, i); 
  
} 

void details(char* upc , int *price , char** desc){
    *desc = "Not Found";
    *price = -1;
    FILE* database;

    database = fopen("data.txt" , "r");
    if(database == NULL){
        *price = -1;
        *desc = "Error: Cannot access database";
        return ;
    }
    char dupc[100];
    char ddesc[100];
    int dd;
    while(fscanf(database , "%s %d %s\n" , dupc , &dd , ddesc) != EOF){
        if(strncmp(dupc , upc , 3) == 0){
            *desc = (char *) malloc(sizeof(ddesc));
            strcpy(*desc , ddesc);
            *price = dd;
            break;
        }
    }
    fclose(database);
}

void driver( int connectionSocket )
{
	int pos=-1;
	int flag=0;
	for(int i1=0;i1<MAX_CLIENTS ; i1++)
	{
		if( clients[i1] == -1)
		{
			pos = i1;
		}
		if( clients[i1] == connectionSocket  )
		{
			flag=1;
			break;
		}
	}

	if( flag==0)
	{
		if(pos==-1)
		{
			printf("Number of clients exceeded\n");
			return;
		}
		clients[pos] = connectionSocket;
	}

	printf("Child process created succesfully and now waiting for querries from the client\n");
	char buffer[MAX_LINE];
	int n;

	int index;
	for(int i1=0;i1<MAX_CLIENTS;i1++)
	{
		if( clients[i1] == connectionSocket )
		{
			index = i1;
			break;
		}
	}

	while(1){

		bzero(buffer , MAX_LINE);
		read ( connectionSocket , buffer , sizeof(buffer) ) ;
		printf("Request received at server side: %s\n", buffer);
		
		char requestType='9';
		char upc[MAX_LINE],number[MAX_LINE];
		int i=2,k1=0,k2=0;

		requestType = buffer[0];
		while(buffer[i]!='\0' && buffer[i]!=' ')
		{
			upc[k1++] = buffer[i++];
			// printf("%d %d\n",k1,i );
		}
		upc[k1]='\0';
		i++;

		while(buffer[i]!='\0' && buffer[i]!=' ' && buffer[i] != '\n')
		{
			number[k2++] = buffer[i++];
		}
		number[k2]='\0';
		
		int num = (int) atoi(number);
		bzero(buffer , MAX_LINE);
		if( requestType == '0' )
		{
			bzero(buffer , MAX_LINE);

			// printf("Cool\n");
			if( upc == NULL || strlen(upc) != 3 )
			{
				int bufferLength=0;
					char *t = "1 Protocol Error";
					for(int i1=0;t[i1]!='\0';i1++)
						buffer[bufferLength++]=t[i1];
					buffer[ bufferLength ]='\0';
				write( connectionSocket , buffer , sizeof(buffer) );
					
			}
			else
			{
				int cost;
				char *desc;
				// printf("Cool2\n");
				details(upc , &cost , &desc);
				    // details("001" , &price , &desc);

				if( cost == -1)
				{
					int bufferLength=0;
					char *t = "1 UPC is not found in database";
					for(int i1=0;t[i1]!='\0';i1++)
						buffer[bufferLength++]=t[i1];
					buffer[ bufferLength ]='\0';
					write( connectionSocket , buffer , sizeof(buffer) );
				}

				else
				{
					totalCost[index] += (int)( cost * num );
					buffer[0] = '0';
					buffer[1] = ' ';
					int bufferLength=2;

					char temp[MAX_LINE];
					itoa( cost , temp,10 );
					int ctr=0;
					printf("Temp = %s\n" , temp);
					while( temp!=NULL && temp[ctr]!='\0' )
					{
						buffer [ bufferLength++ ] = temp[ctr++];
					}
					buffer[bufferLength++] = ' ';
					ctr=0;
					while( desc != NULL && desc[ctr]!='\0' )
					{
						buffer [ bufferLength++ ] = desc[ctr++];
					}
					buffer[bufferLength]='\0';
					printf("%s %s %s %d %d\n",upc , number , desc, cost, totalCost[index]);
					write( connectionSocket , buffer , sizeof(buffer) );			
				}
			}
		}

		else if( requestType == '1' )
		{
			bzero(buffer , MAX_LINE);

			buffer[0]='0';
			buffer[1]=' ' ;
			char temp[MAX_LINE];
			int bufferLength=2;

			itoa(totalCost[index] , temp, 10 );
			for(int i1=0;temp[i1]!='\0';i1++)
			{
				buffer[bufferLength++]=temp[i1];
			}
			buffer[bufferLength]='\0';
			clients[index] = -1;

			write( connectionSocket , buffer , sizeof(buffer) );
			close(connectionSocket);
			break;
		}
		
		else
		{	
			int bufferLength=0;
				char *t = "1 Protocol Error";
				for(int i1=0;t[i1]!='\0';i1++)
					buffer[bufferLength++]=t[i1];
				buffer[ bufferLength ]='\0';
			write( connectionSocket , buffer , sizeof(buffer) );
				
			printf("Kindly specify the correct value of requestType\n");
		}

	}
}

int main( int argc , char ** argv)
{
	signal(SIGINT, ctrlCHandler);
	for( int i1=0 ; i1< MAX_CLIENTS ; i1++)
	{
		clients[i1]=-1;
	}

	if( argc != 2)
	{
		printf("Please enter correct number of arguments!!");
		exit(0);
	}
	int listenSocket , connectionSocket;
	pid_t childPid;

	socklen_t clientLength;
	struct sockaddr_in clientAddress , serverAddress;
	listenSocket = socket( AF_INET , SOCK_STREAM , 0 );
	if( listenSocket < 0)
	{
		printf("Error in creating listening Socket \n");
		exit(0);
	}
	printf("Listening Socket created successfully\n");

	bzero( &serverAddress , sizeof(serverAddress) );
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = htonl( INADDR_ANY );
	serverAddress.sin_port = htons( (int) atoi( argv[1] ) ) ;
	if(bind( listenSocket , (struct sockaddr *) &serverAddress , sizeof(serverAddress) )!=0)
	{
		printf("Error in binding server address to the listening socket\n");
		exit(0);
	}
	if(listen( listenSocket , MAX_CLIENTS ) < 0  )
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
		if( childPid < 0 )
		{
			printf("Error on fork\n");
			exit(0);
		}
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
