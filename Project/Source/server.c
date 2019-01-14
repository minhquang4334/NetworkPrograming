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
#include <pthread.h>

#include "protocol.h"
#include "authenticate.h"
#include "validate.h"
#include "status.h"

#define PORT 5550   /* Port that will be opened */ 
#define BACKLOG 2   /* Number of allowed connections */
#define MAX_SIZE 10e6 * 100
#define STORAGE "./storage/" //default save file place

#define BUFF_SEND 1024
#define PRIVATE_KEY 256
int requestId = 1;
/*
* Check valid port number
* @param int port
* @return 1 if valid port number, else return 0
*/
int validPortNumber(int port) {
	return (port > 0) && (port <= 65535);
}
// count number param of command
int numberElementsInArray(char** temp) {
	int i;
	for (i = 0; *(temp + i); i++)
    {
        // count number elements in array
    }
    return i;
}

void handleLogin(Message mess, int connSock) {
	char** temp = str_split(mess.payload, '\n');
	StatusCode loginCode;
	if(numberElementsInArray(temp) == 3) {
		char** userStr = str_split(temp[1], ' ');
		char** passStr = str_split(temp[2], ' ');
		if((numberElementsInArray(userStr) == 2) && (numberElementsInArray(passStr) == 2)) {
			if(!(strcmp(userStr[0], COMMAND_USER) || strcmp(passStr[0], COMMAND_PASSWORD))) {
				char username[30];
				char password[20];
				strcpy(username, userStr[1]);
				strcpy(password, passStr[1]);
				loginCode = login(username, password);
				if(loginCode!=LOGIN_SUCCESS)
					mess.type=TYPE_ERROR;
				else{
					if(mess.requestId == 0) {
						mess.requestId = requestId++;
					}
				}
			}
			else{
				loginCode = COMMAND_INVALID;
				mess.type=TYPE_ERROR;
			}
		}
		else{
			loginCode = COMMAND_INVALID;
			mess.type=TYPE_ERROR;
		}
	}
	else {
		mess.type=TYPE_ERROR;
		loginCode = COMMAND_INVALID;
		printf("Fails on handle Login!!");
	}
	sendWithCode(mess, loginCode, connSock);
}

void handleRegister(Message mess, int connSock){
	char** temp = str_split(mess.payload, '\n');
	StatusCode registerCode;
	if(numberElementsInArray(temp) == 3) {
		char** userStr = str_split(temp[1], ' ');
		char** passStr = str_split(temp[2], ' ');
		if((numberElementsInArray(userStr) == 2) && (numberElementsInArray(passStr) == 2)) {
			if(!(strcmp(userStr[0], COMMAND_USER) || strcmp(passStr[0], COMMAND_PASSWORD))) {
				char username[30];
				char password[20];
				strcpy(username, userStr[1]);
				strcpy(password, passStr[1]);
				registerCode = registerUser(username, password);
				if(registerCode!=REGISTER_SUCCESS)
					mess.type=TYPE_ERROR;
				else{
					if(mess.requestId == 0) {
						mess.requestId = requestId++;
					}
				}
			}
			else{
				registerCode = COMMAND_INVALID;
				mess.type=TYPE_ERROR;
			}
		}
		else{
			registerCode = COMMAND_INVALID;
			mess.type=TYPE_ERROR;
		}
	}
	else {
		mess.type=TYPE_ERROR;
		registerCode = COMMAND_INVALID;
		printf("Fails on handle Register!!");
	}
	sendWithCode(mess, registerCode, connSock);
}

void handleLogout(Message mess, int connSock){
	char** temp = str_split(mess.payload, '\n');
	StatusCode logoutCode;
	if(numberElementsInArray(temp) != 1) {
		mess.type=TYPE_ERROR;
		logoutCode = COMMAND_INVALID;
		printf("Fails on handle logout!!");
	}
	else{
		logoutCode = logoutUser();
	}
	sendWithCode(mess, logoutCode, connSock);
}

void handleAuthenticateRequest(Message mess, int connSock) {
	char* payloadHeader;
	char temp[PAYLOAD_SIZE];
	strcpy(temp, mess.payload);
	payloadHeader = getHeaderOfPayload(temp);
	if(!strcmp(payloadHeader, LOGIN_CODE)) {
		handleLogin(mess, connSock);
	} else if (!strcmp(payloadHeader, REGISTER_CODE)) {
		handleRegister(mess, connSock);

	} else if(!strcmp(payloadHeader, LOGOUT_CODE)) {
		handleLogout(mess, connSock);
	}
}

void handleRequestFile(Message recvMess, int connSock) {

}
/*
* Handler Request from Client
* @param char* message, int key
* return void*
*/
void* client_handler(void* conn_sock) {
	// char tmpFileName[100];
	int bytes_received;
	// FILE *tmpFile = NULL;
	int connSock;
	// ProtocolStatus status = WAITING_KEYCODE;
	connSock = *((int *) conn_sock);
	Message recvMess;
	// Message sendMess, keyMess;
	
	pthread_detach(pthread_self());
	while(1){
		//receives message from client
		bytes_received = receiveMessage(connSock, &recvMess); //blocking
		if (bytes_received <= 0) {
			printf("\nConnection closed\n");
			break;
		}
		switch(recvMess.type) {
			case TYPE_AUTHENTICATE: 
				handleAuthenticateRequest(recvMess, connSock);
				break;
			case TYPE_REQUEST_FILE: 
				// handleRequestFile(recvMess, connSock);
				break;
			case TYPE_REQUEST_DOWNLOAD: 
				break;
			case TYPE_UPLOAD_FILE: 
				break;
			case TYPE_ERROR: 
				break;
			default: break;
		}

	}

	return NULL;
}

int main(int argc, char **argv)
{
 	int port_number;
 	int listen_sock, conn_sock; /* file descriptors */
	struct sockaddr_in server; /* server's address information */
	struct sockaddr_in client; /* client's address information */
	int sin_size;
	pthread_t tid;
		
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
	
	readFile();
	printList();
	//Step 4: Communicate with client
	while(1) {
		//accept request
		sin_size = sizeof(struct sockaddr_in);
		if ((conn_sock = accept(listen_sock,( struct sockaddr *)&client, (unsigned int*)&sin_size)) == -1) 
			perror("\nError: ");
  
		printf("You got a connection from %s\n", inet_ntoa(client.sin_addr) ); /* prints client's IP */
		
		//start conversation
		pthread_create(&tid, NULL, &client_handler, &conn_sock);	
	}
	
	close(listen_sock);
	return 0;
}