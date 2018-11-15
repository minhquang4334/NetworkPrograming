#include <stdio.h>          /* These are the usual header files */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <stdlib.h>
#include <assert.h>
#include "status.h"

#define PORT 5550   /* Port that will be opened */ 
#define BACKLOG 2   /* Number of allowed connections */
#define BUFF_SIZE 1024
#define LOGIN 1
#define OFFLINE 0
#define BLOCK 0
#define ACTIVE 1
#define ACCOUNT_FILE "account.txt"
#define COMMAND_USER "USER"
#define COMMAND_PASSWORD "PASS"
#define COMMAND_LOGOUT "LOGOUT"

//status of protocol
enum LoginStatus {
	Unidentified,
	UnAuthenticated,
	Authenticated
};
// user structure
typedef struct user{
	char username[254];
	char password[32];
	int isLogin;
	int status;
	int countLoginFails;
	struct user *next;
} User;

typedef struct client {
	int socket_number;
	enum LoginStatus status;
	User *user;
} Client;

User *head = NULL;
User *current = NULL;

// socket variables

int listen_sock, conn_sock; /* file descriptors */
char recv_data[BUFF_SIZE];
int bytes_sent, bytes_received;
struct sockaddr_in server; /* server's address information */
struct sockaddr_in cliaddr; /* client's address information */
int sin_size;
fd_set allset;


// create new user from username, password, status
User *createNewUser(char* username, char* password, int status) {
	User *newUser = (User*)malloc(sizeof(User));
	strcpy(newUser->username, username);
	strcpy(newUser->password, password);
	newUser->status = status;
	newUser->isLogin = OFFLINE;
	newUser->countLoginFails = 0;
	newUser->next = NULL;
	return newUser;
}
// print List users
void printList() {
    User *ptr = head;

    while (ptr != NULL) {
        printf("Username: %s\n", ptr->username);
        printf("Status: %s\n", ((ptr->status == ACTIVE) ? "Active" : "Blocked"));
        printf("----------------------------------\n");

        ptr = ptr->next;
    }
    return;
}
// add user to end of list
void push(User* newUser) {
	if(head == NULL) {
		head = newUser;
		current = newUser;
	}
 	else {
 		current->next = (User*)malloc(sizeof(User));
		current->next = newUser;
		current = newUser;
 	}
	return;
}
// ad user to beginning of list
void pushToBeginning(User** head, User * newUser) {
	User * new_node;
    new_node = (User*)malloc(sizeof(User));
    new_node = newUser;
    new_node->next = *head;
    *head = new_node;
    return;
}

