/* client.cpp

Its a client side code which request information for a server via TCP socket.

Copyright (c) 2019, irbaran. 
All rights reserved.

Permission to use, copy, modify, and distribute this software for any purpose
with or without fee is hereby granted, provided that the above copyright
notice and this permission notice appear in all copies.

The software is provided "as is", without warranty of any kind, express or
implied, including but not limited to the warranties of merchantability,
fitness for a particular purpose and noninfringement of third party rights. 
In no event shall the authors or copyright holders be liable for any claim,
damages or other liability, whether in an action of contract, tort or
otherwise, arising from, out of or in connection with the software or the use
or other dealings in the software.

Except as contained in this notice, the name of a copyright holder shall not
be used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization of the copyright holder.
 */
#include <stdio.h> 
#include <string.h> 
#include <stdlib.h> 
#include <iostream>
#include <unistd.h> 
#include <arpa/inet.h> 
#include <sys/types.h> 
#include <sys/socket.h>
#include <sys/stat.h>  
#include <netinet/in.h> 
#include <fcntl.h>


using namespace std;

//define for socket
#define TRUE 1 
#define FALSE 0 
#define SOCKET_PORT 7000 

//Main status errors
#define RUNSTS_ERROR        0    // Run main status error
#define RUNSTS_OK           1    // Run main status ok

//functions
int send_server_request(int socket, char request_msg[32]);
int init_socket(void);

//////////////////////////
// Function init_socket()
// It initializes and configures server connection and return the socket
/////////////////////////
int init_socket(void)
{
	int error_sts = RUNSTS_OK;           // internal function status error
	struct sockaddr_in address;          // Socket address input structure
	int sock = 0;                        // Main client socket
	int valread_size;                    // Value read size	 
	char buffer[1024] = {0};             // Buffer read from server
	int ret_tmp;                         // Temporary return
	
	//Create TCP socket connection
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == 0) 
	{
		printf("Error: Client socket creation error \n"); 
		error_sts = RUNSTS_ERROR;
		return error_sts; 
	}

	//Clear struct
	memset(&address, '0', sizeof(address)); 

	//type of socket created
	address.sin_family = AF_INET; 
	address.sin_port = htons( SOCKET_PORT ); 

	// Convert IPv4 and IPv6 addresses from text to binary form 
	ret_tmp = inet_pton(AF_INET, "127.0.0.1", &address.sin_addr);
	if(ret_tmp <= 0)  
	{
		printf("Error: Invalid address / Address not supported \n"); 
		error_sts = RUNSTS_ERROR;
		return error_sts;  
	}

	//Connect to server
	ret_tmp = connect(sock, (struct sockaddr *)&address, sizeof(address));
	if (ret_tmp < 0) 
	{
		printf("Error: Connection Failed \n"); 
		error_sts = RUNSTS_ERROR;
		return error_sts;
	}
	
	//Initial server message
	valread_size = read( sock , buffer, 1024);
	if (valread_size <= 0) 
	{
		perror("Error: during read from server socket");
		error_sts = RUNSTS_ERROR;
		return error_sts;
	}
	else
		printf("%s\n",buffer );
	
	return sock;
}	

//////////////////////////
// Function send_server_request()
// It sends a request command message to server
/////////////////////////
int send_server_request(int socket, char request_msg[32])
{
	char client_msg[1024] = {0};         // Client request message
	char server_read_buffer[1024] = {0}; // Server read buffer
	int valread_size;                    // Value read size
	int ret_tmp;                         // Temporary return
	int error_sts = RUNSTS_OK;           // function status error
	
	//Send a request to server
	memset(client_msg, 0, sizeof(client_msg));
	strcpy(client_msg, request_msg);
	ret_tmp = send(socket , client_msg , strlen(client_msg) , 0 );
	if( ret_tmp != strlen(client_msg) )
	{ 
		perror("Error: during send server request");
		error_sts = RUNSTS_ERROR;
	}
	request_msg[strlen(request_msg) - 1] = '\0';
	printf("%s message sent\n", request_msg); 
	
	//Clear array
	memset(server_read_buffer, 0, sizeof(server_read_buffer));
	
	//Read data from server socket
	valread_size = read( socket , server_read_buffer, 1024); 
	if (valread_size <= 0) 
	{
		perror("Error: during read from server socket");
		error_sts = RUNSTS_ERROR;
	}

	//Print size and receiver buffer from server
	cout << server_read_buffer;
	cout << "server buffer size received = " << valread_size << endl;
	sleep(1);
	
	return error_sts;
}

//////////////////////////
// client main function
// First cli argument/parameter is the number of iterarions
/////////////////////////
int main(int argc, char const *argv[]) 
{ 
	int main_error_sts = RUNSTS_OK;      // Main function status error
	int main_error_ststmp;               // Main temporary status error
	int sock = 0;                        // Main client socket
	int num_iterations = atoi(argv[1]);  // Get number of iterarion from first cli parameter
	int iterations = 0;                  // Current iterarion	
	char server_request_cmd[32];         // Server request command
	
	sock = init_socket();
	if (sock == 0) 
	{
		printf("Error: Client socket creation error \n"); 
		main_error_sts = RUNSTS_ERROR;
		return main_error_sts; 
	}

	while (iterations++ <= num_iterations)
	{
		//Test all server possible requests

		memset(server_request_cmd, 0, sizeof(server_request_cmd));
		strcpy(server_request_cmd, "teste\n");
		main_error_ststmp = send_server_request(sock, server_request_cmd);
		main_error_sts = main_error_sts && main_error_ststmp;

		memset(server_request_cmd, 0, sizeof(server_request_cmd));
		strcpy(server_request_cmd, "-server_processtop10\n");
		main_error_ststmp = send_server_request(sock, server_request_cmd);
		main_error_sts = main_error_sts && main_error_ststmp;

		memset(server_request_cmd, 0, sizeof(server_request_cmd));
		strcpy(server_request_cmd, "-server_infomem\n");
		main_error_ststmp = send_server_request(sock, server_request_cmd);
		main_error_sts = main_error_sts && main_error_ststmp;

	}
	return main_error_sts; 
}