/*UDP Echo Server*/
#include <stdio.h>          /* These are the usual header files */
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

#define PORT 5550  /* Port that will be opened */ 
#define BUFF_SIZE 1024
/*
* Check valid port number
* @param int port
* @return 1 if valid port number, else return 0
*/
int validPortNumber(int port) {
	return (port > 0) && (port <= 65535);
}
/*
* Check valid messages
* @param char* mess
* @return 1 if valid, else return 0
*/
int validMessages(char* mess) {
	int i;
	for(i = 0; i < strlen(mess); i++) {
		if((isalpha(mess[i]) == 0) && (isdigit(mess[i]) == 0)) {
			return 0;
		}
	}
	return 1;
}
/*
* get char from messages
* @param char* mess, char* str
* @return void
*/
void charFromMessage(char* mess, char* str) {
	int i;
	int j = 0;
	for(i = 0; i < strlen(mess); i++) {
		if(isalpha(mess[i]) != 0) {
			str[j++] = mess[i];
		}
	}
}
/*
* get number from messages
* @param char* mess, char* str
* @return void
*/
void numberFromMessage(char* mess, char* str) {
	int i;
	int j = 0;
	for(i = 0; i < strlen(mess); i++) {
		if(isdigit(mess[i]) != 0) {
			str[j++] = mess[i];
		}
	}
}
/*
* main function
* @param int argc, char** argv
* @return void
*/
int main(int argc, char **argv)
{
	int port_number;
 	if(argc != 2) {
 		perror(" Error Parameter! Please input only port number\n ");
 		exit(0);
 	}
 	if((port_number = atoi(argv[1])) == 0) {
 		perror(" Please input port number\n");
 		exit(0);
 	}
 	if(!validPortNumber(port_number)) {
 		perror("Invalid Port Number!\n");
 		exit(0);
 	}
 	int server_sock; /* file descriptors */
	char buff[BUFF_SIZE];
	int bytes_sent, bytes_received;
	struct sockaddr_in server; /* server's address information */
	struct sockaddr_in client; /* client's address information */
	int sin_size;

	//Step 1: Construct a UDP socket
	if ((server_sock=socket(AF_INET, SOCK_DGRAM, 0)) == -1 ){  /* calls socket() */
		perror("\nError: ");
		exit(0);
	}
	
	//Step 2: Bind address to socket
	server.sin_family = AF_INET;         
	server.sin_port = htons(port_number);   /* Remember htons() from "Conversions" section? =) */
	server.sin_addr.s_addr = INADDR_ANY;  /* INADDR_ANY puts your IP address automatically */   
	bzero(&(server.sin_zero),8); /* zero the rest of the structure */

  
	if(bind(server_sock,(struct sockaddr*)&server,sizeof(struct sockaddr))==-1){ /* calls bind() */
		perror("\nError: ");
		exit(0);
	}     
	printf("Server running in port %d\n", port_number);
	//Step 3: Communicate with clients
	while(1){
		sin_size=sizeof(client);
    		
		bytes_received = recvfrom(server_sock, buff, BUFF_SIZE-1, 0, (struct sockaddr *) &client,(unsigned int*) &sin_size);
		
		if (bytes_received < 0)
			perror("\nError: ");
		else{
			char number[BUFF_SIZE];
 			char character[BUFF_SIZE];
			buff[bytes_received - 1] = '\0';
			if(!validMessages(buff)) {
				char error[100] = "Message has invalid character!!"; // Message error
				bytes_sent = sendto(server_sock, error, 2000, 0, (struct sockaddr *) &client, sin_size ); /* send to the client welcome message */
				if (bytes_sent < 0)
					perror("\nError: ");
				continue;
			}
			numberFromMessage(buff, number);// get number from message save in number variable
			charFromMessage(buff, character);// get char from message save in character
			printf("[%s:%d]: %s\n", inet_ntoa(client.sin_addr), ntohs(client.sin_port), buff); // print info client
			char responseMessage[2000] = "";
			strcat(responseMessage, character); 
			strcat(responseMessage, "\n");
			strcat(responseMessage, number);
			strcat(responseMessage, "\0"); //response
			bytes_sent = sendto(server_sock, responseMessage, 2000, 0, (struct sockaddr *) &client, sin_size ); /* send to the client welcome message */
			if (bytes_sent < 0)
				perror("\nError: ");
		}				
	}
	
	close(server_sock);
	return 0;
}