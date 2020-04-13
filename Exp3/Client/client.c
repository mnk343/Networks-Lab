/*
									The Client Side 
	This program creates sockets, connects to the server, sends required requests
	to the cash register and displays them appropriately

*/
#include "header_files.h"

// Max buffer side of the client
#define MAX_LINE 1024

// The driver function that will be called
void driver(int clientSocket) 
{ 
    char buffer[MAX_LINE]; 
    int size; 
    while(1) 
    { 
    	// initilizes the buffer with all 0's
        bzero(buffer, MAX_LINE); 
        printf("Enter the request (RequestType UPC Number ): \n"); 
        size = 0; 
        // this varible will be later used to exit the client in case lets say the server exits with ctrl-c 
        int fin=0;

        // take request as input
        while ((buffer[size++] = getchar()) != '\n');
		
		// send the request to the server side
		write(clientSocket, buffer, sizeof(buffer)); 
        
        // in case the client requests to close the connection and see its total cost
        if( buffer[0]=='1')
        	fin=1;

        bzero(buffer, MAX_LINE); 

        // read the data from the server
        read(clientSocket, buffer, sizeof(buffer)); 
		if( buffer[0] == '2' )
			fin = 2;

        printf("Response From Server : %s\n", buffer); 
        
        // if fin is 1 this means the client had reqeusted to close the connection
        if ( fin == 1) { 
            printf("Client Exit\n"); 
            break; 
        } 

        // in case the server exits with ctrl-c it will send a message to the client to close the connection
		if( fin == 2)
		{
			printf("Server Down. Client Exiting\n"); 
            break;
		}
    } 
} 

int main( int argc , char ** argv) 
{
	int clientSocket;
	struct sockaddr_in serverAddress;

	// there should be 3 arguments ie. a.out, server ip address and server port
	if( argc != 3)
	{
		printf("Please enter correct number of arguments!!");
		exit(0);
	}

	// creates a socket of address family INET with tcp used as the transport layer
	// communication since 0 indicates that default transport layer protocol will be
	// used for the address family and sock type pair and in this case ie.
	// for af_net and sock_stream pair default is tcp
	clientSocket = socket(AF_INET , SOCK_STREAM , 0 );
	
	// if client socket returned is negative, this indicates error
	if( clientSocket < 0 )
	{
		printf("Error while creating the client socket\n");
		exit(0);
	}
	printf("Socket created successfully\n");
	
	// initializes the serverAddress with all 0's
	bzero (&serverAddress , sizeof(serverAddress) );

	// set address family of server to INET which is IPv4
	serverAddress.sin_family = AF_INET;

	// htons converts the numerical value into the network order ie. 
	// in a form understood by the network
	serverAddress.sin_port = htons( (int)atoi( argv[2] ) );
	
	// inet_pton converts the ip address from the 'presentation' form
	// to the 'networks' one
	// in case of error 0 or negative value is returned
	if( inet_pton( AF_INET , argv[1] , &serverAddress.sin_addr ) <= 0 )
	{
		printf("INET PTON error!! \n");
		exit(0);
	}

	// tcp connection is made from the client socket to the server socket
	// 3-way handshake is performed by tcp
	// and if there is an error a negative value if returned
	if (connect( clientSocket , (struct sockaddr *) &serverAddress , sizeof(serverAddress)) < 0)
	{
		printf("Connect Error\n");
		exit(0);
	}
	
	printf("TCP Connection established\n\n");

	// after successfull connection, we now move on to the driver function to execute the yet to come requests
	driver(clientSocket);	
	exit(0);

	return 0;
}