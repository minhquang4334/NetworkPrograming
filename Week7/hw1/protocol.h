
#ifndef __PROTOCOL_H__
#define __PROTOCOL_H__

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

#define BUFF_SIZE 40000
#define PAYLOAD_SIZE 32767

#define OP_ENCRYPT '0'
#define OP_DECRYPT '1'
#define OP_FILE '2'
#define OP_ERROR '3'
#define LENGTH_SIZE 2

typedef enum {
   WAITING_KEYCODE,
   WAITING_FILECONTENT,
} ProtocolStatus;

typedef struct {
  char opcode;
  short length;
  char payload[PAYLOAD_SIZE];
} Message;

/*
* check if opcode is encrypt or decrypt
* @param int opcode
* @return 1 if opcode equal OP_ENCRYPT || opcode equal OP_ENCRYPT
*/
int isFunctionalOpcode(int opcode);

/*
* clone Message mess from temp
* @param Message* mess, Message temp
* @return 1 if clone success
*/
int copyMess(Message* mess, Message temp);

/*
* print Message content
* @param Message mess
* @return 1 if print success
*/
int printMess(Message mess);

/*
* send message
* @param int socket, Messgae msg
* @return size of message if valid
*/
int sendMessage(int socket, Message msg);

/*
* recv message
* @param int socket, Messgae msg
* @return size of message if valid 
*/
int receiveMessage(int socket, Message *msg);

#endif