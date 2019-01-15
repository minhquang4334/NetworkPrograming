#include <stdio.h>          /* These are the usual header files */
#include <stdlib.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <sys/uio.h>

#include "protocol.h"
// #include "authenticate.h"
#include "validate.h"
#include "status.h"

int client_sock;
int under_client_sock;
struct sockaddr_in server_addr; /* server's address information */

char choose;
Message *mess;
int isOnline = 0;

// init new socket
// print error if have error
int initSock(){
	int newsock = socket(AF_INET, SOCK_STREAM, 0);
	if (newsock == -1 ){
		perror("\nError: ");
		exit(0);
	}
	return newsock;
}


// bind socket of client 
void bindClient(int port, char *serverAddr){
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	server_addr.sin_addr.s_addr = inet_addr(serverAddr);
}


// connect client to server
// parameter: client socket, server address
// if have error, print error and exit
void connectToServer(SocketType type){
	if(type == UNDER_SOCK){
		if(connect(under_client_sock, (struct sockaddr*) (&server_addr), sizeof(struct sockaddr)) < 0){
			printf("\nError!Can not connect to sever! Client exit imediately!\n");
			exit(0);
		}
	}
	else{
		if(connect(client_sock, (struct sockaddr*) (&server_addr), sizeof(struct sockaddr)) < 0){
			printf("\nError!Can not connect to sever! Client exit imediately!\n");
			exit(0);
		}
	}
}

// get username and password from keyboard to login
void getLoginInfo(char *str){
	char username[255];
	char password[255];
	printf("Enter username?: ");
	scanf("%s", username);
	printf("Enter password?: ");
	scanf("%s", password);
	while(getchar()!='\n');
	sprintf(mess->payload, "LOGIN\nUSER %s\nPASS %s", username, password);
	strcpy(str, username);
}

void loginFunc(char *current_user){
	char username[255];
	mess->type = TYPE_AUTHENTICATE;
	getLoginInfo(username);
	mess->length = strlen(mess->payload);
	sendMessage(client_sock, *mess);
	receiveMessage(client_sock, mess);
	if(mess->type != TYPE_ERROR){
		isOnline=1;
		strcpy(current_user, username);
		//UnderMethodStart();
	}
	printf("%s\n", mess->payload);
}

int getRegisterInfo(char *user){
	char username[255], password[255], confirmPass[255];
	printf("Username: ");
	scanf("%s", username);
	printf("Password: ");
	scanf("%s", password);
	printf("Confirm password: ");
	scanf("%s", confirmPass);
	while(getchar()!='\n');
	if(!strcmp(password, confirmPass)){
		sprintf(mess->payload, "REGISTER\nUSER %s\nPASS %s", username, password);
		strcpy(user, username);
		return 1;
	}
	else{
		printf("Confirm password invalid!\n");
		return 0;
	}
	
}

void registerFunc(char *current_user){
	char username[255];
	if(getRegisterInfo(username)){
		mess->type = TYPE_AUTHENTICATE;
		mess->length = strlen(mess->payload);
		sendMessage(client_sock, *mess);
		receiveMessage(client_sock, mess);
		if(mess->type != TYPE_ERROR){
			isOnline = 1;
			strcpy(current_user, username);
			//UnderMethodStart();
		}
		printf("%s\n", mess->payload);
	}
}

void logoutFunc(char *current_user){
	mess->type = TYPE_AUTHENTICATE;
	sprintf(mess->payload, "LOGOUT\n%s", current_user);
	mess->length = strlen(mess->payload);
	sendMessage(client_sock, *mess);
	receiveMessage(client_sock, mess);
	if(mess->type != TYPE_ERROR){
		isOnline = 0;
		current_user[0] = '\0';
		//UnderMethodClose();
	}
	printf("%s\n", mess->payload);
}

// communicate from client to server
// send and recv message with server
void communicateWithServer(){
	char current_user[255];
	while(1){
		if(!isOnline){
			printf("Choose 0 to login - 1 to register!\n");
			scanf("%c", &choose);
			while(getchar() != '\n');
			switch (choose){
				case '0':
					loginFunc(current_user);
					break;
				case '1':
					registerFunc(current_user);
					break;
				default:
					printf("Syntax Error! Please choose again!\n");
			}
		}
		else{
			printf("Choose 1 to search file - 0 to logout?\n");
			scanf("%c", &choose);
			while(getchar() != '\n');
			switch (choose){
				case '0':
					logoutFunc(current_user);
					isOnline = 0;
					//UnderMethodClose();
					break;
				case '1':
					// searchFileFunc();
					printf("search file func!\n");
					break;
				default:
					printf("Syntax Error! Please choose again!\n");
			}
		}
	}
}

/*
* Main function
* @param int argc, char** argv
* @return 0
*/
int main(int argc, char const *argv[])
{

	// check valid of IP and port number 
	if(argc!=3){
		printf("Error!\nPlease enter two parameter as IPAddress and port number!\n");
		exit(0);
	}
	char *serAddr = malloc(sizeof(argv[1]) * strlen(argv[1]));
	strcpy(serAddr, argv[1]);
	int port = atoi(argv[2]);
	mess = (Message*) malloc (sizeof(Message));
	mess->requestId = 0;
 	if(!validPortNumber(port)) {
 		perror("Invalid Port Number!\n");
 		exit(0);
 	}
	if(!checkIP(serAddr)) {
		printf("Invalid Ip Address!\n"); // Check valid Ip Address
		exit(0);
	}
	strcpy(serAddr, argv[1]);
	if(!hasIPAddress(serAddr)) {
		printf("Not found information Of IP Address [%s]\n", serAddr); // Find Ip Address
		exit(0);
	}

	//Step 1: Construct socket
	client_sock = initSock();
	//Step 2: Specify server address
	bindClient(port, serAddr);
	
	//Step 3: Request to connect server
	connectToServer(SOCK);

	//Step 4: Communicate with server			
	communicateWithServer();

	close(client_sock);
	return 0;
}