
#include <stdio.h>          /* These are the usual header files */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <time.h>
#include <poll.h>

#include "protocol.h"


#define PORT 5550   /* Port that will be opened */ 
#define BACKLOG 2   /* Number of allowed connections */
#define MAX_SIZE 10e6 * 100
#define STORAGE "./storage/" //default save file place

#define EXISTEDFILE "Error: File is existent on server"
#define BUFF_SEND 1024
#define PRIVATE_KEY 256

typedef struct client_status {
	ProtocolStatus status;
	int keycode;
	char opcode;
	char tmpFileName[1000];
} client_status;

/*
* Check valid port number
* @param int port
* @return 1 if valid port number, else return 0
*/
int validPortNumber(int port) {
	return (port > 0) && (port <= 65535);
}

/*
* Caesar Encryption
* @param char* message, int key, int length
* @ encrypt message with key
*/
int encryption(char* message, int key, int length) {
	char ch;
	int i;
	for(i = 0; i < length; ++i){
        ch = message[i];
        
        if((ch + key) > 0) {
    		message[i] = (ch + key) % PRIVATE_KEY;
    	} else {
    		message[i] = (ch + key) % PRIVATE_KEY + 256;
    	}
    }
    return 0;
}

/*
* Caesar Decryption
* @param char* message, int key, int length
* @ Decrypt message with key
*/
int decryption(char* message, int key, int length) {
	char ch;
	int i;
	for(i = 0; i < length; ++i){
        ch = message[i];
        message[i] = (ch - key) % PRIVATE_KEY;
    }
    return 0;
}

/*
* Handler Request from Client
* @param char* message, int key
* return void*
*/
int client_handler(void* conn_sock, Message recvMess, client_status* clientStatus) {
	FILE *tmpFile;
	int connSock;
	connSock = *((int *) conn_sock);
	Message sendMess;
	switch(clientStatus->status) {
		// waiting keycode signal from client
		case WAITING_KEYCODE: 
			if(isFunctionalOpcode(recvMess.opcode)) {
				
				clientStatus->status = WAITING_FILECONTENT;
				clientStatus->opcode = recvMess.opcode;
				clientStatus->keycode = atoi(recvMess.payload);
				sprintf(clientStatus->tmpFileName, "%lu", (unsigned long)time(NULL));
				// open file for save file content sent by client
			}
		break;
		// waiting file signal from client
		case WAITING_FILECONTENT:
			if(recvMess.opcode == OP_FILE) {
				if(recvMess.length > 0) {
					if((tmpFile = fopen(clientStatus->tmpFileName, "ab+")) == NULL) {
						perror("You have not create file permission!!\n");
						return -1;
					}
					if(clientStatus->opcode == OP_ENCRYPT) {
						//encrypt content
						encryption(recvMess.payload, clientStatus->keycode, (int) recvMess.length);
					} else {
						//decrypt content
						decryption(recvMess.payload, clientStatus->keycode, (int) recvMess.length);
					}
					// write encrypt content or decrypt content to file
					fwrite(recvMess.payload, 1, recvMess.length, tmpFile);
					fclose(tmpFile);
					break;
				}
				else if (recvMess.length == 0) {
					// rewind pointer of file to beggin
					if((tmpFile = fopen(clientStatus->tmpFileName, "rb+")) == NULL) {
						perror("You have not create file permission!!\n");
						return -1;
					}
					fseek(tmpFile, 0, SEEK_SET);
					 while(!feof(tmpFile)) {
			            char buffer[PAYLOAD_SIZE];
			           	int bytes_send = fread(buffer, 1, PAYLOAD_SIZE, tmpFile);
			           	if(bytes_send <= 0) {
			           		break;
			           	}
			 	        sendMess.opcode = OP_FILE;
                        sendMess.length = (short) bytes_send;
                        memcpy(sendMess.payload, buffer, bytes_send);
                        if(sendMessage(connSock, sendMess) <= 0) {
                            printf("Connection closed!\n");
                            return -1;
                        }
			        }
			        fclose(tmpFile);
			        // send signal send file success
		        	sendMess.opcode = OP_FILE;
		        	sendMess.length = 0;
		        	strcpy(sendMess.payload, "\0");
		        	if(sendMessage(connSock, sendMess) <= 0) {
		        		return -1;
		        	}
		        	//remove file
		            if (remove(clientStatus->tmpFileName) == 0)
		                printf("Handle Success!!! %s file deleted successfully.\n", clientStatus->tmpFileName);
		            else
		            {
		                printf("Unable to delete the file\n");
		                perror("Following error occurred");
		                return -1;
		            }
		        	clientStatus->status = WAITING_KEYCODE;
				}
			}
		break;
		default:  break;
	}

	return 1;
}

