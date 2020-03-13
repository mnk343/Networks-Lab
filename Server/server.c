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
	signal(SIGINT, ctrlCHandler);
	for(int i=MAX_CLIENTS-1;i>=0;i--){

		if(clients[i] != -1){
			// Message is written in buffer to show on client's side.
			bzero(buffer , MAX_LINE);	
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

// This fuction is used to retrieve details of the item requested by the client.
void details(char* upc , int *price , char** desc){
    *desc = "Not Found";
    *price = -1;
    // File pointer.
    FILE* database;	
	
    // The file is opened using 'r' read option.
    database = fopen("data.txt" , "r");	
    // If the file doesn't exist then show error.	
    if(database == NULL){
        *price = -1;
        *desc = "Error: Cannot access database";
        return ;
    }
    char dupc[100];
    char ddesc[100];
    int dd;
    // Read the file and compare UPC id with each line of file and find the correct entry.
    while(fscanf(database , "%s %d %s\n" , dupc , &dd , ddesc) != EOF){
	// Compare strings.		
        if(strncmp(dupc , upc , 3) == 0){
            *desc = (char *) malloc(sizeof(ddesc));
            strcpy(*desc , ddesc);
            *price = dd;
            break;
        }
    }
    // Close the file.
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
	// Set the signal to invoke ctrlCHandler function on pressing Ctrl-C.
	signal(SIGINT, ctrlCHandler);

	// Initialize the array and set all clients sockets as -1 which means this is not occupied.
	for( int i1=0 ; i1< MAX_CLIENTS ; i1++)
	{
		clients[i1]=-1;
	}
	
	// If there are no valid number of arguments then show error.
	if( argc != 2)
	{
		printf("Please enter correct number of arguments!!");
		exit(0);
	}

	// Sockets are initialized here.
	// listenSocket is the socket on which client sends the connection request.
	// connectionSocket is the socket on which the connection is established and through which
	// the further tranfer of messages will take place.
	int listenSocket , connectionSocket;
	pid_t childPid;
	
	socklen_t clientLength;
	struct sockaddr_in clientAddress , serverAddress;
	// listenSocket initialized. AF_INET is used for Internet.
	listenSocket = socket( AF_INET , SOCK_STREAM , 0 );

	// If the socket is < 0 then it is considered as error.
	if( listenSocket < 0)
	{
		printf("Error in creating listening Socket \n");
		exit(0);
	}
	printf("Listening Socket created successfully\n");
	
	// serverAddress is initialized to 0 using bzero.
	bzero( &serverAddress , sizeof(serverAddress) );

	// Setting of serverAddress.
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = htonl( INADDR_ANY );
	serverAddress.sin_port = htons( (int) atoi( argv[1] ) ) ;
	// bind this listenSocket to the serverAddress.
	if(bind( listenSocket , (struct sockaddr *) &serverAddress , sizeof(serverAddress) )!=0)
	{
		printf("Error in binding server address to the listening socket\n");
		exit(0);
	}
	// listen to this socket so that the number doesn't exceed MAX_CLIENTS.
	if(listen( listenSocket , MAX_CLIENTS ) < 0  )
	{
		printf("Error in listening\n");
		exit(0);
	}
	// loop runs forever.
	while(1)
	{
		// this variable defines the size of the clientAddress. It will be used while allocating socket.
		clientLength = sizeof(clientAddress);

		// connectionSocket is the socket through which all the transmission of messages take place.
		// If the connection request from client is accepted then this will be it's socket number.
		connectionSocket = accept( listenSocket , (struct sockaddr *) &clientAddress , &clientLength );
		// If connectionSocket < 0 then it is treated as an error.
		if( connectionSocket < 0) 
		{
			printf("Error in creating connection socket\n");
			exit(0);
		}

		// fork() return a child process id which is stored as childPid.
		childPid = fork() ;
		// If childPid < 0 then this is an error.
		if( childPid < 0 )
		{
			printf("Error on fork\n");
			exit(0);
		}
		// If childPid == 0 then the server must stop recieving any connection requests for this child Process.
		// Hence the listening Socket must be closed for this child Process.
		if( childPid  == 0 )
		{
			if( close( listenSocket) < 0 )
			{
					printf("Error in closing listening socket\n");
					exit(0);
			}
			// driver function is called for this client which will do all the functions as per client's requirements.
			driver(connectionSocket);
			exit(0);
		}
		// This closes the connectionSocket and if it is < 0 then it shows error.
		if(close( connectionSocket) < 0)
		{
			printf("Error in closing connection socket\n");
			exit(0);
		}

	}	
}
