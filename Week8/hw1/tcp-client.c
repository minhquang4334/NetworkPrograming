
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
#include <time.h>

#include "protocol.h"


#define SERVER_ADDR "127.0.0.1"
#define SERVER_PORT 5550
#define NUMBEROFDOTSINIPV4 3 //number dots in ipv4
#define NUMBEROFDOTSINIPV6 5 //number dots in ipv6
#define NELEMS(x)  (sizeof(x) / sizeof((x)[0]))
#define MAXLINE 4096
#define EXISTEDFILE "Error: File is existent on server"
#define BUFF_SEND 1024
#define ENCRYPT "./encrypt" //default save file place
#define DECRYPT "./decrypt" //default save file place

// global value
char **tokens;
struct in_addr ipv4addr;
struct hostent *host;
struct in_addr **addr_list;
struct in_addr **alias_list;

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
    inet_pton(AF_INET, ip, &ipv4addr);
    host = gethostbyaddr(&ipv4addr, sizeof(ipv4addr), AF_INET);
    if (host != NULL)
    {
        return 1;
    }
    return 0;
}
/*
* Check valid File Name
* @param char* file Name
* @void
*/
void toValidFileName(char *fileName) {
    int i;
    for(i = 0; fileName[i] != '\0'; i++) {
        if(fileName[i] == '\n') {
            fileName[i] = '\0';
        }
    }
}
/*
* TO name of file
* @param char* file Name, char* name
* @void name
*/
void toNameOfFile(char *fileName, char* name ) {
    char** tokens = str_split(fileName, '/');
    int i;
    for (i = 0; *(tokens + i); i++)
    {
        // count number elements in array
    }
    strcpy(name, *(tokens + i -1));
}

int menu(int *choice) {
    int number;
    printf("\n-----------------ENCRYPT AND DECRYPT FUNCTION---------------\n");
    printf("1. Encrypt your file\n");
    printf("2. Decrypt your file\n");
    printf("Please! Type your choice(1-2), others to quit: ");
    scanf("%d%*c", &number);
    *choice = number;
    return 1;
}

int invalidFunctional(int choice) {
    return (choice != 1) && (choice != 2);
}

int enterKeyCode() {
    printf("Please enter your key code for Hash: ");
    int keyCode;
    scanf("%d%*c", &keyCode);
    return keyCode;
}

int hashFileName(char* fileName, char* hashNamePath, int choice) {
    if(choice == 1) {
        sprintf(hashNamePath, "%s/%ld_%s", ENCRYPT, (unsigned long)time(NULL), fileName);
    } else {
        sprintf(hashNamePath, "%s/%ld_%s", DECRYPT, (unsigned long)time(NULL), fileName);
    }
    return 0;
}