int main(int argc, char **argv)
{
 	int port_number;
 	int listen_sock, conn_sock; /* file descriptors */
	struct sockaddr_in server; /* server's address information */
	struct sockaddr_in client; /* client's address information */
	int sin_size;
	struct pollfd sockets[1024];
	int sock_number = 0;
	int current_size;
	int ret;
	int bytes_received;
	client_status clientStatus[1024];
	Message recvMess;
		
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
	if ((listen_sock = socket(AF_INET, SOCK_STREAM, 0)) == -1 ){  /* calls socket() */
		perror("\nError: ");
		return 0;
	}
	
	//Step 2: Bind address to socket
	bzero(&server, sizeof(server));
	server.sin_family = AF_INET;         
	server.sin_port = htons(port_number);   /* Remember htons() from "Conversions" section? =) */
	server.sin_addr.s_addr = htonl(INADDR_ANY);  /* INADDR_ANY puts your IP address automatically */   
	if(bind(listen_sock, (struct sockaddr*)&server, sizeof(server))==-1){ /* calls bind() */
		perror("\nError: ");
		return 0;
	}     
	
	//Step 3: Listen request from client
	if(listen(listen_sock, BACKLOG) == -1){  /* calls listen() */
		perror("\nError: ");
		return 0;
	}
	
	for(int i=0; i<1024; i++)
	{
		sockets[i].fd = -1;
		// clientStatus[i] is sockets[i] information status
		clientStatus[i].status = WAITING_KEYCODE;
		clientStatus[i].opcode = 0;
		clientStatus[i].keycode = OP_ENCRYPT;
	}

	sockets[0].fd = listen_sock;
	sockets[0].events = POLLIN;
	sock_number = 1;

	//Step 4: Communicate with client
	while(1){
		ret = poll(sockets, sock_number, 3500000);
		current_size = sock_number;
		int i, j;
		for(i = 0; i < current_size; i++) {
			if(sockets[0].revents & POLLIN){ //new client
				sin_size = sizeof(struct sockaddr_in);
				if ((conn_sock = accept(listen_sock,( struct sockaddr *)&client, (unsigned int*)&sin_size)) == -1) 
					perror("\nError: ");

				if(sock_number == 1024) 
				{
					printf("too many client!\n");
					close(conn_sock);
					continue;
				}

				printf("You got a connection from %s\n", inet_ntoa(client.sin_addr) ); /* prints client's IP */

				for(j = 0; j < 1024; j++){
					if(sockets[j].fd == -1){
						sockets[j].fd = conn_sock;
						sockets[j].events = POLLIN;
						sock_number++;
						break;
					}
				}
				
			}
			if((sockets[i].fd > -1) && (sockets[i].revents & POLLIN) && (sockets[i].fd != listen_sock)){
				//receives message from client
				bytes_received = receiveMessage(sockets[i].fd, &recvMess); //blocking
				if (bytes_received <= 0){
					//reset sockets[i] and reset clientStatus[i]
					sockets[i].fd = -1;
					sock_number--;
					clientStatus[i].status = WAITING_KEYCODE;
					clientStatus[i].opcode = 0;
					clientStatus[i].keycode = OP_ENCRYPT;
					perror("error: \n");
					printf("\nConnection closed");
					continue;
				}
				if(client_handler(&(sockets[i].fd), recvMess, &clientStatus[i]) < 0) {
					perror("\nError: ");
					close(sockets[i].fd);
					//reset sockets[i] and reset clientStatus[i]
					sockets[i].fd = -1;
					sock_number--;
					clientStatus[i].status = WAITING_KEYCODE;
					clientStatus[i].opcode = 0;
					clientStatus[i].keycode = OP_ENCRYPT;
					continue;
				}
				// errror or client closed
			}
			sockets[0].revents = 0;
		}
	}
	
	close(listen_sock);
	return 0;
}