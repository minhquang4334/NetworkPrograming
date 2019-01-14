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
#include "authenticate.h"
#include "validate.h"
#include "status.h"

#define BUFF_SIZE 1024
#define NUMBEROFDOTSINIPV4 3 //number dots in ipv4
#define NUMBEROFDOTSINIPV6 5 //number dots in ipv6
char **tokens;

int client_sock;
int under_client_sock;
struct sockaddr_in server_addr; /* server's address information */

User *current_user;
int choose;
Message mess;
mess->requestId=0;


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
* Split string
* @param char* string
* @param const char a_delim
* @return boolean
* source https://stackoverflow.com/questions/9210528/split-string-with-delimiters-in-c
*/
char** str_split(char* a_str, const char a_delim)
{
    char** result    = 0;
    size_t count     = 0;
    char* tmp        = a_str;
    char* last_comma = 0;
    char delim[2];
    delim[0] = a_delim;
    delim[1] = 0;

    /* Count how many elements will be extracted. */
    while (*tmp)
    {
        if (a_delim == *tmp)
        {
            count++;
            last_comma = tmp;
        }
        tmp++;
    }

    /* Add space for trailing token. */
    count += last_comma < (a_str + strlen(a_str) - 1);

    /* Add space for terminating null string so caller
       knows where the list of returned strings ends. */
    count++;

    result = malloc(sizeof(char*) * count);

    if (result)
    {
        size_t idx  = 0;
        char* token = strtok(a_str, delim);

        while (token)
        {
            assert(idx < count);
            *(result + idx++) = strdup(token);
            token = strtok(0, delim);
        }
        assert(idx == count - 1);
        *(result + idx) = 0;
    }

    return result;
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
    int i;
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
char *getLoginInfo(char *buff){
	char username[255];
	char password[255];
	printf("Enter username?: ");
	scanf("%s",username);
	printf("Enter password?: ");
	scanf("%s",password);
	strcat("LOGIN",buff);
	strcat("\n",buff);
	strcat("User",buff);
	strcat(username,buff);
	strcat("\n",buff);
	strcat("Pass",buff);
	strcat(password,buff);
	return username;
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
void downloadFileFunc(){
	int i;
	char **listUser;
	listUser = str_split(mess.payload, ',');
	printf("List username:\n");
	for(i=0;i<listUser.length;i++){
		printf("%d : %s\n",i+1, listUser[i]);
	}
	printf("Choose user who you want download file from?\n");
	scanf("%d", &choose);
	if(choose<1||choose>listUser.length){
		printf("Syntax error!\n");
	}
	else{
		mess->type=TYPE_REQUEST_DOWNLOAD;
		strcpy(mess.payload, listUser[i-1]);
		sendMessage(client_sock, mess);
		FILE *ptrFile;
		while(1) {
            if(receiveMessage(client_sock, &recvMsg) <= 0) {
                printf("Connection closed!\n");
                check = 0;
                break;
            }
            fwrite(recvMsg.payload, recvMsg.length, 1, fptr);
        }
        fclose(fptr);
	}
}

// search file func
void searchFileFunc(){
	char fileName[255];
	printf("Enter file name?\n");
	scanf("%s", fileName);
	mess->type=TYPE_REQUEST_FILE;
	mess->length=1;
	strcpy(mess.payload,fileName);
	sendMessage(client_sock,mess);
	receiveMessage(client_sock,mess);
	if(mess->type==TYPE_ERROR){
		printf("%s\n", mess.payload);
	}
	else downloadFileFunc();
}


// communicate from client to server
// send and recv message with server
void communicateWithServer(){
	char username[255];
	while(1){
		if(current_user==NULL){
			printf("Login to connect to server!\n");
			mess->type = TYPE_AUTHENTICATE;
			mess->length=1;
			username = getLoginInfo(mess.payload);
			sendMessage(client_sock, mess);
			receiveMessage(client_sock,mess);
			if(mess->type==TYPE_ERROR){
				printf("%s\n", mess.payload);
			}
			else{
				printf("Login success!\n");
				current_user = searchUser(username);
				UnderMethodStart();
			}
		}
		else{
			printf("Choose 1 to search file?? 0 to logout?\n");
			scanf("%d", &choose);
			if(choose==1)
				searchFileFunc();
			else if(choose==0){
				logout(current_user);
				current_user=NULL;
				UnderMethodClose();
			}
			else{
				printf("Syntax error!\n");
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


	return 0;
}