/*
* Main function
* @param int argc, char** argv
* @return 1
*/
int main(int argc, char **argv){
	char *temp = malloc(sizeof(argv[1]) * strlen(argv[1]));
	strcpy(temp, argv[1]);
	if(argc == 2) {
		printf("Missing server port number!\n"); // Check if missing port number
		exit(0);
	}
	if(argc < 2) {
		printf("Missing server port number and ip address!\n"); // check if missing port number and server ip address
		exit(0);
	}
	if(!checkIP(argv[1])) {
		printf("Invalid Ip Address!\n"); // Check valid Ip Address
		exit(0);
	}
	if(!validPortNumber(atoi(argv[2]))) {
		printf("Invalid Port Number!\n"); // Check Valid Port Number
		exit(0);
	}

	if(!hasIPAddress(temp)) {
		printf("Not found information Of IP Address [%s]\n", temp); // Find Ip Address
		exit(0);
	}
	
	int client_sock;
	struct sockaddr_in server_addr; /* server's address information */
    char sendline[MAXLINE];
    FILE *fileptr;
    int choice;
    Message recvMsg, sendMsg;
	
	//Step 1: Construct socket
	client_sock = socket(AF_INET,SOCK_STREAM,0);
	int ser_port = atoi(argv[2]);
	//Step 2: Specify server address
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(ser_port);
	server_addr.sin_addr.s_addr = inet_addr(temp);
	
	//Step 3: Request to connect server
	if(connect(client_sock, (struct sockaddr*)&server_addr, sizeof(struct sockaddr)) < 0){
		printf("\nError!Can not connect to sever! Client exit imediately! ");
		return 0;
	}
		
	//Step 4: Communicate with server			
	while(1){
        menu(&choice);
        if(invalidFunctional(choice)) break;
        if(choice == 1) {
            sendMsg.opcode = OP_ENCRYPT;
        } else {
            sendMsg.opcode = OP_DECRYPT;
        }

        int keyCode = enterKeyCode();

		//send message
        printf("Input File name: ");
        bzero(sendline,MAXLINE);
        fgets(sendline, MAXLINE, stdin);
		if(sendline[0] == '\0') {
            printf("Please input file name\n");
            continue;
        }
        char fileName[50];
        toValidFileName(sendline); //checkif null
        if ((fileptr = fopen(sendline, "rb+")) == NULL){
            printf("Error: File not found\n");
            fclose(fileptr);
            continue;
        }

        toNameOfFile(sendline, fileName); // to name of file ex: /quang/abc.txt -> abc.txt

        long filelen;
        fseek(fileptr, 0, SEEK_END);          // Jump to the end of the file
        filelen = ftell(fileptr);             // Get the current byte offset in the file       
        rewind(fileptr);    // pointer to start of file
        
        int sizeOfKeyCode = sizeof(int);
        sendMsg.length = (short) sizeOfKeyCode;

        char key[1000];
        sprintf(key, "%d", keyCode);
        memcpy(sendMsg.payload, key, strlen(key));

        if(sendMessage(client_sock, sendMsg) <= 0) {
            printf("Connection closed!\n");
            break;
        }

        int check = 1;
        int sumByte = 0;
        while(!feof(fileptr)) {
            int numberByteSend = PAYLOAD_SIZE;
            if((sumByte + PAYLOAD_SIZE) > filelen) {// if over file size
                numberByteSend = filelen - sumByte; 
            }
            sendMsg.opcode = OP_FILE;
            char* buffer = (char *) malloc((numberByteSend) * sizeof(char));
            fread(buffer, numberByteSend, 1, fileptr); // read buffer with size 
            memcpy(sendMsg.payload, buffer, numberByteSend);
            sendMsg.length = (short) numberByteSend;
            sumByte += numberByteSend; //increase byte send
            if(sendMessage(client_sock, sendMsg) <= 0) {
                printf("Connection closed!\n");
                check = 0;
                break;
            }
            free(buffer);
            if(sumByte >= filelen) {
                break;
            }
        }
        fclose(fileptr); //close file
        if(check == 0) {
            break;
        }

        sendMsg.opcode = OP_FILE;
        sendMsg.length = 0;
        strcpy(sendMsg.payload, "\0");
        if(sendMessage(client_sock, sendMsg) <= 0) {
            printf("Connection closed!\n");
            break;
        }

        char hashNamePath[100];
        hashFileName(fileName, hashNamePath, choice);
        FILE *fptr = fopen(hashNamePath, "w+");

        while(1) {
            if(receiveMessage(client_sock, &recvMsg) <= 0) {
                printf("Connection closed!\n");
                check = 0;
                break;
            }

            if(recvMsg.opcode == OP_FILE){
                if(recvMsg.length > 0){
                    fwrite(recvMsg.payload, recvMsg.length, 1, fptr);
                } else {
                    break;
                }
            }
        }
        fclose(fptr);
        if(check == 0) {
            break;
        }

        if(choice == 1) {
            printf("Encrypted Success!!! File Saved in: %s\n", hashNamePath);
        } else {
            printf("Decrypted Success!!! File Saved in: %s\n", hashNamePath);
        }
	}
	
	//Step 4: Close socket
	close(client_sock);
	return 0;
}