// fucntional
User* search(char *username) {
	User *ptr = head;

    while (ptr != NULL) {
    	if(!strcmp(ptr->username, username)) break;
        ptr = ptr->next;
    }
    
	return ptr;
}
// exit program
void exitProgram() {
	printf("End Program :))\n");
	exit(0);
}
// update file
void updateFile() {
	FILE* fOut;
	fOut = fopen(ACCOUNT_FILE, "w");
	if(!fOut) {
		printf("File not exist??\n");
		exitProgram();
	}

	User *ptr = head;

    while (ptr != NULL) {
        fprintf(fOut, "%s %s %d\n", ptr->username, ptr->password, ptr->status);    	
    	ptr = ptr->next;
    }

    fclose(fOut);
    return;
}
// read file account.txt
void readFile() {
	char username[254];
	char password[32];
	int status;
	char c;
	FILE* fIn;
	fIn = fopen(ACCOUNT_FILE, "r");
	if(!fIn) {
		printf("File not exist??\n");
		exitProgram();
	}

	while(!feof(fIn)) {
		if(fscanf(fIn, "%s %s %d%c", username, password, &status, &c) != EOF) {
			push(createNewUser(username, password, status));
		}
		if(feof(fIn)) break;
	}
	fclose(fIn);
	return;
}

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
* Check valid port number
* @param int port
* @return 1 if valid port number, else return 0
*/
int validPortNumber(int port) {
	return (port > 0) && (port <= 65535);
}
// send messgage to Client
void sendByte(char* message, int sockfd) {
	bytes_sent = send(sockfd, message, strlen(message), 0);
	if(bytes_sent <= 0){
		printf("\nConnection closed!\n");
		FD_CLR(sockfd, &allset);
		close(sockfd);
	}
}
// Compile message with code
void sendToClient(char* message, int code, int sockfd) {
	char mess[200];
	char strCode[10];
	char space[1] = " ";
	snprintf(mess, 200, "%d ", code);
	strcat(mess, message);
	printf("%s\n", mess);
	sendByte(mess, sockfd);
}
// count number param of command
int numberParametersOfCommand(char** temp) {
	int i;
	for (i = 0; *(temp + i); i++)
    {
        // count number elements in array
    }
    return i;
}
// check user input is valid command
int isCommand(char** temp) {
	
	if(numberParametersOfCommand(temp) == 2) {
		char* command = (char*) malloc(strlen(temp[0]) * sizeof(char));
		char* param = (char*) malloc(strlen(temp[1]) * sizeof(char));
		strcpy(command, temp[0]);
		strcpy(param, temp[1]);
		if(!strcmp(command, COMMAND_USER) || !strcmp(command, COMMAND_PASSWORD)) {
			return 1;
		}

	} else if(numberParametersOfCommand(temp) == 1) {
		if(!strcmp(temp[0], COMMAND_LOGOUT)) return 1;
	}
	return 0;
}
// send message to user with message code
void sendMessage(enum StatusCode code, int sockfd) {
	switch (code) {
		//success message
		case LoginSuccess: sendToClient("Login Success", LoginSuccess, sockfd);
		break;
		case UserNameSucess: sendToClient("UserName Success", UserNameSucess, sockfd);
		break;
		case PasswordSuccess: sendToClient("Password Success! You are Logged", PasswordSuccess, sockfd);
		break;
		case LogoutSuccess: sendToClient("Logout Success", LogoutSuccess, sockfd);
		break;
		//error message
		case PhaseError: sendToClient("Phase Error", PhaseError, sockfd);
		break;
		case Logged: sendToClient("You are logged", Logged, sockfd);
		break;
		case NotLogin: sendToClient("You not loggin!", NotLogin, sockfd);
		break;
		//invalid message
		case PasswordIncorrect: sendToClient("Input Password Incorrect", PasswordIncorrect, sockfd);
		break;
		case UserNotExist: sendToClient("User Not Existed", UserNotExist, sockfd);
		break;
		case UserIsBlocked: sendToClient("User is Blocked", UserIsBlocked, sockfd);
		break;
		case CommandInvalid: sendToClient("Command inValid", CommandInvalid, sockfd);
		break;
		// server error message
		case ServerError: sendToClient("ServerError", ServerError, sockfd);
		break;
		default: break;
	}
}
// compare password 
int identifyPassWord(User* user, char* password) {
	return !strcmp(user->password, password);
}
// handle user command
int handle(char* message, int login_status, User** user, int sockfd) {
	char command[10];
	char param[30];
	char** temp = str_split(message, ' ');
	if(isCommand(temp)) {
		if(numberParametersOfCommand(temp) == 2) { // check if USER or PASS command
			strcpy(command, temp[0]);
			strcpy(param, temp[1]);
		} else { // if LOGOUT command
			strcpy(command, temp[0]);
		}
		switch(login_status) {
		case Unidentified:  //if Unidentified Status
			if(!strcmp(command, COMMAND_USER)) {
				User* ptr = search(param); // find user
				if(ptr != NULL) {
					(*user) = ptr;
					if(ptr->status == BLOCK) {
						sendMessage(UserIsBlocked, sockfd); // send block message
					} else {
						sendMessage(UserNameSucess, sockfd);
						return UnAuthenticated; // if username success return UnAuthenticated status
					}
				} else {
					sendMessage(UserNotExist, sockfd); //else send user not found
				}
			}
			else if(!strcmp(command, COMMAND_PASSWORD)) {
				sendMessage(PhaseError, sockfd); // send phase error 
			}
			else if(!strcmp(command, COMMAND_LOGOUT)) {
				sendMessage(NotLogin, sockfd); // if user not loggin and command is LOGOUT
			} 
			else {
				sendMessage(CommandInvalid, sockfd);
			}
			return Unidentified;
		case UnAuthenticated: 
			if(!strcmp(command, COMMAND_USER)) {// if command
				User* ptr = search(param);
				if(ptr != NULL) {
					user = &ptr;
					sendMessage(UserNameSucess, sockfd);
					return UnAuthenticated; // return UnAuthenticated if username exist
				} else {
				*user = NULL;
				sendMessage(UserNotExist, sockfd);
				return Unidentified;
				}
			}
			else if(!strcmp(command, COMMAND_PASSWORD)) {
				if((*user)->status == BLOCK) {
					sendMessage(UserIsBlocked, sockfd);
					return Unidentified; // if user is block
				}
				if(identifyPassWord(*user, param)) {
					(*user)->countLoginFails = 0;
					sendMessage(PasswordSuccess, sockfd);// if password success
					return Authenticated;
				}
				(*user)->countLoginFails ++; //if password invalid, increase count login fails
				if((*user)->countLoginFails == 3) {
					(*user)->status = BLOCK; // block user
					updateFile();
				}
				sendMessage(PasswordIncorrect, sockfd);
				return UnAuthenticated;
			}
			else if(!strcmp(command, COMMAND_LOGOUT)) {
				sendMessage(NotLogin, sockfd);// user not login
				return UnAuthenticated;
			} 
			sendMessage(CommandInvalid, sockfd);
			return UnAuthenticated;
		case Authenticated: 
			if(!strcmp(command, COMMAND_USER)) {
				sendMessage(Logged, sockfd);
			}
			else if(!strcmp(command, COMMAND_PASSWORD)) {
				sendMessage(Logged, sockfd);
			}
			else if(!strcmp(command, COMMAND_LOGOUT)) {
				*user = NULL;
				sendMessage(LogoutSuccess, sockfd); // success logout
				return Unidentified;
			} 
			else {
				sendMessage(CommandInvalid, sockfd);
			}
			return Authenticated;
		default: break;
		}
	} else {
		sendMessage(CommandInvalid, sockfd);
	}
	return login_status; // return login status
}

