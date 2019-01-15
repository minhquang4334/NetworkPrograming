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
#define NUMBEROFDOTSINIPV4 3 //number dots in ipv4
#define NUMBEROFDOTSINIPV6 5 //number dots in ipv6
char **tokens;

int client_sock;
int under_client_sock;
struct sockaddr_in server_addr; /* server's address information */

char choose;
Message *mess;
int isOnline = 0;



/*
* Check valid number in range 0 -> 255
* @param char* value
* @return boolean
*/
int validNumber(char *value)
{
    if(!strcmp(value, "0")) {
        return 1;
    }
    return (atoi(value) > 0) && (atoi(value) <= 255);
}

/*
* Check dots in string equals dots in ip address
* @param char* string
* @return boolean(0,1)
*/
int checkDots(char *str)
{
    tokens = str_split(str, '.');
    if (tokens)
    {
        int i;
        for (i = 0; *(tokens + i); i++)
        {
            // count number elements in array
        }
        if((i-1) == NUMBEROFDOTSINIPV4) {
            return 1;
        }
    }
    return 0;
}
/*
* Check valid Ip
* @param char* string
* @return 1 if valid ip, 0 if invalid ip
*/
int checkIP(char *str)
{
    if(checkDots(str)) {
        if (tokens) {
            int i;
            for (i = 0; *(tokens + i); i++)
            {
                if(!validNumber(*(tokens + i))) {
                    return 0;
                }
                free(*(tokens + i));
            }
            free(tokens);
            return 1;
        }
    }
    return 0;
}
/*
* Check valid port number
* @param int port
* @return 1 if valid port number, else return 0
*/
int validPortNumber(int port) {
	return (port > 0) && (port <= 65535);
}

/*
* Check valid Ip
* @param char* ip
* @return 1 has found ip address, else return 0
*/
int hasIPAddress(char *ip) {
    struct in_addr ipv4addr;
    inet_pton(AF_INET, ip, &ipv4addr);
    struct hostent *host = gethostbyaddr(&ipv4addr, sizeof(ipv4addr), AF_INET);
    if (host != NULL)
    {
        return 1;
    }
    return 0;
}


// init new socket
// print error if have error
int initSock(){
	int newsock=socket(AF_INET, SOCK_STREAM, 0);
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
	printf("blind is success in port %d and server %s\n",port,serverAddr );
}


// connect client to server
// parameter: client socket, server address
// if have error, print error and exit
void connectToServer(SocketType type){
	if(type==UNDER_SOCK){
		if(connect(under_client_sock, (struct sockaddr*)(&server_addr), sizeof(struct sockaddr)) < 0){
			printf("\nError!Can not connect to sever! Client exit imediately!\n");
			exit(0);
		}
	}
	else{
		if(connect(client_sock, (struct sockaddr*)(&server_addr), sizeof(struct sockaddr)) < 0){
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
	scanf("%s",username);
	printf("Enter password?: ");
	scanf("%s",password);
	while(getchar()!='\n');
	sprintf(mess->payload, "LOGIN\nUSER %s\nPASS %s", username, password);
	strcpy(str, username);
}

// start method run on background to wait search and download file request
void UnderMethodStart(){
	under_client_sock=initSock();
	connectToServer(UNDER_SOCK);
}

// close method run on background
void UnderMethodClose(){
	close(under_client_sock);
}

// download file func
// void downloadFileFunc(){
// 	int i;
// 	char **listUser;
// 	listUser = str_split(mmakpayload, ',');
// 	printf("List username:\n");
// 	for(i=0;i<listUser.length;i++){
// 		printf("%d : %s\n",i+1, listUser[i]);
// 	}
// 	printf("Choose user who you want download file from?\n");
// 	scanf("%d", &choose);
// 	if(choose<1||choose>listUser.length){
// 		printf("Syntax error!\n");
// 	}
// 	else{
// 		mess->type=TYPE_REQUEST_DOWNLOAD;
// 		strcpy(mess->payload, listUser[i-1]);
// 		sendMessage(client_sock, &m;
// 		FILE *ptrFile;
// 		while(1) {
//             if(receiveMessage(client_sock, &recvMsg) <= 0) {
//                 printf("Connection closed!\n");
//                 check = 0;
//                 break;
//             }
//             fwrite(recvMsg.payload, recvMsg.length, 1, fptr);
//         }
//         fclose(fptr);
// 	}
// }

// // search file func
// void searchFileFunc(){
// 	char fileName[255];
// 	printf("Enter file name?\n");
// 	scanf("%s", fileName);
// 	mess->type=TYPE_REQUEST_FILE;
// 	strcpy(mess->payload,fileName);
// 	mess->length = strlen(mess->payload);
// 	sendMessage(client_sock,&mess);
// 	receiveMessage(client_sock,mess);
// 	if(mess->type==TYPE_ERROR){
// 		printf("%s\n", mess->payload);
// 	}
// 	else downloadFileFunc();
// }

void loginFunc(char *current_user){
	char username[255];
	mess->type = TYPE_AUTHENTICATE;
	getLoginInfo(username);
	mess->length=strlen(mess->payload);
	sendMessage(client_sock, *mess);
	receiveMessage(client_sock,mess);
	if(mess->type!=TYPE_ERROR){
		isOnline=1;
		strcpy(current_user, username);
		UnderMethodStart();
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
		mess->type=TYPE_AUTHENTICATE;
		mess->length=strlen(mess->payload);
		sendMessage(client_sock, *mess);
		receiveMessage(client_sock,mess);
		if(mess->type!=TYPE_ERROR){
			isOnline=1;
			strcpy(current_user, username);
			UnderMethodStart();
		}
		printf("%s\n", mess->payload);
	}
}

void logoutFunc(char *current_user){
	mess->type=TYPE_AUTHENTICATE;
	sprintf(mess->payload,"LOGOUT\n%s",current_user);
	mess->length=strlen(mess->payload);
	sendMessage(client_sock, *mess);
	receiveMessage(client_sock,mess);
	if(mess->type!=TYPE_ERROR){
		isOnline=0;
		current_user[0]='\0';
		UnderMethodClose();
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
			while(getchar()!='\n');
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
			while(getchar()!='\n');
			switch (choose){
				case '0':
					logoutFunc(current_user);
					isOnline=0;
					UnderMethodClose();
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
	mess = (Message*)malloc(sizeof(Message));
	mess->requestId=0;
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