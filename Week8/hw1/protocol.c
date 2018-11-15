#include "protocol.h"

int isFunctionalOpcode(int opcode) {
  return (opcode == OP_ENCRYPT) || (opcode == OP_DECRYPT);
}

int copyMess(Message* mess, Message temp) {
  mess->opcode = temp.opcode;
  mess->length = temp.length;
  memcpy(mess->payload, temp.payload, temp.length);
  return 1;
}

int printMess(Message mess) {
  printf("opcode: %c\n", mess.opcode);
  printf("length: %d\n", mess.length);
  printf("payload: %s\n", mess.payload);
  return 1;
}

int sendMessage(int socket, Message msg){
  int dataLength, nLeft, idx;
  nLeft = sizeof(Message);
  idx = 0;
  while (nLeft > 0){
    dataLength = send(socket, &((char*)&msg)[idx], nLeft, 0);
    if (dataLength < 0) return dataLength;
    nLeft -= dataLength;
    idx += dataLength;
  }

  return sizeof(Message);
}

int receiveMessage(int socket, Message *msg){

  char recvBuff[BUFF_SIZE];
  int ret, nLeft, idx, bytes_recv;
  Message recvMessage;

  // init value for variables
  ret = 0;
  idx = 0;
  nLeft = sizeof(Message);

  while (nLeft > 0) {
    // if sizeof message big than buff_size// bytes receive = buffsize else nleft
    bytes_recv = nLeft > BUFF_SIZE ? BUFF_SIZE : nLeft;
    ret = recv(socket, recvBuff, bytes_recv, 0);
    // if recv <= 0 return ret <= 0
    if (ret <= 0) return ret;

    //clone ret to message with idx position
    memcpy(&(((char*)&recvMessage)[idx]), recvBuff, ret); 
    
    idx += ret;
    // set number of bytes of message left 
    nLeft -= ret;
    
  }

  // copy message to msg
  copyMess(&(*msg), recvMessage);
  
  return sizeof(Message);
}