int initSocket(int port_number) {
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
	return 1;
}

int receiveData(int s, char *buff, int size, int flags){
	int n;
	n = recv(s, buff, size, flags);
	if(n < 0)
		perror("Error: ");
	return n;
}

/*
* main function
* @param int argc, char** argv
* @return void
*/
int main(int argc, char **argv)
{
 	int port_number;
 	int i, maxi, maxfd, sockfd;
 	int nready;
 	ssize_t	ret;
	fd_set	readfds;
	socklen_t clilen;
	Client client[FD_SETSIZE];

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
	//Step 1: Construct a TCP socket to listen connection request
	initSocket(port_number);
	maxfd = listen_sock;			/* initialize */
	maxi = -1;				/* index into client[] array */
	for (i = 0; i < FD_SETSIZE; i++)
	{
		client[i].socket_number = -1;
		client[i].status = Unidentified;
		client[i].user = NULL;	
	}		/* -1 indicates available entry */
	FD_ZERO(&allset);
	FD_SET(listen_sock, &allset);

	readFile();

	//Step 4: Communicate with client
	while(1){
		readfds = allset;		/* structure assignment */
		nready = select(maxfd+1, &readfds, NULL, NULL, NULL);
		if(nready < 0){
			perror("\nError: ");
			return 0;
		}

		if (FD_ISSET(listen_sock, &readfds)) {	/* new client connection */
			clilen = sizeof(cliaddr);
			if((conn_sock = accept(listen_sock, (struct sockaddr *) &cliaddr, &clilen)) < 0)
				perror("\nError: ");
			else{
				printf("You got a connection from %s\n", inet_ntoa(cliaddr.sin_addr)); /* prints client's IP */
				for (i = 0; i < FD_SETSIZE; i++)
					if (client[i].socket_number < 0) {
						client[i].socket_number = conn_sock;
						/* save descriptor */
						client[i].user = (User*) malloc(sizeof(User));
						break;
					}
				if (i == FD_SETSIZE){
					printf("\nToo many clients");
					close(conn_sock);
				}

				FD_SET(conn_sock, &allset);	/* add new descriptor to set */
				if (conn_sock > maxfd)
					maxfd = conn_sock;		/* for select */
				if (i > maxi)
					maxi = i;		/* max index in client[] array */

				if (--nready <= 0)
					continue;		/* no more readable descriptors */
			}
		}

		for (i = 0; i <= maxi; i++) {	/* check all clients for data */
			if ( (sockfd = client[i].socket_number) < 0)
				continue;
			if (FD_ISSET(sockfd, &readfds)) {
				printf("1\n");
				ret = receiveData(sockfd, recv_data, BUFF_SIZE, 0);
				if (ret <= 0){
					FD_CLR(sockfd, &allset);
					client[i].socket_number = -1;
					client[i].status = Unidentified;
					client[i].user = NULL;
					close(sockfd);
				}
				
				else {
					recv_data[ret - 1] = '\0';
					client[i].status = handle(recv_data, client[i].status, &(client[i].user), sockfd);
					if(!FD_ISSET(sockfd, &readfds)) {
						client[i].socket_number = -1;
						client[i].status = Unidentified;
						client[i].user = NULL;
					}
				}

				if (--nready <= 0)
					break;		/* no more readable descriptors */
			}
		}
	}
	return 0;
}